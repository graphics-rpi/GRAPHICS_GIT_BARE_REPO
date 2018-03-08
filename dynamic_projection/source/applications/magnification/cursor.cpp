#include "cursor.h"

#include <random>
#include <ctime>

#include "canvas.h"
#include "argparser.h"

// Static variable since all cursors have same base model

bool Cursor::initialized_ = false;
GLuint Cursor::cursor_pos_vbo_ = 0;
GLuint Cursor::cursor_color_vbo_ = 0;
GLuint Cursor::cursor_vao_ = 0;
ArgParser* Cursor::args_;

glm::mat4 Cursor::model_ = glm::mat4(1.0);

void Cursor::Print( std::ostream & stream ) const {
    stream << "ID: "  << id_ << " POS: " << glm::to_string(pos_);
    return;
}

Cursor::Cursor(ArgParser* args, int id, int x, int y) : id_(id) , pos_(x, y){
    // Hack to do static vbo and vao setup.
    if( !initialized_ ){
        args_ = args;
        GLfloat dummy_pos[(args_->num_cursors_ - 1) * 2];
        GLfloat dummy_color[(args_->num_cursors_ - 1)  * 3];

        for( int i = 0; i < args_->num_cursors_ - 1; i++ ){
            dummy_pos[i*2] = 0.0f;
            dummy_pos[i*2+1] = 0.0f;
            if( i % 4 == 0 ){
                dummy_color[i*3] = 1.0f;
                dummy_color[i*3+1] = 0.0f;
                dummy_color[i*3+2] = 0.0f;
            }
            else if( i % 4 == 1 ){
                dummy_color[i*3] = 0.0f;
                dummy_color[i*3+1] = 1.0f;
                dummy_color[i*3+2] = 0.0f;
            }
            else if( i % 4 == 2 ){
                dummy_color[i*3] = 0.0f;
                dummy_color[i*3+1] = 0.0f;
                dummy_color[i*3+2] = 1.0f;
            }
            else{
                dummy_color[i*3] = 1.0f;
                dummy_color[i*3+1] = 1.0f;
                dummy_color[i*3+2] = 1.0f;
            }
        }

        glGenVertexArrays(1, &cursor_vao_);        
        glBindVertexArray(cursor_vao_);
        
        glGenBuffers(1, &cursor_pos_vbo_);        
        glBindBuffer(GL_ARRAY_BUFFER, cursor_pos_vbo_);
        glBufferData( GL_ARRAY_BUFFER, sizeof(dummy_pos), dummy_pos, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &cursor_color_vbo_);
        glBindBuffer(GL_ARRAY_BUFFER, cursor_color_vbo_);
        glBufferData( GL_ARRAY_BUFFER, sizeof(dummy_color), dummy_color, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, cursor_pos_vbo_);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, cursor_color_vbo_);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0); 

        glBindVertexArray(0);
        
        initialized_ = true;
    }
    rng_.seed( std::time(0));
    rng_.discard( id * 3 );

    double r = std::generate_canonical<double, 10>(rng_);
    double g = std::generate_canonical<double, 10>(rng_);
    double b = std::generate_canonical<double, 10>(rng_);
    color_ = glm::vec3( (float)r, (float)g, (float)b );

    glm::vec2 model_space = ConvertToModelCoordinates( pos_ ); 
    Canvas::HandleGLError("AFTER CONVERT TO MODEL");

    glBindBuffer(GL_ARRAY_BUFFER, cursor_pos_vbo_);
    Canvas::HandleGLError("AFTER BIND BUFFER");
    if( id != 0 ){
        glBufferSubData( GL_ARRAY_BUFFER, (id-1) * sizeof(float) * 2, sizeof(float) * 2, glm::value_ptr(model_space));
        Canvas::HandleGLError("AFTER BUFFER SUB DATA");
    }

    //glBindBuffer(GL_ARRAY_BUFFER, cursor_color_vbo_);
    //glBufferSubData( GL_ARRAY_BUFFER, id * sizeof(float) * 3, sizeof(float) * 3, glm::value_ptr(color_));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Cursor::Draw(){
    glPointSize(10);
    Canvas::HandleGLError("AFTER POINT SIZE");
    glDepthMask( GL_FALSE );
    glDisable( GL_DEPTH_TEST);

    glBindVertexArray(cursor_vao_);
    Canvas::HandleGLError("AFTER BINDING VAO");
    glDrawArrays(GL_POINTS, 0, args_->num_cursors_ - 1 );

    Canvas::HandleGLError("AFTER DRAWING ARRAY");
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDepthMask( GL_TRUE );

    Canvas::HandleGLError("AFTER UNBINDING VAO");
    return;
}

glm::vec2 Cursor::ConvertToModelCoordinates(const glm::vec2 & input ){
    float x = (float)input[0] / ((float)(args_->width_) / 2.0f) - 1.0f;
    float y = (float)input[1] / ((float)(args_->height_) / 2.0f) - 1.0f;
    y = y * -1.0f;

    return glm::vec2(x, y);
}

AICursor::AICursor(ArgParser* args, int id, int x, int y) : Cursor( args, id, x, y ){
    // initialize distributions
    std::mt19937 rng;
    rng.seed( std::time(0));
    rng.discard( id * 2 );
    
    x_dis_ = std::uniform_int_distribution<int>(0, args_->width_);
    y_dis_ = std::uniform_int_distribution<int>(0, args_->height_);

    GenerateNewDest();
}

void AICursor::Print(std::ostream & stream) const {
    stream << "ID: "  << id_ << " POS: " << glm::to_string(pos_) << "\n"
        << "VEL: " << glm::to_string(vel_) << "\n"
        << "DEST: " << glm::to_string(dest_);
    return;
}
void AICursor::Update(){ 
    glm::vec2 new_pos = pos_ + vel_;
    glm::vec2 model_coordinates = ConvertToModelCoordinates( new_pos );
    glBindBuffer( GL_ARRAY_BUFFER, cursor_pos_vbo_ );
    glBufferSubData( GL_ARRAY_BUFFER, (id_ - 1) * sizeof(float) * 2, sizeof(float) * 2, glm::value_ptr(model_coordinates));
    pos_ = new_pos;

    //std::cout << "UL ORIGIN " << id_ << " : " << glm::to_string(pos_) << std::endl;

    // compare distance for dest
    if( glm::distance(pos_ , dest_) < glm::length(vel_)){
        GenerateNewDest();
    }

    return;
}

void AICursor::GenerateNewDest(){
    int x_dest = x_dis_(rng_);
    int y_dest = y_dis_(rng_);

    dest_ = glm::vec2(x_dest, y_dest);
    vel_ = dest_ - pos_;
    vel_ = glm::normalize(vel_);
    return;
}

void ControlledCursor::Update(){
    pos_ = next_pos_;
}

void ControlledCursor::Print( std::ostream & stream ) const{
    stream << "ID: "  << id_ << " POS: " << glm::to_string(pos_);
}
