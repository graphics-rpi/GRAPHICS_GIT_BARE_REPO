#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#endif
#include <sys/types.h>
//Removing to make windows compatible
//#include <unistd.h>
#include <signal.h>
//#include <ucontext.h>
//#include <sys/time.h> /* itimer */
#include <cstring> // memset
#include <cstdio>
#include <ctime>

#include "boundingbox.h"
#include "utils.h"
#include "mesh.h"
#include <cassert>

void InsertNormal(const double a[3], const double b[3], const double c[3]) {
  Vec3f _a = Vec3f(a[0],a[1],a[2]);
  Vec3f _b = Vec3f(b[0],b[1],b[2]);
  Vec3f _c = Vec3f(c[0],c[1],c[2]);
  InsertNormal(_a,_b,_c);
}

void InsertNormal(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3) {
  Vec3f v12 = p2;
  v12 -= p1;
  Vec3f v23 = p3;
  v23 -= p2;
  Vec3f normal;
  Vec3f::Cross3(normal,v12,v23);
  normal.Normalize();
  glNormal3f(normal.x(), normal.y(), normal.z());
}


bool Intersect(const Vec3f &p1, const Vec3f &v1, 
	       const Vec3f &p2, const Vec3f &v2, Vec3f &intersection) {
  // requires: the lines defined by point p1 along the ray v1
  //           and point p2 along the ray v2  intersect 
  //           (are not skew or parallel)
  // modifies: intersection
  // computes the interesection of these two lines

  //assert (fabs(v1.Length() - 1) < EPSILON);
  //  assert (fabs(v2.Length() - 1) < EPSILON);

  if ((fabs(v1.Length() - 1) > EPSILON) ||
      (fabs(v2.Length() - 1) > EPSILON)) return false;

  assert (fabs(v1.Length() - 1) < EPSILON);
  assert (fabs(v2.Length() - 1) < EPSILON);

  double angle = AngleBetweenNormalized(v1,v2);
  if (!(angle > ANGLE_EPSILON && angle < M_PI - ANGLE_EPSILON))
    return false;

  assert (angle > ANGLE_EPSILON && angle < M_PI - ANGLE_EPSILON);

  Vec3f cross; Vec3f::Cross3(cross, v1,v2);
  double l = cross.Length();

  if (!(l > 0.0001 * EPSILON))
    return false;
  assert (l > 0.0001 * EPSILON);

  double u;

  double denomx = v1.y()*v2.z() - v2.y()*v1.z();
  double denomy = v1.z()*v2.x() - v2.z()*v1.x();
  double denomz = v1.x()*v2.y() - v2.x()*v1.y();

  if (fabs(denomx) <= 0.00001 * EPSILON &&
      fabs(denomy) <= 0.00001 * EPSILON &&
      fabs(denomz) <= 0.00001 * EPSILON) {
    printf ("l %f\n",l);
    p1.Print("p1");
    p2.Print("p2");
    v1.Print("v1");
    v2.Print("v2");
    printf ("%f %f %f\n", denomx,denomy,denomz);
    return false;
  }

  if (fabs(denomx) >= fabs(denomy) && fabs(denomx) >= fabs(denomz)) {
    assert (fabs(denomx) > 0.001 * EPSILON);
    u = (v2.y()*(p1.z()-p2.z()) - v2.z()*(p1.y()-p2.y())) / denomx;
  } else if (fabs(denomy) >= fabs(denomz)) {
    if (fabs(denomy) <= 0.001 * EPSILON) return false;
    assert (fabs(denomy) > 0.001 * EPSILON);
    u = (v2.z()*(p1.x()-p2.x()) - v2.x()*(p1.z()-p2.z())) / denomy;
  } else {
    assert (fabs(denomz) > 0.001 * EPSILON);
    u = (v2.x()*(p1.y()-p2.y()) - v2.y()*(p1.x()-p2.x())) / denomz;
  }

  intersection = v1;
  intersection.Scale(u, u, u);
  intersection += p1;
  return true;
}

// return true if the segment & line intersect, false otherwise
bool LineSegmentIntersection(const Vec3f &line1, const Vec3f &line2, 
			     const Vec3f &segment1, const Vec3f &segment2, Vec3f &intersection) {
  
  Vec3f vec_line = line2 - line1; 
  Vec3f vec_segment = segment2 - segment1;
  //double line_len = vec_line.Length();
  double segment_len = vec_segment.Length();
  vec_line.Normalize();
  vec_segment.Normalize();
  
  if (vec_segment.Length() < 0.9) return false;

  // calculate intersection (this checks for parallel lines)
  bool answer = Intersect(line1,vec_line,segment1,vec_segment,intersection);
  if (!answer) return false;

  Vec3f vec_seg_1 = intersection - segment1;
  Vec3f vec_seg_2 = intersection - segment2;
  // determine fraction of distance along the segment to intersection
  double f_1 = vec_seg_1.Length() / segment_len;
  double f_2 = vec_seg_2.Length() / segment_len;
  if (f_1 >= 0.99 || f_2 >= 0.99) return false;

  // otherwise they do intersect!
  return true;
}

// return true if the segments intersect, false otherwise
bool SegmentsIntersect(const Vec3f &a1, const Vec3f &a2, 
		       const Vec3f &b1, const Vec3f &b2, Vec3f &intersection) {

  Vec3f va = a2 - a1; 
  Vec3f vb = b2 - b1; 
  double len_a = va.Length();
  double len_b = vb.Length();
  va.Normalize();
  vb.Normalize();

  //  if (va.Length() < 0.9) return false;
  //if (vb.Length() < 0.9) return false;
  
  if (len_a < 0.00000001) return false;
  if (len_b < 0.00000001) return false;

  double tolerance = min2(len_a,len_b)*0.00001;

  // check to see if any of the endpoints touch
  if (SquaredDistanceBetweenTwoPoints(a1,b1) < tolerance*tolerance ||
      SquaredDistanceBetweenTwoPoints(a1,b2) < tolerance*tolerance ||
      SquaredDistanceBetweenTwoPoints(a2,b1) < tolerance*tolerance ||
      SquaredDistanceBetweenTwoPoints(a2,b2) < tolerance*tolerance) {
    return false;
  }

  // calculate intersection (this checks for parallel lines)
  bool answer = Intersect(a1,va,b1,vb,intersection);
  if (!answer) return false;

  Vec3f v_a = a2 - a1;
  Vec3f v_b = b2 - b1;
  Vec3f v_ai = intersection - a1;
  Vec3f v_bi = intersection - b1;

  // determine fraction of distance along the segment to intersection
  double f_a = v_ai.Length() / v_a.Length();
  double f_b = v_bi.Length() / v_b.Length();
  if (f_a >= 1.0 || f_b >= 1.0) return false;

  /*
  v_a.Normalize();
  v_b.Normalize();
  v_ai.Normalize();
  v_bi.Normalize();
  */

  // make sure it's on the segment... (not the wrong side)
  double dot_a = v_ai.Dot3(v_a);
  double dot_b = v_bi.Dot3(v_b);
  //if (dot_a < 0.9 || dot_b < 0.9) return false;
  if (dot_a < 0 || dot_b < 0) return false;

  // otherwise they do intersect!
  return true;
}


bool SphereIntersection(const Vec3f &m, double r, const Vec3f &org,
			const Vec3f &dir, double &k) {
  double p,q,d;
  Vec3f pm;
  Vec3f::Sub(pm, m,org);
  d = dir.Dot3(dir);//SqrLength(dir);
  p = pm.Dot3(dir) / d;
  q = (pm.Dot3(pm)/*(SqrLength(pm)*/ - r*r) / d;
  d = p*p - q;
  if (d < 0) return false;
  k = p + sqrt(d);
  return true;
}


// ------------------------------------------------------------

double DistanceToPlane(const Vec3f &p, 
		      const Vec3f &t1, const Vec3f &t2, const Vec3f &t3, 
		      Vec3f &pt) {
  // compute the triangle face normal
  Vec3f n;
  computeNormal(t1,t2,t3,n);
  double d = p.Dot3(n) - t1.Dot3(n);
  pt = n; pt *= (double)d; pt += p;
  return d; 
}

// ------------------------------------------------------------

static int WithinTriangleBorder(const Vec3f &pt,const Vec3f &v0,const Vec3f &v1,const Vec3f &v2) {

  Vec3f n;
  computeNormal(v0,v1,v2,n);
  if (n.Length() < 0.9) {
    //printf ("\nBAD NORMAL!\n");
    return 0;
  }

  Vec3f v01 = v1 - v0;
  Vec3f v12 = v2 - v1;
  Vec3f v20 = v0 - v2;
  Vec3f v0pt = pt - v0;
  Vec3f v1pt = pt - v1;
  Vec3f v2pt = pt - v2;
  Vec3f cp0; Vec3f::Cross3(cp0,v01,v1pt);
  Vec3f cp1; Vec3f::Cross3(cp1,v12,v2pt);
  Vec3f cp2; Vec3f::Cross3(cp2,v20,v0pt);
  double dp0 = cp0.Dot3(n);
  double dp1 = cp1.Dot3(n);
  double dp2 = cp2.Dot3(n);
#if 0
  int in0 = dp0 >= 0;
  int in1 = dp1 >= 0;
  int in2 = dp2 >= 0;
#else
  int in0 = dp0 >= EPSILON;
  int in1 = dp1 >= EPSILON;
  int in2 = dp2 >= EPSILON;
#endif

  if (in0 && in1 && in2) return 1;
  return 0;
}

// ------------------------------------------------------------


double PerpendicularDistanceToLine(const Vec3f &pt, const Vec3f &v0, const Vec3f &v1){
  /*          pt
   *      ^    o
   *     /    /| \
   *    /    / |   \ 
   *   b    /  |     \ 
   *       /   |       \
   *  v0  o---pp---------o  v1
   *            
   *         a ---->
   */

  //  cout << endl << endl;
  Vec3f a = v1 - v0;
  Vec3f b = pt - v0;

  /*
  v1.Print("v1");
  v0.Print("v0");
  pt.Print("pt");
  */
    
  double numer = a.Dot3(b);
  double denom = a.Dot3(a);
  if (denom <= 0.000001 * numer) {//denom <= 0.001 * EPSILON) {
    std::cout << "numer " << numer << "   denom " << denom << std::endl;
    std::cout << " copping out" << std::endl;
    return DistanceBetweenTwoPoints(pt,v0);
  }
  assert (denom > 0.0000001 * numer); //0.001 * EPSILON);
  //if (denom < EPSILON) return -1;
  double proj = numer/denom;
  //if (proj <= 0.001 || proj >= 0.999) return -1;
  Vec3f pp = a; pp *= proj; pp += v0;
  double d = DistanceBetweenTwoPoints(pp,pt);
  return d;
}

Vec3f ProjectToSegment(const Vec3f &pt, const Vec3f &v0, const Vec3f &v1) {
  // return -1 if pt projected perpendicularly to line does not
  // lie on the segment from l1 to l2, otherwise return distance to line

  /*          pt
   *      ^    o
   *     /    /| \
   *    /    / |   \ 
   *   b    /  |     \ 
   *       /   |       \
   *  v0  o---pp---------o  v1
   *            
   *         a ---->
   */

  Vec3f a = v1 - v0;
  Vec3f b = pt - v0;
	    
  double numer = a.Dot3(b);
  double denom = a.Dot3(a);
  assert (denom > EPSILON);
  double proj = numer/denom;
  assert (proj > 0.001 || proj < 0.999);
  Vec3f pp = a; pp *= proj; pp += v0;
  return pp;
}






double PerpendicularDistanceToLineSegment(const Vec3f &pt, const Vec3f &v0, const Vec3f &v1){
  // return -1 if pt projected perpendicularly to line does not
  // lie on the segment from l1 to l2, otherwise return distance to line

  /*          pt
   *      ^    o
   *     /    /| \
   *    /    / |   \ 
   *   b    /  |     \ 
   *       /   |       \
   *  v0  o---pp---------o  v1
   *            
   *         a ---->
   */

  Vec3f a = v1 - v0;
  Vec3f b = pt - v0;
	    
  double numer = a.Dot3(b);
  double denom = a.Dot3(a);
  if (denom < EPSILON) return -1;
  double proj = numer/denom;
  if (proj <= 0.001 || proj >= 0.999) return -1;
  Vec3f pp = a; pp *= proj; pp += v0;
  double d = DistanceBetweenTwoPoints(pp,pt);
  return d;
}




double AreaOfTriangle(double a, double b, double c) {
  // Area of Triangle =  (using Heron's Formula)
  //  sqrt[s*(s-a)*(s-b)*(s-c)]
  //    where s = (a+b+c)/2
  // also... Area of Triangle = 0.5 * x * c
  assert(!my_isnan(a));
  assert(!my_isnan(b));
  assert(!my_isnan(c));
  double s = (a+b+c) / (double)2;
  assert(!my_isnan(s));
  double tmp = s*(s-a)*(s-b)*(s-c);
  if (tmp < 0) return 0;
  double answer = sqrt(tmp);
  assert(!my_isnan(answer));
  return answer;
}

double AreaOfTriangle2(double a, double b, double c) {
  // Area of Triangle = 
  // 1/4 * sqrt[(a+b+c)*(b+c-a)*(c+a-b)*(a+b-c)]
  assert(!my_isnan(a));
  assert(!my_isnan(b));
  assert(!my_isnan(c));
  double tmp = (a+b+c)*(b+c-a)*(c+a-b)*(a+b-c);
  assert(!my_isnan(tmp));
  if (tmp < 0) return 0;
  double answer = 0.25*sqrt(tmp);
  assert(!my_isnan(answer));
  return answer;
}


double AreaOfTriangle(const Vec3f &a, const Vec3f &b, const Vec3f &c) {
  assert(!a.isNaN());
  assert(!b.isNaN());
  assert(!c.isNaN());
  double aside = DistanceBetweenTwoPoints(a,b);
  double bside = DistanceBetweenTwoPoints(b,c);
  double cside = DistanceBetweenTwoPoints(c,a);
  assert(!my_isnan(aside));
  assert(!my_isnan(bside));
  assert(!my_isnan(cside));
  return AreaOfTriangle2(aside,bside,cside);
}

// ------------------------------------------------------------

//enum SIDE EdgeSide(const Vec3f &pt, Triangle *t, int v1, int v2, Mesh *base_mesh);
//enum SIDE VertexSide(const Vec3f &pt, Triangle *t, int v, TriangleMesh *base_mesh);


double DistanceBetweenPointAndTriangle
(const Vec3f &p, const Vec3f &v0, const Vec3f &v1, const Vec3f &v2) {

#if 1
  // find closest point in the plane of the triangle, if it's within
  // the borders of the triangle return the distance to that point
  Vec3f pt;
  double distToPlane = DistanceToPlane(p,v0,v1,v2,pt);
  if (WithinTriangleBorder(pt,v0,v1,v2)) {
    //return 0;
    return distToPlane;
  }
#endif

#if 1
  // find closest point on each line of each edge, return the distance
  // to closest of these that lies ON the edge (if any)
  double answer = -1;
  double d01 = PerpendicularDistanceToLineSegment(p,v0,v1);
  double d12 = PerpendicularDistanceToLineSegment(p,v1,v2);
  double d20 = PerpendicularDistanceToLineSegment(p,v2,v0);
  if (d01 >= 0)
    answer = d01;
  if (d12 >= 0 && (answer < 0 || d12 < answer)) 
    answer = d12;
  if (d20 >= 0 && (answer < 0 || d20 < answer))
    answer = d20;
  if (answer >= 0) return answer;
#endif

#if 1
  // check all the vertices
  double d0 = DistanceBetweenTwoPoints(p,v0);
  double d1 = DistanceBetweenTwoPoints(p,v1);
  double d2 = DistanceBetweenTwoPoints(p,v2);
  assert (d0 >= 0 && d1 >= 0 && d2 >= 0);
  if (d0 >= 0)
    answer = d0;
  if (d1 >= 0 && (answer < 0 || d1 < answer))
    answer = d1;
  if (d2 >= 0 && (answer < 0 || d2 < answer))
    answer = d2;
#endif
  
  assert (answer >= 0);
  return answer;
}

// ------------------------------------------------------------

double IntersectSphereWithRay(const Vec3f &p, double radius, 
			     const Vec3f &p1, const Vec3f &p2) {

  double x1 = p1.x();  double x2 = p2.x();  double x3 = p.x();
  double y1 = p1.y();  double y2 = p2.y();  double y3 = p.y();
  double z1 = p1.z();  double z2 = p2.z();  double z3 = p.z();

  double a = square(x2 - x1) + square(y2 - y1) + square(z2 - z1);
  double b = 2 * ( (x2 - x1)*(x1 - x3) + 
		  (y2 - y1)*(y1 - y3) + 
		  (z2 - z1)*(z1 - z3) ); 
  double c = square(x3) + square(y3) + square(z3) + 
    square(x1) + square(y1) + square(z1) - 
    2*(x3*x1 + y3*y1 + z3*z1) - square(radius);
  
  double radical = b*b - 4*a*c;
  if (radical < EPSILON) return -1;

  radical = sqrt(radical);
  double u1 = (-b + radical) / (double)(2*a);
  double u2 = (-b - radical) / (double)(2*a);

  if (fabs(u1-0.5) < fabs(u2-0.5))
    return u1;
  return u2;
}

// ------------------------------------------------------------

void SolveCubic(double b, double c, double d, double *x1, double *x2, double *x3)
{
/* solves x^3 + b*x^2 + c*x + d = 0, returns real part of solutions */
  double e,f,g,u,s, sq, cosf;
  double r,rx,phi, x,y;
  
  u = -b/3;
  e = 3*u*u + 2*u*b + c;
  f = u*u*u + u*u*b + u*c + d;
  s = -e/3;
  g = -e*e*e/27;

  sq = f*f - 4*g;
  if (sq >= 0) {  
    sq = sqrt(sq);
    r = (-f + sq)/2; phi = 0.0;
    if (r < 0) { r = -r; phi = M_PI; }
    if (r > EPSILON) r = exp(1.0/3*log(r));
  }
  else {
    sq = sqrt(-sq);
    x = -f/2; y = sq/2;
    r = sqrt(x*x + y*y);
    if (r < EPSILON) { r = 0.0; phi = 0.0; }
    else { 
      cosf = x/r; 
      if (cosf > 1.0) cosf = 1.0;
      if (cosf < -1.0) cosf = -1.0;
      phi = acos(cosf)/3; r = exp(1.0/3*log(r)); 
    }
  }  
  if (r > EPSILON) rx = r + s/r; else rx = 0.0;
  *x1 = rx*cos(phi) + u;
  *x2 = rx*cos(phi + 2.0*M_PI/3.0) + u;
  *x3 = rx*cos(phi + 4.0*M_PI/3.0) + u;
/* the y's are needed if the solutions are complex
   not needed here because the Eigenvalues of a symmetric matix 
   are always real
  if (r > EPSILON) ry = r - s/r; else ry = 0.0;
  *y1 = ry*sin(phi);
  *y2 = ry*sin(phi + 2.0*M_PI/3.0);
  *y3 = ry*sin(phi + 4.0*M_PI/3.0);
*/  
}

// ------------------------------------------------------------


void LargestEigenvalue(double *tensor, double *lambda, Vec3f &v)
{
/* tensor is double[6], the six independend components of a
   symmetric 3x3 tensor.
   The proc returns lambda, the largest eigenvalue and
   a corresponing eigenvector v */
  double inv1,inv2,inv3;
  double a[3][3];
  double x[3];
  double l1,l2,l3,l;
  int i,j, i0,i1, j0,j1;
  int mi0 = 0; int mi1 = 0; int mj0 = 0; int mj1 = 0; int mj2 = 0;
  double det,d0,d1, max, s;
  
  s = tensor[0] + tensor[1] + tensor[2] + tensor[3] + tensor[4] + tensor[5];
  if (fabs(s) < 1.0) s = 1.0;
  else s = 1.0/s;
  if (s < 0.0) s = -s;
  
  a[0][0] = s*tensor[0]; a[0][1] = s*tensor[3]; a[0][2] = s*tensor[5];  
  a[1][0] = s*tensor[3]; a[1][1] = s*tensor[1]; a[1][2] = s*tensor[4];  
  a[2][0] = s*tensor[5]; a[2][1] = s*tensor[4]; a[2][2] = s*tensor[2];  

  inv1 = a[0][0] + a[1][1] + a[2][2];
  inv2 = a[0][0]*a[1][1]-a[0][1]*a[1][0] + 
         a[0][0]*a[2][2]-a[0][2]*a[2][0] + 
         a[1][1]*a[2][2]-a[1][2]*a[2][1];
  inv3 = a[0][0]*a[1][1]*a[2][2] + a[0][1]*a[1][2]*a[2][0] + a[0][2]*a[1][0]*a[2][1] 
        -a[0][0]*a[1][2]*a[2][1] - a[0][1]*a[1][0]*a[2][2] - a[0][2]*a[1][1]*a[2][0];

  SolveCubic(-inv1,inv2,-inv3, &l1,&l2,&l3); 

//  if (fabs(l1) > fabs(l2)) l = l1; else l = l2;
//  if (fabs(l3) > fabs(l)) l = l3;
  if (l1 > l2) l = l1; else l = l2;  /* tension only */
  if (l3 > l) l = l3;
  
  
  a[0][0] -= l; a[1][1] -= l; a[2][2] -= l;
  *lambda = l/s;
  
  max = 0.0;
  i0 = 1; i1 = 2; 
  for (i = 0 ; i < 3; i++) {
    if (i == 1) i0--; if (i == 2) i1--;
    j0 = 1; j1 = 2;
    for (j = 0; j < 3; j++) {
      if (j == 1) j0--; if (j == 2) j1--;
      det = fabs(a[i0][j0]*a[i1][j1] - a[i0][j1]*a[i1][j0]);
      if (det > max) {
        max = det; 
        mi0 = i0; mi1 = i1;
        mj0 = j0; mj1 = j1; mj2 = 3-j0-j1;
      }
    }
  }
  if (max > EPSILON) {	/* single eigenvalue */
    x[mj2] = -1.0;
    det = a[mi0][mj0]*a[mi1][mj1] - a[mi0][mj1]*a[mi1][mj0];
    d0  = a[mi0][mj2]*a[mi1][mj1] - a[mi0][mj1]*a[mi1][mj2];
    d1  = a[mi0][mj0]*a[mi1][mj2] - a[mi0][mj2]*a[mi1][mj0];
    x[mj0] = d0/det;
    x[mj1] = d1/det;
  }
  else {	
    max = 0.0;
    for (i = 0; i < 3; i++) {
      for (j = 0; j < 3; j++) {
        if (fabs(a[i][j]) > max) {
          mi0 = i; mj0 = j; max = fabs(a[i][j]);
        }
      }  
    }
    if (max > EPSILON) { /* double eigenvalue */
      mj1 = mj0+1; if (mj1 > 2) mj1 = 0; mj2 = 3-mj0-mj1;
      x[mj1] = -1.0;
      x[mj0] = a[mi0][mj1]/a[mi0][mj0];
      x[mj2] = 0.0;
    }
    else {  /* triple eigenvalue */
      x[0] = 1.0; x[1] = 0.0; x[2] = 0.0;
    }    
  }
  v = Vec3f(x[0],x[1],x[2]);
  v.Normalize();
}


// =======================================================================
/*
#define BOUNDS_EPSILON 1.0e-9

int boundsOverlap(const Bounds3d &b1, const Bounds3d &b2) {
  if (b1.x1() - BOUNDS_EPSILON > b2.x2() ||
      b2.x1() - BOUNDS_EPSILON > b1.x2() ||      
      b1.y1() - BOUNDS_EPSILON > b2.y2() ||
      b2.y1() - BOUNDS_EPSILON > b1.y2() ||      
      b1.z1() - BOUNDS_EPSILON > b2.z2() ||
      b2.z1() - BOUNDS_EPSILON > b1.z2())
    return 0;
  return 1;
}

int boundsInside(const Bounds3d &small, const Bounds3d &large) {
  if (small.x1() + BOUNDS_EPSILON >= large.x1() &&
      small.x2() - BOUNDS_EPSILON <= large.x2() &&
      small.y1() + BOUNDS_EPSILON >= large.y1() &&
      small.y2() - BOUNDS_EPSILON <= large.y2() &&
      small.z1() + BOUNDS_EPSILON >= large.z1() &&
      small.z2() - BOUNDS_EPSILON <= large.z2())
    return 1;
  return 0;
}

void GrowBounds(Bounds3d &b, double grow) {
  // increase the bounds in each dimension by 
  // grow * the length of the longest spanning dimension
  grow *= max3(b.dx(),b.dy(),b.dz());
  assert (grow >= 0);
  Vec3f min = Vec3f(b.x1()-grow, b.y1()-grow, b.z1()-grow);
  Vec3f max = Vec3f(b.x2()+grow, b.y2()+grow, b.z2()+grow);
  b.IncludePoint(min);
  b.IncludePoint(max);
}
*/

// =======================================================================

// NOTE:  See also LARGE_PRIME 's in utils.h

/*
// a list of primes...  roughly 2 times as big as previous
//  chosen randomishly from a web listing
static  unsigned int  primes[] = {
        11, 
        37, 
        79, 
       127, 
       239, 
       421, 
      1021, 
      2383, 
      5749, 
      7127, 
     10079, 
     13627, 
     16007, 
     21163, 
     46307, 
     78191, 
    100459,
    213977, 
    453137, 
   1299827, 
   2599829, 
   5399833, 
  11099833, 
  24099857,
  52099877,
 100000000};

#define MAX_PRIME 100000000


static int NextLargestPrime(unsigned int x) {
  if (x > MAX_PRIME) {
    printf("ERROR! requested size (%d) for hash table is too large (max %d)\n",
	   x, MAX_PRIME);
    return x;
  }
  int i = 0;
  while (x > primes[i]) i++;
  return primes[i];
}
*/

