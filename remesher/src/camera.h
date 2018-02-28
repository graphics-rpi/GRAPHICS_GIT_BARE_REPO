#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <cassert>
#include "vectors.h"

// ====================================================================

class Camera {

public:
  // CONSTRUCTOR & DESTRUCTOR
  Camera(Vec3f &c, Vec3f &p, Vec3f &u);
  virtual ~Camera() {}

  // GL NAVIGATION
  virtual void glInit(int w, int h) = 0;
  void glPlaceCamera(void);
  virtual void dollyCamera(double dist) = 0;
  virtual void zoomCamera(double dist) = 0;
  virtual void truckCamera(double dx, double dy) = 0;
  virtual void rotateCamera(double rx, double ry) = 0;
  
  const Vec3f& getCOI() const { return point_of_interest; }

  virtual double getAngle() const = 0;

  Vec3f getDir() const { return getDirection(); }
  
protected:
  Camera() { assert(0); } // don't use

  // HELPER FUNCTIONS
  Vec3f getHorizontal() const {
    Vec3f answer;
    Vec3f::Cross3(answer, getDirection(), up);
    answer.Normalize();
    return answer; }
  Vec3f getScreenUp() const {
    Vec3f answer;
    Vec3f::Cross3(answer, getHorizontal(), getDirection());
    return answer; }
  Vec3f getDirection() const {
    Vec3f answer = point_of_interest - camera_position;
    answer.Normalize();
    return answer; }

  // REPRESENTATION
  Vec3f point_of_interest;
  Vec3f camera_position;
  Vec3f up;
  int width;
  int height;
};


// ====================================================================

class PerspectiveCamera : public Camera {

public:
  // CONSTRUCTOR & DESTRUCTOR
  PerspectiveCamera(Vec3f &c, Vec3f &p, Vec3f &u, double a);
 ~PerspectiveCamera(void) { }

  // GL NAVIGATION
  void glInit(int w, int h);
  void dollyCamera(double dist);
  void zoomCamera(double dist);
  void truckCamera(double dx, double dy);
  void rotateCamera(double rx, double ry);
  friend std::ostream& operator<<(std::ostream &ostr, const PerspectiveCamera &c) {
    ostr << "PerspectiveCamera {" << std::endl;
    ostr << "  point_of_interest  " << c.point_of_interest;
    ostr << "  camera_position    " << c.camera_position;
    ostr << "  direction          " << c.getDirection();
    ostr << "  up                 " << c.up;
    ostr << "  angle              " << c.angle << std::endl;
    ostr << "}" << std::endl;
    return ostr;
  }
  
  double getAngle() const { return angle; }

private:
  PerspectiveCamera() { assert(0); } // don't use

  // REPRESENTATION
  double angle;
};

// ====================================================================

#endif

