#include <stdint.h>
#include <cstdio>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#include <GL/glut.h>
#endif
#include <cmath>
#include "vectors.h"

//#define M_PI 3.14159

namespace {
  static const int TEXTURE_WIDTH = 512;
  static const int TEXTURE_HEIGHT = 512;
  double* projectionMatrix;
  double* modelviewMatrix;
}


// generate a orthographic camera 
bool genOrthoCamera(const Vec3f& loc, const Vec3f& dir, float width,
		    float height, float nearPoint, float farPoint, double*
		    projection_matrix, double* modelview_matrix)
{
  if(!projection_matrix || !modelview_matrix) {
    fprintf(stderr, "projection or modelview matrix is null");
    return false;
  }

  float halfWidth = width/2.0;
  float halfHeight = height/2.0; 

  double left = -halfWidth;
  double right = halfWidth;
  double bottom = -halfHeight;
  double top = halfHeight;
  
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  
  glOrtho(left, right, bottom, top, static_cast<double> (nearPoint),
	  static_cast<double> (farPoint));
  
  //gluPerspective(90, 1, 0.1, 100);

  // save the projection matrix
  glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);
  glPopMatrix();

  // modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslated(static_cast<double> (-loc.x()), static_cast<double> (-loc.y()),
	       static_cast<double> (-loc.z()) );
  Vec3f axis;
  Vec3f ogl_camera_default_dir(0,0,-1);
  Vec3f::Cross3(axis, dir, ogl_camera_default_dir);
  axis.Normalize();
  double cos_theta = dir.Dot3(ogl_camera_default_dir);
  double angle = acos(cos_theta)*180/(static_cast<double> (M_PI));
  glRotated(angle, axis.x(), axis.y(), axis.z());
  printf("axis:%f,%f,%f,angle:%f\n", axis.x(),axis.y(),axis.z(), angle);
  fflush(stdout);

  glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
  glPopMatrix();

  return true;
}

bool writeOrthoCamera(const char* fileName, const Vec3f& loc, 
		      const double* projection_matrix, 
		      const double* modelview_matrix)
{
  FILE *fp = fopen(fileName, "w");
  if(!fp){
    fprintf(stderr, "unable to open %s for output\n", fileName);
    return false;
  }

  fprintf(fp, "# width height\n");
  fprintf(fp, "%6d %6d\n", TEXTURE_WIDTH, TEXTURE_HEIGHT);
  fprintf(fp, "# projection matrix\n");
  for (int i=0; i<4; i++){
    for (int j=0; j<4; j++){
      fprintf(fp, "%10.5f ", projection_matrix[4*i+j]);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "# modelview matrix\n");
  for (int i=0; i<4; i++){
    for (int j=0; j<4; j++){
      fprintf(fp, "%10.5f ", modelview_matrix[4*i+j]);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "# camera center\n");
  fprintf(fp, "%10.5f %10.5f %10.5f\n",
	  loc.x(), loc.y(), loc.z());

  return true;
}


void display()
{
  glColor3f(1.f, 0.f, 1.f);  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixd(projectionMatrix);
  //gluPerspective(90, 1, 0.1, 100);
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixd(modelviewMatrix);
  //glRotatef(-90, 0, 1, 0);
  //glTranslatef(0, 0, -1);
  glBegin(GL_QUADS);
#if 0
  glVertex3f(-0.5, -0.5, -1);
  glVertex3f( 0.5, -0.5, -1);
  glVertex3f( 0.5,  0.5, -1);
  glVertex3f(-0.5, 0.5, -1);
#else
  glVertex3f(-1, -0.5,  0.5);
  glVertex3f(-1, -0.5, -0.5);
  glVertex3f(-1,  0.5, -0.5);
  glVertex3f(-1,  0.5,  0.5);
#endif
  glEnd();

#if 0
  double modelview_matrix[16];
  double projection_matrix[16];
  
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
  glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);

  Vec3f loc(0,0,0);
  writeOrthoCamera("camera2.glcam", loc, projection_matrix, modelview_matrix);
#endif

  glutSwapBuffers();
}

void keyboard(unsigned char key, int x,int y)
{
  switch(key)
  {
  case 27:
  case 'q':
  case 'Q':
    exit(0);
  }
}

/*
int main(int argc, char** argv)
{ 
  glutInit(&argc,argv);
  glutInitWindowPosition(0, 0);
  glutInitWindowSize(800, 800);
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
  glutCreateWindow(argv[0]);

  Vec3f loc(0,0,0);
  Vec3f dir(-1, 0,0);
  projectionMatrix = new double[16];
  modelviewMatrix = new double[16];
  genOrthoCamera(loc, dir, 1, 1, -0.1, 10, projectionMatrix, modelviewMatrix);
  writeOrthoCamera("camera.glcam", loc, projectionMatrix, modelviewMatrix);

  glutDisplayFunc( display);
  glutReshapeFunc( 0);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc (0);
  
  / * Transfer control to GLUT * /
  glutMainLoop();

  delete[] projectionMatrix;
  delete[] modelviewMatrix;
  return 0;
}
*/
