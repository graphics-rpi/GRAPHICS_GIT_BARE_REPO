#ifndef _QUAD_H_
#define _QUAD_H_

#include <cmath>
#include "../common/CalibratedCamera.h"
#include "../common/Image.h"
#include "point.h"
#include "line.h"
#include <cmath>

// ======================================================================
// QUAD
// ======================================================================

class Quad {
public:
  Point corner_points[4];
  Line edges[4];
  int edge_side[4];
  bool valid;
  v3d world_points[4];

  std::vector<int> edge_line_ids;
  std::vector<Point> e_points;

  Quad(){}
  Quad(std::vector<Point> &edge_points, std::vector<Point> &points);



  void draw(Image<sRGB> &image);

  void project(CalibratedCamera &camera){
    double height = 0.;
    for (int i=0; i<4; i++){
      world_points[i] = camera.PointFromPixel(corner_points[i].row,
                                              corner_points[i].col,
                                              height);
    }    
  }

  void write(FILE *fp){
    for (int i=0; i<4; i++){
      fprintf(fp, "%+8.4f %+8.4f ",
              world_points[i].x(),  
              world_points[i].z());
    }
    fprintf(fp, "\n");
  }

  bool inside(Point &p){
    for (int i=0; i<4; i++){
      if (edges[i].side_or_line(p) != edge_side[i]){
        return false;
      }
    }
    return true;
  }

  int num_inside(std::vector<Point> &points){
    int count = 0;
    for (unsigned i=0; i<points.size(); i++){
      if (inside(points[i])){
        count++;
      }
    }
    return count;
  }

  double edge_length(int i) const {
    int j = (i+1)%4;
    return corner_points[i].distance(corner_points[j]);
  }


  Point getCenter() const {
    return 0.25 * (corner_points[0] + 
                   corner_points[1] + 
                   corner_points[2] + 
                   corner_points[3]);
                   
  }

  double angle(int j) const {
    // (that is, the angle between edges ij and jk)
    int i = (j+3)%4;
    int k = (j+1)%4;

    Vec2f ij(corner_points[j].row-corner_points[i].row,
	     corner_points[j].col-corner_points[i].col);
    Vec2f jk(corner_points[j].row-corner_points[k].row,
	     corner_points[j].col-corner_points[k].col);
    ij.normalize();
    jk.normalize();
    double dot_product = ij.x*jk.x + ij.y*jk.y;
    if (dot_product <= -1 || dot_product >= 1)
      return 0;

    //	assert (dot_product >= -1 && dot_product <= 1);
    double angle_in_radians = acosf(dot_product);
    assert (angle_in_radians >= 0 && angle_in_radians <= M_PI);
    // convert to degrees
    return angle_in_radians * 180 / M_PI;
  }
};


/*
// estimate pixel error in fitting a quadrilateral
unsigned quad_consistancy(std::vector<point> &edge_points){
  std::vector<Vec2f> edgeNorms(edge_points.size());
  // compute edgel gradients
  
}
*/

#endif
