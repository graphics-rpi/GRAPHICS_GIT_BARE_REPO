#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "shader.h"

#include "../argparser.h"
extern ArgParser *ARGS;

ShaderSet::ShaderSet(){
  program = glCreateProgramObjectARB();
}

bool ShaderSet::loadVertexShader(const char* filename){
  return loadShader(filename, GL_VERTEX_SHADER_ARB);
}

bool ShaderSet::loadFragmentShader(const char* filename){
  return loadShader(filename, GL_FRAGMENT_SHADER_ARB);
}

void ShaderSet::bindVertexAttribute(const char* attr, int index){
  glBindAttribLocation(program, index, attr);
}

void ShaderSet::initUniform2f(const char* name, float x, float y){
  glUniform2f(glGetUniformLocation(program, name), x, y);
}

void ShaderSet::initUniform3f(const char* name, float x, float y, float z){
  glUniform3f(glGetUniformLocation(program, name), x, y, z);
}

void ShaderSet::enable(){
  glUseProgramObjectARB(program);
}

void ShaderSet::disable(){
  glUseProgramObjectARB(0);
}

bool ShaderSet::readShaderSource(const char* filename, GLcharARB* &source){
  int length;

  std::ifstream is(filename);

  // get length of file:
  is.seekg (0, std::ios::end);
  length = is.tellg();
  is.seekg (0, std::ios::beg);

  // allocate memory:
  source = new char [length+1];

  // read data as a block:
  is.read (source,length);
  source[length] = '\0';
  is.close();

  //std::cout.write (source,length);

  return true;
}

bool ShaderSet::loadShader(const char* filename, GLenum shader_type){
  GLcharARB* source;
  
  (*ARGS->output) << "Loading shader from file: " << filename << "\r" << std::endl;

  if(!readShaderSource(filename, source)){
    (*ARGS->output) << "Error reading source from file\r" << std::endl;
    return false;
  }

  GLenum shader = glCreateShaderObjectARB(shader_type);
  
  // compile shader
  glShaderSourceARB(shader, 1, (const GLcharARB**) &source, NULL);
  glCompileShader(shader);
  
  int result;
  glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &result); 
 
  if(result!=0){
    (*ARGS->output) << "successful compile\r\n";
  }
  else{
    (*ARGS->output) << "failed to compile\r\n";
    GLcharARB log[1024];
    int len;
    glGetInfoLogARB(shader, 1024, &len, log);
    //std::cout << log << std::endl;  
    return false;
  }
  glAttachObjectARB(program, shader);
  glLinkProgramARB(program);
  glUseProgramObjectARB(program);

  return true;
}
