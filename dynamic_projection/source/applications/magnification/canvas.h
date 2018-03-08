#ifndef CANVAS_H_
#define CANVAS_H_

#include "../paint/gl_includes.h"

#include <string>
#include <vector>

class ArgParser;
class Mesh;
class Camera;
class ShaderManager;
class Shader;
class Cursor;

class Canvas{
  public:
    // Sets up the overall freeglut canvas and enters the rendering loop
    static void Initialize(ArgParser* args, Mesh* mesh);
  //    static int HandleGLError(std::string foo); 

  private:
    // helper functions
    static void InitShaders();
    static void InitTextures();
    static void InitCursors();

    static void InitUniforms();
    static void BindUniforms();

    static std::vector< glm::vec3 > CalculateMagAreas();
    static glm::vec2 ConvertToObjectSpace( const glm::vec2 & input );

    // helper draw functions
    static void DrawCursors();

    // freeglut callback functions for mouse and keyboard events
    static void Display(void);
    static void Reshape(int w, int h);
    static void Mouse(int button, int state, int x, int y);
    static void Drag(int x, int y);
    static void Motion(int x, int y);
    static void Keyboard(unsigned char key, int x, int y);
    static void SpecialKeyboard(int key, int x, int y);
    static void SpecialKeyboardRelease(int key, int x, int y);
    static void Idle();

    // Helper classes
    static ArgParser* args_;
    static Mesh* mesh_;
    static Camera* camera_;
    static ShaderManager* manager_;
    static Shader* current_shader_;

    // Mouse variables
    static int mouse_button_;
    static int mouse_x_;
    static int mouse_y_;

    static int magnification_factor_;

    static glm::vec2 object_mouse_pos_;
    // Cursors
    static std::vector<Cursor*> cursors_;
    static std::vector<glm::vec2> cursor_positions_;

    // Program
    static GLuint program_id_;

    // Uniform variables
    static GLuint u_matrix_id_;
    static GLuint u_texture_id_;
    static GLuint u_mouse_pos_id_;
    static GLuint u_window_size_id_;
    static GLuint u_magnification_id_;
    static GLuint u_magnification_factor_id_;

};


#endif
