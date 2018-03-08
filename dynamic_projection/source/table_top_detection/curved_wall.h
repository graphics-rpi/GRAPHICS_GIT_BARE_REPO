#ifndef _CURVED_WALL_H_
#define _CURVED_WALL_H_

#include <cmath>
#include "circle.h"
#include "object.h"
#include "histogram.h"

// ======================================================================
// CURVED WALL
// ======================================================================

class CurvedWall : public Object {
public:

  CurvedWall(std::vector<Point> &edge_points, 
             std::vector<Point> &points, 
             Image<byte> &component_image,
             Circle init_circle, Histogram &histogram);

  // closest distance to any point on the curved wall
  double point_distance(Point p){
    double closest = std::numeric_limits<double>::max();
    
    double x, y, r, dist;
    
    // check outer circle ends
    // point 1
    x = centercircle.get_xc() + outercircle.get_r()*cos(min_angle);
    y = centercircle.get_yc() + outercircle.get_r()*sin(min_angle);
    dist = p.distance(Point(y, x));
    if (dist < closest) closest = dist;

    // point 2
    x = centercircle.get_xc() + outercircle.get_r()*cos(max_angle);
    y = centercircle.get_yc() + outercircle.get_r()*sin(max_angle);
    dist = p.distance(Point(y, x));
    if (dist < closest) closest = dist;

    // check outer circle
    r = sqrt((p.col - centercircle.get_xc()) * 
             (p.col - centercircle.get_xc()) +
             (p.row - centercircle.get_yc()) *
             (p.row - centercircle.get_yc()));
    dist = fabs(r - outercircle.get_r());
    if (dist < closest){
      // also must be on the section of arc
      double th = atan2(p.row - centercircle.get_yc(), 
                        p.col - centercircle.get_xc());
      if (th < 0.) th += 2*M_PI;
      if (th >= min_angle && th <= max_angle){
        closest = dist;
      }
    }

    // check inner circle ends
    // point 1
    x = centercircle.get_xc() + innercircle.get_r()*cos(min_angle);
    y = centercircle.get_yc() + innercircle.get_r()*sin(min_angle);
    dist = p.distance(Point(y, x));
    if (dist < closest) closest = dist;

    // point 2
    x = centercircle.get_xc() + innercircle.get_r()*cos(max_angle);
    y = centercircle.get_yc() + innercircle.get_r()*sin(max_angle);
    dist = p.distance(Point(y, x));
    if (dist < closest) closest = dist;

    // check inner circle
    r = sqrt((p.col - centercircle.get_xc()) * 
             (p.col - centercircle.get_xc()) +
             (p.row - centercircle.get_yc()) *
             (p.row - centercircle.get_yc()));
    dist = fabs(r - innercircle.get_r());
    if (dist < closest){
      // also must be on the section of arc
      double th = atan2(p.row - centercircle.get_yc(), 
                        p.col - centercircle.get_xc());
      if (th < 0.) th += 2*M_PI;
      if (th >= min_angle && th <= max_angle){
        closest = dist;
      }
    }

    return closest;
  }

  void draw(Image<sRGB> &image){
sRGB tmp;
tmp = COLOR_sRGB[material_idx];

    for (int i=0; i<200; i++){
      double th = min_angle + (max_angle-min_angle)*i/200.;
      int row = int(centercircle.get_yc() + centercircle.get_r()*sin(th));
      int col = int(centercircle.get_xc() + centercircle.get_r()*cos(th));
      if (row >= 0 && col >= 0 && 
          row < image.getRows() && col < image.getCols()){
        image(row,col) = sRGB(70, 70, 70);
      }
    }
    
    for (int i=0; i<200; i++){
      double th = min_angle + (max_angle-min_angle)*i/200.;
      int row = int(innercircle.get_yc() + innercircle.get_r()*sin(th));
      int col = int(innercircle.get_xc() + innercircle.get_r()*cos(th));
      if (row >= 0 && col >= 0 && 
          row < image.getRows() && col < image.getCols()){
        image(row,col) = sRGB(255, 255, 255);
      }
    }
    
    for (int i=0; i<200; i++){
      double th = min_angle + (max_angle-min_angle)*i/200.;
      int row = int(outercircle.get_yc() + outercircle.get_r()*sin(th));
      int col = int(outercircle.get_xc() + outercircle.get_r()*cos(th));
      if (row >= 0 && col >= 0 && 
          row < image.getRows() && col < image.getCols()){
        image(row,col) = sRGB(255, 255, 255);
      }
    }
  }

  virtual void project(CalibratedCamera &camera) { //, double extension, double angle_adjust){


    double extension = 0;
    double angle_adjust = 0;

    center = camera.PointFromPixel(int(centercircle.get_yc()),
                                   int(centercircle.get_xc()),
                                   height);
    // project corner points to determine true radiuses
    int row = int(outercircle.get_yc() + outercircle.get_r()*sin(min_angle));
    int col = int(outercircle.get_xc() + outercircle.get_r()*cos(min_angle));
    v3d p_outer = camera.PointFromPixel(row, col, height);
    outer_radius = (center - p_outer).length();

    row = int(innercircle.get_yc() + innercircle.get_r()*sin(min_angle));
    col = int(innercircle.get_xc() + innercircle.get_r()*cos(min_angle));
    v3d p_inner = camera.PointFromPixel(row, col, height);
    inner_radius = (center - p_inner).length();

    // extend tips
    double dtheta = 0.5 * extension / (inner_radius + outer_radius);
    min_angle -= dtheta;
    max_angle += dtheta;

    min_angle += angle_adjust;
    max_angle += angle_adjust;
  }

  void write(FILE *fp){
    fprintf(fp, "curved_wall %+8.4f %+8.4f %8.4f %8.4f %8.4f %8.4f %+8.4f %d\n",
            center.x(), center.z(), 
            inner_radius, outer_radius,
            min_angle, max_angle, center.y(), color_idx);
  }


  double height;
  v3d center;
  double inner_radius;
  double outer_radius;
  Circle centercircle;
  Circle innercircle;
  Circle outercircle;
  double min_angle;
  double max_angle;
  int material_idx;
  //bool valid;
  int color_idx;
};


inline void draw_curved_wall(CurvedWall &curvedwall, Image<byte> &im){

  for (int i=0; i<200; i++){
    Circle centercircle = curvedwall.centercircle;
    double th = curvedwall.min_angle + 
      (curvedwall.max_angle-curvedwall.min_angle)*i/200.;
    int row = int(centercircle.get_yc() + centercircle.get_r()*sin(th));
    int col = int(centercircle.get_xc() + centercircle.get_r()*cos(th));
    if (row >= 0 && col >= 0 && 
        row < im.getRows() && col < im.getCols()){
      im(row,col) = 70;
    }
  }

  for (int i=0; i<200; i++){
    Circle centercircle = curvedwall.innercircle;
    double th = curvedwall.min_angle + 
      (curvedwall.max_angle-curvedwall.min_angle)*i/200.;
    int row = int(centercircle.get_yc() + centercircle.get_r()*sin(th));
    int col = int(centercircle.get_xc() + centercircle.get_r()*cos(th));
    if (row >= 0 && col >= 0 && 
        row < im.getRows() && col < im.getCols()){
      im(row,col) = 255;
    }
  }

  for (int i=0; i<200; i++){
    Circle centercircle = curvedwall.outercircle;
    double th = curvedwall.min_angle + 
      (curvedwall.max_angle-curvedwall.min_angle)*i/200.;
    int row = int(centercircle.get_yc() + centercircle.get_r()*sin(th));
    int col = int(centercircle.get_xc() + centercircle.get_r()*cos(th));
    if (row >= 0 && col >= 0 && 
        row < im.getRows() && col < im.getCols()){
      im(row,col) = 255;
    }
  }
}


// DOLCEA: this needs to be renamed!
// estimate pixel error in fitting a curved wall
// !!! this is a hack, clean this up, and don't do this calculation twice
inline unsigned curvedwall_consistancy(std::vector<Point> &edge_points,
				Circle centercircle, 
			  double inlier_thresh){
  // fit inner and outer circles
  std::vector<Point> inside_points;
  std::vector<Point> outside_points;
  for (unsigned i=0; i<edge_points.size(); i++){
    if (centercircle.inside(edge_points[i])){
      inside_points.push_back(edge_points[i]);
    } else {
      outside_points.push_back(edge_points[i]);
    }
  }
  Point com(0,0); //!!!! use the real center here
  double max_inlier_thresh = 6.0;
  double min_inlier_thresh = 1.0;
  Circle innercircle(inside_points, com, max_inlier_thresh,
		     min_inlier_thresh);
  Circle outercircle(outside_points, com, max_inlier_thresh,
		     min_inlier_thresh);
  int count = 0;
  for (unsigned i=0; i<inside_points.size(); i++){
    if (innercircle.point_error(inside_points[i]) < inlier_thresh){
      count++;
    }
  }
  for (unsigned i=0; i<outside_points.size(); i++){
    if (outercircle.point_error(outside_points[i]) < inlier_thresh){
      count++;
    }
  }
  return count;
}

#endif
