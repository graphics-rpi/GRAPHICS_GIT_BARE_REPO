#include "wall.h"
#include "curved_wall.h"

#include "argparser.h"

extern ArgParser *args;

CurvedWall::CurvedWall(std::vector<Point> &edge_points, 
                       std::vector<Point> &points, 
                       Image<byte> &component_image,
                       Circle init_circle, Histogram &histogram){
  centercircle = init_circle;
  (*args->output) << "trying to make a curved wall" << std::endl;
  
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
  innercircle = Circle(inside_points, com, max_inlier_thresh,
                       min_inlier_thresh);
  outercircle = Circle(outside_points, com, max_inlier_thresh,
                       min_inlier_thresh);
  
  // estimate common center
  double xc = (innercircle.get_xc() + outercircle.get_xc())/2;
  double yc = (innercircle.get_yc() + outercircle.get_yc())/2;
  double inlier_thresh = 1.;
  double r_inner = 0.;
  int count = 0;
  for (unsigned i=0; i<inside_points.size(); i++){
    if (innercircle.point_error(inside_points[i]) < inlier_thresh){
      r_inner += sqrt((inside_points[i].col - xc)*
                      (inside_points[i].col - xc)+
                      (inside_points[i].row - yc)*
                      (inside_points[i].row - yc));
      count++;
    }
  }
  r_inner /= count;
  double r_outer = 0.;
  count = 0;
  for (unsigned i=0; i<outside_points.size(); i++){
    if (outercircle.point_error(outside_points[i]) < inlier_thresh){
      r_outer += sqrt((outside_points[i].col - xc)*
                      (outside_points[i].col - xc)+
                      (outside_points[i].row - yc)*
                      (outside_points[i].row - yc));
      count++;
    }
  }
  r_outer /= count;
  innercircle.set_xc(xc);
  innercircle.set_yc(yc);
  innercircle.set_r(r_inner);
  outercircle.set_xc(xc);
  outercircle.set_yc(yc);
  outercircle.set_r(r_outer);

  /*
  // OLD VALUES FOR low res camera
  float min_thickness = 5.0;
  float max_thickness = 19.0;
  */

  // NEW VALUES FOR high res camera
  float min_thickness = 15.0;
  float max_thickness = 25.0;
    
  //  std::cout << (r_outer - r_inner) << std::endl;
      
  // check min, max thickness
  if ((r_outer - r_inner) < min_thickness || (r_outer - r_inner) > max_thickness){


    (*args->output) << "curved radius issue" << std::endl;
    throw -1;
  }
  
  // re-estimate centerline circle for angle detemination
  double r_center = 0.;
  count = 0;
  for (unsigned i=0; i<edge_points.size(); i++){
    r_center += sqrt((edge_points[i].col - xc)*
                     (edge_points[i].col - xc)+
                     (edge_points[i].row - yc)*
                     (edge_points[i].row - yc));
    count++;
  }
  r_center /= count;
  centercircle.set_xc(xc);
  centercircle.set_yc(yc);
  centercircle.set_r(r_center);
  
  // find angular interval: first, find all angles 0-2pi
  std::vector<double> theta;
  for (unsigned i=0; i<edge_points.size(); i++){
    double th = atan2(edge_points[i].row - yc, 
                      edge_points[i].col - xc);
    if (th < 0.) th += 2*M_PI;
    theta.push_back(th);
  }
  std::sort(theta.begin(), theta.end());
  
  // find angular interval: now, find largest angular gap
  bool angles_set = false;
  double max_gap = 0.;
  for (unsigned i=0; i<theta.size(); i++){
    unsigned j = (i+1) % theta.size();
    double gap = theta[j] - theta[i];
    if (gap < 0) gap += 2*M_PI;
    if (gap > max_gap){
      max_gap = gap;
      angles_set = true;
      min_angle = theta[j];
      max_angle = theta[i];
    }
  }
  if (angles_set != true) {
    (*args->output) << "angles not set somehow" << std::endl;
    throw -1;
  }

  if (max_angle < min_angle){
    max_angle += 2*M_PI;
  }
  
  // no curved walls > pi
  if (max_gap < M_PI){
    (*args->output) << "curved gap issue" << std::endl;
    throw -1;
  }
  
  // find dominant color to get height
  /*
  Histogram hist(8);
  for (unsigned i=0; i<points.size(); i++){
    hist(component_image(int(points[i].row), int(points[i].col)))++;
  }
  */
  height = wall_idx_to_height(histogram.peak_idx());
  if (height == 0.){
    (*args->output) << "curved height issue" << std::endl;
    throw -1;
  }
  (*args->output) << "FOUND CURVED WALL" << std::endl;

  // HACK: DONT LOOK FOR CURVED WALLS
  //throw -1;
}

