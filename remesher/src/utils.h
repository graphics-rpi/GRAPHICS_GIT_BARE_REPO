#ifndef _UTILS_H
#define _UTILS_H

#include <cassert>
#include <vector>
#include <iostream>
#include "vectors.h"

#define INCH_IN_METERS 0.0254
#define FEET_IN_METERS 12 * INCH_IN_METERS

#ifndef M_PI
#define M_PI 3.14159265
#endif

class Mesh;
class Triangle;

// NOTE:  These primes are used for hashing, and should
// not be the same as any of the primes used as sizes of 
// the hash tables (see utils.C)
#define LARGE_PRIME_A 10007
#define LARGE_PRIME_B 11003
#define LARGE_PRIME_C 12007
#define LARGE_PRIME_D 13001

#define max2(x,y) (((x)>=(y))?(x):(y))
#define min2(x,y) (((x)>=(y))?(y):(x))
#define max3(x,y,z) (((x)>=(y)&&(x)>=(z))?(x):(((y)>=(x)&&(y)>=(z))?(y):(z)))
#define min3(x,y,z) (((x)<=(y)&&(x)<=(z))?(x):(((y)<=(x)&&(y)<=(z))?(y):(z)))
#define max4(x,y,z,w) (max2(max2((x),(y)),max2((z),(w))))
#define min4(x,y,z,w) (min2(min2((x),(y)),min2((z),(w))))
#define mid3(x,y,z) ( (((y)<(x))&&((x)<(z))) ? (x) : \
		      ( (((z)<(x))&&((x)<(y))) ? (x) : \
		        ( (((x)<(y))&&((y)<(z))) ? (y) : \
		          ( (((z)<(y))&&((y)<(x))) ? (y) : (z) ))))
#define square(x) ((x)*(x))

inline void MinReplace(double &variable, double v) {
  if (v < variable) variable = v; }

enum SIDE { UNKNOWN_SIDE, ON_SURFACE, INSIDE, OUTSIDE };

double AreaOfTriangle(double a, double b, double c);
double AreaOfTriangle2(double a, double b, double c);
double AreaOfTriangle(const Vec3f &a, const Vec3f &b, const Vec3f &c);
double DistanceBetweenPointAndTriangle
(const Vec3f &p, const Vec3f &v0, const Vec3f &v1, const Vec3f &v2);

double PerpendicularDistanceToLineSegment(const Vec3f &pt, const Vec3f &v0, const Vec3f &v1);
double PerpendicularDistanceToLine(const Vec3f &pt, const Vec3f &v0, const Vec3f &v1);
double DistanceToPlane(const Vec3f &p, 
		      const Vec3f &v0, const Vec3f &v1, const Vec3f &v2, Vec3f &pt);

// ================
// NORMAL FUNCTIONS
void InsertNormal(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3);
void InsertNormal(const double[3], const double[3], const double[3]);

// ======================
// INTERSECTION FUNCTIONS
bool Intersect(const Vec3f &p1, const Vec3f &v1, 
	       const Vec3f &p2, const Vec3f &v2, Vec3f &intersection);
bool SegmentsIntersect(const Vec3f &a1, const Vec3f &a2, 
		       const Vec3f &b1, const Vec3f &b2, Vec3f &intersection);
bool LineSegmentIntersection(const Vec3f &line1, const Vec3f &line2, 
			     const Vec3f &segment1, const Vec3f &segment2, Vec3f &intersection);

bool SphereIntersection(const Vec3f &m, double r, 
			const Vec3f &org, const Vec3f &dir, double &k);
double IntersectSphereWithRay(const Vec3f &p, double radius, 
			     const Vec3f &p1, const Vec3f &p2);

// =====================================================================
// INLINED FUNCTIONS
// =====================================================================

inline double DistanceBetweenTwoPoints(const Vec3f &p1, const Vec3f &p2) {
  Vec3f v = p1 - p2; 
  return v.Length();
}

inline double SquaredDistanceBetweenTwoPoints(const Vec3f &p1, const Vec3f &p2) {
  Vec3f v = p1 - p2;
  return v.Dot3(v); //v.Length();
}

inline double DistanceToLineSegment(const Vec3f &pt, const Vec3f &v0, const Vec3f &v1) {
  double tmp = PerpendicularDistanceToLineSegment(pt,v0,v1);
  if (tmp >= 0) return tmp;
  return min2(DistanceBetweenTwoPoints(pt,v0),DistanceBetweenTwoPoints(pt,v1));
}

inline void computeNormal(const Vec3f &pta, const Vec3f &ptb, 
		   const Vec3f &ptc, Vec3f &normal) {
  Vec3f ab = ptb - pta;
  Vec3f bc = ptc - ptb;
  Vec3f::Cross3(normal,ab,bc);
  normal.Normalize();
}

#define EPSILON 0.00001
#define ANGLE_EPSILON 0.00000001

inline double SignedAngleBetweenNormalized(const Vec3f &v1, const Vec3f &v2, const Vec3f &perp) {
  assert (!v1.isNaN());
  assert (!v2.isNaN());
  double tmp = v1.Dot3(v2);
  tmp = max2(-1,min2(tmp,1));
  assert (!my_isnan(tmp));
  double answer = acosf(tmp);
  assert (!my_isnan(answer));
  assert (answer >= 0 && answer < M_PI + 0.00001);
  if (answer >= M_PI) answer = M_PI - 0.0000001;
  if (answer <= 0) answer =  0.0000001;
  assert (answer > 0 && answer < M_PI);

  Vec3f cross;
  Vec3f::Cross3(cross,v1,v2);
  cross.Normalize();
  double test = perp.Dot3(cross);
  if (fabs(test) < 0.01) return 0; 
  assert (fabs(test) > 0.8);
  if (test < 0) answer *= -1;

  return answer;
}

inline double AngleBetweenNormalized(const Vec3f &v1, const Vec3f &v2) {
  assert (!v1.isNaN());
  assert (!v2.isNaN());
  double tmp = v1.Dot3(v2);
  tmp = max2(-1,min2(tmp,1));
  assert (!my_isnan(tmp));
  double answer = acosf(tmp);
  assert (!my_isnan(answer));
  assert (answer >= 0 && answer < M_PI + 0.00001);
  if (answer >= M_PI) answer = M_PI - 0.0000001;
  if (answer <= 0) answer =  0.0000001;
  assert (answer > 0 && answer < M_PI);
  return answer;
}

inline double AngleBetween(Vec3f v1, Vec3f v2) {
  assert (!v1.isNaN());
  assert (!v2.isNaN());
  double l1_squared = v1.Dot3(v1);
  double l2_squared = v2.Dot3(v2);
  if (l1_squared < EPSILON*EPSILON ||
      l2_squared < EPSILON*EPSILON) return 0;
  v1.Normalize();
  v2.Normalize();
  return AngleBetweenNormalized(v1,v2);
}

inline double MinAngle(const Vec3f &a, const Vec3f &b, const Vec3f &c) {
  assert (!a.isNaN());
  assert (!b.isNaN());
  assert (!c.isNaN());
  Vec3f v1 = a - b; v1.Normalize();
  Vec3f v2 = b - c; v2.Normalize();
  Vec3f v3 = c - a; v3.Normalize();
  Vec3f zero(0,0,0);
  Vec3f _v1 = zero - v1;
  Vec3f _v2 = zero - v2;
  Vec3f _v3 = zero - v3;
  return min3(AngleBetweenNormalized(v1,_v2),
	      AngleBetweenNormalized(v2,_v3),
	      AngleBetweenNormalized(v3,_v1));
}

inline double TrilinearInterpolate(double values[8], double a, double b, double c) {
  double tmp1 = values[0]*(1-c) + values[1]*(c);
  double tmp2 = values[2]*(1-c) + values[3]*(c);
  double tmp3 = values[4]*(1-c) + values[5]*(c);
  double tmp4 = values[6]*(1-c) + values[7]*(c);
  double tmp5 = tmp1*(1-b) + tmp2*(b);
  double tmp6 = tmp3*(1-b) + tmp4*(b);
  return tmp5*(1-a) + tmp6*(a);
}

inline Vec3f ProjectToPlane(const Vec3f &pt, const Vec3f &normal, double d) {
    return (pt + normal * (d-pt.Dot3(normal)));
}


Vec3f ProjectToSegment(const Vec3f &pt, const Vec3f &seg_1, const Vec3f &seg_2);

inline const Vec3f ProjectVerticalToPlane(const Vec3f &pt, const Vec3f &normal, double d) {
  // find z such that...
  if (normal.z() < 0.0001) {
    std::cout << " can't project vertical! " << std::endl;
    return pt;
  }
  double z = (d - pt.x()*normal.x() - pt.y()*normal.y()) / normal.z();
  return Vec3f(pt.x(),pt.y(),z);
}

// =====================================================================
// MISCELLANEOUS FUNCTIONS
void SolveCubic(double b, double c, double d, double *x1, double *x2, double *x3);
void LargestEigenvalue(double *tensor, double *lambda, Vec3f &v);

template <class T> bool Member(const std::vector<T> &v, const T& elem) {
  for (unsigned int i = 0; i < v.size(); i++) {
    if (v[i] == elem) return true;
  }
  return false;
}

template <class T>
bool CommonElement(const std::vector<T> &a, const std::vector<T> &b) {
  for (unsigned int i = 0; i < a.size(); i++) {
    if (Member(b,a[i])) return true;
  }
  return false;
}


bool BarycentricCoordinates(const Vec3f &a, const Vec3f &b, const Vec3f &c,
			    const Vec3f &v,
			    double &alpha, double &beta, double &gamma);

#endif
