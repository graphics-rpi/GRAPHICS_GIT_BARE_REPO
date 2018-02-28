#include <cmath>
#include <cassert>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "camera.h"
#include "matrix.h"

//#define M_PI 3.14159

// ====================================================================
// ====================================================================
// CONSTRUCTORS

Camera::Camera(Vec3f &c, Vec3f &p, Vec3f &u) {
  camera_position = c;
  point_of_interest = p;
  up = u;
  up.Normalize();
}

PerspectiveCamera::PerspectiveCamera(Vec3f &c, Vec3f &p, Vec3f &u, double a) : Camera(c,p,u) {
  angle = a;
}

// ====================================================================
// ====================================================================
// GL INIT
// Create a camera with the appropriate dimensions that
// crops the screen in the narrowest dimension.

void PerspectiveCamera::glInit(int w, int h) {
  width = w;
  height = h;
  //cout << "glinit w " << w << " h " << h << endl;

  int mode;
  glGetIntegerv(GL_MATRIX_MODE,&mode);
  //  assert (mode == GL_PROJECTION);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  double aspect = double(width)/double(height);

  //double aspect = 1.0;

  //double aspect = double(height)/double(width);

  double fovy = 0.5*angle * 180/M_PI;

  //if (aspect > 1) fovy /= aspect;
  //fovy /= aspect; //if (aspect > 1) fovy /= aspect;
  //if (aspect > 1) asp_angle *= aspect;

  double nearThreshold = 0.1;
  double farThreshold = 100.0;

  gluPerspective(fovy, aspect, nearThreshold, farThreshold);
}

// ====================================================================
// ====================================================================
// GL PLACE CAMERA
// Place a camera within an OpenGL scene

void Camera::glPlaceCamera(void) {
  Vec3f lookAt = camera_position + getDirection();
  gluLookAt(camera_position.x(), camera_position.y(), camera_position.z(),
            lookAt.x(), lookAt.y(), lookAt.z(),
            up.x(), up.y(), up.z());
}

// ====================================================================
// dollyCamera: Move camera along the direction vector
// ====================================================================

void PerspectiveCamera::dollyCamera(double dist) {
  Vec3f diff = camera_position - point_of_interest;
  double d = diff.Length();
  diff.Normalize();
  d *= pow(1.003,dist);
  camera_position = point_of_interest + diff * d;
}

// ====================================================================
// zoomCamera: Change the camera angle
// ====================================================================

void PerspectiveCamera::zoomCamera(double dist) {
  angle *= pow(1.003,dist);
  glInit(width,height);
}

// ====================================================================
// truckCamera: Translate camera perpendicular to the direction vector
// ====================================================================

void PerspectiveCamera::truckCamera(double dx, double dy) {
  Vec3f diff = camera_position - point_of_interest;
  double d = diff.Length();
  Vec3f translate = (d*0.0007)*(getHorizontal()*dx + getScreenUp()*dy);
  camera_position += translate;
  point_of_interest += translate;
}

// ====================================================================
// rotateCamera: Rotate around the up and horizontal vectors
// ====================================================================

void PerspectiveCamera::rotateCamera(double rx, double ry) {
  // Don't let the model flip upside-down (There is a singularity
  // at the poles when 'up' and 'direction' are aligned)
  double tiltAngle = acos(up.Dot3(getDirection()));
  if (tiltAngle-ry > 3.13)
    ry = tiltAngle - 3.13;
  else if (tiltAngle-ry < 0.01)
    ry = tiltAngle - 0.01;

  Mat rotMat;
  rotMat.SetToIdentity();
  rotMat *= Mat::MakeTranslation(point_of_interest);
  rotMat *= Mat::MakeAxisRotation(up, rx);
  rotMat *= Mat::MakeAxisRotation(getHorizontal(), ry);
  rotMat *= Mat::MakeTranslation(-point_of_interest);
  rotMat.Transform(camera_position);
}

// ====================================================================
// ====================================================================
