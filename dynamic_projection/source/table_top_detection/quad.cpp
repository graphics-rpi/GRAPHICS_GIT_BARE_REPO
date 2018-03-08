#include "quad.h"

Quad::Quad(std::vector<Point> &edge_points, std::vector<Point> &points){

  // don't fit quads with fewer than this many edge pixels
  if (points.size() < 200 || 
      edge_points.size() < 200){
    valid = false;
    return;
  }
  
  e_points = edge_points;
  
  // find first line
  Line l1 = ransac_line(edge_points, points);
  std::vector<Point> remaining_points;
  double inlier_thresh = 2.0;
  for (unsigned i=0; i<edge_points.size(); i++){
    double err = l1.point_error(edge_points[i]);
    if (err > inlier_thresh){
      remaining_points.push_back(edge_points[i]);
      edge_line_ids.push_back(-1); // point is unassigned
    }
    else{
      edge_line_ids.push_back(0); // point is assigned to the first line
    }
  }
  
  // second line
  Line l2 = ransac_line(remaining_points, points);
  std::vector<Point> remaining_points2;
  //for (unsigned i=0; i<remaining_points.size(); i++){
  for(unsigned i = 0; i < edge_points.size(); i++){
    if(edge_line_ids[i] == -1){
      double err = l2.point_error(edge_points[i]); //remaining_points[i]);
      if (err > inlier_thresh){
	remaining_points2.push_back(edge_points[i]); // remaining_points[i]);
      }
      else{
	edge_line_ids[i] = 1; // assign point
      }
    }
  }
  
  // third line
  Line l3 = ransac_line(remaining_points2, points);
  remaining_points.clear();
  //for (unsigned i=0; i<remaining_points2.size(); i++){
  for(unsigned i = 0; i < edge_points.size(); i++){
    if(edge_line_ids[i] == -1){
      double err = l3.point_error(edge_points[i]); //remaining_points2[i]);
      if (err > inlier_thresh){
	remaining_points.push_back(edge_points[i]); //remaining_points2[i]);
      }
      else{
	edge_line_ids[i] = 2;
      }
    }
  }
  
  ////  std::cerr << "remaining points : " << remaining_points.size() << " ";// << std::endl;
  
  //  fourth line
  Line l4 = ransac_line(remaining_points, points);
  for(unsigned i = 0; i < edge_points.size(); i++){
    if(edge_line_ids[i] == -1){
      double err = l4.point_error(edge_points[i]);
      if (err <= inlier_thresh){
	edge_line_ids[i] = 3;
      }
    }
  }
  
  // choose 4 most perpendicular pairs of lines to intersect for corners
  std::vector<Linepair> pairs;
  pairs.push_back(Linepair(l1, l2));
  pairs.push_back(Linepair(l1, l3));
  pairs.push_back(Linepair(l1, l4));
  pairs.push_back(Linepair(l2, l3));
  pairs.push_back(Linepair(l2, l4));
  pairs.push_back(Linepair(l3, l4));
  std::sort(pairs.begin(), pairs.end(), linepair_by_dot);
  
  // calculate center of 4 corners
  double xc = 0.;
  double yc = 0.;
  std::vector<Point> corners;
  for (unsigned i=0; i<4; i++){
    corners.push_back(pairs[i].intersection);
    xc += pairs[i].intersection.col;
    yc += pairs[i].intersection.row;
  }
  xc /= 4.;
  yc /= 4.;
  
  // sort points into angular order around the center
  // N^2 sort (but 4^2 = 16)
  for (int i=0; i<3; i++){
    double ang1 = atan2(corners[i].row - yc, corners[i].col - xc);
    if (ang1 < 0.){
      ang1 += 2*M_PI;
    }    
    int closest = 0;
    double min_dist = 2*M_PI;
    for (int j=i+1; j<4; j++){
      double ang2 = atan2(corners[j].row - yc, corners[j].col - xc);
      if (ang2 < 0.){
	ang2 += 2*M_PI;
      }
      // find clockwise angular distance between points
      double dist = ang2 - ang1;
      if (dist < 0.){
	dist += 2*M_PI;
      }
      if (dist < min_dist){
	min_dist = dist;
	closest = j;
      }
    }
    Point temp = corners[i+1];
    corners[i+1] = corners[closest];
    corners[closest] = temp;
  }
  
  for (int i=0; i<4; i++){
    corner_points[i] = corners[i];
  }
  
  valid = true;
}


void Quad::draw(Image<sRGB> &image){
  //PixelSetter<sRGB> setPixel(sRGB(255, 255, 255));
  for (int i=0; i<4; i++){
    int j = (i+1) % 4;
    if (corner_points[i].row >= 0 &&
	corner_points[i].row < image.getRows() &&
	corner_points[i].col >= 0 &&
	corner_points[i].col < image.getCols() &&
	corner_points[j].row >= 0 &&
	corner_points[j].row < image.getRows() &&
	corner_points[j].col >= 0 &&
	corner_points[j].col < image.getCols()){
      image.setLinePixels(
			  //image.applyFunctorOnLine(setPixel, 
			       int(corner_points[i].row),
			       int(corner_points[i].col),
			       int(corner_points[j].row),
			       int(corner_points[j].col),
			       sRGB(255,255,255));
    }
  }
  
  
  for(unsigned int i = 0; i < e_points.size(); i++){
    sRGB color(255,255,255);
    if(edge_line_ids[i] == 0){
      color = sRGB(255,0,0);
    }
    else if(edge_line_ids[i] == 1){
      color = sRGB(0,255,0);
    }
    else if(edge_line_ids[i] == 2){
      color = sRGB(0,0,255);
    }
    else if(edge_line_ids[i] == 3){
      color = sRGB(255,255,0);
    }
    
    image(e_points[i].row, e_points[i].col) = color;
  }
  
}

