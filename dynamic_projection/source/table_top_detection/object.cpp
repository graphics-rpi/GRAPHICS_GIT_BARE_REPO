#include <iostream>
#include "object.h"
#include "ramp.h"
#include "platform.h"
#include "wall.h"
#include "circle.h"
#include "curved_wall.h"
#include "soldier.h"
#include "unidentified.h"
#include "arrow.h"
#include "person.h"
#include "colortoken.h"

#include "scene.h"


void FillObjectInImage(Image<byte> &image, std::vector<Point> points, bool fill){
  const int dilate = 2;

  if (fill){
    // find the vertical extents of the object
    int min_row = image.getRows()-1;
    int max_row = 0;
    for (unsigned i=0; i<points.size(); i++){
      if (points[i].row < min_row) min_row = points[i].row;
      if (points[i].row > max_row) max_row = points[i].row;
    }
    
    // find the extents along each row
    int num_rows = max_row - min_row + 1;
    int *min_col = new int[num_rows];
    int *max_col = new int[num_rows];
    for (int i=0; i<num_rows; i++){
      min_col[i] = image.getCols()-1;
      max_col[i] = 0;
    }
    for (unsigned i=0; i<points.size(); i++){
      int r = points[i].row - min_row;
      if (points[i].col < min_col[r]) min_col[r] = points[i].col;
      if (points[i].col > max_col[r]) max_col[r] = points[i].col;
    }

    // fill between extents on each row
    for (int i=0; i<num_rows; i++){
      int r = i + min_row;
      for (int col=min_col[i]; col<= max_col[i]; col++){
	image(r, col) = 0;
      }

      // dilate (only needed at edge points)
      for (int row=i+min_row-dilate; row<=i+min_row+dilate; row++){
	for (int col=min_col[i]-dilate; col<=min_col[i]+dilate; col++){
	  if (row >=0 && row < image.getRows() &&
	      col >=0 && col < image.getCols()){
	    image(row, col) = 0;
	  }
	}
	for (int col=max_col[i]-dilate; col<=max_col[i]+dilate; col++){
	  if (row >=0 && row < image.getRows() &&
	      col >=0 && col < image.getCols()){
	    image(row, col) = 0;
	  }
	}
      }
    }

    delete [] min_col;
    delete [] max_col;
  } else {			      
    // fill just the points
    for (unsigned i=0; i<points.size(); i++){
      int r = points[i].row;
      int c = points[i].col;
      for (int row=r-dilate; row<=r+dilate; row++){
	for (int col=c-dilate; col<=c+dilate; col++){
	  if (row >=0 && row < image.getRows() &&
	      col >=0 && col < image.getCols()){
	    image(row, col) = 0;
	  }
	}
      }
    }
  }
}



void Blob::compute_gradients(Image<int>& image, int label){
  for(std::vector<Point>::iterator p = edge_points.begin(); p != edge_points.end(); p++){
    // sobel
    int dx = 0;
    int dy = 0;
    
    bool notTopRow = (p->row > 0);
    bool notBottomRow = (p->row < image.getRows());
    bool notFirstCol = (p->col > 0);
    bool notLastCol = (p->col < image.getCols());
    
    if(notTopRow){
      if(notFirstCol){
	dx += -(image(p->row - 1, p->col - 1) == label);
	dy += -(image(p->row - 1, p->col - 1) == label);
      }
      dy += -2 * (image(p->row - 1, p->col) == label);
      if(notLastCol){
	dx += (image(p->row - 1, p->col + 1) == label);
	dy += -(image(p->row - 1, p->col + 1) == label);
      }
    }
    
    dx += (notFirstCol) * -2 * (image(p->row, p->col - 1) == label);
    dx += (notLastCol) * 2 * (image(p->row, p->col + 1) == label);
    
    if(notBottomRow){
      if(notFirstCol){
	dx += -(image(p->row + 1, p->col - 1) == label);
	dy += (image(p->row + 1, p->col - 1) == label);
      }
      dy += 2 * (image(p->row - 1, p->col) == label);
      if(notLastCol){
	dx += (image(p->row + 1, p->col + 1) == label);
	dy += (image(p->row + 1, p->col + 1) == label);
      }
    }
    
    edge_gradients.push_back(Vec2f(dx,dy));
  }
}



Object* Blob::classify(Scene *scene) {
  //std::cout << "--------------------------------- " << std::endl;
  //std::cout << "classify an object " << std::endl;

  // ========================================
  // toss small objects
  int mass_thresh = 65;
  if (mass < mass_thresh){
    if (mass > 50) {
      (*args->output) << "too small " << mass << std::endl;
    }
    return NULL;
  }

  // ========================================
  // check entropy of color class distribution
  //int sum = 0;
  //  for (int i=0; i<NUM_COLORS; i++){
  //sum += ColorHist[i];
  //}

  /*  double entropy = 0.;
  for (int i=0; i<NUM_COLORS; i++){
    double p = double(ColorHist[i])/double(sum);
    if (p > 0.){
      entropy +=  - p * log(p);
    }
  }
  entropy /= log(2.);  // convert nats to bits
  double max_entropy = 1.7;
  if (entropy > max_entropy){
    std::cerr << "Entropy test failed\n";
    ///return NULL; // too many mixed colors to be a wall
  }
  */

  // calculate point stats
  Vec2f mean;
  Matrix2x2 cov = point_stats(edge_points, mean);
  double l1 = fabs(cov.l1());
  double l2 = fabs(cov.l2());
  aspect = std::min(l1, l2) / std::max(l1, l2);
  
  // fit line to all edge points
  Vec2f v;
  if (l1 > l2){
    v = cov.evect1(); 
  } else {
    v = cov.evect2(); 
  }
  double c = v.y * mean.x - v.x * mean.y; 
  Line centerline(-v.y, v.x, c);
  //double line_error = centerline.point_error(edge_points);
  double inlier_thresh = 2.0;
  int line_consistancy= wall_consistancy(edge_points, centerline,
					 inlier_thresh);
  // fit a circle to all edge points
  Point com(0,0); //!!!! use the real center here
  Circle centercircle(points, com, 0., 2.);
  //    double circle_error = centercircle.point_error(edge_points);
  int circle_consistancy = curvedwall_consistancy(edge_points, centercircle,
						  inlier_thresh);
  
  histogram = dominant_color_idx(points, scene->IMAGE_selected_components, scene->IMAGE_colors);


  std::vector<Object*> possible_answers;

  // ===========================================
  // DETECT PEOPLE
  try {
    Person *person = new Person(*scene->IMAGE_selected_components, points, edge_points, scene->camera,histogram);
    FillObjectInImage(*scene->IMAGE_table_mask, points, true);   	
    possible_answers.push_back(person);
  } catch (int) { }

  // ===========================================
  // NORTH ARROW
  try {
    Arrow *arrow = new Arrow(centerline, edge_points, histogram);
    FillObjectInImage(*scene->IMAGE_table_mask, points, false);
    scene->north = arrow->north;
    possible_answers.push_back(arrow);
  } catch (int) { }

  // ===========================================
  // PLATFORMS & RAMPS
  Quad quad(edge_points, points);
  if (quad.valid){
    try {
      Ramp *ramp = new Ramp(*scene->IMAGE_selected_components, quad, points);
      FillObjectInImage(*scene->IMAGE_table_mask, points, false);
      possible_answers.push_back(ramp);
    } catch (int) {
      try {
	Platform *plat = new Platform(quad);
	FillObjectInImage(*scene->IMAGE_table_mask, points, false);
	possible_answers.push_back(plat);
      } catch (int) {
      
      // ===========================================
      // COLOR TOKENS
	try {
	  ColorToken *colortoken = new ColorToken(edge_points,points,*scene->IMAGE_selected_components, *scene->IMAGE_white_balance, quad, histogram); //points);
	  FillObjectInImage(*scene->IMAGE_table_mask, points, false);
	  possible_answers.push_back(colortoken);
	  //return colortoken;
	} catch (int) {
	}
      }
    }
  }


  // ===========================================
  // STRAIGHT WALLS
  if (line_consistancy > circle_consistancy){
    try {
      Wall *wall = new Wall(edge_points, centerline, points, *scene->IMAGE_selected_components); 
      FillObjectInImage(*scene->IMAGE_table_mask, points, false);
      possible_answers.push_back(wall);
    } catch (int) {
    }
  }
  else {
    // ===========================================
    // CURVED WALL
    try {
      CurvedWall *curvedwall = new CurvedWall(edge_points, points, *scene->IMAGE_selected_components, centercircle,histogram);
      FillObjectInImage(*scene->IMAGE_table_mask, points, false);
      possible_answers.push_back(curvedwall);
    }
    catch (int) { 
      // could be a column (assume no curved walls for now)
      try {
	Column *column = new Column(points, edge_points, scene->camera, histogram);
	FillObjectInImage(*scene->IMAGE_table_mask, points, true);   	
	possible_answers.push_back(column);
      } catch (int) {
      }
    }
  }

  // ===========================================
  // SOLDIERS
  try {
    Soldier *soldier = new Soldier(points,edge_points, scene->camera, histogram); 
    FillObjectInImage(*scene->IMAGE_table_mask, points, true);   
    possible_answers.push_back(soldier);
    //return soldier;
  } catch (int) { }


  std::sort(possible_answers.begin(),possible_answers.end());

  (*args->output) << "POSSIBLE ANSWERS " << possible_answers.size() << std::endl;
  if (possible_answers.size() > 0) 
    return possible_answers[0];


  // otherwise, it's an unidentified object!
  Object *answer = new UnidentifiedObject(*scene->IMAGE_selected_components, points);
  (*args->output) << "Unidentified blob." << std::endl;      
  FillObjectInImage(*scene->IMAGE_table_mask, points, true);   
  return answer;

}
