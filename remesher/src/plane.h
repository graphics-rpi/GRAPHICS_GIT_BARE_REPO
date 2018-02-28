#ifndef _PLANE_H_
#define _PLANE_H_

#include "vectors.h"
#include <vector>

class Plane {

public:

  // construct from a normal & offset
  Plane(const Vec3f &n, double o) { normal = n; offset = o; }
  // construct from 3 points
  Plane(const Vec3f &a, const Vec3f &b, const Vec3f &c);
  ~Plane() {}

  // ACCESSORS
  Vec3f getNormal() const { return normal; }
  double getOffset() const { return offset; }

  // COMPUTATION
  bool ComputeIntersection(Plane *p1, Plane *p2, const std::vector<Vec3f> &points, Vec3f &answer, double tolerance) const;
  bool onPlane(const Vec3f &v, double typical_distance) const;

  double SignedDistanceToPlane(const Vec3f &pt) const;

private:

  bool ClosestPointToPlane(const std::vector<Vec3f> &points, Vec3f &answer) const;
  bool ClosestPointToIntersectionLine(Plane *p1, const std::vector<Vec3f> &points, Vec3f &answer) const;
  bool IntersectPlanes(Plane *p1, Plane *p2, const std::vector<Vec3f> &points, Vec3f &answer) const;

  // REPRESENTATION
  Vec3f normal;
  double offset;

};

#endif
