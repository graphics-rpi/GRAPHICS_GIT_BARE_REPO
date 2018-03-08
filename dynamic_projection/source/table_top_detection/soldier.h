#ifndef _SOLDIER_H_
#define _SOLDIER_H_

#include <cmath>
#include <vector>

#include "../common/CalibratedCamera.h"
#include "../common/Image.h"

#include "point.h"
#include "color.h"
#include "object.h"
#include "argparser.h"

extern ArgParser *args;

// ======================================================================
// SOLDIER
// ======================================================================

class Soldier : public Object {
public:

  Soldier(/*Image<byte> &component_image, */std::vector<Point> &points,
	  std::vector<Point> &edge_points, CalibratedCamera &camera, /*int mat_idx,*/ Histogram &histogram);

  void project(CalibratedCamera &camera) { }

  void write(FILE *fp){
    if (material_idx == RED_IDX) {
    fprintf(fp, "soldier %+8.4f %+8.4f %+8.4f   red\n", xc, zc, radius); 
    } else {
      assert (material_idx == GREEN_IDX);
      fprintf(fp, "soldier %+8.4f %+8.4f %+8.4f   green\n", xc, zc, radius); 
    }
  }

  void draw(Image<sRGB> &image){
    sRGB tmp;
    assert (material_idx == GREEN_IDX ||
            material_idx == RED_IDX);
    tmp = COLOR_sRGB[material_idx];


    int n_pts = 300;
    for (int i=0; i<n_pts; i++){
      double th = (2.*M_PI*i)/n_pts;
      int r = center_row + pixel_radius * cos(th);
      int c = center_col + pixel_radius * sin(th);
      if (r >= 0 && r < image.getRows() &&
	  c >= 0 && c < image.getCols()){
	image(r, c) = tmp;
      }
    }
    for(int i = -5; i < 6; i++){
      image(center_row + i, center_col + i) = tmp;
      image(center_row + i, center_col - i) = tmp;
      image(center_row - i, center_col + i) = tmp;
      image(center_row - i, center_col - i) = tmp;
    }
  }

private:
  double xc, zc;
  double height;
  double radius;
  int material_idx;
  double center_row;
  double center_col;
  double pixel_radius;
};

#endif
