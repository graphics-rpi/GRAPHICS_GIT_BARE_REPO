#include "OffScreenBuffer.hpp"
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <cstdio>
#include "OpenGLProjector.h"
#include "vectors.h"

#include "argparser.h"
extern ArgParser *ARGS;

//
// create a *.glcam file to image an ortho view to a texture
//
void SurfaceCamera(const Vec3f &eye, const Vec3f &direction, const Vec3f &up,
                   GLdouble height, GLdouble width,
                   GLint texture_height, GLint texture_width,
                   GLdouble nearPoint, GLdouble farPoint,
                   const char *glcam_filename){

  GLdouble projection_matrix[16];
  GLdouble modelview_matrix[16];

  //  std::cout<< "eye = " << eye.x() << "," << eye.y() << "," << eye.z() << std::endl;

  //#define SURFACE_CAM_USE_OPENGL
#ifdef SURFACE_CAM_USE_OPENGL
  // projection matrix 
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(-width/2., width/2., -height/2., height/2., near, far);
  glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);
  glPopMatrix();
#else
  // equations from: http://www.opengl.org/sdk/docs/man/xhtml/glOrtho.xml
  double right  =  width/2.;
  double left   = -width/2.;
  double top    =  height/2.;
  double bottom = -height/2.;
  
  double tx = -(right + left)/(right - left);
  double ty = -(top + bottom)/(top - bottom);
  double tz = -(farPoint + nearPoint)/(farPoint - nearPoint);

  for (int i=0; i<16; i++){
    projection_matrix[i] = 0.;
  }
  // note: opengl matrixes are stored column-major, so this looks transposed
  //       relative to documentation
  projection_matrix[0*4 + 0] = 2./(right-left);
  projection_matrix[3*4 + 0] = tx;
  projection_matrix[1*4 + 1] = 2./(top-bottom);
  projection_matrix[3*4 + 1] = ty;
  projection_matrix[2*4 + 2] = -2./(farPoint-nearPoint);
  projection_matrix[3*4 + 2] = tz;
  projection_matrix[3*4 + 3] = 1.;
#endif

#ifdef SURFACE_CAM_USE_OPENGL
  // modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  Vec3f center = eye + direction;
  gluLookAt(eye.x(), eye.y(), eye.z(),
            center.x(), center.y(), center.z(),
            up.x(), up.y(), up.z());
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
  glPopMatrix();
#else
  // equations from: http://www.opengl.org/sdk/docs/man/xhtml/gluLookAt.xml
  Vec3f center = eye + direction;
  Vec3f f = center - eye;
  f.Normalize();
  Vec3f unit_up = up;
  unit_up.Normalize();
  Vec3f s, u;
  Vec3f::Cross3(s, f, unit_up);
  Vec3f::Cross3(u, s, f);
  double temp_matrix[16];
  for (int i=0; i<16; i++){
    temp_matrix[i] = 0.;
  }
  temp_matrix[0*4 + 0] =  s.x();
  temp_matrix[1*4 + 0] =  s.y();
  temp_matrix[2*4 + 0] =  s.z();
  temp_matrix[0*4 + 1] =  u.x();
  temp_matrix[1*4 + 1] =  u.y();
  temp_matrix[2*4 + 1] =  u.z();
  temp_matrix[0*4 + 2] = -f.x();
  temp_matrix[1*4 + 2] = -f.y();
  temp_matrix[2*4 + 2] = -f.z();
  temp_matrix[3*4 + 3] =  1.;
 
  // translate by -eye
  double trans_matrix[16];
  for (int i=0;i<16;i++){
    trans_matrix[i] = 0.;
  }
  trans_matrix[3*4 + 0] = -eye.x();
  trans_matrix[3*4 + 1] = -eye.y();
  trans_matrix[3*4 + 2] = -eye.z();
  trans_matrix[0*4 + 0] = 1.;
  trans_matrix[1*4 + 1] = 1.;
  trans_matrix[2*4 + 2] = 1.;
  trans_matrix[3*4 + 3] = 1.;
  
  for (int r=0; r<4; r++){
    for (int c=0; c<4; c++){
      modelview_matrix[r*4+c] = 0.;
      for (int k=0; k<4; k++){
	modelview_matrix[r*4+c] += trans_matrix[r*4 + k] * temp_matrix[k*4 + c];
      }
    }
  }

#endif
  if (ARGS->verbose) {
    printf("glcamname %s \n", glcam_filename);
  }
  //assert(0);
  FILE *fp = fopen(glcam_filename, "wt");
  assert(NULL != fp);
  fprintf(fp, "# width height\n");
  fprintf(fp, "%6d %6d\n", texture_width, texture_height);
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
          eye.x(), eye.y(), eye.z());    

  // make vertical vector orthogonal to view direction
  Vec3f vertical = up - up.Dot3(direction) * direction;
  vertical.Normalize();
  // make horizontal vector orthogonal to up and view direction
  Vec3f horizontal;  Vec3f::Cross3(horizontal, vertical, direction); // = vertical.cross(direction);
  horizontal.Normalize();
  // compute near plane points
  Vec3f c = eye + nearPoint * direction; // center of near plane
  Vec3f p0 = c + 0.5 * (+ width * horizontal + height * vertical);
  Vec3f p1 = c + 0.5 * (+ width * horizontal - height * vertical);
  Vec3f p2 = c + 0.5 * (- width * horizontal - height * vertical);
  Vec3f p3 = c + 0.5 * (- width * horizontal + height * vertical);
  
  fprintf(fp, "# near plane points\n");    
  fprintf(fp, "%10.5f %10.5f %10.5f\n", p0.x(), p0.y(), p0.z());
  fprintf(fp, "%10.5f %10.5f %10.5f\n", p1.x(), p1.y(), p1.z());
  fprintf(fp, "%10.5f %10.5f %10.5f\n", p2.x(), p2.y(), p2.z());
  fprintf(fp, "%10.5f %10.5f %10.5f\n", p3.x(), p3.y(), p3.z());

  fprintf(fp, "# distance of near and far plane\n");
  fprintf(fp, "%10.5f %10.5f\n", nearPoint, farPoint); 

  fprintf(fp, "# camera direction\n");
  fprintf(fp, "%10.5f %10.5f %10.5f\n", direction.x(),direction.y(),direction.z()); 

  fclose(fp);
}

#if 0
int main(int argc, char **argv){
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(1,1);
  glutCreateWindow("");

  // sample quad for viewing
  Vec3f p0(0., 0., 0.);
  Vec3f c0(1., 0., 0.);
  Vec3f p1(0., 2., 0.);
  Vec3f c1(0., 1., 0.);
  Vec3f p2(2., 2., 0.);
  Vec3f c2(0., 0., 1.);
  Vec3f p3(2., 0., 0.);
  Vec3f c3(1., 1., 1.);

  double height = 2.5;
  double width = 2.5;
  int texture_height = 256;
  int texture_width = 256;

  Vec3f eye(1., 1., -1.0);
  Vec3f direction(0., 0., 1.);
  Vec3f up(0., 1., 0.);
  double near = 0;
  double far = 100;
  
  // create & write the camera
  SurfaceCamera(eye, direction, up,
                height, width,
                texture_height, texture_width,
                near, far,
                "test.glcam");

  OpenGLProjector camera("test.glcam"); 
  OffScreenBuffer buffer(camera.getHeight(), camera.getWidth(), false, true, GL_RGB);
  buffer.Select();

  camera.setOpenGLCamera();  

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_SMOOTH);
  glColor3f(1., 0., 1.);
  glBegin(GL_QUADS);
  glColor3d(c0.r(), c0.g(), c0.b());
  glVertex3d(p0.x(), p0.y(), p0.z());
  glColor3d(c1.r(), c1.g(), c1.b());
  glVertex3d(p1.x(), p1.y(), p1.z());
  glColor3d(c2.r(), c2.g(), c2.b());
  glVertex3d(p2.x(), p2.y(), p2.z());
  glColor3d(c3.r(), c3.g(), c3.b());
  glVertex3d(p3.x(), p3.y(), p3.z());
  glEnd();

  buffer.WritePPM("test.ppm");
}
#endif
