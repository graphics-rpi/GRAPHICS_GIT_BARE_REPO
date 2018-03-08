/*
 * BoundingBox2f.h
 *
 *  Created on: Aug 9, 2011
 *      Author: drillprp
 */

#ifndef BOUNDINGBOX2F_H_
#define BOUNDINGBOX2F_H_

#include "../paint/gl_includes.h"


#include <iostream>
//#include "../../../../synenv/visualizations/MapView/realtimedebug/Debug.h"
#include "../../calibration/planar_interpolation_calibration/planar_calibration.h"


class BoundingBox2f {
  
public:

  // CONSTRUCTORS
  BoundingBox2f() { is_initialized = false; }
  BoundingBox2f(Pt t1, Pt t2);

  // ACCESSORS
  Pt GetCenter() const;
  double Distance(const BoundingBox2f &box) const;
  bool Overlap(const BoundingBox2f & other) const;
  bool PointInside(const Pt & pt) const;
  Pt GetMin() const { assert (isInitialized()); return min; }
  Pt GetMax() const { assert (isInitialized()); return max; }
  double Radius() const { 
    assert (isInitialized());
    double dx = min.x-max.x;
    double dy = min.y-max.y;
    return 0.5*sqrt(dx*dx + dy*dy); 
  }
  bool isInitialized() const {
    if (!is_initialized) return false;
    assert (min.x <= max.x);
    assert (min.y <= max.y);
    return true;
  }

  // MODIFIERS
  void Extend(Pt p);
 

  void DrawBB() const;
  
  friend std::ostream& operator<< (std::ostream &ostr, const BoundingBox2f &b) {
    assert (b.isInitialized());
    ostr << "BoundingBox2f { " << b.min << "     " << b.max << "} ";
    fflush(stdout);
    return ostr;
  }
  
private:
  bool is_initialized;
  Pt min, max;
  
};


#endif /* BOUNDINGBOX2F_H_ */
