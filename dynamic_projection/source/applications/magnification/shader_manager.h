#ifndef SHADER_MANAGER_H_
#define SHADER_MANAGER_H_

#include "canvas.h"
#include "../../common/glm/glm/glm.hpp"

#include <map>
#include <string> 

class ArgParser;

class Shader{
    public:
        Shader(std::string filename, std::string name);
        void InitUniform( std::string uniform_name );

        void BindUniform( std::string uniform_name, std::vector<glm::mat4> & value, int num = 1 );
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
        GLuint LoadShader(const char* vertex_file_path, const char* fragment_file_path);
};

class ShaderManager{
    public:
        ShaderManager(ArgParser* args) : args_(args){};
        ~ShaderManager();

        void CreateProgram( std::string filename );
        void BindProgram( std::string name );

        Shader* GetProgram( std::string name );
    private:
        ArgParser* args_;
        std::map<std::string, Shader*> shaders_;
};

#endif
