#ifndef _CIRCLE_H_
#define _CIRCLE_H_

#include "point.h"
#include <cstdio>

#include "../common/Matrix3.h"

// ======================================================================
// CIRCLE
// ======================================================================

class Circle {
public:
  Circle(){
    xc = yc = r = 0;
  }
  Circle (double xc, double yc, double r){
    this->xc = xc;
    this->yc = yc;
    this->r = r;
  }
  Circle(std::vector<Point> &points, Point com = Point(0,0), 
         double max_inlier_thresh = 0., double min_inlier_thresh = 0.){
    if (max_inlier_thresh > 0.){
      const int steps = 10;
      fit(points, com, max_inlier_thresh, false);
      for (int i=1; i<steps; i++){
	double inlier_thresh = max_inlier_thresh - 
	  ((max_inlier_thresh - min_inlier_thresh) * i) / (steps-1);
	fit(points, com, inlier_thresh, true);
      }
    } else {
      fit(points, com, 0., false);
      fit(points, com, min_inlier_thresh, true);
    }
  }

  double point_error(Point &p){
    double rp = sqrt((p.col - xc)*(p.col - xc) + (p.row - yc)*(p.row - yc));
    return fabs(rp - r);
  }

  double point_error(std::vector<Point> &points){
    double sum = 0.;
    for (unsigned i=0; i<points.size(); i++){
      sum += point_error(points[i]);
    }
    return sum / points.size();
  }

  double get_xc(){return xc;}
  double get_yc(){return yc;}
  double get_r(){return r;}


  void set_xc(double xc){this->xc = xc;}
  void set_yc(double yc){this->yc = yc;}
  void set_r(double r){this->r = r;}

  bool inside(Point &p){
    if (r*r > ((p.col-xc)*(p.col-xc) + (p.row-yc)*(p.row-yc))){
      return true;
    } else {
      return false;
    }
  }

  // com = center of mass
  void fit(std::vector<Point> &points, Point &com, 
	   double inlier_thresh, bool init){
    double sxx = 0.;
    double sxy = 0.;
    double syy = 0.;
    double  sx = 0.;
    double  sy = 0.;
    double   n = 0.;
    double sx3 = 0.;
    double sy3 = 0.;
    double sxy2 = 0.;
    double sx2y = 0.;

    // note: points centered before fitting
    for (unsigned i=0; i<points.size(); i++){
      if (!init || point_error(points[i]) < inlier_thresh){
	double x2 = (points[i].col - com.col) * (points[i].col - com.col);
	sxx += x2;
	sx3 += x2 * (points[i].col - com.col);
	
	sxy += (points[i].col - com.col) * (points[i].row - com.row); 
	
	double y2 = (points[i].row - com.row) * (points[i].row - com.row); 
	syy += y2;
	sy3 += y2 * (points[i].row - com.row); 
	
	sxy2 += (points[i].col - com.col) * y2;
	sx2y += x2 * (points[i].row - com.row);
	
	sx += (points[i].col - com.col);
	sy += (points[i].row - com.row);
	n += 1;
      }
    }
      
    m3d A(sxx, sxy, sx,
	  sxy, syy, sy,
	  sx,   sy,  n);
    v3d b(sx3 + sxy2, sx2y + sy3, sxx + syy);
    v3d fit = -(A.inverse() * b);
    
    // note: add center of mass back to circle center
    xc = com.col - 0.5*fit.x();
    yc = com.col - 0.5*fit.y();
    r = sqrt(0.25*(fit.x()*fit.x()+fit.y()*fit.y()) - fit.z());
  }
private:
  double xc;
  double yc;
  double r;
};

#endif
