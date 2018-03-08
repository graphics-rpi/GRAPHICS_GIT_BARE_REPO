#include "ir_data_point.h"

//int brightness_threshold_seed = 25;
int brightness_threshold_seed = 30;

IR_Data_Point::IR_Data_Point() 
  : ip_(0),x_(-1),y_(-1),led_image_(Image<byte>()) {
  for (int i = 0; i < NUM_BRIGHTNESS_BINS; i++) {
    interpolable_data_.brightness_histogram_[i] = 0;
  }  
}

IR_Data_Point::IR_Data_Point(unsigned long ip, double x, double y, const Image<byte> &micro_image) 
  : ip_(ip),x_(x),y_(y), led_image_(micro_image) {
  calculateHistogram( );
} // IR_Data_Point



void IR_Data_Point::calculateHistogram( )
{
  // CHRIS STUETZLE
  // Create a histogram of the brightness values, with a width of 2 pixels each
  //  std::vector< double > 
  int num_pixels_per_bin[NUM_BRIGHTNESS_BINS];
  for (int i = 0; i < NUM_BRIGHTNESS_BINS; i++) {
    interpolable_data_.brightness_histogram_[i] = 0;
    num_pixels_per_bin[i] = 0;
  }
  //double interpolable_data_.brightness_histogram_ = std::vector< double > ( NUM_BRIGHTNESS_BINS, 0.0 );

  //std::vector< int > num_pixels_per_bin( NUM_BRIGHTNESS_BINS, 0 );
  // For all the pixels in the connected component, add up the brightness
  // Maybe try larger windows...
  double dist;
  int bin;
  int min_row = 0;//seeds[ i ].row - NUM_BRIGHTNESS_BINS / 2;
  int max_row = led_image_.getRows();//seeds[ i ].row + NUM_BRIGHTNESS_BINS / 2;
  int min_col = 0;//seeds[ i ].col - NUM_BRIGHTNESS_BINS / 2;
  int max_col = led_image_.getCols();//seeds[ i ].col + NUM_BRIGHTNESS_BINS / 2;
  int center_x = max_row / 2;
  int center_y = max_col / 2;
  for (int row=min_row; row<max_row; row++){
    for (int col=min_col; col<max_col; col++){
      // Find the histogram bin the squared distance fits in
      dist = sqrt( ( row - center_x ) * ( row - center_x ) + 
		   ( col - center_y ) * ( col - center_y ) );
      bin = (int) ( 1.0 * dist )/* / ( 1.0 * NUM_BRIGHTNESS_BINS * NUM_BRIGHTNESS_BINS ) */;
      if( bin >= NUM_BRIGHTNESS_BINS )
        bin = NUM_BRIGHTNESS_BINS - 1;
      assert (bin >= 0 && bin < NUM_BRIGHTNESS_BINS);
      // std::cout << "here " << bin << " " << row << "," << col << " " << led_image(row,col) << std::endl;
      interpolable_data_.brightness_histogram_[ bin ] += led_image_(row, col);
      num_pixels_per_bin[ bin ]++;
    } // for
  } // for
  // Now average all the bins
  for( int i = 0 ; i < NUM_BRIGHTNESS_BINS ; i++ )
  {
    if( num_pixels_per_bin[ i ] > 0 )
      interpolable_data_.brightness_histogram_[ i ] = interpolable_data_.brightness_histogram_[ i ] / ( 1.0 * num_pixels_per_bin[ i ] );
  } // for

  interpolable_data_.set_valid(true);
} // calculateHistogram



void IR_Data_Point::print(std::ostream &ostr) {
  for (unsigned int i = 0; i < NUM_BRIGHTNESS_BINS; i++) {
    ostr << " " << std::fixed << std::setw(5) << std::setprecision(1) << interpolable_data_.brightness_histogram_[i]; } 
  ostr << std::endl;
}


std::ostream& operator<<(std::ostream &ostr, const IR_Data_Point &pt) {
  ostr << std::setw(15) << pt.ip_ << " " 
       << std::setprecision(3) << std::fixed << std:: setw(10) << pt.x_ << " " 
       << std::setprecision(3) << std::fixed << std:: setw(10) << pt.y_ << " " 
       << pt.interpolable_data_;
  return ostr;
}

std::istream& operator>>(std::istream &istr, IR_Data_Point &pt) {  
  istr 
    >> pt.ip_
    >> pt.x_
    >> pt.y_;
  istr >> pt.interpolable_data_;
  assert (pt.interpolable_data_.valid());  
  return istr;
}

//=========================================================================

std::ostream& operator<<(std::ostream &ostr, const IR_Interpolable_Data &id) {
  ostr << " < ";
  for (int i = 0; i < NUM_BRIGHTNESS_BINS; i++) {
    ostr << std::setprecision(3) << std::fixed << std::setw(10) << id.brightness_histogram_[i] << " ";	
  }
  ostr << " > ";
  return ostr;
}

std::istream& operator>>(std::istream &istr, IR_Interpolable_Data &id) {  
  std::string s;
  istr >> s;
  for (int i = 0; i < NUM_BRIGHTNESS_BINS; i++) {
    istr >> id.brightness_histogram_[i];
  }
  istr >> s;
  /*
  if (s != ">") {
    std::cout << "some error not equal to '>'" << s << std::endl;
  }  
  */
  //assert (s == ">");
  id.set_valid(true);
  return istr;
}


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


double IR_Interpolable_Data::CalculateDifference(const IR_Interpolable_Data &IRD) const {
  assert (this->valid());
  assert (IRD.valid());

  double absolute = 0;
  // sum of squared differences
  for (int i = 0; i < NUM_BRIGHTNESS_BINS; i++) {
    double a = this->brightness_histogram_[i] / 255;
    double b = IRD.brightness_histogram_[i] / 255;
    double d = a-b;
    /*
      // attempt at weight the bins differently, didn't do much
    double scale;
    if (i < 3) scale = 0.1;
    else if (i < 8) scale = 0.3;
    else scale = 0.01;
    */
    absolute += d*d;
  }



  // slope
  double slope = 0;
  for (int i = 0; i < NUM_BRIGHTNESS_BINS-1; i++) {

    double a1 = this->brightness_histogram_[i] / 255;
    double b1 = IRD.brightness_histogram_[i] / 255;
    double a2 = this->brightness_histogram_[i+1] / 255;
    double b2 = IRD.brightness_histogram_[i+1] / 255;

    double slope_a = a2-a1;
    double slope_b = b2-b1;
    double d = slope_a - slope_b;
    //double d = this->brightness_histogram_[i] - IRD.brightness_histogram_[i];
    slope += d*d;
  }

  //std::cout << "ratio:" << absolute / slope << std::endl;
  
  // this ratio of absolute to slope is ok to start...  but may want to change!
  double answer = absolute + 1*slope;

  return answer;
}


void IR_Interpolable_Data::ScaleWithAverage(const IR_Interpolable_Data &avg) {

  for (int i = 0; i < NUM_BRIGHTNESS_BINS; i++) {
    double a = this->brightness_histogram_[i]; 
    double b = avg.brightness_histogram_[i];

    this->brightness_histogram_[i] = a*a/b;
  }


}

#define BLUE_THRESH 40
#define CYAN_THRESH 70
#define GREEN_THRESH 120
#define YELLOW_THRESH 170
#define RED_THRESH 220
#define WHITE_THRESH 255



void IR_Data_Point::write_false_color_led_image(const char* s)  {
  int w = led_image_.getRows();
  int h = led_image_.getCols();
  Image<sRGB> false_color_led_image_ (w,h);
  
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      int p = led_image_(i,j);
      if (p < BLUE_THRESH) {
	int tmp = p*255.0/double(BLUE_THRESH);
	false_color_led_image_(i,j).r() = 0;
	false_color_led_image_(i,j).g() = 0;
	false_color_led_image_(i,j).b() = tmp;
      } else if (p < CYAN_THRESH) {
	int tmp = (p-BLUE_THRESH)*255.0/double(CYAN_THRESH-BLUE_THRESH);
	false_color_led_image_(i,j).r() = 0;
	false_color_led_image_(i,j).g() = tmp;
	false_color_led_image_(i,j).b() = 255;
      } else if (p < GREEN_THRESH) {
	int tmp = (p-CYAN_THRESH)*255.0/double(GREEN_THRESH-CYAN_THRESH);
	false_color_led_image_(i,j).r() = 0;
	false_color_led_image_(i,j).g() = 255;
	false_color_led_image_(i,j).b() = 255-tmp;	
      } else if (p < YELLOW_THRESH) {
	int tmp = (p-GREEN_THRESH)*255.0/double(GREEN_THRESH-YELLOW_THRESH);
	false_color_led_image_(i,j).r() = tmp;
	false_color_led_image_(i,j).g() = 255;
	false_color_led_image_(i,j).b() = 0;
      } else if (p < RED_THRESH) {
	int tmp = (p-YELLOW_THRESH)*255.0/double(YELLOW_THRESH-RED_THRESH);
	false_color_led_image_(i,j).r() = 255;
	false_color_led_image_(i,j).g() = 255-tmp;
	false_color_led_image_(i,j).b() = 0;
      } else {
	int tmp = (p-RED_THRESH)*255.0/double(WHITE_THRESH-RED_THRESH);
	false_color_led_image_(i,j).r() = 255;
	false_color_led_image_(i,j).g() = tmp;
	false_color_led_image_(i,j).b() = tmp;
      }


      //	false_color_led_image_(i,j).r() = p;
      //	false_color_led_image_(i,j).g() = 255;
      //false_color_led_image_(i,j).b() = 255;	    
      //}
    }
  }

  false_color_led_image_.write(s);

}



////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
