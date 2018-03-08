#ifndef _WALL_H_
#define _WALL_H_

#include "histogram.h"
#include "rectangle.h"
#include "color.h"
#include "object.h"
#include "window.h"


inline double wall_idx_to_height(int wall_idx){
  switch(wall_idx){
  case RED_IDX:
    return RED_HEIGHT;
    break;
  case GREEN_IDX:
    return GREEN_HEIGHT;
    break;
  case BLUE_IDX:
    return BLUE_HEIGHT;
    break;
  default:
    fprintf(stderr, "invalid wall color index\n");
    return 0.;
  }
}

// ======================================================================
// WALL
// ======================================================================

class Wall : public Object {
public:

  Wall(std::vector<Point> &edge_points, Line centerline,
       std::vector<Point> &points, Image<byte> &component_image);


  void setColor(const sRGB &c) { color = c; }
  bool isWall() { return true; }
  
  double point_distance(Point p){
    double closest = std::numeric_limits<double>::max();

    // test the four corner points
    for (int i=0; i<4; i++){
      double d = p.distance(rectangle.corner_points[i]);
      if (d < closest) closest = d;
    }

    // test distance to centroid of corner points
    double row = 0.;
    double col = 0.;
    for (int i=0; i<4; i++){
      row += rectangle.corner_points[i].row;
      col += rectangle.corner_points[i].col;
    }
    row /= 4.;
    col /= 4.;
    
    double dist = p.distance(Point(row,col));
    if (dist < closest) closest = dist;

    return closest;
  }

  void draw(Image<sRGB> &image){
    rectangle.draw(image);
    for (unsigned i=0; i<windows.size(); i++){
      windows[i].draw(image);
    }
  }

  void project(CalibratedCamera &camera){
    rectangle.project_and_extend(camera, y, wall_tips);
    for (unsigned i=0; i<windows.size(); i++){
      windows[i].project(camera, y);
    }
  }

  void write(FILE *fp){

    fprintf (fp, "wall_material %f %f %f\n", color.r()/255.0, color.g()/255.0, color.b()/255.0);

    fprintf(fp, "wall   ");
    rectangle.write(fp);
    fprintf(fp, "%+8.4f\n", y);
    for (unsigned i=0; i<windows.size(); i++){
      windows[i].write(fp);
    }
  }

  sRGB color;

private:
  int n_points;
  Rectangle rectangle;
  Line line1;
  Line line2;
  Line cap1;
  Line cap2;
  Line centerline;
  Point end_point1, end_point2;
  double y;
  std::vector<Window> windows;
};


// estimate pixel error in fitting a wall
// !!! this is a hack, clean this up, and don't do this calculation twice
inline unsigned wall_consistancy(std::vector<Point> &edge_points, Line centerline, 
			  double inlier_thresh){
  // fit lines to both sides
  std::vector<Point> side1_points;
  std::vector<Point> side2_points;
  for (unsigned i=0; i<edge_points.size(); i++){
    if (centerline.side(edge_points[i])){
      side1_points.push_back(edge_points[i]);
    } else {
      side2_points.push_back(edge_points[i]);
    }
  }
  Line line1(side1_points, inlier_thresh);
  Line line2(side2_points, inlier_thresh);
  int count = 0;
  for (unsigned i=0; i<side1_points.size(); i++){
    if (line1.point_error(side1_points[i]) < inlier_thresh){
      count++;
    }
  }
  for (unsigned i=0; i<side2_points.size(); i++){
    if (line2.point_error(side2_points[i]) < inlier_thresh){
      count++;
    }
  }
  return count;
}

#endif
