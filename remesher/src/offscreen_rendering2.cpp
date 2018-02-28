#include "OffScreenBuffer.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cstdlib>
#include <iostream>

/*
int HandleGLError(string foo) {
  GLenum error;
  int i = 0;
  while ((error = glGetError()) != GL_NO_ERROR) {
    printf ("GL ERROR(#%d == 0x%x):  %s\n", i, error, gluErrorString(error));
    cout << foo << endl;
    if (error != GL_INVALID_OPERATION) i++;
  }
  if (i == 0) return 1;
  return 0;
}
*/

// draw simple colored triangle 
void render(){
  glDisable(GL_LIGHTING);
  HandleGLError("render 1");
  glClearColor(1.0,0,0,0);
  //glClearDepth(0.0);
  glShadeModel(GL_SMOOTH);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.f, 1.f, -1.f, 1.f, -10.f, 10.f);
  //glEnable(GL_DEPTH);
  //glDisable(GL_DEPTH);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glColor3f(1.0f,0.0f,0.0f);
  glBegin(GL_TRIANGLES);	
  glVertex3f(0.0f, 1.0f, 0.0f);
  glColor3f(0.0f,1.0f,0.0f);
  glVertex3f(-1.0f,-1.0f, 0.0f);
  glColor3f(0.0f,0.0f,1.0f);
  glVertex3f(1.0f,-1.0f, 0.0f);
  glEnd();
  HandleGLError("render 4");
  glEnable(GL_LIGHTING);
}

// GLUT callback function
void display_func(){
  render();
  glutSwapBuffers();
}

void main2(void) {//int argc, char **argv){

  //  glutInit(&argc, argv);
  //glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  //glutInitWindowSize(400, 400);
  //glutCreateWindow("on-screen buffer");

  // create the off-screen buffer
  int width = 500;
  int height = 500;

  /*
  int width = 2048;
  int height = 2048;
  */
  HandleGLError("main 1");

#if 1
  std::cout << "a" << std::endl;
  OffScreenBuffer buffer(width, height, false);

  HandleGLError("main 2");
  // select the off-screen buffer for rendering
  buffer.Select();
  HandleGLError("main 3");

  std::cout << "b" << std::endl;

  // draw an example triangle
  render();

  HandleGLError("main 4");

  std::cout << "c" << std::endl;
  // save the color buffer
  HandleGLError("main 5");
  buffer.WritePPM("test2.ppm");
  system("mogrify -format png test2.ppm");
  system("open test2.png");
  HandleGLError("main 6");

  std::cout << "d" << std::endl;

  // unselect the off-screen buffer
  buffer.UnSelect();
#endif

  std::cout << "e" << std::endl;

  // now, render a triangle to the on-screen buffer
  // note: this step is not necessary, it just illustrates switching between
  //       on- and off-screen rendering
  glutDisplayFunc(display_func);

  std::cout << "f" << std::endl;

  // set a timer to exit after 2 seconds
  glutTimerFunc(2000, exit, 0);

  //glutMainLoop();
  //  HandleGLError("main end");
}
