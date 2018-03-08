#ifndef PATH_MANAGER_H_
#define PATH_MANAGER_H_

#include "path.h"

#include "gl_includes.h"
#include "base_argparser.h"
#include "shader_manager.h"
#include "TextManager.h"

#include <vector>

namespace EdgeVAO {
    enum Enum {
        EDGE,
        VERTEX
    };
};

namespace EdgeVBO {
    enum Enum {
        EDGE_POSITIONS,
        EDGE_COLORS,
        EDGE_WIDTHS,
        VERTEX_POSITIONS,
        VERTEX_COLORS,
        VERTEX_RADII
    };
};

class PathManager: public TextManager{
    public:
        PathManager(ArgParser* args, bool dynamic = true);

        virtual void Draw(glm::mat4 model_view, glm::mat4 projection, bool fbo_enabled = false );

        void AddPath(Path* path);

    private:
        void LoadData(glm::mat4 model_view, glm::mat4 projection);

        ArgParser* m_args;
        std::vector<Path*> m_paths;

        GLuint m_edge_vao[2];
        GLuint m_edge_vbo[6];

        unsigned int m_max_edge_num;
        unsigned int m_max_vert_num;

        bool m_dynamic;
        bool m_loaded;

        std::vector<glm::vec3> m_edge_positions;
        std::vector<glm::vec4> m_edge_colors;
        std::vector<GLfloat> m_edge_widths;

        std::vector<glm::vec3> m_vertex_positions;
        std::vector<glm::vec4> m_vertex_colors;
        std::vector<GLfloat> m_vertex_radii;
};

#endif
