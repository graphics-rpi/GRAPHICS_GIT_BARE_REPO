#ifndef OFFSCREEN_BUFFER_HPP_INCLUDED_
#define OFFSCREEN_BUFFER_HPP_INCLUDED_

#include <cstdio>
#include <cassert>
#define GL_GLEXT_PROTOTYPES
#define GL_API

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <string>
#include <iostream>

int HandleGLError(std::string);

class OffScreenBuffer {
public:
  OffScreenBuffer(int width, int height, bool has_stencil,
                  GLint color_format = GL_RGBA, 
                  GLint depth_format = GL_DEPTH_COMPONENT){
    this->width = width;
    this->height = height;

    // save the old framebuffer and read and write buffers
    GLint old_fb_id;
    GLint old_read_buf;
    GLint old_draw_buf;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &old_fb_id);
    glGetIntegerv(GL_READ_BUFFER, &old_read_buf);
    glGetIntegerv(GL_DRAW_BUFFER, &old_draw_buf);

    // create new framebuffer
    glGenFramebuffersEXT(1, &framebuffer_id);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer_id);

    // create color buffer and attach to framebuffer
    glGenRenderbuffersEXT(1, &color_id);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, color_id);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
			     color_format, width, height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
				 GL_COLOR_ATTACHMENT0_EXT, 
				 GL_RENDERBUFFER_EXT,
				 color_id);

    check_framebuffer_status();
 
    // create the depth buffer and attach to the framebuffer
    glGenRenderbuffersEXT(1, &depth_id);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_id);
    if (has_stencil){
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
                               GL_DEPTH_STENCIL_EXT, width, height);
      glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                                   GL_STENCIL_ATTACHMENT_EXT,
                                   GL_RENDERBUFFER_EXT,
                                   depth_id);
    } else {
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
                               depth_format, width, height);
    }

    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
				 GL_DEPTH_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT,
				 depth_id);

    check_framebuffer_status();

    glViewport(0,0, width, height);

    // restore old read and write buffers
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, old_fb_id);
    glReadBuffer(old_read_buf);
    glDrawBuffer(old_draw_buf);
  }

  ~OffScreenBuffer(){
    if (glIsRenderbufferEXT(color_id)){
      glDeleteRenderbuffersEXT(1, &color_id);
    }
    if (glIsRenderbufferEXT(depth_id)){
      glDeleteRenderbuffersEXT(1, &depth_id);
    }
    if (glIsFramebufferEXT(framebuffer_id)){
      glDeleteFramebuffersEXT(1, &framebuffer_id);
    }
  }

  void Select(){
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer_id);
  }

  void UnSelect(GLint framebuffer_id = 0, GLenum buffer = GL_BACK){
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer_id);
    glDrawBuffer(buffer);
    glReadBuffer(buffer);
  };

  void WritePPM(const char *filename){
    GLubyte *image_line;
    FILE *fp = fopen(filename, "wb");
    assert(fp != NULL);
    
    assert(HandleGLError("WritePPM 1"));

    image_line = new GLubyte[width * 3];
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    
    assert(HandleGLError("WritePPM 3"));

    fprintf(fp, "P6\n%d %d\n255\n", width, height);
    // save current read buffer
    GLint old_read_buf;
    glGetIntegerv(GL_READ_BUFFER, &old_read_buf);

    assert(HandleGLError("WritePPM 4"));

    //glReadBuffer(GL_AUX0);
    //GLint t;
    //glGetIntegerv(GL_READ_BUFFER,&t);
    //assert(HandleGLError("WritePPM 4a"));
    //std::cout << "bef t " << GL_AUX0 << " " << t << std::endl;
    //    glReadBuffer(GL_AUX0);
    //glGetIntegerv(GL_READ_BUFFER,&t);
    //assert(HandleGLError("WritePPM 4b"));
    
    //    std::cout << "aft t " << t << std::endl;
    assert(HandleGLError("WritePPM 5"));
    for (int row=height-1;row>=0;row--){
    assert(HandleGLError("WritePPM 6"));
      glReadPixels(0, row, (GLsizei)width, (GLsizei)1,
                   GL_RGB, GL_UNSIGNED_BYTE, image_line);
    assert(HandleGLError("WritePPM 7"));
      fwrite(image_line, 3, width, fp);
    }
    assert(HandleGLError("WritePPM 8"));
    // restore read buffer
    glReadBuffer(old_read_buf);

    assert(HandleGLError("WritePPM 9"));
    
    delete[] image_line;
    fclose(fp); 

    assert(HandleGLError("WritePPM 10"));
  }
  
private:
  int width;
  int height;
  GLuint framebuffer_id;
  GLuint depth_id;
  GLuint color_id;
  GLuint stencil_id;

  void check_framebuffer_status(){
    GLenum status; 
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); 
    switch(status) { 
    case GL_FRAMEBUFFER_COMPLETE_EXT: 
      break; 
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      fprintf(stderr, "Error: GL_FRAMEBUFFER_UNSUPPORTED\n");
      assert(0);
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
      fprintf(stderr, "Error: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
      assert(0);
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      fprintf(stderr, "Error: GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS\n");
      assert(0);
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      fprintf(stderr, "Error: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
      assert(0);
    default: 
      assert(0); 
    } 
  }
};

#endif // #ifndef OFFSCREEN_BUFFER_HPP_INCLUDED_
