#ifndef BUTTON_MANAGER_H_
#define BUTTON_MANAGER_H_

#include "../paint/gl_includes.h"

#include "shader_manager.h"

#include <vector>

#include "TextManager.h"
#include "button.h"

#include "base_argparser.h"

namespace ButtonManagerVBO {
  enum Enum {
    RECTANGLE,
    RECTANGLE_TEXTURE,
    CIRCLE,
    CIRCLE_TEXTURE
  };
}
namespace ButtonManagerTexture {
  enum Enum {
    SMALL,
    LARGE
  };
}

namespace ButtonType {
  enum Enum {
    RECT,
    CIRC
  };
}

namespace ButtonRadiusType { 
  enum Enum {
    FLAT_COLOR,
    TEXTURED
  };
}

class ButtonManager 
: public TextManager 
{
    public:
        // Constructor
        ButtonManager(ArgParser* args);

        virtual void Draw(glm::mat4 model_view, glm::mat4 projection, bool text_offset = true);

        void AddButton(Button* button, bool verbose = false){ 
            if (verbose) {
                printf("BUTTON SIZE: %d\n", m_buttons.size()); 
            }
            m_buttons.push_back(button); 
        }

    private:
        // Helper functions
        void MyBindUniforms( glm::mat4 model_view, glm::mat4 projection, int texture_id = -1 );
        void BindMagnifierUniforms( bool apply_magnification );

        ArgParser* m_args;

        // Container
        std::vector<Button*> m_buttons;

        // OpenGL
        GLuint m_vao[4];
        GLuint m_position_vbo[4];

        GLuint m_color_vbo[2];
        GLuint m_uv_vbo[2];
        GLuint m_radius_vbo[2];

        GLuint m_texid[2];
};

#endif
