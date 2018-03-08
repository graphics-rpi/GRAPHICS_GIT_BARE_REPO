#include "canvas.h"
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <cstdlib>
#include <cstring>

#include "argparser.h"
#include "mesh.h"
#include "camera.h"
#include "cursor.h"
#include "shader_manager.h"

#include "../../common/glm/glm/glm.hpp"
#include "../../common/glm/glm/gtx/string_cast.hpp"
#include "../../common/glm/glm/gtc/matrix_transform.hpp"
#include "../../common/glm/glm/gtc/type_ptr.hpp"

// Static variables
int Canvas::mouse_button_;
int Canvas::mouse_x_ = 400;
int Canvas::mouse_y_ = 300;
int Canvas::magnification_factor_ = 2;

glm::vec2 Canvas::object_mouse_pos_ = glm::vec2(0.0f, 0.0f);

std::vector<Cursor*> Canvas::cursors_ ;
std::vector<glm::vec2> Canvas::cursor_positions_;

// Linking GLSL shaders
GLuint Canvas::program_id_;

// Uniform variables
GLuint Canvas::u_matrix_id_;
GLuint Canvas::u_texture_id_;
GLuint Canvas::u_mouse_pos_id_;
GLuint Canvas::u_window_size_id_;
GLuint Canvas::u_magnification_id_;
GLuint Canvas::u_magnification_factor_id_;

Camera* Canvas::camera_;
Mesh* Canvas::mesh_;
ArgParser* Canvas::args_;
ShaderManager* Canvas::manager_;
Shader* Canvas::current_shader_;

void Canvas::Initialize(ArgParser* args, Mesh* mesh){
    args_ = args;
    mesh_ = mesh;
    camera_ = new Camera(args_);
    manager_ = new ShaderManager(args_);

    /*
     *GLenum glew_error = glewInit();
     *if( glew_error != GLEW_OK ){
     *    return;
     *}
     */

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(args_->width_ ,args_->height_);
    //glutInitWindowSize(args->tiled_display.my_width,args->tiled_display.my_height);

    glutCreateWindow("Magnification");
    glutDisplayFunc(Display);
    glutIdleFunc(Idle);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeyboard);
    glutMouseFunc(Mouse);
    glutMotionFunc(Drag);
    glutPassiveMotionFunc(Motion);


    InitShaders();
    Canvas::HandleGLError("AFTER INIT SHADERS");
    
    InitTextures();
    Canvas::HandleGLError("AFTER INIT TEXTURES");

    mesh_->Initialize();
    Canvas::HandleGLError("AFTER INIT MESH");

    InitCursors();
    Canvas::HandleGLError("AFTER INIT CURSORS");
    
    glEnable(GL_DEPTH_TEST);
    Canvas::HandleGLError("AFTER GL_ENABLE");
    glDepthFunc(GL_LEQUAL);
    Canvas::HandleGLError("AFTER GLDEPTH FUNC");

    glutMainLoop();
    return;
}

void Canvas::InitCursors(){
    for( int i = 0; i < args_->num_cursors_; i++ ){
        if( i == 0 ){
            Cursor* primary_mouse = new ControlledCursor( args_, 0, mouse_x_, mouse_y_ );
            cursors_.push_back(primary_mouse);
        }
        else{
            Cursor* new_cursor = new AICursor(args_, i, i*20 + 5, i*20 + 5);
            cursors_.push_back( new_cursor );
        }
        cursor_positions_.push_back( glm::vec2(0.0f,0.0f) );
    }
    return;
}

void Canvas::BindUniforms(){ 
    if( current_shader_->name().compare("texture") == 0 ){
        //std::vector< glm::vec3 > results = CalculateMagAreas();
        std::vector<glm::mat4> matricies;
        std::vector<glm::vec2> vectors;
        std::vector<int> integers;
        std::vector<float> floats;

        // MVP 
        glm::mat4 mvp = camera_->ComputeMVPMatrix( mesh_->model() );
        matricies.push_back(mvp);
        current_shader_->BindUniform( "MVP", matricies );
        matricies.clear();
        
        // Texture
        integers.push_back(0);
        current_shader_->BindUniform( "texture_sampler", integers );
        integers.clear();

        // Mouse position
        vectors.push_back(object_mouse_pos_);
        current_shader_->BindUniform( "mouse_pos", vectors);
        vectors.clear();

        // Screen size
        glm::vec2 window_size = glm::vec2( args_->width_, args_->height_ );
        vectors.push_back( window_size ); 
        current_shader_->BindUniform( "window_size", vectors);
        vectors.clear();

        // Magnification status
        integers.push_back(args_->magnification_enabled_);
        current_shader_->BindUniform( "magnification", integers );
        integers.clear();

        floats.push_back(magnification_factor_);
        current_shader_->BindUniform( "magnification_factor", floats);
        floats.clear();

        vectors = cursor_positions_;
        integers.push_back( vectors.size() );

        current_shader_->BindUniform( "num_cursors", integers);
        current_shader_->BindUniform( "cursors", vectors, vectors.size());
        integers.clear();
        vectors.clear();
        

    }
    return;
}

std::vector<glm::vec3> Canvas::CalculateMagAreas(){
    std::vector<bool> should_draw( cursors_.size(), true );
    std::vector<glm::vec2> object_cursors;
    for( int i = 0; i < cursors_.size(); i++ ){
        object_cursors.push_back(ConvertToObjectSpace( cursors_[i]->pos() ));
    }
    std::vector<glm::vec3> results;
    glm::vec3 current_vec;
    for( int i = 0; i < object_cursors.size(); i++ ){
        if( should_draw[i] ){
            std::vector<glm::vec2> nearby_cursors;
            for( int j = 0; j < object_cursors.size(); j++ ){
                if( i != j && glm::distance( object_cursors[i], object_cursors[j] ) < 2 * 0.2f){
                    should_draw[i] = false;
                    nearby_cursors.push_back( object_cursors[j] );
                }
            }
        }
    }
    return results;
}
glm::vec2 Canvas::ConvertToObjectSpace( const glm::vec2 & input ){
    glm::vec3 temp = camera_->ComputeScreenToWorld( input[0], input[1]);
    glm::vec4 world_coordinates = glm::vec4( temp[0], temp[1], temp[2], 0.0f);

    if( fabs(input[1] - args_->height_ / 2) < 10 ){
        std::cout << "SCREEN: " << glm::to_string(input) << std::endl;
        std::cout << "WORLD : " << glm::to_string(world_coordinates) << std::endl; 
    }

    glm::mat4 plane_model_matrix = mesh_->model();
    glm::vec4 object_space_coordinates = plane_model_matrix * world_coordinates;

    // Normalize to 0 - 1
    mesh_->Normalize(object_space_coordinates);

    return glm::vec2( object_space_coordinates[0], object_space_coordinates[1] );
}

void Canvas::InitUniforms(){
    Shader* shader = manager_->GetProgram( "texture" );
    shader->InitUniform( "texture_sampler");
    shader->InitUniform( "mouse_pos");
    shader->InitUniform( "MVP");
    shader->InitUniform( "window_size");
    shader->InitUniform( "magnification");
    shader->InitUniform( "magnification_factor"); 
    shader->InitUniform( "cursors" );
    shader->InitUniform( "num_cursors");
    return;
}
void Canvas::InitTextures(){
    mesh_->LoadTexture(args_->image_filename_);
    return;
}

// helper functions
void Canvas::InitShaders(){
    manager_->CreateProgram( "texture" ); 
    manager_->CreateProgram( "cursor" );
    //program_id_ = LoadShaders(args_->vertex_filename_.c_str(), args_->fragment_filename_.c_str());
    InitUniforms();
    return;
}

void Canvas::DrawCursors(){
    Cursor::Draw();
    return;
}

// Display callback function 
void Canvas::Display(void){
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Canvas::HandleGLError("AFTER CLEAR");

    // use glsl shaders
    current_shader_ = manager_->GetProgram("texture");
    current_shader_->BindProgram();
    Canvas::HandleGLError("AFTER BIND PROGRAM(TEXTURE)");

    // pass in uniform varaibles
    BindUniforms();
    Canvas::HandleGLError("AFTER BIND UNIFORMS PROGRAM(TEXTURE)");

    // draw mesh
    mesh_->Draw();
    Canvas::HandleGLError("AFTER DRAW MESH");

    // Use cursor program
    current_shader_ = manager_->GetProgram("cursor");
    current_shader_->BindProgram();
    Canvas::HandleGLError("AFTER BIND PROGRAM(CURSOR)");
    
    // draw cursors
    DrawCursors();

    Canvas::HandleGLError("AFTER DRAW CURSOR");

    // swap buffers
    glutSwapBuffers();
}


// Window resize callback function 
void Canvas::Reshape(int w, int h){
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    args_->width_ = w;
    args_->height_ = h;
}

// Mouse click / release callback function
void Canvas::Mouse(int button, int state, int x, int y){
    mouse_x_ = x;
    mouse_y_ = y;

    mouse_button_ = button;
    if( button == GLUT_LEFT_BUTTON){
        // Calculate object space coordinates.
        cursors_[0]->set_next_pos( glm::vec2(x, y) );
        //glm::vec2 object_mouse_pos_ = ConvertToObjectSpace( glm::vec2(mouse_x_, mouse_y_ ));
    }
}

// Mouse drag callback function
void Canvas::Drag(int x, int y){
}

// Passive Mouse movement
void Canvas::Motion(int x, int y){
    if( args_->constant_motion_ ){
        cursors_[0]->set_next_pos( glm::vec2(x, y) );
        mouse_x_ = x;
        mouse_y_ = y;
        //glm::vec2 object_mouse_pos_ = ConvertToObjectSpace( glm::vec2(mouse_x_, mouse_y_ ));
    }
    return;
}

void Canvas::SpecialKeyboard(int key, int x, int y){
    glm::vec3 right = glm::vec3( 1.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);

    switch(key){
        case GLUT_KEY_LEFT:
            printf("Moving camera!\n");
            camera_->Move(-right);
            break;
        case GLUT_KEY_RIGHT:
            printf("Moving camera!\n");
            camera_->Move(right);
            break;
        case GLUT_KEY_UP:
            printf("Moving camera!\n");
            camera_->Move(up);
            break;
        case GLUT_KEY_DOWN:
            printf("Moving camera!\n");
            camera_->Move(-up);
            break;
        case GLUT_KEY_PAGE_UP:
            printf("Moving camera!\n");
            camera_->Move(forward);
            break;
        case GLUT_KEY_PAGE_DOWN:
            printf("Moving camera!\n");
            camera_->Move(-forward);
            break;
        default:
            fprintf(stderr, "Unknown special keyboard input recieved: '%d'\n", key);
    } 
}

void Canvas::SpecialKeyboardRelease(int key, int x, int y ){
    return;
}

// Keyboard event callback function
void Canvas::Keyboard(unsigned char key, int x, int y){
    switch (key){
        case 'q': case 'Q':
            exit(0);
            break;
        case 'm': case 'M':
            printf("Toggling magnification!\n");
            args_->magnification_enabled_ = !args_->magnification_enabled_;
            break;
        case 'p': case 'P':
            printf("Toggling passive motion!\n");
            args_->constant_motion_ = !args_->constant_motion_;
            break;
        case 's': case 'S':
            printf("Toggling automated movement!\n");
            args_->movement_enabled_ = !args_->movement_enabled_;
            break;
        case 'x': case 'X':
            printf("Re-linking program!\n");
            InitShaders();
            break;
        case '+': case '=':
            magnification_factor_ = magnification_factor_ + 1;
            printf("Zooming in on texture. Mag factor: %d\n", magnification_factor_);
            break;
        case '-': case '_':
            magnification_factor_ = magnification_factor_ - 1;
            if( magnification_factor_ < 1 ){
                magnification_factor_ = 1;
            }
            printf("Zooming out on texture. Mag factor: %d\n", magnification_factor_);
            break;
        default:
            fprintf(stderr, "Unknown keyboard input recieved: '%c'\n", key);
    }
}

// idle is called repeatedly, allows for animation.
void Canvas::Idle(){
    for( int i = 0; i < args_->num_cursors_; i++ ){
        if( i == 0 || args_->movement_enabled_){
            cursors_[i]->Update();
            cursor_positions_[i] = ConvertToObjectSpace(cursors_[i]->pos());
            if( i != 0 ){
                //std::cout << glm::to_string(cursors_[i]->pos()) << std::endl;
                //std::cout << glm::to_string(cursor_positions_[i]) << std::endl;
            }
        }
    }
    glutPostRedisplay();
    usleep(1000);
}

int Canvas::HandleGLError(std::string foo) {
    GLenum error;
    int i = 0;
    while ((error = glGetError()) != GL_NO_ERROR) {
        printf ("GL ERROR(#%d == 0x%x):  %s\n", i, error, gluErrorString(error));
        std::cout << foo << std::endl;
        if (error != GL_INVALID_OPERATION) i++;
    }
    if (i == 0) return 1;
    return 0;
}
