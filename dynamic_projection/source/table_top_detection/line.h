#ifndef _LINE_H_
#define _LINE_H_


#include <cstdlib>
#include "point.h"

// ======================================================================
// line
// ======================================================================

// ======================================================================

// ax + by + c = 0
// note x = col; y = row
struct Line
{
  double a;
  double b;
  double c;
  Line(){};

  Line(double a, double b, double c)
  {
    this->a = a;
    this->b = b;
    this->c = c;
    normalize();
  }

  Line(std::vector<Point> &points, double inlier_thresh = 0.){
    fit(points, inlier_thresh, false);
    if (inlier_thresh > 0.){
      fit(points, inlier_thresh, true);
    }
  }

  void refit(std::vector<Point> &points, double inlier_thresh = 0.){
    fit(points, inlier_thresh, true);
  }

  void fit (std::vector<Point> &points, double inlier_thresh = 0., bool 
            init = false){
    Vec2f m(0.0, 0.0);
    int n = 0;
    
    std::vector<Point>::iterator pt;
    for (pt = points.begin(); pt != points.end(); ++pt){
      if (!init || point_error(*pt) < inlier_thresh){
        m.x += pt->col;
        m.y += pt->row;
        n++;
      }
    }
    m.x /= double(n);
    m.y /= double(n);
    
    double sxx = 0.0;
    double sxy = 0.0;
    double syy = 0.0;
    for (pt = points.begin(); pt != points.end(); ++pt){
      if (!init || point_error(*pt) < inlier_thresh){
        sxx += (pt->col - m.x) * (pt->col - m.x);
        sxy += (pt->col - m.x) * (pt->row - m.y);
        syy += (pt->row - m.y) * (pt->row - m.y);
      }
    }
    
    Matrix2x2 cov(sxx/n, sxy/n, sxy/n, syy/n);
    Vec2f v;
    if (fabs(cov.l1()) > fabs(cov.l2())){
      v = cov.evect1(); 
    } else {
      v = cov.evect2(); 
    }
    a = -v.y;
    b = v.x;
    c = -(a * m.x + b * m.y); 
    normalize();
    //printf("%u %f %f %f\n", points.size(), a, b, c);
  }

  void normalize()
  {
    double d = sqrt(a*a+b*b);
    a /= d;
    b /= d;
    c /= d;
  }

  bool side(Point &p){
    double d = a*p.col + b*p.row + c;
    if (d < 0.){
      return true;
    } else {
      return false;
    }
  }

  // +1 one side
  // -1 other side
  //  0 to close to call (within tolerance of line)
  int side_or_line(Point &p, double tolerance = 0.){
    double d = a*p.col + b*p.row + c;
    if (d > tolerance){
      return +1;
    } else if (d < -tolerance){
      return -1;
    }
    return 0;
  }

  double point_error(Point &p){
    return  fabs(a * p.col + b * p.row + c);
  }

  double point_error(std::vector<Point> &points){
    double err_sum = 0.;
    for (unsigned i=0; i<points.size(); i++){
      err_sum += point_error(points[i]);
    }
    return err_sum / points.size();
  }

  int points_closer_than(std::vector<Point> &points, double inlier_thresh){
    int count = 0;
    for (unsigned i=0; i<points.size(); i++){
      if(point_error(points[i]) < inlier_thresh){
        count++;
      }
    }
    return count;
  }

  void dump() {printf("%g %g %g\n", a, b, c);};
};



inline Line ransac_line(std::vector<Point> &edge_points, 
		 std::vector<Point> &points){
  if (edge_points.size() < 2){
    fprintf(stderr, "too few points in ransac_line()\n");
    exit(-1);
  }

  const int maxiter = 200;
  Line best_line;
  int best_score = 0;
  for (int iter=0; iter<maxiter; iter++){

    // choose a line between two random points
    int i1, i2 = rand() % edge_points.size();
    do {
      i1 = rand() % edge_points.size();
    } while(i1 == i2);
    std::vector<Point> pts;
    pts.push_back(edge_points[i1]);
    pts.push_back(edge_points[i2]);
    Line l(pts);

    // fit line with points closer than threshold
    double inlier_thresh = 2.0;
    for (int i=0; i<3; i++){
      l.refit(edge_points, inlier_thresh);
    }

    int num_inliers = l.points_closer_than(edge_points, inlier_thresh);
    if (num_inliers < best_score){
      continue;
    }

    // reject line if most points are not on one side
    int count1 = 0;
    int count2 = 0;
    double side_tolerance = 2.5;
    for (unsigned i=0; i<points.size(); i++){
      int side = l.side_or_line(points[i], side_tolerance);
      if (side < 0){
	count1++;
      } else if (side > 0) {
	count2++;
      }
    }
    double side_thresh = 0.05;
    if (std::min(count1, count2)/double(points.size()) > side_thresh){
      continue;
    }

    best_score = num_inliers;
    best_line = l;
  }

  return best_line;
}

struct Linepair {
  Linepair(const Line &l1, const Line &l2){
    this->l1 = l1;
    this->l2 = l2;
    dot = l1.a * l2.a + l1.b * l2.b;
    Matrix2x2 m(l1.a, l1.b, l2.a, l2.b);
    Vec2f v = m.inverse() * Vec2f(-l1.c, -l2.c);  
    intersection = Point(v.y, v.x);    
  }
  Line l1;
  Line l2;
  double dot;
  Point intersection;
};

// sort predicate for linepairs - sorted by perpendicularity
inline bool linepair_by_dot(const Linepair &a, const Linepair &b){
  return fabs(a.dot) < fabs(b.dot);
}

#endif
