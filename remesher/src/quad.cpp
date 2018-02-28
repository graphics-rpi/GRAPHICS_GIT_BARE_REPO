#include <cstdio>
#include "utils.h"
#include "mesh.h"
#include "element.h"
#include "quad.h"
#include "vertex.h"
#include "triangle.h"

extern int VERBOSE_FLAG;
extern char LAST_ACTION[200];

Quad::Quad(int a, int b, int c, int d, Mesh *mesh, std::string mat) : Element(mesh,mat) {
  assert (a >= 0 && b >= 0 && c >= 0 && d >= 0);
  assert (a != b && b != c && a != c && a != d && b != d && c != d);
  verts[0] = a;
  verts[1] = b;
  verts[2] = c;
  verts[3] = d;
  Vec3f va = mesh->getVertex((*this)[0])->get();
  Vec3f vb = mesh->getVertex((*this)[1])->get();
  Vec3f vc = mesh->getVertex((*this)[2])->get();
  ::computeNormal(va,vb,vc,normal);
}

Quad::~Quad() {
}

bool Quad::IntersectsSegment
(const Vec3f &q1, const Vec3f &q2, const Vec3f &q3, const Vec3f &q4, const Vec3f &normal,
 const Vec3f &s1, const Vec3f &s2) {

  Vec3f _N_;
  ::computeNormal(q1,q2,q3,_N_);
  //double tmp_dot = _N_.Dot3(normal);
  //  std::cout << "tmp_dot " << tmp_dot << " " << std::endl;
  if (_N_.Dot3(normal) < 0.9) return false; 
  if (_N_.Dot3(normal) < 0.99) {
    //std::cout << "FIX Intersectssegment for ramps" << std::endl;
    return false;
  } 
  assert (_N_.Dot3(normal) > 0.99); 

  double d = normal.Dot3(q1);
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
  Vec3f pt = s1 * d2 + s2 * d1;
  
  return PointInside(q1,q2,q3,q4,normal,pt);
}

bool Quad::PointInside
(const Vec3f &q1, const Vec3f &q2, const Vec3f &q3, const Vec3f &q4, const Vec3f &normal,
 const Vec3f &pt) {

  // if the intersection point is interior to the quad
  // they intersect
  Vec3f v12 = q2-q1; //v12.Normalize();
  Vec3f v1 = pt-q1; //v1.Normalize();
  Vec3f c1; Vec3f::Cross3(c1, v12, v1); //c1.Normalize();
  double dot1 = normal.Dot3(c1);
  if (dot1 < 0) return false;

  Vec3f v23 = q3-q2; //v23.Normalize();
  Vec3f v2 = pt-q2; //v2.Normalize();
  Vec3f c2; Vec3f::Cross3(c2, v23, v2); //c2.Normalize();
  double dot2 = normal.Dot3(c2);
  if (dot2 < 0) return false;

  Vec3f v34 = q4-q3; //v34.Normalize();
  Vec3f v3 = pt-q3; //v3.Normalize();
  Vec3f c3; Vec3f::Cross3(c3, v34, v3); //c3.Normalize();
  double dot3 = normal.Dot3(c3);
  if (dot3 < 0) return false;

  Vec3f v41 = q1-q4; //v41.Normalize();
  Vec3f v4 = pt-q4; //v4.Normalize();
  Vec3f c4; Vec3f::Cross3(c4, v41, v4); //c4.Normalize();
  double dot4 = normal.Dot3(c4);
  if (dot4 < 0) return false;

  return true;
}


void Quad::Check() const {
  for (int i = 0; i < 4; i++) {
    //Element *e = getNeighbor(i);
    std::vector<Element*> vec = getNeighbors(i);    
    for (unsigned int j = 0; j < vec.size(); j++) {
      Element *e = vec[j];
      //assert (e->isAQuad());
      //Quad *q = (Quad*)e;
      //int j = t->WhichVertex((*this)[(i+1)%3]);
      //assert (j >= 0 && j < 3);
      //assert ((Quad*)t->getNeighbor(j) == this);
      assert (e->hasNeighbor(this));
    }
  }
}

/*
void Quad::WeightedNormalAtVertex(int vert, Vec3f &normal) {

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



void Quad::Print(const char *s) const {
  printf ("%s QUAD %4d: %4d %4d %4d %4d", s, (int)this->getID(), 
	  verts[0],verts[1],verts[2],verts[3]);
  printf ("\n");
  getMesh()->getVertex((*this)[0])->get().Print("  0");
  getMesh()->getVertex((*this)[1])->get().Print("  1");
  getMesh()->getVertex((*this)[2])->get().Print("  2");
  getMesh()->getVertex((*this)[3])->get().Print("  3");
  std::cout << "  material " << getRealMaterialName() << std::endl;
}

void Quad::PrintLots(const char *s) const {
  Print(s);
  Vec3f center; computeCentroid(center);
  center.Print("center");
  for (int i = 0; i < 4; i++) {
    std::vector<Element*> vec = getNeighbors(i);
    printf ("neighbors %d: \n",i); 
    for (unsigned int j = 0; j < vec.size(); j++) {
      vec[j]->Print(); 
    }
  }
}

bool Quad::IntersectsSegment(const Vec3f &s1, const Vec3f &s2) const {
  Mesh *m = getMesh();
  return IntersectsSegment(m->getVertex(verts[0])->get(),
			   m->getVertex(verts[1])->get(),
			   m->getVertex(verts[2])->get(),
			   m->getVertex(verts[3])->get(),
			   normal,
			   s1,s2);
}


double Quad::Area() const {
  Mesh *m = getMesh();
  return Area(m->getVertex(verts[0])->get(),
	      m->getVertex(verts[1])->get(),
	      m->getVertex(verts[2])->get(),
	      m->getVertex(verts[3])->get());
}

/*
void Quad::computeMovedNormal(Vec3f &normal) const {
  Mesh *m = getMesh();
  ::computeNormal(m->getVertex(verts[0])->getPositionOrMovedPosition(),
                  m->getVertex(verts[1])->getPositionOrMovedPosition(),
                  m->getVertex(verts[2])->getPositionOrMovedPosition(),
                  //m->getVertex(verts[3])->getPositionOrMovedPosition(),
                  normal);
}

double Quad::MovedPositionArea() const {
  Mesh *m = getMesh();
  return Area(m->getVertex(verts[0])->getPositionOrMovedPosition(),
              m->getVertex(verts[1])->getPositionOrMovedPosition(),
              m->getVertex(verts[2])->getPositionOrMovedPosition(),
              m->getVertex(verts[3])->getPositionOrMovedPosition());
}

double Quad::ShortestMovedEdge() const {
  Vec3f a = getMesh()->getVertex((*this)[0])->getPositionOrMovedPosition();
  Vec3f b = getMesh()->getVertex((*this)[1])->getPositionOrMovedPosition();
  Vec3f c = getMesh()->getVertex((*this)[2])->getPositionOrMovedPosition();
  Vec3f d = getMesh()->getVertex((*this)[3])->getPositionOrMovedPosition();
  return min4(DistanceBetweenTwoPoints(a,b),
	      DistanceBetweenTwoPoints(b,c),
	      DistanceBetweenTwoPoints(c,d),
	      DistanceBetweenTwoPoints(d,a));
}
*/

double Quad::Area(const Vec3f &a, const Vec3f &b, const Vec3f &c, const Vec3f &d) {
  return ::AreaOfTriangle(a,b,c) + ::AreaOfTriangle(a,c,d);
}


int Quad::ShortestEdgeIndex() const {
  Vec3f a = getMesh()->getVertex((*this)[0])->get();
  Vec3f b = getMesh()->getVertex((*this)[1])->get();
  Vec3f c = getMesh()->getVertex((*this)[2])->get();
  Vec3f d = getMesh()->getVertex((*this)[3])->get();
  double ab = DistanceBetweenTwoPoints(a,b);
  double bc = DistanceBetweenTwoPoints(b,c);
  double cd = DistanceBetweenTwoPoints(c,d);
  double da = DistanceBetweenTwoPoints(d,a);
  if (ab <= bc && ab <= cd && ab <= da) return 0;
  if (bc <= ab && bc <= cd && bc <= da) return 1;
  if (cd <= ab && cd <= bc && cd <= da) return 2;
  assert (da <= ab && da <= bc && da <= cd);
  return 3;
}

/*
void Quad::setBlendWeight(unsigned int i, unsigned int j, double weight) {
  assert (i >= 0 && i < 4);
  while (j >= blend_weights[0].size()) {
    blend_weights[0].push_back(0.0);
    blend_weights[1].push_back(0.0);
    blend_weights[2].push_back(0.0);
    blend_weights[3].push_back(0.0);
  }
  assert (j < blend_weights[i].size());
  blend_weights[i][j] = weight;
}
*/

/*
void Quad::normalizeBlendWeights() {
  for (int i = 0; i < 4; i++) {
    double total = 0;
    unsigned int num = blend_weights[i].size();
    for (unsigned int j = 0; j < num; j++) {
      if (blend_weights[i][j] < 0.001) continue;
      //assert (blend_weights[j] > 0.999);
      total += blend_weights[i][j];
    }
    if (total < 0.001) continue;
    for (unsigned int j = 0; j < num; j++) {
      setBlendWeight(i,j,blend_weights[i][j]/total);
    }
    / *
    total = 0;
    for (unsigned int j = 0; j < num; j++) {
      total += blend_weights[j];
    }
    if (fabs(total-1) > 0.0001) return 0; // ? this shouldn't be necessary
    assert (fabs(total-1) < 0.0001);
    return 1;
    * /
  }
}
*/
