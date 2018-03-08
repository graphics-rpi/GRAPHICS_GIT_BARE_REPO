#include "PathManager.h"
#include "../../multi_cursor/interaction.h"

#include <algorithm>

PathManager::PathManager(ArgParser* args, bool dynamic) : TextManager(args), m_args(args), m_max_edge_num(0), m_max_vert_num(0), m_dynamic(dynamic), m_loaded(false) {
    glGenVertexArrays(2, &m_edge_vao[0]);
    glGenBuffers(6, &m_edge_vbo[0]);

    // Edge settings
    glBindVertexArray(m_edge_vao[EdgeVAO::EDGE]);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::EDGE_POSITIONS]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::EDGE_COLORS]);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::EDGE_WIDTHS]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Vertex settings
    glBindVertexArray(m_edge_vao[EdgeVAO::VERTEX]);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::VERTEX_POSITIONS]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::VERTEX_COLORS]);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::VERTEX_RADII]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);

}

void PathManager::LoadData(glm::mat4 model_view, glm::mat4 projection){
    m_edge_positions.clear();
    m_edge_colors.clear();
    m_edge_widths.clear();

    m_vertex_positions.clear();
    m_vertex_colors.clear();
    m_vertex_radii.clear();

    for( int i = 0; i < m_paths.size(); i++ ){
        Path* current_path = m_paths[i];
        if( current_path->isVisible() ){
            current_path->populatePath( m_edge_positions, m_edge_colors, m_edge_widths );
            current_path->populateJoints( m_vertex_positions, m_vertex_colors, m_vertex_radii );
            if( current_path->displayText() ){
                double z = current_path->getZOrder();

                std::deque<Pt> points = current_path->getMagnifiedTrail();
                Pt first = points.front();
                Pt last = points.back();

                double dy = last.y - first.y;
                double dx = last.x - first.x;
                
                double deg = atan2( dy, dx ) * 180 / 3.14159265;
                std::vector<glm::vec3> screen_points;
                if( 90 < deg || deg < -90 ){
                    for( int i = points.size() - 1; i >= 0; i-- ){
                        glm::vec2 position = glm::vec2(projection * model_view * glm::vec4(points[i].x, points[i].y, 0.0f, 1.0f));
                        glm::vec2 pixel_position = 0.5f * (position + glm::vec2(1.0f, 1.0f)) * glm::vec2(1600, 1000);
                        screen_points.push_back( glm::vec3(pixel_position, z));
                    }
                }
                else{
                    for( int i = 0; i < points.size(); i++ ){
                        glm::vec2 position = glm::vec2(projection * model_view * glm::vec4(points[i].x, points[i].y, 0.0f, 1.0f));
                        glm::vec2 pixel_position = 0.5f * (position + glm::vec2(1.0f, 1.0f)) * glm::vec2(1600, 1000);
                        screen_points.push_back( glm::vec3(pixel_position, z));
                    }
                }
                TextManager::addStrings( current_path->getText(), screen_points, glm::vec3(1.0f, 1.0f, 1.0f));
            }
        }
    }
    if( m_dynamic ){
        glBindVertexArray(m_edge_vao[EdgeVAO::EDGE]);
        if( m_edge_positions.size() > m_max_edge_num ){
            glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::EDGE_POSITIONS]);
            glBufferData(GL_ARRAY_BUFFER, m_edge_positions.size() * sizeof(glm::vec3), &m_edge_positions[0],
                    GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::EDGE_COLORS]);
            glBufferData(GL_ARRAY_BUFFER, m_edge_colors.size() * sizeof(glm::vec4), &m_edge_colors[0],
                    GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::EDGE_WIDTHS]);
            glBufferData(GL_ARRAY_BUFFER, m_edge_widths.size() * sizeof(GLfloat), &m_edge_widths[0],
                    GL_DYNAMIC_DRAW);
            
            m_max_edge_num = std::max(m_max_edge_num, (unsigned int)m_edge_positions.size());
        }
        else{
            glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::EDGE_POSITIONS]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, m_edge_positions.size() * sizeof(glm::vec3), &m_edge_positions[0]);

            glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::EDGE_COLORS]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, m_edge_colors.size() * sizeof(glm::vec4), &m_edge_colors[0]);

            glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::EDGE_WIDTHS]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, m_edge_widths.size() * sizeof(GLfloat), &m_edge_widths[0]);

        }

        glBindVertexArray(m_edge_vao[EdgeVAO::VERTEX]);
        if( m_vertex_positions.size() > m_max_vert_num ){
            glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::VERTEX_POSITIONS]);
            glBufferData(GL_ARRAY_BUFFER, m_vertex_positions.size() * sizeof(glm::vec3), &m_vertex_positions[0],
                    GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::VERTEX_COLORS]);
            glBufferData(GL_ARRAY_BUFFER, m_vertex_colors.size() * sizeof(glm::vec4), &m_vertex_colors[0],
                    GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::VERTEX_RADII]);
            glBufferData(GL_ARRAY_BUFFER, m_vertex_radii.size() * sizeof(GLfloat), &m_vertex_radii[0],
                    GL_DYNAMIC_DRAW);
            
            m_max_vert_num = std::max(m_max_vert_num, (unsigned int)m_vertex_positions.size());
        }
        else{
            glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::VERTEX_POSITIONS]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertex_positions.size() * sizeof(glm::vec3), &m_vertex_positions[0]);

            glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::VERTEX_COLORS]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertex_colors.size() * sizeof(glm::vec4), &m_vertex_colors[0]);

            glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::VERTEX_RADII]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertex_radii.size() * sizeof(GLfloat), &m_vertex_radii[0]);

        }
    }
    else{
        glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::EDGE_POSITIONS]);
        glBufferData(GL_ARRAY_BUFFER, m_edge_positions.size() * sizeof(glm::vec3), &m_edge_positions[0],
                GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::EDGE_COLORS]);
        glBufferData(GL_ARRAY_BUFFER, m_edge_colors.size() * sizeof(glm::vec4), &m_edge_colors[0],
                GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::EDGE_WIDTHS]);
        glBufferData(GL_ARRAY_BUFFER, m_edge_widths.size() * sizeof(GLfloat), &m_edge_widths[0],
                GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::VERTEX_POSITIONS]);
        glBufferData(GL_ARRAY_BUFFER, m_vertex_positions.size() * sizeof(glm::vec3), &m_vertex_positions[0],
                GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::VERTEX_COLORS]);
        glBufferData(GL_ARRAY_BUFFER, m_vertex_colors.size() * sizeof(glm::vec4), &m_vertex_colors[0],
                GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_edge_vbo[EdgeVBO::VERTEX_RADII]);
        glBufferData(GL_ARRAY_BUFFER, m_vertex_radii.size() * sizeof(GLfloat), &m_vertex_radii[0],
                GL_STATIC_DRAW);
    }

    m_loaded = true;
}

void PathManager::Draw(glm::mat4 model_view, glm::mat4 projection, bool fbo_enabled ){
    if( m_dynamic || !m_loaded ){
        LoadData(model_view, projection);
    }

    if( fbo_enabled ){
        ShaderManager::BindProgram("path_edge_fbo");
    }
    else{
        ShaderManager::BindProgram("path_edge");
    }

    std::vector<glm::mat4> matricies;
    std::vector<glm::vec2> vectors;

    matricies.push_back( model_view );
    ShaderManager::current_shader->BindUniform( "model_view_matrix", matricies );
    matricies.clear();

    matricies.push_back( projection );
    ShaderManager::current_shader->BindUniform( "projection_matrix", matricies );
    matricies.clear();

    vectors.push_back( glm::vec2( m_args->tiled_display.my_width, m_args->tiled_display.my_height ));
    ShaderManager::current_shader->BindUniform( "screen_resolution", vectors );
    vectors.clear();


    glBindVertexArray(m_edge_vao[EdgeVAO::EDGE]);
    glDrawArrays(GL_LINES, 0, m_edge_positions.size());
    
    if( fbo_enabled ){
        ShaderManager::BindProgram("path_vert_fbo");
    }
    else{
        ShaderManager::BindProgram("path_vert");
    }


    matricies.push_back( model_view );
    ShaderManager::current_shader->BindUniform( "model_view_matrix", matricies );
    matricies.clear();

    matricies.push_back( projection );
    ShaderManager::current_shader->BindUniform( "projection_matrix", matricies );
    matricies.clear();

    vectors.push_back( glm::vec2( m_args->tiled_display.my_width, m_args->tiled_display.my_height ));
    ShaderManager::current_shader->BindUniform( "screen_resolution", vectors );
    vectors.clear();


    glBindVertexArray(m_edge_vao[EdgeVAO::VERTEX]);
    glDrawArrays(GL_POINTS, 0, m_vertex_positions.size());
    

    TextManager::Draw(model_view, projection);
    return;
}

void PathManager::AddPath(Path* path){ 
    // If we're adding a new path, the old data >> has << to be reloaded
    m_loaded = false;

    m_paths.push_back(path); 
}
