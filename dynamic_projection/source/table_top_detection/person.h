#ifndef _PERSON_H_
#define _PERSON_H_

#include <cmath>
#include "../common/CalibratedCamera.h"
#include "wall.h"
#include "object.h"
#include "argparser.h"

extern ArgParser *args;

#define INCHES_IN_METER 39.3701

// ======================================================================
// PERSON
// ======================================================================

class Person : public Object {
public:
  Person(Image<byte> &component_image, std::vector<Point> &points,
	 std::vector<Point> &edge_points, CalibratedCamera &camera, Histogram &histogram){
    // find dominant color and height

    material_idx = histogram.peak_idx();
    
    if (!args->find_architectural_design) { throw -1; }
    if (material_idx != LIME_IDX) { throw -1; }
    
    height = 2.7 / (double)INCHES_IN_METER;

    // find centroid2 (world space)
    v3d centroid2(0., 0., 0.);
    v3d face2(0., 0., 0.);
    int face_count = 0;
    for (unsigned i=0; i<points.size(); i++){
      v3d tmp = camera.PointFromPixel(int(points[i].row),
				      int(points[i].col),
				      height);
      centroid2 += tmp;

      int color_idx = component_image(int(points[i].row), int(points[i].col));
      if (color_idx == BROWN_IDX) {
	face2 += tmp;
	face_count++;
      }      
    }
    centroid2 = centroid2 / double(points.size());
    xc = centroid2.x();
    zc = centroid2.z();

    if (face_count == 0) throw -1;
    face2 = face2 / double(face_count);

    // estimate rough radius (world-space coords)
    double r_sum = 0.;
    for (unsigned i=0; i<edge_points.size(); i++){
      v3d p =  camera.PointFromPixel(int(edge_points[i].row),
				     int(edge_points[i].col),
				     height);
      double r = (centroid2 - p).length();
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
      double r = (centroid2 - p).length();
      if (fabs(r - radius) < inlier_thresh){
	r_sum += r;
	count++;
      }
    }
    radius = r_sum / count;
    
    



    // check for proper size 7/8" diameter = 0.0111 m radius
    //  but some are actually 1" diameter, so make tolerance larger
    double tol = 0.010;  //  +/-20% tolerance
    double upper_thresh = 0.0111 + tol;
    double lower_thresh = 0.0111 - tol;
    //    printf ("person_radius = %f (%f - %f)\n", 
    //	    radius, lower_thresh, upper_thresh);
    if (radius < lower_thresh || radius > upper_thresh){
      throw -1;
      //valid = false;
    }

    // estimate image-space center and radius for overlay image
    v3d p = camera.PixelFromPoint(centroid2);
    v3d p2 = camera.PixelFromPoint(face2);

    v3d delta = p2-p;
    p2 = p + 3.0*delta;

    centroid = Point(p.y(),p.x());
    face = Point(p2.y(),p2.x());
    //center_row = p.y();
    //center_col = p.x();
    v3d p1 = camera.PixelFromPoint(centroid2 + v3d(radius, 0., 0.));
    pixel_radius = (p - p1).length();



    direction = atan2(centroid.row - face.row,
		      centroid.col - face.col);
    
    

  }

  void project(CalibratedCamera &camera) {}

  void write(FILE *fp){
    fprintf(fp, "person %+8.4f %+8.4f %+8.4f %+8.4f\n",
	    xc, zc, direction, height); //, material_idx);
  }

  void draw(Image<sRGB> &image){
    int n_pts = 300;
    for (int i=0; i<n_pts; i++){
      double th = (2.*M_PI*i)/n_pts;
      int r = centroid.row + pixel_radius * cos(th);
      int c = centroid.col + pixel_radius * sin(th);
      if (r >= 0 && r < image.getRows() &&
	  c >= 0 && c < image.getCols()){
	image(r, c) = sRGB(0, 255, 0);
      }
    }
    //PixelSetter<sRGB> setPixel(sRGB(0, 255, 0));
    
    //image.applyFunctorOnLine(setPixel, 
    image.setLinePixels(
			     int(centroid.row),
			     int(centroid.col),
			     int(face.row),
			     int(face.col),

			     sRGB(0,255,0)
			);
    
  }

  //bool valid;
private:
  double xc, zc;
  double height;
  double direction;
  double radius;
  int material_idx;
  // image space coordintes:
  //double center_row;
  //double center_col;
  
  Point centroid;
  Point face;
  double pixel_radius;
};

#endif
