#ifndef SHADER_MANAGER_H_
#define SHADER_MANAGER_H_

#include "gl_includes.h"
#include <glm/glm.hpp>

#include <vector>
#include <map>
#include <string> 

class ArgParser;

class Shader{
    public:
        Shader(std::string filename, std::string name, bool geometry_shader = false);
        void InitUniform( std::string uniform_name );

        void BindUniform( std::string uniform_name, std::vector<glm::mat4> & value, int num = 1 );
        void BindUniform( std::string uniform_name, std::vector<glm::vec4> & value, int num = 1);
        void BindUniform( std::string uniform_name, std::vector<glm::vec2> & value, int num = 1);
        void BindUniform( std::string uniform_name, std::vector<float> & value, int num = 1 );
        void BindUniform( std::string uniform_name, std::vector<int> & value, int num = 1 );
        
        void BindProgram();

        std::string name(){ return name_; }
    private:
        std::map<std::string, GLuint> uniform_ids_;
        GLuint program_id_;
        std::string name_;

        bool CheckForUniform(std::string uniform_name);

        // shader loading and compiling
        GLuint LoadShader(const char* vertex_file_path, const char* fragment_file_path, const char* geometry_file_path = NULL);
};

class ShaderManager{
    public:
        static void Initialize( ArgParser* args );
        static void Finalize();

        static void CreateProgram( std::string filename, bool geometry_shader = false );
        static void BindProgram( std::string name );

        static Shader* GetProgram( std::string name );
        static Shader* current_shader;
    private:
        static ArgParser* args_;
        static std::map<std::string, Shader*> shaders_;
};

#endif
