#include "plane.h"
#include "utils.h"
#include <cassert>

// ===========
// CONSTRUCTOR

Plane::Plane(const Vec3f &a, const Vec3f &b, const Vec3f &c) {
  Vec3f ba = b - a;
  Vec3f ca = c - a;
  //double d = max2(ba.Length(), ca.Length());
  ba.Normalize();
  ca.Normalize();
  Vec3f::Cross3(normal,ba,ca);
  normal.Normalize();
  offset = normal.Dot3(a);
}


// =============
// ON PLANE TEST

bool Plane::onPlane(const Vec3f &v, double typical_distance) const {
  double tmp = fabs(offset - v.Dot3(normal));
  double tmp2 = fabs(0.000001*typical_distance);
  /*
  if (tmp < tmp2) {
  } else {
    printf ("  not on plane %15.12f %15.12f\n", tmp,tmp2);
  }
  */
  return (tmp < tmp2);
}

double Plane::SignedDistanceToPlane(const Vec3f &pt) const {
  return fabs(offset - pt.Dot3(normal));
}


// =====================================================================================
// =====================================================================================
// =====================================================================================

void RowSwap(double M[3][3], int i, int j);
void RowSubtract(double M[3][3], int i, int j, double scale);
void RowScale(double M[3][3], int i, double scale);

// INTERSECTIONS!

bool Plane::ComputeIntersection(Plane *p1, Plane *p2, const std::vector<Vec3f> &points, Vec3f &answer, double tolerance) const {

  Vec3f normal0 = this->getNormal();
  if (p1 != NULL) {
    Vec3f normal1 = p1->getNormal();
    double dot01 = normal0.Dot3(normal1);
    if (dot01 > tolerance) {
      p1 = NULL;
    }
  }
  if (p2 != NULL) {
    Vec3f normal2 = p2->getNormal();
    double dot02 = normal0.Dot3(normal2);
    if (dot02 > tolerance) {
      p2 = NULL;
    }
  }
  if (p1 != NULL && p2 != NULL) {
    Vec3f normal1 = p1->getNormal();
    Vec3f normal2 = p2->getNormal();
    double dot12 = normal1.Dot3(normal2);
    if (dot12 > tolerance) {
      p2 = NULL;
    }
  }

  if (p1 == NULL && p2 == NULL) {
    // return closest point on plane to faux_vert
    return ClosestPointToPlane(points, answer);
  } else if (p1 == NULL) {
    // return closest point on intersection line to faux_vert
    return ClosestPointToIntersectionLine(p2,points, answer);
  } else if (p2 == NULL) {
    // return closest point on intersection line to faux_vert
    return ClosestPointToIntersectionLine(p1,points, answer);
  } else {
    // return intersection point of 3 planes
    return IntersectPlanes(p1,p2,points,answer);
  }  
  exit(0);
}

bool Plane::ClosestPointToPlane(const std::vector<Vec3f> &points, Vec3f &answer)  const {
  Vec3f normal0 = this->getNormal();
  double d0 = this->getOffset();
  assert (points.size() > 0);
  double dot = points[0].Dot3(normal0);
  double diff = d0 - dot;
  answer = points[0] + diff * normal0;
  double dot_after = answer.Dot3(normal0);
  // HACK NOTE:  This should also be compared relative to the scale of the model...
  //printf ("%30.20f %30.20f\n", dot_after,d0);
  assert (fabs(dot_after-d0) < 0.001);
  return 1;
}


bool Plane::ClosestPointToIntersectionLine(Plane *p1, const std::vector<Vec3f> &points, Vec3f &answer) const {
  assert (p1 != NULL && this != p1);

  Vec3f normal0 = this->getNormal();
  double d0 = this->getOffset();
  Vec3f normal1 = p1->getNormal();
  double d1 = p1->getOffset();

  assert (points.size() > 0);
  Vec3f pt = points[0];
  
  // find intersection line
  Vec3f cross;  Vec3f::Cross3(cross,normal0,normal1);
  if (cross.Length() < 0.00001) {
    printf ("ClosestPointToIntersectionLine: planes are parallel\n");
    // whoops, these planes are parallel!
    
    double t1 = normal0.Dot3(pt);
    double t2 = normal1.Dot3(pt);

    // should really compare this to some epsilon!
    if (fabs(t1-t2) > 0.0001) return 0;
    
    // otherwise close enough to same plane!
    return ClosestPointToPlane(points,answer);    
  }
  cross.Normalize();

  // equation of line 
  // p = c0 normal0 + c1 normal1 + t normal0 x normal1
  double n0_dot_n0 = normal0.Dot3(normal0);
  double n0_dot_n1 = normal0.Dot3(normal1);
  double n1_dot_n1 = normal1.Dot3(normal1);
  double denom = n0_dot_n0*n1_dot_n1 - n0_dot_n1*n0_dot_n1;
  double c0 = (d0*n1_dot_n1 - d1*n0_dot_n1)/denom;
  double c1 = (d1*n0_dot_n0 - d0*n0_dot_n1)/denom;
  Vec3f tmp = c0*normal0 + c1*normal1;
  
  // find t to minimize distance to pt
  Vec3f vect = pt - tmp;
  double distance = fabs(vect.Length());
  vect.Normalize();
  double angle = AngleBetween(cross,vect);
  double t = distance*cos(angle);
  answer = tmp + t*cross;
  return 1;
}




bool Plane::IntersectPlanes(Plane *p1, Plane *p2, const std::vector<Vec3f> &points, Vec3f &answer) const {

  assert (p1 != NULL && p2 != NULL);
  assert (this != p1 && this != p2 && p1 != p2);
  
  Vec3f normal0 = this->getNormal();
  Vec3f normal1 = p1->getNormal();
  Vec3f normal2 = p2->getNormal();

  double d0 = getOffset();
  double d1 = p1->getOffset();
  double d2 = p2->getOffset();


  double M[3][3] = { { normal0.x(),normal0.y(),normal0.z() },
                     { normal1.x(),normal1.y(),normal1.z() },
                     { normal2.x(),normal2.y(),normal2.z() } };

  double I[3][3] = { { 1,0,0 }, { 0,1,0 }, { 0,0,1} };

  /*

  double dot01 = normal0.Dot3(normal1);
  double dot02 = normal0.Dot3(normal2);
  double dot12 = normal1.Dot3(normal2);


  if (fabs(dot01) > 0.9999 ||
      fabs(dot02) > 0.9999 ||
      fabs(dot12) > 0.9999) {

    normal0.Print("n0: ");
    normal1.Print("n1: ");
    normal2.Print("n2: ");
    printf("parallel planes %f %f %f\n",dot01,dot02,dot12);

    return 0;
  }
  */
      
  //PrintMat(M);
  //PrintMat(I);

  double scale;

  // pivot for column 1
  if (fabs(M[0][0]) < fabs(M[1][0])) {  RowSwap(M,0,1); RowSwap(I,0,1);  }
  if (fabs(M[0][0]) < fabs(M[2][0])) {  RowSwap(M,0,2); RowSwap(I,0,2);  }
  if (fabs(M[0][0]) < 0.0001) { 
    //printf ("whoops crappy x pivot\n");
    return 0;
  }
  
  // column 1
  scale = 1 / M[0][0];     RowScale(M,0,scale);       RowScale(I,0,scale);
  scale = M[1][0];         RowSubtract(M,1,0,scale);  RowSubtract(I,1,0,scale);
  scale = M[2][0];         RowSubtract(M,2,0,scale);  RowSubtract(I,2,0,scale);

  // pivot for column 2
  if (fabs(M[1][1]) < fabs(M[2][1])) {  RowSwap(M,1,2);  RowSwap(I,1,2); }
  if (fabs(M[1][1]) < 0.0001) { 
    //printf ("whoops crappy y pivot\n");
    return 0;
  }
  
  // column 2
  scale = 1 / M[1][1];     RowScale(M,1,scale);       RowScale(I,1,scale);
  scale = M[0][1];         RowSubtract(M,0,1,scale);  RowSubtract(I,0,1,scale);
  scale = M[2][1];         RowSubtract(M,2,1,scale);  RowSubtract(I,2,1,scale);

  if (fabs(M[2][2]) < 0.0001) { 
    //printf ("whoops crappy z pivot\n");
    return 0;
  }

  // column 3
  scale = 1 / M[2][2];     RowScale(M,2,scale);       RowScale(I,2,scale);
  scale = M[0][2];         RowSubtract(M,0,2,scale);  RowSubtract(I,0,2,scale);
  scale = M[1][2];         RowSubtract(M,1,2,scale);  RowSubtract(I,1,2,scale);

  //  PrintMat(M);
  //PrintMat(I);

  /*
  double Q[3][3] = { { normal0.x(),normal0.y(),normal0.z() },
                     { normal1.x(),normal1.y(),normal1.z() },
                     { normal2.x(),normal2.y(),normal2.z() } };
  */
  //TESTMUL(Q,I);
  //TESTMUL(I,Q);

  //printf ("testing  %f %f %f\n", d0,d1,d2);

  double x = I[0][0]*d0 + I[0][1]*d1 + I[0][2]*d2;
  double y = I[1][0]*d0 + I[1][1]*d1 + I[1][2]*d2;
  double z = I[2][0]*d0 + I[2][1]*d1 + I[2][2]*d2;

  answer = Vec3f(x,y,z);

  // make sure the point is on each plane
  double check0 = answer.Dot3(normal0)-d0;
  double check1 = answer.Dot3(normal1)-d1;
  double check2 = answer.Dot3(normal2)-d2;

  if (fabs(check0) >= 0.0001 ||
      fabs(check1) >= 0.0001 ||
      fabs(check2) >= 0.0001) {
    printf ("check error!  %f %f %f\n", check0, check1,check2); 
    assert (0);
    return 0;
  }

  return 1;
}


// =====================================================================================
// =====================================================================================
// =====================================================================================


// ====================================================================

void RowSwap(double M[3][3], int i, int j) {
  double tmp0 = M[i][0];
  double tmp1 = M[i][1];
  double tmp2 = M[i][2];
  M[i][0] = M[j][0];
  M[i][1] = M[j][1];
  M[i][2] = M[j][2];
  M[j][0] = tmp0;
  M[j][1] = tmp1;
  M[j][2] = tmp2;
}

void RowSubtract(double M[3][3], int i, int j, double scale) {
  M[i][0] -= scale * M[j][0];
  M[i][1] -= scale * M[j][1];
  M[i][2] -= scale * M[j][2];
}

void RowScale(double M[3][3], int i, double scale) {
  M[i][0] *= scale;
  M[i][1] *= scale;
  M[i][2] *= scale;
}

void PrintMat(double M[3][3]) {
  printf ("mat\n");
  printf ("  %12.8f %12.8f %12.8f\n", M[0][0],M[0][1],M[0][2]);
  printf ("  %12.8f %12.8f %12.8f\n", M[1][0],M[1][1],M[1][2]);
  printf ("  %12.8f %12.8f %12.8f\n", M[2][0],M[2][1],M[2][2]);
}

void TESTMUL(double M[3][3],double I[3][3]) {
  printf ("TESTMUL\n");
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {

      printf("  %f", 
             M[i][0]*I[0][j]+
             M[i][1]*I[1][j]+
             M[i][2]*I[2][j]);
             
    }
    printf ("\n");
  }

}

