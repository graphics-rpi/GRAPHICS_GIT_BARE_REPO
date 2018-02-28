#ifndef _QUAD_H
#define _QUAD_H

#include <iostream>
#include <climits>
#include <string>
#include <cassert>
#include "element.h"
#include "boundingbox.h"
#include "element.h"

class Element;
class Mesh;

// ===========================================================

class Quad : public Element {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Quad(int a, int b, int c, int d, Mesh *mesh, std::string mat);
  ~Quad();

  // =========
  // ACCESSORS
  bool isATriangle() const { return 0; }
  bool isAQuad() const { return 1; }
  bool isAPolygon() const { return 0; }
  int numVertices() const { return 4; }
  int operator[](int i) const { 
    assert (i >= 0 && i < numVertices()); 
    return verts[i]; }

  double getDensity() const;

  // =========
  // MODIFIERS
  void setVertex(int i, int vert) {
    assert (i >= 0 && i < 4); 
    verts[i]=vert;
  }

  // ===========
  // COMPUTATION
  static double Area(const Vec3f &a, const Vec3f &b, const Vec3f &c, const Vec3f &d);
  double Area() const;
  bool IntersectsSegment(const Vec3f &s1, const Vec3f &s2) const;
  static bool IntersectsSegment
    (const Vec3f &q1, const Vec3f &q2, const Vec3f &q3, const Vec3f &q4, const Vec3f &normal,
     const Vec3f &s1, const Vec3f &s2);

  static bool PointInside
    (const Vec3f &q1, const Vec3f &q2, const Vec3f &q3, const Vec3f &q4, const Vec3f &normal,
     const Vec3f &pt);

  // =====
  // Print
  void Check() const;
  virtual void Print(const char *s = "") const;  
  virtual void PrintLots(const char *s = "") const;

  friend std::ostream& operator<<(std::ostream& ostr, const Quad &q) {
    ostr << "quad  " << q.verts[0] << " " << q.verts[1] << " " << q.verts[2] << " " << q.verts[3] << std::endl;
    return ostr;
  }

  int ShortestEdgeIndex() const;

  static bool NearZeroArea2(const Vec3f &a, const Vec3f &b, const Vec3f &c, const Vec3f &d, Mesh *m);
  static bool NearZeroArea2(int a, int b, int c, int d, Mesh *m);

  static bool NearZeroAngle2(const Vec3f &a, const Vec3f &b, const Vec3f &c, const Vec3f &d, Mesh *m);
  static bool NearZeroAngle2(int a, int b, int c, int d, Mesh *m);

protected:

  // don't use these
  Quad() : Element(NULL,"") { assert(0); }
  Quad& operator = (const Quad &q) { assert(0); return *this; }
  
  // ==============
  // REPRESENTATION
  int verts[4];
  std::vector<double> blend_weights[4];
  Vec3f normal;
};


// ===========================================================

#endif
