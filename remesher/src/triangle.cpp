#include <cstdio>
#include <cassert>

#include "utils.h"
#include "mesh.h"
#include "element.h"
#include "triangle.h"
#include "vertex.h"
#include "argparser.h"
#include "accelerationgrid.h"

extern int VERBOSE_FLAG;
extern char LAST_ACTION[200];

// UGLY HACK :(
extern ArgParser *ARGS;

Line::Line(int a, int b, Mesh *mesh, const std::string &mat) : Element(mesh,mat) {
  assert (a >= 0 && b >= 0);
  assert (a != b);
  verts[0] = a;
  verts[1] = b;
}


Triangle::Triangle(int a, int b, int c, Mesh *mesh, std::string mat) : Element(mesh,mat) {
  assert (a >= 0 && b >= 0 && c >= 0);
  assert (a != b && b != c && a != c);
  verts[0] = a;
  verts[1] = b;
  verts[2] = c;
  area = -1;
  if (Element::acceleration_grid != NULL) { Element::acceleration_grid->AddElement(this); }
  num_projectors = ARGS->projector_names.size();
  my_blend_weights = std::vector<double>(num_projectors*3,0);
  my_blend_weights_with_distance = std::vector<double>(num_projectors*3,0);
}

Triangle::~Triangle() {
}

bool Triangle::PointInside(const Vec3f &pt) const {
  const Vec3f &a = getMesh()->getVertex((*this)[0])->get();
  const Vec3f &b = getMesh()->getVertex((*this)[1])->get();
  const Vec3f &c = getMesh()->getVertex((*this)[2])->get();
  return Triangle::PointInside(pt,a,b,c);
}

// only works for y = 0 triangles
bool Triangle::PointInside(Vec3f pt, Vec3f a, Vec3f b, Vec3f c) {

  // project to the y=0 plane first
  pt.zero_y();
  a.zero_y();
  b.zero_y();
  c.zero_y();

  Vec3f va1 = b-a;  //va1.Normalize();
  Vec3f va2 = pt-a; //va2.Normalize();
  Vec3f cross_a; Vec3f::Cross3(cross_a,va1,va2);
  if (cross_a.y() < 0) return false;

  Vec3f vb1 = c-b;  //vb1.Normalize();
  Vec3f vb2 = pt-b; //vb2.Normalize();
  Vec3f cross_b; Vec3f::Cross3(cross_b,vb1,vb2);
  if (cross_b.y() < 0) return false;

  Vec3f vc1 = a-c;  //vc1.Normalize();
  Vec3f vc2 = pt-c; //vc2.Normalize();
  Vec3f cross_c; Vec3f::Cross3(cross_c,vc1,vc2);
  if (cross_c.y() < 0) return false;

  return true;
}

bool Triangle::IntersectsSegment
(const Vec3f &t1, const Vec3f &t2, const Vec3f &t3,
 const Vec3f &s1, const Vec3f &s2) {
  
  // if points are on same side of plane formed by triangle
  // they don't intersect
  Vec3f normal; ::computeNormal(t1,t2,t3,normal);
  //normal.Normalize();
  double d = normal.Dot3(t1);
  double d1 = normal.Dot3(s1) - d;
  double d2 = normal.Dot3(s2) - d;
  if ((d1 >= -EPSILON && d2 >= -EPSILON) ||
      (d1 <= EPSILON && d2 <= EPSILON))
    return false;

  // compute intersection point
  double tmp = fabs(d1 - d2);
  assert (tmp > EPSILON);
  d1 = fabs(d1/tmp);
  d2 = fabs(d2/tmp);
  if (!(fabs(d1 + d2 - 1) < EPSILON)) {
    std::cout << "UGH " << d1 << " " << d2 << " " << tmp << std::endl;
  }
  assert (fabs(d1 + d2 - 1) < EPSILON);
  Vec3f i = s1; i *= d2;
  Vec3f i2 = s2; i2 *= d1;
  i += i2;
  
  // if the intersection point is interior to the triangle
  // they intersect
  Vec3f v12 = t2; v12 -= t1;
  Vec3f v23 = t3; v23 -= t2;
  Vec3f v31 = t1; v31 -= t3;
  Vec3f v1 = i; v1 -= t1;
  Vec3f v2 = i; v2 -= t3;
  Vec3f v3 = i; v3 -= t3;
  Vec3f c1; Vec3f::Cross3(c1, v12, v1); //c1.Normalize();
  Vec3f c2; Vec3f::Cross3(c2, v23, v2); //c2.Normalize();
  Vec3f c3; Vec3f::Cross3(c3, v31, v3); //c3.Normalize();
  double dot1 = normal.Dot3(c1);
  double dot2 = normal.Dot3(c2);
  double dot3 = normal.Dot3(c3);

  if (dot1 > 0 && dot2 > 0 && dot3 > 0) 
    return true;
  return false;
}


void Triangle::Check() const {
  for (int i = 0; i < 3; i++) {
    //Element *e = getNeighbor(i);
    std::vector<Element*> vec = getNeighbors(i);
    
    for (unsigned int j = 0; j < vec.size(); j++) {
      Element *e = vec[j];
      assert (e->isATriangle());
      Triangle *t = (Triangle*)e;
      //int j = t->WhichVertex((*this)[(i+1)%3]);
      //assert (j >= 0 && j < 3);
      //assert ((Triangle*)t->getNeighbor(j) == this);
      assert (t->hasNeighbor(this));
    }
  }
}

/*
void Triangle::WeightedNormalAtVertex(int vert, Vec3f &normal) {

  assert (HasVertex(vert));
  Mesh *m = getMesh();
  
  Vec3f a = m->getVertex((*this)[0])->get();
  Vec3f b = m->getVertex((*this)[1])->get();
  Vec3f c = m->getVertex((*this)[2])->get();
  
  computeNormal(normal);
  double angle;
  Vec3f vab(b,a); vab.Normalize();
  Vec3f vbc(c,b); vbc.Normalize();
  Vec3f vca(a,c); vca.Normalize();
  static Vec3f zero(0,0,0);
  Vec3f _vab(zero,vab);
  Vec3f _vbc(zero,vbc);
  Vec3f _vca(zero,vca);
  if (vert == (*this)[0]) 
    angle = AngleBetweenNormalized(vca,_vab);
  else if (vert == (*this)[1]) 
    angle = AngleBetweenNormalized(vab,_vbc);
  else {
    assert (vert == (*this)[2]);
    angle = AngleBetweenNormalized(vbc,_vca);
  }
  normal *= angle;
}
*/



void Triangle::Print(const char *s) const {
  printf ("%s TRIANGLE %4d: %4d %4d %4d", s, (int)this->getID(), 
	  verts[0],verts[1],verts[2]);
  printf ("\n");
  getMesh()->getVertex((*this)[0])->get().Print("  0");
  getMesh()->getVertex((*this)[1])->get().Print("  1");
  getMesh()->getVertex((*this)[2])->get().Print("  2");
  std::cout << "  material " << getFakeMaterial() << std::endl;
}

void Triangle::PrintLots(const char *s) const {
  Print(s);
  Vec3f center; computeCentroid(center);
  center.Print("center");
  for (int i = 0; i < 3; i++) {
    std::vector<Element*> vec = getNeighbors(i);
    printf ("neighbors 0: \n"); 
    for (unsigned int j = 0; j < vec.size(); j++) {
      vec[j]->Print(); 
    }
  }
}

double Triangle::Area() const {
  ///if (area > -0.5) return area;
  Mesh *m = getMesh();
  Triangle *t = (Triangle*)(this);
  t->area = ::AreaOfTriangle(m->getVertex(verts[0])->get(),
			     m->getVertex(verts[1])->get(),
			     m->getVertex(verts[2])->get());
  assert (area > -0.5);
  return area;
}

/*
inline double AreaOfTriangle(double a, double b, double c) {
  // from the lengths of the 3 edges, compute the area
  // Area of Triangle = (using Heron's Formula)
  //  sqrt[s*(s-a)*(s-b)*(s-c)]
  //    where s = (a+b+c)/2
  // also... Area of Triangle = 0.5 * x * c
  double s = (a+b+c) / (double)2.0;
  return sqrt(s*(s-a)*(s-b)*(s-c));
}

inline double AreaOfTriangle(const Vec3f &a, const Vec3f &b, const Vec3f &c) {
  double aside = DistanceBetweenTwoPoints(a,b);
  double bside = DistanceBetweenTwoPoints(b,c);
  double cside = DistanceBetweenTwoPoints(c,a);
  return AreaOfTriangle(aside,bside,cside);
}
*/

/*double Triangle::Area(const Vec3f &a, const Vec3f &b, const Vec3f &c) {

  return AreaOfTriangle2(a,b,c);
  / *
  Vec3f v1 = a - b; //(a,b);
  Vec3f v2 = c - b; //(c,b);
  Vec3f v3;
  Vec3f::Cross3(v3,v1,v2);
  return v3.Length() / 2.0;
  * /
}*/


int Triangle::ShortestEdgeIndex() const {
  Vec3f a = getMesh()->getVertex((*this)[0])->get();
  Vec3f b = getMesh()->getVertex((*this)[1])->get();
  Vec3f c = getMesh()->getVertex((*this)[2])->get();
  double ab = DistanceBetweenTwoPoints(a,b);
  double bc = DistanceBetweenTwoPoints(b,c);
  double ca = DistanceBetweenTwoPoints(c,a);
  if (ab <= bc && ab <= ca) return 0;
  if (bc <= ab && bc <= ca) return 1;
  assert (ca <= ab && ca <= bc);
  return 2;
}

/*
void Element::normalizeBlendWeights() {
  assert (this->isATriangle());
  for (int i = 0; i < 3; i++) {
    double total = 0;
    for (int j = 0; j < num_projectors; j++) {
      if (getBlendWeight(i,j) < 0.001) continue;
      total += getBlendWeight(i,j);
    }
    if (total < 0.001) continue;
    for (int j = 0; j < num_projectors; j++) {
      setBlendWeight(i,j,getBlendWeight(i,j)/total);
    }
  }
}
*/


void Vertex::setBlendDistanceFromOcclusion(unsigned int i, double dist) {
  assert (dist >= 0);
  while (i >= blending_distance_from_occlusion.size())
    blending_distance_from_occlusion.push_back(0.0);
  assert (i < blending_distance_from_occlusion.size());
  blending_distance_from_occlusion[i] = dist;
}

