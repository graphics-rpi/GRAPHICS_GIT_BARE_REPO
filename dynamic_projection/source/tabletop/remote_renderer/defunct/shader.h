#ifndef _SHADER_H_
#define _SHADER_H_

#define GL_GLEXT_PROTOTYPES
#define GL_API
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>

class ShaderSet{
 public:
  ShaderSet();
  
  bool loadVertexShader(const char* filename);
  bool loadFragmentShader(const char* filename);

  void bindVertexAttribute(const char* attr, int index);

  void initUniform2f(const char* name, float x, float y);
  void initUniform3f(const char* name, float x, float y, float z);

  void enable();
  void disable();

 private:
  bool readShaderSource(const char* filename,  GLcharARB* &source);
  bool loadShader(const char* filename, GLenum shader_type);

  GLenum program;
};

#endif
