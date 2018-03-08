#include "TextManager.h"

#include "shader_manager.h"

#include "../../multi_cursor/interaction.h"

#include <string>
#include <vector>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <math.h>
#include <algorithm>
#include <cstdio>
#include <iostream>

#include "base_argparser.h"

Font::Font(FT_Library ft, std::string font_name, int size, TextStyles::Enum style){
    std::string filename;
#if __APPLE__
      std::string path = "/usr/local/texlive/2012/texmf-dist/fonts/truetype/public/gnu-freefont/";
#else
      std::string path = "/usr/share/fonts/truetype/freefont/";
#endif
    if( style == TextStyles::REGULAR ){
        filename = path + font_name + ".ttf";
    }
    /*
    else if(){

    }
    else{

    }
    */
    int error = FT_New_Face(ft, filename.c_str(), 0, &m_face);
    if( error == FT_Err_Unknown_File_Format ){
        fprintf(stderr, "Could not load %s font! Unsupported filetype! ( path: %s)\n", font_name.c_str(), filename.c_str());
        exit(1);
    }
    else if( error ){
        fprintf(stderr, "Could not load %s font! ( path: %s)\n", font_name.c_str(), filename.c_str());
        exit(1);
    }

    FT_Set_Pixel_Sizes(m_face, 0, size);
}

int Font::closestDistance( int x, int y, int color, int state[], int height, int width ){
    int closest = 100000;

    for( int i = 0; i < height; i++ ){
        for( int j = 0; j < width; j++ ){
            if(color != state[i * width + j]){
                double dx = x - i;
                double dy = y - j;
                
                int distance = (int)(sqrt( dx * dx + dy * dy ));
                if( distance < closest ){
                    closest = distance;
                }
            }
        }
    }
    if( color == 0 ){
       closest = closest * -1; 
    }
    return closest;
}

void Font::createSignedDistanceField(int height, int width, unsigned char* buffer, unsigned char* result){
    /*
    printf( "HEIGHT: %d, WIDTH: %d\n", height, width );
    printf( "MAX SIZE: %d\n", height * width);
    */
    int state[height * width];
    int distance[height][width];

    
    for( int i = 0; i < height; i++ ){
        for( int j = 0; j < width; j++ ){
            int cur_index = i * width + j;

            if( (int)(buffer[i * width + j]) > 128 ){
                state[i * width + j] = 1 ;
            }
            else{
                state[i * width + j] = 0;
            }
        }
    }
    /*
    for( int i = 0; i < height; i++ ){
        for( int j = 0; j < width; j++ ){
            printf("%d ", state[i * width + j]);
        }
        printf("\n");
    }
    printf("\n");
    */

    for( int i = 0; i < height; i++ ){
        for( int j = 0; j < width; j++ ){
            distance[i][j] = closestDistance( i, j, state[i * width + j], state, height, width );
        }
    }
    /*
    for( int i = 0; i < height; i++ ){
        for( int j = 0; j < width; j++ ){
            printf("%d ", distance[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    */

    int lo_in =  1000;
    int hi_in = -1000;

    int lo_out = -1000;
    int hi_out =  1000;

    for( int i = 0; i < height; i++ ){
        for( int j = 0; j < width; j++ ){
            if( state[i * width + j] == 1 ){
                if( distance[i][j] < lo_in ){
                    lo_in = distance[i][j];
                }
                if( distance[i][j] > hi_in ){
                    hi_in = distance[i][j];
                }
            }
            else{
                if( distance[i][j] > lo_out ){
                    lo_out = distance[i][j];
                }
                if( distance[i][j] < hi_out ){
                    hi_out = distance[i][j];
                }
            }
        }
    }

    int in_range = hi_in - lo_in;
    int out_range = (hi_out - lo_out) * -1;

    double norm_distance[height][width];
    for( int i = 0; i < height; i++ ){
        for( int j = 0; j < width; j++ ){
            if( state[i * width + j] == 1 ){
                norm_distance[i][j] = (double)(distance[i][j] - lo_in) / (double)(in_range * 2.0);
                norm_distance[i][j] += 0.5;
            }
            else{
                norm_distance[i][j] = (double)(distance[i][j] - lo_out) / (double)(out_range * 2.0);
                norm_distance[i][j] += 0.5;
            }
        }
    }

    for( int i = 0; i < height; i++ ){
        for( int j = 0; j < width; j++ ){
            result[i * width + j] = norm_distance[i][j] * 256;
            if(result[i * width + j] > 256){
                result[i * width + j] = 256;
            }
        }
    }
    /*
    for( int i = 0; i < height; i++ ){
        for( int j = 0; j < width; j++ ){
            printf("%d ", result[i * width + j]);
        }
        printf("\n");
    }
    printf("\n");
    */
}

void Font::createFontTexture(){
    setFontTextureDimensions(); 

    // Open GL initialization
    glGenTextures(1, &m_tex);
    glBindTexture(GL_TEXTURE_2D, m_tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
     
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    // Fill in font texture
    FT_GlyphSlot g = m_face->glyph;
    int x = 0;

    for( int i = 32; i < 128; i++ ){
        if(FT_Load_Char(m_face, i, FT_LOAD_RENDER)){
            continue;
        }

        /*
        printf( "data for char %c\n", (char)i );
        unsigned char result[g->bitmap.rows * g->bitmap.width];
        createSignedDistanceField(g->bitmap.rows, g->bitmap.width, g->bitmap.buffer, &result[0]);
        printf( "DONE\n" );
        */

        glTexSubImage2D(GL_TEXTURE_2D, 0, 
                x, 0, g->bitmap.width, g->bitmap.rows, 
                GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer/*&result[0]*/);

        // Fill in CharInfo array
        // Bitshift right 6 because these are measured as 1/64th pixel units...
        m_characters[i].ax = g->advance.x >> 6;
        m_characters[i].ay = g->advance.y >> 6;

        m_characters[i].bw = g->bitmap.width;
        m_characters[i].bh = g->bitmap.rows;

        m_characters[i].bl = g->bitmap_left;
        m_characters[i].bt = g->bitmap_top;

        m_characters[i].tx = (float)x / (float)m_width;

        x += g->bitmap.width;
    }
}

void Font::bindFont(){
    glBindTexture(GL_TEXTURE_2D, m_tex);
    return;
}

void Font::setFontTextureDimensions(){
    FT_GlyphSlot g = m_face->glyph;
    int w = 0;
    int h = 0;

    for( int i = 32; i < 128; i++ ){
        if(FT_Load_Char(m_face, i, FT_LOAD_RENDER)){
          fprintf(stderr, "Loading character %c failed!\n", i);
            continue;
        }

        w += g->bitmap.width;
        h = std::max(h, g->bitmap.rows);
    }

    m_width = w;
    m_height = h;

    printf( "FONT TOTAL_WIDTH: %d\n", m_width );
    printf( "FONT TOTAL_HEIGHT: %d\n", m_height );
}


// ============================================================================
// Text Manager

TextManager::TextManager(ArgParser* args){
    m_args = args;

    if(FT_Init_FreeType(&m_ft)){
       fprintf(stderr, "Could not initialize freetype library!\n"); 
       exit(1);
    }

    Font* f = new Font(m_ft, "FreeSans", 96/4, TextStyles::REGULAR);
    m_fonts.push_back(f); 

    for( int i = 0; i < m_fonts.size(); i++ ){
        m_fonts[i]->createFontTexture();
    }

    // GL Init
    glGenVertexArrays(1, &m_text_vao);
    HandleGLError("AFTER TEXT MANAGER GEN VAO");
    glGenBuffers(3, &m_text_vbo[0]);
    HandleGLError("AFTER TEXT MANAGER GEN VBO");

    glBindVertexArray(m_text_vao);
    HandleGLError("AFTER TEXT MANAGER BIND VAO");
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_text_vbo[TextVBO::POSITION]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    HandleGLError("AFTER TEXT MANAGER 0 VAP");

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, m_text_vbo[TextVBO::UV]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    HandleGLError("AFTER TEXT MANAGER 1 VAP");

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_text_vbo[TextVBO::COLOR]);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    HandleGLError("AFTER TEXT MANAGER 2 VAP");

    glBindVertexArray(0);
    HandleGLError("AFTER TEXT MANAGER END");
}

TextManager::TextManager(std::string font){
    /*
    if(FT_Init_FreeType(&m_ft)){
       fprintf(stderr, "Could not initialize freetype library!\n"); 
       exit(1);
    }

    FT_Face regular_face;
    if(FT_NEW_FACE(ft, "FreeSans.ttf", 0, &regular_face)){
        fprintf(stderr, "Could not load FreeSans font!\n");
        exit(1);
    }

    m_faces.push_back( regular_face );

    FT_Set_Pixel_Sizes(m_faces[TextStyles::REGULAR], 0, 32);
    */
}

void TextManager::BindUniforms(glm::mat4 model_view, glm::mat4 projection){
    // Bind Uniform Data
    std::vector<glm::mat4> matricies;
    std::vector<int> integers;

    glm::mat4 mv = glm::mat4(1.0f);
    matricies.push_back( mv );
    ShaderManager::current_shader->BindUniform( "model_view_matrix", matricies );
    matricies.clear();

    matricies.push_back( projection );
    ShaderManager::current_shader->BindUniform( "projection_matrix", matricies );
    matricies.clear();
    
    integers.push_back(0);
    ShaderManager::current_shader->BindUniform( "texture_sampler", integers);
    integers.clear();
}

void TextManager::partitionString( std::vector<std::string> & split_strings, std::string original_string
        , std::vector<double> distances ){

    double sx = 1.0f;
    sx *= (/*0.25 * */0.5);

    Font* f = m_fonts[TextStyles::REGULAR];

    int index = 0;
    double accum = 0.0;
    for( int i = 0; i < distances.size(); i++ ){
        double current_dist = distances[i]; 
        std::string split_string = "";

        bool done = false;
        double x = 0.0;
        while( !done ){
            CharInfo c_info = f->getCharInfo((int)original_string[index % original_string.size()]);

            double new_x = x + c_info.bl * sx;
            double width = c_info.bw * sx;

            x += c_info.ax * sx;

            width += c_info.bl * sx + c_info.bw * sx;
            if( new_x + width > current_dist ){
                done = true;
                accum += current_dist;
                if( accum > (new_x + width) ){
                    split_string += original_string[index % original_string.size()] ;
                    index++;
                    accum = 0.0;
                }
            }
            else{
                split_string += original_string[index % original_string.size()];
                index++;
            }
        }
        split_strings[i] = split_string;
    }
    /*for( int i = 0; i < split_strings.size(); i++ ){
        //printf("%d: DIST: %f, STRING: %s\n", i, distances[i], split_strings[i].c_str());
    }*/
}

void TextManager::Draw(glm::mat4 model_view, glm::mat4 projection){
    glBindVertexArray(m_text_vao);

    m_fonts[TextStyles::REGULAR]->bindFont();

    ShaderManager::BindProgram("text");
    BindUniforms(model_view, projection);

    glBindBuffer( GL_ARRAY_BUFFER, m_text_vbo[TextVBO::POSITION]);
    glBufferData( GL_ARRAY_BUFFER, m_text_position.size() * sizeof(glm::vec3), &m_text_position[0], 
            GL_DYNAMIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, m_text_vbo[TextVBO::UV]);
    glBufferData( GL_ARRAY_BUFFER, m_text_uv.size() * sizeof(glm::vec2), &m_text_uv[0], 
            GL_DYNAMIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, m_text_vbo[TextVBO::COLOR]);
    glBufferData( GL_ARRAY_BUFFER, m_text_color.size() * sizeof(glm::vec4), &m_text_color[0], 
            GL_DYNAMIC_DRAW );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawArrays( GL_TRIANGLES, 0, m_text_position.size() );

    glDisable(GL_BLEND);

    m_text_position.clear();
    m_text_uv.clear();
    m_text_color.clear();

    return;
}

void TextManager::addString(const char* text, float x, float y, float z, float sx, float sy, 
        glm::vec3 text_color, glm::mat4 transform){

    // Hack
    sx *= (/*0.25 * */0.5);
    sy *= (/*0.25 * */0.5);

    float original_x = x;
    float original_y = y;
    // TODO: CHANGE BASED ON INPUT
    Font* f = m_fonts[TextStyles::REGULAR];
    float tex_width = f->getWidth();
    float tex_height = f->getHeight();

    for( const char* pos = text; *pos != '\0'; pos++ ){
        CharInfo c_info = f->getCharInfo((int)(*pos));

        float new_x = x + c_info.bl * sx;
        float new_y = -y - c_info.bt * sy;
        float width = c_info.bw * sx;
        float height= c_info.bh * sy;

        x += c_info.ax * sx;
        y += c_info.ay * sy;

        if( width == 0 || height == 0 ){
            continue;
        }

        glm::vec4 a = transform * glm::vec4( new_x        , -new_y            , z, 1.0f );
        glm::vec4 b = transform * glm::vec4( new_x + width, -new_y            , z, 1.0f );
        glm::vec4 c = transform * glm::vec4( new_x        , -new_y - height   , z, 1.0f );
        glm::vec4 d = transform * glm::vec4( new_x + width, -new_y - height   , z, 1.0f );

        m_text_position.push_back(glm::vec3( a ));
        m_text_position.push_back(glm::vec3( b ));
        m_text_position.push_back(glm::vec3( c ));

        m_text_position.push_back(glm::vec3( b )); 
        m_text_position.push_back(glm::vec3( c )); 
        m_text_position.push_back(glm::vec3( d ));

        m_text_uv.push_back(glm::vec2(c_info.tx,                            0.0f));
        m_text_uv.push_back(glm::vec2(c_info.tx + c_info.bw / tex_width,    0.0f));
        m_text_uv.push_back(glm::vec2(c_info.tx,                            c_info.bh / tex_height));

        m_text_uv.push_back(glm::vec2(c_info.tx + c_info.bw / tex_width,    0.0f));
        m_text_uv.push_back(glm::vec2(c_info.tx,                            c_info.bh / tex_height));
        m_text_uv.push_back(glm::vec2(c_info.tx + c_info.bw / tex_width,    c_info.bh / tex_height));

        m_text_color.push_back(glm::vec4(text_color, 1.0f));
        m_text_color.push_back(glm::vec4(text_color, 1.0f));
        m_text_color.push_back(glm::vec4(text_color, 1.0f));

        m_text_color.push_back(glm::vec4(text_color, 1.0f));
        m_text_color.push_back(glm::vec4(text_color, 1.0f));
        m_text_color.push_back(glm::vec4(text_color, 1.0f));
    }
    return;
}

void TextManager::addString(const char* text, float x, float y, float z, float sx, float sy, 
        glm::vec3 text_color, glm::vec2 max_dimension){
    sx *= (/*0.25 * */0.50);
    sy *= (/*0.25 * */0.50);

    float original_x = x;
    float original_y = y;
    // TODO: CHANGE BASED ON INPUT
    Font* f = m_fonts[TextStyles::REGULAR];
    float tex_width = f->getWidth();
    float tex_height = f->getHeight();

    for( const char* pos = text; *pos != '\0'; pos++ ){
        CharInfo c_info = f->getCharInfo((int)(*pos));

        float new_x = x + c_info.bl * sx;
        float new_y = -y - c_info.bt * sy;
        float width = c_info.bw * sx;
        float height= c_info.bh * sy;

        x += c_info.ax * sx;
        y += c_info.ay * sy;

        if( width == 0 || height == 0 ){
            continue;
        }

        if( new_x + width > max_dimension[0] ){
            break;
        }


        glm::vec4 a = glm::vec4( new_x        , -new_y            , z, 1.0f );
        glm::vec4 b = glm::vec4( new_x + width, -new_y            , z, 1.0f );
        glm::vec4 c = glm::vec4( new_x        , -new_y - height   , z, 1.0f );
        glm::vec4 d = glm::vec4( new_x + width, -new_y - height   , z, 1.0f );

        m_text_position.push_back(glm::vec3( a ));
        m_text_position.push_back(glm::vec3( b ));
        m_text_position.push_back(glm::vec3( c ));

        m_text_position.push_back(glm::vec3( b )); 
        m_text_position.push_back(glm::vec3( c )); 
        m_text_position.push_back(glm::vec3( d ));

        m_text_uv.push_back(glm::vec2(c_info.tx,                            0.0f));
        m_text_uv.push_back(glm::vec2(c_info.tx + c_info.bw / tex_width,    0.0f));
        m_text_uv.push_back(glm::vec2(c_info.tx,                            c_info.bh / tex_height));

        m_text_uv.push_back(glm::vec2(c_info.tx + c_info.bw / tex_width,    0.0f));
        m_text_uv.push_back(glm::vec2(c_info.tx,                            c_info.bh / tex_height));
        m_text_uv.push_back(glm::vec2(c_info.tx + c_info.bw / tex_width,    c_info.bh / tex_height));

        m_text_color.push_back(glm::vec4(text_color, 1.0f));
        m_text_color.push_back(glm::vec4(text_color, 1.0f));
        m_text_color.push_back(glm::vec4(text_color, 1.0f));

        m_text_color.push_back(glm::vec4(text_color, 1.0f));
        m_text_color.push_back(glm::vec4(text_color, 1.0f));
        m_text_color.push_back(glm::vec4(text_color, 1.0f));
    }
    return;
}
void TextManager::addStrings(std::vector<std::string> & text_lines, 
        glm::vec3 screen_coordinates, 
        glm::vec3 text_color,
        glm::vec2 max_dimension){
    float screen_width = m_args->tiled_display.my_width;
    float screen_height = m_args->tiled_display.my_height;

    for(int i = 0; i < text_lines.size(); i++){
        float new_x = screen_coordinates.x;
        float new_y = screen_coordinates.y - (15.0f * i);
        
        addString(text_lines[i].c_str(), new_x, new_y, screen_coordinates.z, 
                1.0f, 1.0f, text_color,max_dimension);
    }
    return;
}
void TextManager::addStrings(std::string text,
        std::vector<glm::vec3> screen_coordinates,
        glm::vec3 text_color){


    std::vector<glm::mat4> matrices(screen_coordinates.size() - 1, glm::mat4(1.0));
    std::vector<double> distances(screen_coordinates.size() - 1, 0.0);
    std::vector<std::string> split_strings(screen_coordinates.size() - 1, "" );

    glm::vec2 screen_dimensions = glm::vec2(m_args->tiled_display.my_width, m_args->tiled_display.my_height);

    for( int i = 1; i < screen_coordinates.size(); i++ ){
        // Calculate distance
        double distance = glm::distance(glm::vec2(screen_coordinates[i]), glm::vec2(screen_coordinates[i-1]));

        distances[i-1] = distance;

        // Calculate rotation / translation matrix
        double dx = screen_coordinates[i].x - screen_coordinates[i-1].x;
        double dy = screen_coordinates[i].y - screen_coordinates[i-1].y;

        float c_x = screen_coordinates[i-1].x + dx / 2.0;
        float c_y = screen_coordinates[i-1].y + dy / 2.0;

        c_x = screen_coordinates[i-1].x;
        c_y = screen_coordinates[i-1].y;

        double deg = atan2(dy, dx) * 180 / 3.14159265;
        glm::mat4 translate_to_origin = glm::translate( 
                glm::mat4(1.0f), 
                glm::vec3(-c_x, -c_y, 0.0f));
        glm::mat4 rotate = glm::rotate(
                glm::mat4(1.0f),
                (float)(deg),
                glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 translate_from_origin = glm::translate(
                glm::mat4(1.0f),
                glm::vec3(c_x, c_y, 0.0f));
        glm::mat4 aspect_ratio_scale = glm::scale(
                glm::mat4(1.0f),
                glm::vec3((float)(screen_dimensions.y) / (float)(screen_dimensions.x), 1.0f, 1.0f));


        glm::mat4 transform = translate_from_origin * rotate * translate_to_origin;
        matrices[i-1] = transform;
    }

    partitionString( split_strings, text, distances ); 

    for( int i = 0; i < split_strings.size(); i++ ){
        addString(split_strings[i].c_str(), screen_coordinates[i].x, screen_coordinates[i].y, 
                screen_coordinates[i].z,  1.0f, 1.0f, text_color, matrices[i]);
    }
}
