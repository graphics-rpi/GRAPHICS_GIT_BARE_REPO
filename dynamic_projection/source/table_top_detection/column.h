#ifndef _COLUMN_H_
#define _COLUMN_H_

#include <cmath>
#include "../common/CalibratedCamera.h"
#include "wall.h"
#include "object.h"

// ======================================================================
// COLUMN
// ======================================================================

class Column : public Object {
public:
  Column(std::vector<Point> &points, std::vector<Point> &edge_points, CalibratedCamera &camera, Histogram &histogram){

    material_idx = histogram.peak_idx();
    
    if (material_idx != RED_IDX && material_idx != BLUE_IDX) { throw -1; }

    height = wall_idx_to_height(material_idx);
    if (height == 0.){
      throw -1;
    }

    // find centroid (world space)
    v3d centroid(0., 0., 0.);
    for (unsigned i=0; i<points.size(); i++){
      centroid += camera.PointFromPixel(int(points[i].row),
					int(points[i].col),
					height);
    }
    centroid = centroid / double(points.size());
    xc = centroid.x();
    zc = centroid.z();

    // estimate rough radius (world-space coords)
    double r_sum = 0.;
    for (unsigned i=0; i<edge_points.size(); i++){
      v3d p =  camera.PointFromPixel(int(edge_points[i].row),
				     int(edge_points[i].col),
				     height);
      double r = (centroid - p).length();
      r_sum += r;
    }
    radius = r_sum / edge_points.size();

    // re-estimate radius, ignoring outliers
    double inlier_thresh = 1.5;
    r_sum = 0.;
    int count = 0;
    for (unsigned i=0; i<edge_points.size(); i++){
      v3d p =  camera.PointFromPixel(int(edge_points[i].row),
				     int(edge_points[i].col),
				     height);
      double r = (centroid - p).length();
      if (fabs(r - radius) < inlier_thresh){
	r_sum += r;
	count++;
      }
    }
    radius = r_sum / count;

    // check for proper size 7/8" diameter = 0.0111 m radius
    //  but some are actually 1" diameter, so make tolerance larger
    //double tol = 0.005;  //  +/-20% tolerance

    //double tol = 0.0025;  //  +/-10% tolerance

    double thresh;
    if (material_idx != RED_IDX) {  thresh = 0.0127; }
    else { thresh = 0.0111; }

    double upper_thresh = thresh * 1.2;
    double lower_thresh = thresh * 0.8;
    //    printf ("column_radius = %f (%f - %f)\n", 
    //	    radius, lower_thresh, upper_thresh);
    if (radius < lower_thresh || radius > upper_thresh){
      throw -1;
      //valid = false;
    }

    // estimate image-space center and radius for overlay image
    v3d p = camera.PixelFromPoint(centroid);
    center_row = p.y();
    center_col = p.x();
    v3d p1 = camera.PixelFromPoint(centroid + v3d(radius, 0., 0.));
    pixel_radius = (p - p1).length();
    (*args->output) << "FOUND COLUMN" << std::endl;
  }

  void project(CalibratedCamera &camera) {}

  void write(FILE *fp){
    fprintf(fp, "column %+8.4f %+8.4f %+8.4f %+8.4f\n",
	    xc, zc, radius, height); //, material_idx);
  }

  void draw(Image<sRGB> &image){
    sRGB tmp;
    tmp = COLOR_sRGB[material_idx];
    int n_pts = 300;
    for (int i=0; i<n_pts; i++){
      double th = (2.*M_PI*i)/n_pts;
      int r = center_row + pixel_radius * cos(th);
      int c = center_col + pixel_radius * sin(th);
      if (r >= 0 && r < image.getRows() &&
	  c >= 0 && c < image.getCols()){
	image(r, c) = tmp; //sRGB(255, 255, 255);
      }
    }
  }

  //Bool;
private:
  double xc, zc;
  double height;
  double radius;
  // image space coordintes:
  int material_idx;
  double center_row;
  double center_col;
  double pixel_radius;
};

#endif
