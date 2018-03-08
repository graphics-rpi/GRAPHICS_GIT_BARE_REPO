#include <vector>
#include "Image.h"

void ColorBalance(Image<sRGB> &image, double lower_percentile, double upper_percentile, bool verbose) {
  int rows = image.getRows();
  int cols = image.getCols();

  std::vector<int> red_histogram(256,0);
  std::vector<int> green_histogram(256,0);
  std::vector<int> blue_histogram(256,0);

  for (int i = 0; i < rows; i++) { 
    for (int j = 0; j < cols; j++) {
      sRGB tmp = image(i,j);
      int red = tmp.r();
      int green = tmp.g();
      int blue = tmp.b();
      assert (red >= 0 && red < 256);
      assert (green >= 0 && green < 256);
      assert (blue >= 0 && blue < 256);
      red_histogram[red]++;
      green_histogram[green]++;
      blue_histogram[blue]++;
      //Pixel
    }
  }

  int red_sum = 0;
  int green_sum = 0;
  int blue_sum = 0;
  int bottom[3] = {0,0,0};
  int top[3] = {0,0,0};
  
  //  double TOP = 0.5;
  //double BOT = 0.2;

  for (int i = 0; i < 256; i++) {
    if (red_sum < lower_percentile * rows * cols) bottom[0] = i;
    if (green_sum < lower_percentile * rows * cols) bottom[1] = i;
    if (blue_sum < lower_percentile * rows * cols) bottom[2] = i;
    // if (red_sum < MID * rows * cols) middle[0] = i;
    //if (green_sum < MID * rows * cols) middle[1] = i;
    //if (blue_sum < MID * rows * cols) middle[2] = i;
    if (red_sum < upper_percentile * rows * cols) top[0] = i;
    if (green_sum < upper_percentile * rows * cols) top[1] = i;
    if (blue_sum < upper_percentile * rows * cols) top[2] = i;
    red_sum += red_histogram[i];
    green_sum += green_histogram[i];
    blue_sum += blue_histogram[i];
  }


  if (verbose) {
    std::cout << "BOTTOM " << bottom[0] << " " << bottom[1] << " " << bottom[2] << std::endl;
    std::cout << "TOP    " << top[0] << " " << top[1] << " " << top[2] << std::endl;
  }

  //std::cout << "MIDDLE " << middle[0] << " " << middle[1] << " " << middle[2] << std::endl;
  //std::cout << "upper_percentile    " << top[0] << " " << top[1] << " " << top[2] << std::endl;


  if (bottom[0] > 200 && bottom[1] > 200 && bottom[2] > 200) {
    std::cerr << "ERROR: looks like the exposure is too long (or aperture too big)" << std::endl;
    throw -1;
  }
  
  if (top[0] < 50 && top[1] < 50 && top[2] < 50) {
    std::cerr << "ERROR: looks like the exposure is too short (or aperture too small)" << std::endl;
    throw -1;
  }

  if (top[0] <= bottom[0] + 10 ||
      top[1] <= bottom[1] + 10 ||
      top[2] <= bottom[2] + 10) {
    std::cerr << "ERROR: bad exposure" << std::endl;
    std::cerr << "BOTTOM " << bottom[0] << " " << bottom[1] << " " << bottom[2] << std::endl;
    std::cerr << "TOP    " << top[0] << " " << top[1] << " " << top[2] << std::endl;
    throw -1;
  }

  int target_bottom = 255 * lower_percentile;
  //  int target_middle = 255 * MID;
  int target_top = 255 * upper_percentile;

  //    std::cout << "TARGET   " << target_bottom << " " << target_middle << " " << target_top << std::endl;
  //std::cout << "TARGET   " << target_bottom << " " << target_top << std::endl;
  
  for (int i = 0; i < rows; i++) { 
    for (int j = 0; j < cols; j++) {
      sRGB tmp = image(i,j);
      int red = tmp.r();
      int green = tmp.g();
      int blue = tmp.b();

      red   = target_bottom + (red  -bottom[0]) * (target_top - target_bottom) / (top[0]-bottom[0]);
      green = target_bottom + (green-bottom[1]) * (target_top - target_bottom) / (top[1]-bottom[1]);
      blue  = target_bottom + (blue -bottom[2]) * (target_top - target_bottom) / (top[2]-bottom[2]);


      if (red < 0) red = 0;
      if (green < 0) green = 0;
      if (blue < 0) blue = 0;
      if (red > 255) red = 255;
      if (green > 255) green = 255;
      if (blue > 255) blue = 255;

      image(i,j) = sRGB(red,green,blue);
    }
  }

  
}
