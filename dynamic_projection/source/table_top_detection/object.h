#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <vector>

#include "../common/Image.h"
#include "../common/CalibratedCamera.h"
#include "point.h"
#include "color.h"
#include "histogram.h"

class Scene;

// ======================================================================
// OBJECT
// ======================================================================

class Object {

 public:

  Object() { confidence = -1; }

  virtual ~Object() {}
  virtual void project(CalibratedCamera &camera) = 0;
  virtual void write(FILE *fp) = 0;
  virtual void draw(Image<sRGB> &image) = 0;

  virtual bool isWall() { return false; }
  virtual bool isColorToken() { return false; }
  virtual const Point& getCenter() const { assert (0); exit(0); }

  double getConfidence() const { assert (confidence >= 0 && confidence <= 1); return confidence; }

protected:

  double confidence;

};

inline bool operator<(const Object &o1, const Object &o2) {
  return o1.getConfidence() < o2.getConfidence();
}


// ======================================================================
// BLOB
// ======================================================================

class Blob {
public:
  
  Blob() {
    mass = 0;
    area = 0;
    aspect = 1;
    max_radius = 0.;
  }
  
  void compute_gradients(Image<int>& image, int label);
  Object* classify(Scene *scene);

  // accessors
  double get_max_radius() const { return max_radius; }
  double get_aspect() const { return aspect; }
  double get_area() const { return area; }

  // modifiers
  void adjust_max_radius(double r) { if (r > max_radius) max_radius = r; }
  void increase_area(double a) { area += a; }
  void incr_mass() { mass++; }
  void add_point(const Point &p) { points.push_back(p); }
  void add_edge_point(const Point &p) { edge_points.push_back(p); }

private:

  // REPRESENTATION
  std::vector<Point> points;
  std::vector<Point> edge_points;
  std::vector<Vec2f> edge_gradients;
  Point center; 
  int mass;
  int area;
  double max_radius; // from table center, in pixels (image space)
  double aspect;
  Histogram histogram;
};

#endif
