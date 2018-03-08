#ifndef PROJECTOR_RENDERER_H_
#define PROJECTOR_RENDERER_H_

#define GL_GLEXT_PROTOTYPES
#define GL_API
#define RESOLUTION_HACK

#include "mpi.h" // ?
#include "LiThreadDispatcher.hxx"
#include "LiWindow.hxx"
#include "SocketReader.h"

#include <string.h>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <cstdio>
#include <unistd.h>

using namespace std;

//#include <GL/glut.h>
// gl includes
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glut.h>
//#include "glxfont.c"

// boost includes
#include <boost/bind.hpp>
#include <boost/ref.hpp>

//#include <SDL/SDL.h>
//#include <SDL/SDL_opengl.h>
#include <OpenGLProjector.h>
#include <util.h>
#include <Vector3.h>
#include <Matrix3.h>
#include <Image.h>
#include <X11/Xlib.h>
//#include <mpi.h>
#include "Mesh.hpp"
#include <zlib.h>
//#include "VolumetricData.hpp"

//#include "shader.h"

class ProjectorRenderer{
 public:
  ProjectorRenderer(int width, int height);
  
  // virtual void loadShader(); 
  virtual void loadCompressedTexture(GLuint texture_idx, SocketReader & sr);
  virtual void loadCompressedTexture_z(GLuint texture_idx, SocketReader & sr);
  
  virtual void parseCommandStream(SocketReader &sr, const LI::Window& w);
  virtual void blank();
  virtual void render();
  virtual void flip(); // necessary ? 

  virtual void displayImage(Image<sRGB> &image);
  virtual void loadUncompressedTexture(int texture_idx, Image<sRGB> &image);
  virtual void updateTexture(int texture_idx, Image<sRGB> &image);

 protected:
  ProjectorRenderer();

  int width, height;
  OpenGLProjector glProjector;
  Mesh mesh;
  std::vector<int> textures;
  GLuint displayList;
  bool listBuilt;

  
};

#endif
