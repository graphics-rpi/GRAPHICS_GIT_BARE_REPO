#include "soldier.h"

Soldier::Soldier(/*Image<byte> &component_image, */std::vector<Point> &points,
		 std::vector<Point> &edge_points, CalibratedCamera &camera, Histogram &histogram){


  if (!args->find_army_soldiers) { throw -1; }

  material_idx = histogram.peak_idx();

  if (material_idx != RED_IDX &&
      material_idx != GREEN_IDX) { throw -1; }

  height = 0.0508; // hard-coded soldier height (about 2 inches)
  
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
  
  //std::cerr << "soldier radius: " << radius << std::endl;
  
  // check for proper size 7/8" diameter = 0.0111 m radius
  //  but some are actually 1" diameter, so make tolerance larger
  double avg_radius = 0.008927355; // hard-coded average
  double tol = avg_radius * 0.5; // 40% tolerance
  double upper_thresh = avg_radius + 2*tol;
  double lower_thresh = avg_radius - tol;
  //    printf ("column_radius = %f (%f - %f)\n", 
  //	    radius, lower_thresh, upper_thresh);
  if (radius < 0.5*lower_thresh) {
    //std::cerr << "invalid radius (too small): " << radius << " < " << lower_thresh << std::endl;
    throw -1; //valid = false;
  } else if (radius > 2.0*upper_thresh){
    //std::cerr << "invalid radius (too large): " << radius << " > " << upper_thresh << std::endl;
    throw -1; //valid = false;
  }
  
  // estimate image-space center and radius for overlay image
  v3d p = camera.PixelFromPoint(centroid);
  center_row = p.y();
  center_col = p.x();
  v3d p1 = camera.PixelFromPoint(centroid + v3d(radius, 0., 0.));
  pixel_radius = (p - p1).length();

  (*args->output) << "FOUND SOLDIER" << std::endl;
}

