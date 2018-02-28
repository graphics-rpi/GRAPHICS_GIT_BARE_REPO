#ifndef _TRIANGLE_H
#define _TRIANGLE_H

#include <iostream>
#include <string>
#include <climits>
#include <cassert>

#include "element.h"
#include "boundingbox.h"
#include "element.h"

class Mesh;

// ===========================================================

class Line : public Element {
 public:
  Line(int a, int b, Mesh *mesh, const std::string &mat);

  bool isALine() const { return 1; }
  bool isATriangle() const { return 0; }
  bool isAQuad() const { return 0; }
  bool isAPolygon() const { return 0; }
  int numVertices() const { return 2; }
  int operator[](int i) const { 
    assert (i >= 0 && i < 2);
    return verts[i]; }
  double Area() const { return 0; }
  
 private:
  int verts[2];

};


class Triangle : public Element {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Triangle(int a, int b, int c, Mesh *mesh, std::string mat);
  ~Triangle();

  // =========
  // ACCESSORS
  bool isATriangle() const { return 1; }
  bool isAQuad() const { return 0; }
  bool isAPolygon() const { return 0; }
  int numVertices() const { return 3; }
  int operator[](int i) const { 
    assert (i >= 0 && i < 3);
    return verts[i]; }
  int OtherVertex(int a, int b) const {
    assert (HasVertex(a)); assert (HasVertex(b));
    for (int i = 0; i < 3; i++) 
      if (verts[i] != a && verts[i] != b) return verts[i]; 
    assert (0); return -1; }

  double getDensity() const;

  bool PointInside(const Vec3f &pt) const; // only works for y = 0 triangles
  static bool PointInside(Vec3f pt, Vec3f a, Vec3f b, Vec3f c); // only works for y = 0 triangles

  // =========
  // MODIFIERS
  void setVertex(int i, int vert) {
    assert (i >= 0 && i < 3); 
    verts[i]=vert;
  }

  // ===========
  // COMPUTATION
  //static double Area(const Vec3f &a, const Vec3f &b, const Vec3f &c);
  double Area() const;
  static bool IntersectsSegment
    (const Vec3f &t1, const Vec3f &t2, const Vec3f &t3, const Vec3f &s1, const Vec3f &s2);
  
  // =====
  // Print
  void Check() const;
  virtual void Print(const char *s = "") const;  
  virtual void PrintLots(const char *s = "") const;

  friend std::ostream& operator<<(std::ostream& ostr, const Triangle &t) {
    ostr << "element part " << *((Element*)&t) << std::endl;
    ostr << "triangle  " << t.verts[0] << " " << t.verts[1] << " " << t.verts[2] << std::endl;
    return ostr;
  }

  int ShortestEdgeIndex() const;
  static bool NearZeroArea2(const Vec3f &a, const Vec3f &b, const Vec3f &c, Mesh *m);
  static bool NearZeroArea2(int a, int b, int c, Mesh *m);

  static bool NearZeroAngle2(const Vec3f &a, const Vec3f &b, const Vec3f &c, Mesh *m);
  static bool NearZeroAngle2(int a, int b, int c, Mesh *m);


  /*
    // JUST MOVING THIS ALL TO ELEMENT!  HACK
  double getBlendWeight(int vert, int proj) const {
    //assert (my_blend_weights != NULL);
    assert (vert >= 0 && vert < 3);
    assert (proj >= 0 && proj < num_projectors);
    return my_blend_weights[vert*num_projectors + proj];
  }

  void setBlendWeight(int vert, int proj, double weight) {
    //assert (my_blend_weights != NULL);
    assert (vert >= 0 && vert < 3);
    assert (proj >= 0 && proj < num_projectors);
    my_blend_weights[vert*num_projectors + proj] = weight;
  }
  void normalizeBlendWeights();
  */
  
  //void setBlendWeight(unsigned int vert, unsigned int proj, double weight);

protected:

  // don't use these
  Triangle() : Element(NULL,"") { assert(0); }
  Triangle& operator = (const Triangle &t) { assert(0); return *this; }
  
  // ==============
  // REPRESENTATION
  int verts[3];
  //double *all_blend_weights;
  double area;
};


// ===========================================================

#endif
