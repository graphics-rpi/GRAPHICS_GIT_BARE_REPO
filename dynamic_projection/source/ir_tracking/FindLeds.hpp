#include <vector>
#include <algorithm>
#include <strings.h>
#include <string.h>

#include "../common/Image.h"
#include "../common/ImageOps.h"
#include "../common/FisheyeCamera.hpp"

const int X_AXIS = 0;
const int Y_AXIS = 1;
const int Z_AXIS = 2;

// Chris Stuetzle
//const int NUM_BRIGHTNESS_BINS = 10;



void printelapsedtime();


#include "ir_data_point.h"

bool sort_by_x(const IR_Data_Point &a, const IR_Data_Point &b) {
  //if (a.x() <= a.y()) return true;
  if (a.x() <= b.x()) return true;
  return false;
}

void MergeNearbyLEDS(std::vector<IR_Data_Point> &detected_leds,
		     double expected_max_led_radius) {


  std::sort(detected_leds.begin(),detected_leds.end(),sort_by_x);
  double sq_dist_max = expected_max_led_radius*expected_max_led_radius;

  std::vector<int> to_erase;

  for (unsigned int i = 0; i < detected_leds.size(); i++) {
    IR_Data_Point &a = detected_leds[i];
    if (a.brightness() < 0) continue;
    for (unsigned int j = i+1; j < detected_leds.size(); j++) {
      IR_Data_Point &b = detected_leds[j];
      if (b.brightness() < 0) continue;
      if (a.x() < b.x()-expected_max_led_radius) break;
      double sq_dist = (a.x()-b.x())*(a.x()-b.x()) + (a.y()-b.y())*(a.y()-b.y());
      if (sq_dist < sq_dist_max) {
	std::cout << "MERGING" << std::endl;
	//double a_frac = a.brightness() / (a.brightness()+b.brightness());
	//assert (a_frac > 0 && a_frac < 1);
	//double b_frac = 1-a_frac;
	//assert (b_frac > 0 && b_frac < 1);
	
	std::cout << "FIX MERGING" << std::endl;
	//a.x = a.x()*a_frac + b.x()*b_frac;	
	//a.y = a.y()*a_frac + b.y()*b_frac;
	//a.estimated_velocity = a.estimated_velocity*a_frac + b.estimated_velocity*b_frac;
	//a.brightness = a.brightness + b.brightness;
	//a.radius = a.radius + b.radius;
	//b.brightness = -1;
      }      
    }
  }

  std::vector<IR_Data_Point> answer;
  
  for (unsigned int i = 0; i < detected_leds.size(); i++) {
    IR_Data_Point &a = detected_leds[i];
    if (a.brightness() > 0) {
      answer.push_back(a);
    }
  }

  if (detected_leds.size() != answer.size()) {
    std::cout << "before " << detected_leds.size() << " after " << answer.size() << std::endl;
  }

  detected_leds = answer;
}



class LEDFinder {
 public:

  LEDFinder(){
  }

  ~LEDFinder(){
  }

  struct Point
  {
    double row;
    double col;
    v3d world_pos;
    Point(){}
    Point(double row, double col){
      this->row = row;
      this->col = col;
    }
    Point(double row, double col, v3d world_pos){
      this->row = row; 
      this->col = col;
      this->world_pos = world_pos;
    }
    double distance(Point p){
      return sqrt((p.row-row)*(p.row-row)+(p.col-col)*(p.col-col));
    }
    void dump() { std::cout << row << " " << col << "\n"; }
  };

  struct BackProjection
  { 
    BackProjection(){
      useMask = false;
      constraintAxis = Z_AXIS;
      constraint = 0;
    }

    BackProjection(int axis, double cons){
      useMask = false;
      constraintAxis = axis;
      constraint = cons;
    }

    BackProjection(int axis, double cons, Image<byte> m){
      useMask = true;
      mask = m;
      constraintAxis = axis;
      constraint = cons;
    }

    bool useMask;
    Image<byte> mask;
    int constraintAxis; // 0 = x, 1 = y, 2 = z
    double constraint;
  };






  //std::vector<IR_Data_Point> 
  void FindLEDsPixelsOnly(
			  unsigned long ip,
			  Image<byte> *input_image, 
			  std::vector<IR_Data_Point> &answer,
			  Image<sRGB> *debug_image,
			  byte brightness_thresh_seed,
			  byte brightness_thresh_cc,
			  /* on a scale of 1->255 */
			  double expected_min_led_radius, /* in pixels */
			  double expected_max_led_radius /* in pixels */,
			  int mask_min_row,
			  int mask_max_row,
			  int mask_min_col,
			  int mask_max_col
			  ) {


    assert (input_image != NULL);

    std::vector<IR_Data_Point> detected_leds;

    assert (mask_min_row >= 0 && mask_max_row < input_image->getRows());
    assert (mask_min_col >= 0 && mask_max_col < input_image->getCols());

#ifndef MYNDEBUG
    if (debug_image != NULL) {
      //std::cout << "initialize debug image" << std::endl;
      for (int row=0; row<input_image->getRows(); row ++) {
	for (int col=0; col<input_image->getCols(); col++) {	  
	  (*debug_image)(row,col).r() = (*input_image)(row,col);
	  (*debug_image)(row,col).g() = (*input_image)(row,col);
	  (*debug_image)(row,col).b() = (*input_image)(row,col);
	  if (row < mask_min_row || row >= mask_max_row || col < mask_min_col || col >= mask_max_col) {
	    (*debug_image)(row,col).r() = 100;
	    (*debug_image)(row,col).g() = 100;
	  }
	} 
      }
    }    
#endif

    double led_total_brightness_min = 3*expected_min_led_radius*expected_min_led_radius*brightness_thresh_cc;//seed;

    // create & clear a mask image to mark which pixels have been examined & marked as an LED
    Image<byte> marked(input_image->getRows(), input_image->getCols());
		//Tyler commented out for Human Paintbrush
    //bzero(marked.getData(), marked.getRows() * marked.getCols() * sizeof(byte));
		memset(marked.getData(),'\0', marked.getRows() * marked.getCols() * sizeof(byte));

    // take a rough pass over the image looking for "seed" points on blobs
    std::vector<Point> seeds;
    int pixel_skip = expected_min_led_radius; 
    for (int row=mask_min_row; row<=mask_max_row; row += pixel_skip) {
      for (int col=mask_min_col; col<mask_max_col; col += pixel_skip) {
	if ((*input_image)(row, col) > brightness_thresh_seed) {
	  seeds.push_back(Point(row,col));
#ifndef MYNDEBUG
	  if (debug_image != NULL) {
	    (*debug_image)(row,col).g() = 255;
	  }
#endif
	}
      }
    }
    if (seeds.size() > 1000) {
      std::cout << "num seeds " << seeds.size() << std::endl;
      return; // answer;
    }

    //    std::cout << "num seeds " << seeds.size() << std::endl;

    // go through all seed points, examining a surrounding region
    int window_size = 2 * expected_max_led_radius; // half-width of window
    Image<byte> block(2*window_size+1, 2*window_size+1);
    for (unsigned i=0; i<seeds.size(); i++){

      int min_row = std::max(0., seeds[i].row - window_size);
      int min_col = std::max(0., seeds[i].col - window_size);
      int max_row = std::min(input_image->getRows()-1., seeds[i].row + window_size);
      int max_col = std::min(input_image->getCols()-1., seeds[i].col + window_size);

      // threshold the image block
      for (int row=min_row; row<=max_row; row++){
	for (int col=min_col; col<=max_col; col++){
	  if ((*input_image)(row, col) > brightness_thresh_seed / 2.0){
	    block(row-min_row, col-min_col) = 255;
#ifndef MYNDEBUG
	    if (debug_image != NULL && 
		(*debug_image)(row,col).g() != 255) {
	      (*debug_image)(row,col).b() = 255;
	    }
#endif
	  } else {
	    block(row-min_row, col-min_col) = 0;
	  }
	}
      }

      // find connected components in the region
      int num_components;
      Image<int> labels = ConnectedComponents(block, num_components);
      int seed_label = labels(seeds[i].row - min_row, seeds[i].col - min_col);

      // collect stats for the seeded blob
      double row_sum = 0.;
      double col_sum = 0.;
      double weight_sum = 0.;
      
      for (int row=min_row; row<=max_row; row++){
	for (int col=min_col; col<=max_col; col++){
	  if (seed_label == labels(row-min_row, col-min_col)){
	    weight_sum += (*input_image)(row, col);
	    row_sum += row * (*input_image)(row, col);
	    col_sum += col * (*input_image)(row, col);
	  }
	}
      }
      
      // if the sum at this seed is sufficiently bright
      if (weight_sum > led_total_brightness_min) {
	int led_id = -1;

	row_sum /= weight_sum;
	col_sum /= weight_sum;

	//	std::cout << "tmp " << row_sum << " " << col_sum << std::endl;

	// the led position
	Point detected(row_sum,col_sum); ///weight_sum, col_sum/weight_sum);


	// NEW WAY
	double count = 0;
	for (int row=min_row; row<=max_row; row++) {
	  for (int col=min_col; col<=max_col; col++) {
	    if (seed_label == labels(row-min_row, col-min_col)){
	      // NOT SURE IF IT SHOULD BE WEIGHTED
	      count++;
	    }
	  }
	}

	// MAKE MICRO IMAGE

	Image<byte> micro_image(window_size*2+1,window_size*2+1/*, 0*/);  // not sure when this changed, hope this works... (barb)
	for (int my_i=-window_size; my_i <= window_size; my_i++) {
	  for (int my_j=-window_size; my_j <= window_size; my_j++) {
	    int row = int(row_sum+0.5) + my_i;
	    int col = int(col_sum+0.5) + my_j;
	    if (row < 0 || row >= input_image->getRows()) continue;
	    if (col < 0 || col >= input_image->getCols()) continue;
	    int i2 = my_i + window_size;
	    int j2 = my_j + window_size;
	    assert (i2 >= 0 && i2 < micro_image.getRows());
	    assert (j2 >= 0 && j2 < micro_image.getCols());
	    micro_image(i2,j2) = (*input_image)(row,col);
	    // NOT SURE THIS CHECK IS RIGHT
	    if (1) { //seed_label == labels(row-min_row2, col-min_col2)){
	      //	      micro_image(my_i+window_size,my_j+window_size) = 0; //(*input_image)(row,col);
	      //} else {
	      //micro_image(my_i,my_j) = 0;
	    }
	  }
	}

	// check to see if any of these pixels has already been used for another led
	for (int row=min_row; row<=max_row; row++){
	  for (int col=min_col; col<=max_col; col++){
	    if (seed_label == labels(row-min_row, col-min_col)){
	      if (marked(row, col) != 0) led_id = marked(row,col);
	    }
	  }
	}

	IR_Data_Point newPoint(ip,detected.row,detected.col,micro_image);

	if (led_id != -1) {
	  // if the old position had a lower weighted sum, replace it
	  double prev_weight = detected_leds[led_id-1].brightness();
	  if (prev_weight < weight_sum) {
	    detected_leds[ led_id - 1 ] = newPoint;
	  }
	} else {
	  // if this is a new led, add it!
	  detected_leds.push_back( newPoint );
	  led_id = detected_leds.size();
	}
	
	// label all these pixels with the led id
	for (int row=min_row; row<=max_row; row++){
	  for (int col=min_col; col<=max_col; col++){
	    if (seed_label == labels(row-min_row, col-min_col)){
	      marked(row, col) = led_id;
	    }
	  }
	}
      }
    }
    
    MergeNearbyLEDS(detected_leds,expected_max_led_radius);
    std::sort(detected_leds.begin(),detected_leds.end());

    // ------------------------------------------------------
    // PREPARE THE ANSWER VECTOR
    //std::cout << "detected leds " << detected_leds.size();
    for (unsigned i=0; i<detected_leds.size(); i++){
      answer.push_back(detected_leds[i]);
      

      
#ifndef MYNDEBUG
      double row = detected_leds[i].y();
      double col = detected_leds[i].x();

      // BARB:: row/col confusion
      int N = 20;
      int min_r = std::max(0., row-N);
      int max_r = std::min(input_image->getCols()-1., row+N);
      int min_c = std::max(0., col-N);
      int max_c = std::min(input_image->getRows()-1., col+N);
      int mid_r = std::min(std::max(0., row+0.5), input_image->getCols()-1.);
      int mid_c = std::min(std::max(0., col+0.5), input_image->getRows()-1.);

      if (debug_image != NULL) {

	// ----------------------------------------
	// MARK THE LED LOCATION IN THE INPUT IMAGE
	for (int r=min_r; r<=max_r; r++){
	  (*debug_image)( mid_c,r).r() = 255; // - input_image(r, mid_c);
	  //(*debug_image)(r, mid_c).r() = 255; // - input_image(r, mid_c);
	}
	for (int c=min_c; c<=max_c; c++){
	  //(*debug_image)(mid_r, c).r() = 255; // - input_image(mid_r, c);
	  (*debug_image)(c,mid_r).r() = 255; // - input_image(mid_r, c);
	}  
      }	
#endif
	
    }

    //std::cout << "RETURNING " << answer.size() << " detected LEDS" << std::endl;

    return; // answer;
  }

private:
  
  LEDFinder(const LEDFinder &);
  LEDFinder &operator=(const LEDFinder &);
};
