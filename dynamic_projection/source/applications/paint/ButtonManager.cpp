#include "TextManager.h"
#include "ButtonManager.h"

#include "button.h"
#include "ClickableObject.h"
#include "text.h"
#include "../../common/Image.h"
#include "../../multi_cursor/interaction.h"
#include <glm/gtx/string_cast.hpp>

#include "base_argparser.h"
//extern int HandleGLError(std::string foo);

ButtonManager::ButtonManager(ArgParser* args) : TextManager(args), m_args(args){
    // Generate
    HandleGLError("BUTTON MANAGER: BEFORE GEN VERTEX ARRAYS");
    glGenVertexArrays(4, &m_vao[0]);

    HandleGLError("BEFORE GEN BUFFERS");
    glGenBuffers(2, &m_radius_vbo[0]);
    glGenBuffers(4, &m_position_vbo[0]);
    glGenBuffers(2, &m_color_vbo[0]);
    glGenBuffers(2, &m_uv_vbo[0]);

    // Rectangle
    HandleGLError("BEFORE BIND VERTEX ARRAY");
    glBindVertexArray(m_vao[ButtonManagerVBO::RECTANGLE]);

    HandleGLError("BEFORE ENABLE VERTEX ATTRIB");
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_position_vbo[ButtonManagerVBO::RECTANGLE]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    HandleGLError("BEFORE ENABLE VERTEX ATTRIB");
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_color_vbo[ButtonType::RECT]);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Textured Rectangle
    HandleGLError("BEFORE BIND VERTEX ARRAY");
    glBindVertexArray(m_vao[ButtonManagerVBO::RECTANGLE_TEXTURE]);

    HandleGLError("BEFORE ENABLE VERTEX ATTRIB");
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_position_vbo[ButtonManagerVBO::RECTANGLE_TEXTURE]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    HandleGLError("BEFORE ENABLE VERTEX ATTRIB");
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, m_uv_vbo[ButtonType::RECT]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Circle
    HandleGLError("BEFORE BIND VERTEX ARRAY");
    glBindVertexArray(m_vao[ButtonManagerVBO::CIRCLE]);

    HandleGLError("BEFORE ENABLE VERTEX ATTRIB");
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_position_vbo[ButtonManagerVBO::CIRCLE]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    HandleGLError("BEFORE ENABLE VERTEX ATTRIB");
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_color_vbo[ButtonType::CIRC]);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    HandleGLError("BEFORE ENABLE VERTEX ATTRIB");
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, m_radius_vbo[ButtonRadiusType::FLAT_COLOR]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Textured Circle
    HandleGLError("BEFORE BIND VERTEX ARRAY");
    glBindVertexArray(m_vao[ButtonManagerVBO::CIRCLE_TEXTURE]);

    HandleGLError("BEFORE ENABLE VERTEX ATTRIB");
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_position_vbo[ButtonManagerVBO::CIRCLE_TEXTURE]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    HandleGLError("BEFORE ENABLE VERTEX ATTRIB");
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, m_uv_vbo[ButtonType::CIRC]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    HandleGLError("BEFORE ENABLE VERTEX ATTRIB");
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, m_radius_vbo[ButtonRadiusType::TEXTURED]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);
}

static bool SortByZOrder(const Button* a, const Button* b){
    return a->getZOrder() < b->getZOrder();
}

void ButtonManager::MyBindUniforms(glm::mat4 model_view, glm::mat4 projection, int texture_id){
    // Bind Uniform Data
    std::vector<glm::mat4> matricies;
    std::vector<int> integers;

    matricies.push_back( model_view );
    ShaderManager::current_shader->BindUniform( "model_view_matrix", matricies );
    matricies.clear();

    matricies.push_back( projection );
    ShaderManager::current_shader->BindUniform( "projection_matrix", matricies );
    matricies.clear();
    
    if( texture_id != -1 ){
        integers.push_back(texture_id);
        ShaderManager::current_shader->BindUniform( "texture_sampler", integers );
        integers.clear();
    }
}

void ButtonManager::BindMagnifierUniforms( bool apply_magnification ){
    std::vector<int> integers;
    std::vector<glm::vec2> vectors;

    if( apply_magnification ){
        integers.push_back(1);
        ShaderManager::current_shader->BindUniform( "magnify", integers );
        integers.clear();

        vectors.push_back( glm::vec2(m_args->tiled_display.my_width, m_args->tiled_display.my_height ) );
        ShaderManager::current_shader->BindUniform( "screen_resolution", vectors );
        vectors.clear();
    }
    else{
        integers.push_back(0);
        ShaderManager::current_shader->BindUniform( "magnify", integers );
        integers.clear();
    }
}

void ButtonManager::Draw(glm::mat4 model_view, glm::mat4 projection, bool text_offset){

    // Circle Border
    std::vector<glm::vec3> border_circle_pos;
    std::vector<glm::vec4> border_circle_color;
    std::vector<GLfloat> border_circle_radius;

    // Rect Border
    std::vector<glm::vec3> border_rectangle_pos;
    std::vector<glm::vec4> border_rectangle_color;

    // Flat Rectangle
    std::vector<glm::vec3> flat_rectangle_pos;
    std::vector<glm::vec4> rectangle_color;

    // Tex Rectangle
    std::vector<glm::vec3> tex_rectangle_pos;
    std::vector<glm::vec2> rectangle_uv;

    // Flat Circle
    std::vector<glm::vec3> flat_circle_pos;
    std::vector<glm::vec4> circle_color;
    std::vector<GLfloat> flat_circle_radius;

    // Tex Circle
    std::vector<glm::vec3> tex_circle_pos;
    std::vector<glm::vec2> circle_uv;
    std::vector<GLfloat> tex_circle_radius;

    //std::sort( m_buttons.begin(), m_buttons.end(), SortByZOrder );


    GLuint LargeTexID = -1;
    GLuint SmallTexID = -1;

    HandleGLError("BUTTON MANAGER: 10");

    HandleGLError("BUTTON MANAGER: 11");
    // Retrieve Data
    //printf("NUM BUTTONS: %d\n", m_buttons.size() );
    for( int i = 0; i < m_buttons.size(); i++ ){
        Button* current_button = m_buttons[i];
        if( current_button->isVisible() ){
            if( current_button->isCircle() && !current_button->GetRenderText() ){
                // Circular Buttons
                // Border rendering
                current_button->populateCircleBorder( flat_circle_pos, circle_color, flat_circle_radius );
                if( current_button->SmallImageSpecifiedAndLoaded() ){
                    // Textured button
                    current_button->populateCircleTexturedButton( tex_circle_pos, circle_uv, tex_circle_radius );
                    SmallTexID = current_button->getSmallTexName();
                }
                else{
                    // Untextured button
                    current_button->populateCircleButton( flat_circle_pos, circle_color, flat_circle_radius );
                }
            }
            else {
                // Rectangular buttons
                // Border Rendering
                current_button->populateRectangleBorder( flat_rectangle_pos, rectangle_color );

                //printf("%d: enabled_texture: %d spec_and_loaded: %d\n", i, current_button->is_texture_enabled(), current_button->LargeImageSpecifiedAndLoaded());
                if (current_button->is_texture_enabled() && current_button->LargeImageSpecifiedAndLoaded()){
                    // Textured button
                    current_button->populateRectangleTexturedButton( tex_rectangle_pos, rectangle_uv );
                    LargeTexID = current_button->getLargeTexName();
                }
                else {
                    // Untextured button
                    current_button->populateRectangleButton( flat_rectangle_pos, rectangle_color );
                }
            }
            // Handle Text
            if( current_button->GetRenderText() && current_button->numTextStrings() > 0 ){
                glm::vec4 button_corner = glm::vec4(current_button->getUpperLeftText(), 1.0f);
                glm::vec2 screen_coordinates = glm::vec2(projection * model_view * button_corner) ;

                float x_dim = m_args->tiled_display.my_width;
                float y_dim = m_args->tiled_display.my_height;

                glm::vec2 pixel_loc = 0.5f * (screen_coordinates + glm::vec2(1.0f, 1.0f)) * 
                    glm::vec2(x_dim, y_dim);

                pixel_loc = pixel_loc + glm::vec2(5.0f, -10.0f);
                if( !text_offset ){
                    pixel_loc = pixel_loc + glm::vec2(0.0f, -20.0f);
                }

                glm::vec4 opposite_corner = glm::vec4(current_button->getLowerRightText(), 1.0f);
                glm::vec2 opposite_screen = glm::vec2(projection * model_view * opposite_corner);
                glm::vec2 opposite_pixel = 0.5f * (opposite_screen + glm::vec2(1.0f, 1.0f)) * 
                    glm::vec2(x_dim, y_dim);


                addStrings(current_button->getTextStrings(), 
                        glm::vec3(pixel_loc, current_button->getZOrder()), 
                        glm::vec3(1.0f, 1.0f, 1.0f), 
                        opposite_pixel);
            }
        }
    }

    // Perform draw functions

    HandleGLError("BUTTON MANAGER: 1");
    // ==================================
    // Drawing Flat Circles
    ShaderManager::BindProgram("flat_circle");
    MyBindUniforms(model_view, projection);
    //BindMagnifierUniforms( apply_magnification );

    // Allow Blending for borders
    glEnable(GL_BLEND);

    // ==================================
    // Bind and draw circle button buffers
    glBindVertexArray(m_vao[ButtonManagerVBO::CIRCLE]);
    glBindBuffer(GL_ARRAY_BUFFER, m_position_vbo[ButtonManagerVBO::CIRCLE]);
    glBufferData(GL_ARRAY_BUFFER, flat_circle_pos.size() * sizeof(glm::vec3), &flat_circle_pos[0],
            GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_color_vbo[ButtonType::CIRC]);
    glBufferData(GL_ARRAY_BUFFER, circle_color.size() * sizeof(glm::vec4), &circle_color[0],
            GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_radius_vbo[ButtonRadiusType::FLAT_COLOR]);
    glBufferData(GL_ARRAY_BUFFER, flat_circle_radius.size() * sizeof(GLfloat), &flat_circle_radius[0],
            GL_DYNAMIC_DRAW);

    glDrawArrays(GL_POINTS, 0, flat_circle_pos.size());

    glDisable(GL_BLEND);

    // ==================================
    // Bind and draw textured circle buttons
    ShaderManager::BindProgram("tex_circle");
    MyBindUniforms(model_view, projection, 0);
    if (SmallTexID != -1) {
      glBindTexture(GL_TEXTURE_2D, SmallTexID);
    }

    glBindVertexArray(m_vao[ButtonManagerVBO::CIRCLE_TEXTURE]);
    glBindBuffer(GL_ARRAY_BUFFER, m_position_vbo[ButtonManagerVBO::CIRCLE_TEXTURE]);
    glBufferData(GL_ARRAY_BUFFER, tex_circle_pos.size() * sizeof(glm::vec3), &tex_circle_pos[0],
            GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_uv_vbo[ButtonType::CIRC]);
    glBufferData(GL_ARRAY_BUFFER, circle_uv.size() * sizeof(glm::vec4), &circle_uv[0],
            GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_radius_vbo[ButtonRadiusType::TEXTURED]);
    glBufferData(GL_ARRAY_BUFFER, tex_circle_radius.size() * sizeof(GLfloat), &tex_circle_radius[0],
            GL_DYNAMIC_DRAW);

    glDrawArrays(GL_POINTS, 0, tex_circle_pos.size());


    HandleGLError("BUTTON MANAGER: 2");
    // ==================================
    // Drawing Flat Rectangles
    ShaderManager::BindProgram("flat_rectangle");
    MyBindUniforms(model_view, projection);
    //BindMagnifierUniforms( apply_magnification );

    /*
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    */
    // ==================================
    // Bind and draw rectangle button buffers
    glBindVertexArray(m_vao[ButtonManagerVBO::RECTANGLE]);
    glBindBuffer(GL_ARRAY_BUFFER, m_position_vbo[ButtonManagerVBO::RECTANGLE]);
    glBufferData(GL_ARRAY_BUFFER, flat_rectangle_pos.size() * sizeof(glm::vec3), &flat_rectangle_pos[0],
            GL_DYNAMIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, m_color_vbo[ButtonType::RECT]);
    glBufferData(GL_ARRAY_BUFFER, rectangle_color.size() * sizeof(glm::vec4), &rectangle_color[0],
            GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, flat_rectangle_pos.size());
    /*
    glDisable(GL_BLEND);
    */


    HandleGLError("BUTTON MANAGER: 3");
    // ==================================
    // Drawing Flat Rectangles
    ShaderManager::BindProgram("textured_rectangle");
    MyBindUniforms(model_view, projection, 0);
    //BindMagnifierUniforms( apply_magnification );
    if (LargeTexID != -1) {
      glBindTexture(GL_TEXTURE_2D, LargeTexID);
    }

    // ===================================
    // Bind and draw textured rectangle button buffers
    glBindVertexArray(m_vao[ButtonManagerVBO::RECTANGLE_TEXTURE]);
    glBindBuffer(GL_ARRAY_BUFFER, m_position_vbo[ButtonManagerVBO::RECTANGLE_TEXTURE]);
    glBufferData(GL_ARRAY_BUFFER, tex_rectangle_pos.size() * sizeof(glm::vec3), &tex_rectangle_pos[0],
            GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_uv_vbo[ButtonType::RECT]);
    glBufferData(GL_ARRAY_BUFFER, rectangle_uv.size() * sizeof(glm::vec2), &rectangle_uv[0],
            GL_DYNAMIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, tex_rectangle_pos.size());

    // Draw Text
    //TextManager::addString("Hello World", 73486, 860965);
    TextManager::Draw(model_view, projection);
}

