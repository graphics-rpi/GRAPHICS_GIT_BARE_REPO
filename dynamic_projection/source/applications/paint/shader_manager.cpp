#include "shader_manager.h"

#include <fstream>
#include <algorithm> 
#include <stdlib.h>

#include "base_argparser.h"

#include <glm/gtc/type_ptr.hpp>

typedef std::map<std::string, GLuint> ShaderMap;
typedef std::map<std::string, GLuint>::iterator ShaderIterator;
typedef std::pair<std::string, GLuint> ShaderPair;

// ------
// Shader

Shader::Shader(std::string path, std::string name, bool geometry_shader /*= false*/) : name_(name){
    std::string vert_name = std::string(path + ".vert");
    std::string frag_name = std::string(path + ".frag");
    std::string geom_name = std::string(path + ".geom");

    if( geometry_shader ){
        program_id_ = LoadShader(vert_name.c_str(), frag_name.c_str(), geom_name.c_str());
    }
    else {
        program_id_ = LoadShader(vert_name.c_str(), frag_name.c_str(), NULL);
    }
}
void Shader::InitUniform( std::string uniform_name ){
    if( CheckForUniform(uniform_name) ){
        std::cerr << "UNIFORM '" << uniform_name << "'ALREADY BOUND" << std::endl;
        return;
    }
    GLuint uniform_id = glGetUniformLocation(program_id_, uniform_name.c_str() ); 
    uniform_ids_.insert( ShaderPair( uniform_name, uniform_id ));
    return;
}

// 4x4 Matrix
void Shader::BindUniform( std::string uniform_name, std::vector<glm::mat4> & value, int num ){
    if( !CheckForUniform(uniform_name )){
        std::cerr << name_ << " UNIFORM '" << uniform_name << "' NOT BOUND" << std::endl;
        return;
    }

    ShaderIterator it = uniform_ids_.find(uniform_name);
    glUniformMatrix4fv( it->second, num, GL_FALSE, &value[0][0][0]);
    return;
}

// Vec2
void Shader::BindUniform( std::string uniform_name, std::vector<glm::vec2> & value, int num ){
    if( !CheckForUniform(uniform_name )){
        std::cerr << name_ << " UNIFORM '" << uniform_name << "' NOT BOUND" << std::endl;
        return;
    }
    ShaderIterator it = uniform_ids_.find(uniform_name);
    glUniform2fv( it->second, num, &value[0][0]);
    return;
}

// Vec4
void Shader::BindUniform( std::string uniform_name, std::vector<glm::vec4> & value, int num ){
    if( !CheckForUniform(uniform_name )){
        std::cerr << name_ << " UNIFORM '" << uniform_name << "' NOT BOUND" << std::endl;
        return;
    }
    ShaderIterator it = uniform_ids_.find(uniform_name);
    glUniform4fv( it->second, num, &value[0][0]);
    return;
}

// Float
void Shader::BindUniform( std::string uniform_name, std::vector<float> & value, int num ){
    if( !CheckForUniform(uniform_name )){
        std::cerr << name_ << " UNIFORM '" << uniform_name << "' NOT BOUND" << std::endl;
        return;
    }
    ShaderIterator it = uniform_ids_.find(uniform_name);
    glUniform1fv( it->second, num, &value[0]);
    return;
}

// Int
void Shader::BindUniform( std::string uniform_name, std::vector<int> & value, int num ){
    if( !CheckForUniform(uniform_name )){
        std::cerr << name_ << " UNIFORM '" << uniform_name << "' NOT BOUND" << std::endl;
        return;
    }
    ShaderIterator it = uniform_ids_.find(uniform_name);
    glUniform1iv( it->second, num, &value[0] );
    return;
}

void Shader::BindProgram(){
    glUseProgram( program_id_ );
    return;
}

bool Shader::CheckForUniform(std::string uniform_name){
    return uniform_ids_.find( uniform_name ) != uniform_ids_.end();
}

GLuint Shader::LoadShader(const char* vertex_file_path, const char* fragment_file_path, const char* geometry_file_path){
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint GeometryShaderID;

    if( geometry_file_path != NULL ){
        GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
    }

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }


    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    std::string GeometryShaderCode;
    if( geometry_file_path != NULL ){
        std::ifstream GeometryShaderStream(geometry_file_path, std::ios::in);
        if(GeometryShaderStream.is_open()){
            std::string Line = "";
            while(getline(GeometryShaderStream, Line))
                GeometryShaderCode += "\n" + Line;
            GeometryShaderStream.close();
        }
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    if( Result == GL_TRUE ){
        printf("Compiled Vertex Shader successfully!\n");
    }
    else {
        glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            std::vector<char> VertexShaderErrorMessage(InfoLogLength);
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            fprintf(stdout, "VERTEX_ERROR: %s\n", &VertexShaderErrorMessage[0]);
        }
        exit(1);
    }

    if( geometry_file_path != NULL ){
        // Compile Geometry Shader
        printf("Compiling shader : %s\n", geometry_file_path);
        char const * GeometrySourcePointer = GeometryShaderCode.c_str();
        glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer , NULL);
        glCompileShader(GeometryShaderID);

        // Check Geometry Shader
        glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
        if( Result == GL_TRUE ){
            printf("Compiled Geometry Shader successfully!\n");
        } 
        else {
            glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
            if ( InfoLogLength > 0 ){          
                std::vector<char> GeometryShaderErrorMessage(InfoLogLength);
                glGetShaderInfoLog(GeometryShaderID, InfoLogLength, NULL, &GeometryShaderErrorMessage[0]);
                fprintf(stdout, "GEOMETRY_ERROR: %s\n", &GeometryShaderErrorMessage[0]);
            }
            exit(1);
        }
    }

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    if( Result == GL_TRUE ){
        printf("Compiled Fragment Shader successfully!\n");
    }
    else {
        glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
            glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
            fprintf(stdout, "FRAGMENT_ERROR: %s\n", &FragmentShaderErrorMessage[0]);
        }
        exit(1);
    }


    // Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    if( geometry_file_path != NULL ){
        glAttachShader(ProgramID, GeometryShaderID);
    }
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    if( Result == GL_TRUE ){
        printf("Linked Shaders successfully!\n");
    }
    else {
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        std::vector<char> ProgramErrorMessage( std::max(InfoLogLength, int(1)) );
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        fprintf(stdout, "PROGRAM_ERROR: %s\n", &ProgramErrorMessage[0]);
        exit(1);
    }

    glDeleteShader(VertexShaderID);
    if( geometry_file_path != NULL ){
        glDeleteShader(GeometryShaderID);
    }
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

typedef std::map<std::string, Shader*> ShaderManagerMap;
typedef std::map<std::string, Shader*>::iterator ShaderManagerIterator;
typedef std::pair<std::string, Shader*> ShaderManagerPair;

// --------------
// Shader Manager 

ArgParser* ShaderManager::args_;
ShaderManagerMap ShaderManager::shaders_;
Shader* ShaderManager::current_shader = NULL;


void ShaderManager::Initialize(ArgParser* args ){
    args_ = args;
    return;
}

void ShaderManager::Finalize(){
    return;
}

void ShaderManager::CreateProgram( std::string filename, bool geometry_shader /*= false*/){
    ShaderManagerIterator it = shaders_.find(filename);
    if( it != shaders_.end() ){
        std::cerr << "Shader '" << filename << "' already exists!" << std::endl;
        return;
    }

    std::string full_path = args_->m_shader_filepath + filename; 
    printf("\nCreating new shader: %s\n", filename.c_str());
    Shader* new_program = new Shader( full_path, filename, geometry_shader );

    shaders_.insert( ShaderManagerPair( filename, new_program ));
}

void ShaderManager::BindProgram( std::string name ){
    ShaderManagerIterator it = shaders_.find(name);
    if( it == shaders_.end() ){
        std::cerr << "Shader '" << name << "' doesn't exist in the map!" << std::endl;
        return;
    }

    ShaderManager::current_shader = it->second;

    it->second->BindProgram();
    return;
}

Shader* ShaderManager::GetProgram( std::string name ){
    ShaderManagerIterator it = shaders_.find(name);
    if( it == shaders_.end() ){
        std::cerr << "Shader '" << name << "' doesn't exist in the map!" << std::endl;
        return NULL;
    }
    return it->second;
}

