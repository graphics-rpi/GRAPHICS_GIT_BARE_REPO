#include "colortoken.h"

#include <iostream>
#include "argparser.h"

extern ArgParser *args;


bool white_bright_compare(const std::pair<double,sRGB> &a, const std::pair<double,sRGB> &b) {
  return  (a.first > b.first);
}


sRGB operator* (double d, sRGB c) { return sRGB(c.r() * d, c.g() * d, c.b() *d); }

ColorToken::ColorToken (std::vector<Point> &edge_points, 
			std::vector<Point> &points, 
			Image<byte> &component_image,
			Image<sRGB> &color_image,
			Quad quad, /*Circle init_circle, */ Histogram &histogram) {
  
  if (!args->find_architectural_design) { throw -1; }

  // average the color of the inside patch
  center = quad.getCenter();
  

  
  std::vector<double> lengths;
  for (int i = 0; i < 4; i++) {
    lengths.push_back(quad.edge_length(i));
    //(*args->output) << "CT " << quad.edge_length(i) << std::endl;
  }
  
  const int target_length = 46;

  std::sort(lengths.begin(),lengths.end());
  
  //(*args->output) << "TOKEN TARGET LENGTH " << (lengths[0] + lengths[3]) * 0.5 << std::endl;

  if (lengths[0] < target_length*0.7 ||
      lengths[3] > target_length*1.3) { 
    /*
      (*args->output) << "quad wrong " << 
      target_length*0.8 << " " <<
      target_length*1.2
      << std::endl; 
    */
    // for (int i = 0; i < 4; i++) {
    //(*args->output) << "CT " << lengths[i] << std::endl;
    //}
    throw -1; }



  std::vector<std::pair<double,sRGB> > values;

  // color patch radius
  const double r = 3.5;
  double r2 = r*r;
  // get the average "white enough" pixels from an annulus surrounding the
  // color swatch
  const double inner_r = 9.;
  const double outer_r = 12.;
  double inner_r2 = inner_r * inner_r;
  double outer_r2 = outer_r * outer_r;
  v3d sum(0.0, 0.0, 0.0);
  //(*args->output) << "sum initial " << sum << std::endl;
  int count = 0;
  v3d white_sum(0.0, 0.0, 0.0);
  int white_count = 0;
  for (int row = int(center.row-outer_r-1); row <= int(center.row+outer_r+1); row++) {
    for (int col = int(center.col-outer_r-1); col <= int(center.col+outer_r+1); col++){

      double rad2 = ((row-center.row)*(row-center.row) +
                     (col-center.col)*(col-center.col));
      v3d v = v3d(color_image(row, col));
      if (rad2 < r2){
        sum += v;
        count++;
      }

      if (rad2 > inner_r2 && rad2 < outer_r2){
	double score = v.r()*v.r() + v.g()*v.g() + v.b()*v.b();

	values.push_back(std::make_pair(score,v));
        //(*args->output) << v.max() << " - " << v.min() << " / " << v.min() << " + " << v.max() << std::endl;

        //(*args->output) << "score " << score << std::endl;
        //if (score < 0.1){
	//if (score < 0.2){
	    //(*args->output) << "white v " << v << std::endl;
	// white_sum += v;
	//white_count++;
        //}
      }
    }
  }
 
  std::sort(values.begin(),values.end(),white_bright_compare);


  for (int i = 0; i < values.size()*0.25; i++) {
    white_sum += values[i].second;
    white_count++;
  }

  white_sum = white_sum/double(white_count);
  sum = sum/double(count);

  sum = v3d(std::min(sum.r(),white_sum.r()),
	    std::min(sum.g(),white_sum.g()),
	    std::min(sum.b(),white_sum.b()));
		     

  sRGB color_a = 255.0 * v3d(sRGBdegamma(sum.r()) / sRGBdegamma(white_sum.r()),
			     sRGBdegamma(sum.g()) / sRGBdegamma(white_sum.g()),
			     sRGBdegamma(sum.b()) / sRGBdegamma(white_sum.b()));
  sRGB color_b = color = sum / white_sum * 255.0;
  double weight = 0.4;
  color = weight * color_a + (1-weight) * color_b;

#if 0
  (*args->output) << "color " << sum << std::endl;
  (*args->output) << "white " << white_sum << std::endl;
  (*args->output) << "normalized token color " << color << std::endl;
#endif 
  
  int peak = histogram.peak_idx();
  type = peak;
  
  //  (*args->output) << "MADE TOKEN " << peak << " type " << type << std::endl;
}
