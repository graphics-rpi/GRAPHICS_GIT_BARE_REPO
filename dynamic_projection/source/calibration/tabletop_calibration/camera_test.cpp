#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <Vector3.h>
#include <util.h>
#include <vector>
#include <math.h>
#include <Image.h>
#include "OpenGLProjector.h"

std::vector<v3d> model_points;
Image<sRGB> image;
int height, width;

void render(){
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glViewport(0, 0, width, height);
  glOrtho(0.0,                
          (GLdouble)width,
          (GLdouble)height,
          0.0,                
          0.0, 1.0);          

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glPixelZoom(1.0, -1.0);
  glRasterPos2i(0, 0);
  glDrawPixels(image.getCols(), image.getRows(), GL_RGB, GL_UNSIGNED_BYTE, 
               image.getData());
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  glColor3f(1., 1., 0.);
  for (unsigned int i = 0; i < model_points.size(); i++){
    glPushMatrix();
    v3d center = model_points.at(i);
    glTranslated(center.x(), center.y(), center.z());
    glutSolidSphere(0.0095, 20, 10);
    glPopMatrix();
  }
}

void KeyEvent(unsigned char key, int x, int y){
  if (key == 'q'){
    exit(0);
  }
}

void display(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  render();
  glutSwapBuffers();
}

void read_data(const char *filename){
  FILE *fp = fopen(filename, "rt");
  if (NULL == fp){
    FATAL_ERROR("cannot open %s for reading", filename);
  }
  while (!feof(fp)){
    v3d point;
    fscanf(fp, "%lf %lf %lf", &point.x(), &point.y(), &point.z());
    if (!feof(fp)){
      model_points.push_back(point);
    }
  }
  fclose(fp);
}

int main(int argc, char **argv){

  OpenGLProjector cam("camera_images/camera_calibration.glcam");
  width = cam.getWidth();
  height = cam.getHeight();
  
  read_data("projector_images/A_1.3d");
  image.load("projector_images/A_1.ppm");

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(cam.getWidth(),cam.getHeight());
  glutCreateWindow("OpenGL camera test");

  glutKeyboardFunc(KeyEvent);
  glutDisplayFunc(display);

  cam.setOpenGLCamera();
                      
  glutMainLoop();

  return 0;
}
