#include <Image.h>
#include <ImageOps.h>
#include <vector>
#include <algorithm>
#include <limits>
#include <string.h>
#include <iostream>

#include "../../common/MersenneTwister.h"
#include "../../table_top_detection/debugging_images.h"

extern "C"{
//#include <atlas_enum.h>
#include <clapack.h>
}
#define NONE sRGB(0,0,0)
#define BLACK sRGB(0,0,0)
#define RED sRGB(255,0,0)
#define GREEN sRGB(0,255,0)
#define BLUE sRGB(0,0,255)

template <typename pixel_t>
Image<pixel_t> dilate_image(const Image<pixel_t> &in,
                            const Image<pixel_t> &kernel,
                            pixel_t foreground, pixel_t background){
  Image<pixel_t> out(in.getRows(), in.getCols());

  int roff = kernel.getRows()/2;
  int coff = kernel.getCols()/2;

  // !!! optimize this with split loops later
  for (int row=0; row<in.getRows(); row++){
    for (int col=0; col<in.getCols(); col++){
      out(row, col) = background;
      for (int r=0; r<kernel.getRows(); r++){
        for (int c=0; c<kernel.getCols(); c++){
          if (bool(kernel(r,c))){
            int ri = row + r - roff;
            int ci = col + c - coff;
            if (ri >= 0 && ri < in.getRows() &&
                ci >= 0 && ci < in.getCols()){
              if (bool(in(ri, ci))){
                out(row, col) = foreground;
                goto skip;
              }
            }
          }
        }
      }
    skip:
      continue;
    }
  }

  return out;
}

template <typename pixel_t>
Image<pixel_t> erode_image(const Image<pixel_t> &in,
                           const Image<pixel_t> &kernel,
                           pixel_t foreground, pixel_t background){
  Image<pixel_t> out(in.getRows(), in.getCols());

  int roff = kernel.getRows()/2;
  int coff = kernel.getCols()/2;

  // !!! optimize this with split loops later
  for (int row=0; row<in.getRows(); row++){
    for (int col=0; col<in.getCols(); col++){
      out(row, col) = foreground;
      for (int r=0; r<kernel.getRows(); r++){
        for (int c=0; c<kernel.getCols(); c++){
          if (bool(kernel(r,c))){
            int ri = row + r - roff;
            int ci = col + c - coff;
            if (ri >= 0 && ri < in.getRows() &&
                ci >= 0 && ci < in.getCols()){
              if (!bool(in(ri, ci))){
                out(row, col) = background;
                goto skip;
              }
            }
          }
        }
      }
    skip:
      continue;
    }
  }

  return out;
}

template <typename pixel_t>
Image<pixel_t> open_image(const Image<pixel_t> &in,
                          const Image<pixel_t> &kernel,
                          pixel_t foreground, pixel_t background){
  Image<pixel_t> eroded = erode_image(in, kernel, foreground, background);
  return dilate_image(eroded, kernel, foreground, background);
}

template <typename pixel_t>
Image<pixel_t> close_image(const Image<pixel_t> &in,
                           const Image<pixel_t> &kernel,
                           pixel_t foreground, pixel_t background){
  Image<pixel_t> dilated = dilate_image(in, kernel, foreground, background);
  return erode_image(dilated, kernel, foreground, background);
}

template <typename pixel_t>
Image<pixel_t> disk_kernel(pixel_t foreground, pixel_t background, int radius){
  Image<pixel_t> out(2*radius+1, 2*radius+1);

  for (int row=0; row<out.getRows(); row++){
    for (int col=0; col<out.getCols(); col++){
      int r = ( (radius - row) * (radius - row) + 
                (radius - col) * (radius - col) );
      if (r <= radius*radius){
        out(row, col) = foreground;
      } else {
        out(row, col) = background;
      }
    }
  }

  return out;
}

template <typename pixel_t>
Image<pixel_t> largest_component(Image<pixel_t> &in, 
                                 pixel_t foreground, pixel_t background){
  int num_labels;
  Image<int> labels = ConnectedComponents(in, num_labels);

  int mass[num_labels];
  for (int i=1; i<num_labels; i++){
    mass[i] = 0;
  }
  for (int row=0; row<labels.getRows(); row++){
    for (int col=0; col<labels.getCols(); col++){
      mass[labels(row,col)]++;
    }
  }
  int largest_idx = 0;
  int largest_mass = 0;
  for (int i=1; i<num_labels; i++){
    if (mass[i] > largest_mass){
      largest_mass = mass[i];
      largest_idx = i;
    }
  }

  Image<pixel_t> out(labels.getRows(), labels.getCols());
  for (int row=0; row<labels.getRows(); row++){
    for (int col=0; col<labels.getCols(); col++){
      if (labels(row, col) == largest_idx){
        out(row, col) = foreground;
      } else {
        out(row, col) = background;
      }
    }
  }

  return out;
}

template <typename in_pixel_t, typename out_pixel_t>
Image<out_pixel_t> choose_pixels(Image<in_pixel_t> &in, in_pixel_t val, 
                                 out_pixel_t foreground, 
                                 out_pixel_t background){
  Image<out_pixel_t> out(in.getRows(), in.getCols());
  for (int row=0; row<in.getRows(); row++){
    for (int col=0; col<in.getCols(); col++){
      if (in(row, col) == val){
        out(row, col) = foreground;
      } else {
        out(row, col) = background;
      }
    }
  }

  return out;
}

template <typename image_pixel_t, typename mask_pixel_t>
Image<image_pixel_t> mask_image(Image<image_pixel_t> &in, 
                                Image<mask_pixel_t> &mask,
                                image_pixel_t background){
  Image<image_pixel_t> out(in.getRows(), in.getCols());
  for (int row=0; row<in.getRows(); row++){
    for (int col=0; col<in.getCols(); col++){
      if (bool(mask(row,col))){
        out(row, col) = in(row, col);
      } else {
        out(row, col) = background;
      }
    }
  }
  return out;
}

Image<byte> adaptive_thresh(Image<byte> &in, int window, double ratio, 
                            bool adj_edges){
  Image<byte> out(in.getRows(), in.getCols());
  int hist[256];
  int width = window;
  int skip = 2;
  for (int row=0; row<in.getRows(); row++){
    for (int col=0; col<in.getCols(); col++){
      for (int i=0; i<256; i++){
        hist[i] = 0;
      }
      int min_row = max(row-width, 0);
      int max_row = min(row+width, in.getRows()-1);
      int min_col = max(col-width, 0);
      int max_col = min(col+width, in.getCols()-1);
      for (int r=min_row; r<=max_row; r+=skip){
        for (int c=min_col; c<max_col; c+=skip){
          hist[in(r,c)]++;
        }
      }

      int total = 0;
      for (int i=0; i<256; i++){
        total += hist[i];
      }

      // assume edges had darkest value
      if (adj_edges){
        hist[0] += ((2*width+1)/skip)*((2*width+1)/skip) - total;
      }

      int sum_thresh = int(ratio*total);
      int sum = 0;
      int thresh = 0;
      for (int i=0; i<256; i++){
        sum += hist[i];
        if (sum > sum_thresh){
          thresh = i;
          break;
        }
      }
       
      thresh = max(thresh, 20);
      if (in(row,col) >= thresh){
        out(row, col) = 0;
      } else {
        out(row, col) = 255;
      }
    }
  }
   
  return out;
}

struct blob_mass {
  int id;
  int mass;
  static bool compare(const blob_mass &a, const blob_mass &b){
    return a.mass < b.mass;
  }
};

// implement an order-statistic filter on blob masses
Image<byte> median_mass(Image<int> &in, int num_labels, int num_targets){
  std::vector<blob_mass> mass;
  mass.resize(num_labels);
  for (int i=0; i<num_labels; i++){
    mass.at(i).id = i;
    mass.at(i).mass = 0;
  }
  for (int row=0; row<in.getRows(); row++){
    for (int col=0; col<in.getCols(); col++){
      mass.at(in(row,col)).mass++;
    }
  }
  std::sort(mass.begin(), mass.end(), blob_mass::compare);
  
  // search for the set of N-3 blobs with the lowest mass standard deviation
  int num_half_targets = num_targets - 3;
  double lowest_sigma = std::numeric_limits<double>::infinity();
  int best_start = 0;
  for (int start=0; start<num_labels-num_half_targets; start++){
    double mean = 0.;
    for (int i=0; i<num_half_targets; i++){
      mean += mass.at(i+start).mass;
    }
    mean /= num_half_targets;
    double sigma = 0.;
    for (int i=0; i<num_half_targets; i++){
      sigma += (mean - mass.at(i+start).mass) * (mean - mass.at(i+start).mass);
    }
    sigma /= num_half_targets;
    if (sigma < lowest_sigma){
      lowest_sigma = sigma;
      best_start = start;
    }
  }

  std::vector<int> valid_target(num_labels);
  for (int i=0; i<num_labels; i++){
    if (i >= best_start && i < (best_start + num_targets)){
      valid_target[mass.at(i).id] = 1;
    } else {
      valid_target[mass.at(i).id] = 0;
    }
  }

  // kill blobs not in chosen weight range
  Image<byte> out(in.getRows(), in.getCols());
  for (int row=0; row<in.getRows(); row++){
    for (int col=0; col<in.getCols(); col++){
      if (valid_target[in(row,col)]){
	out(row, col) = 255;
      } else {
	out(row, col) = 0;
      }
    }
  }

  return out;
}

struct blob_stats {
  double ctr_row;
  double ctr_col;
  double r, g, b;
  int mass;
  int min_row, max_row;
  int min_col, max_col;
  int logical_row;
  int logical_col;
  bool valid;
  // sub-pixel center estimate
  double image_x;
  double image_y;
  // known position (meters)
  double true_x;
  double true_y;
};

struct BlobSorter {
  BlobSorter(int logical_cols){
    this->logical_cols = logical_cols;
  }
  bool operator()(const blob_stats &a, const blob_stats &b){
    return (a.logical_row*logical_cols + a.logical_col) < 
      (b.logical_row*logical_cols + b.logical_col);
  }
private:
  int logical_cols;
};

std::vector<blob_stats> collect_stats(Image<sRGB> &rgb, 
				      Image<int> &labels, int num_labels){
  std::vector<blob_stats> stats;
  stats.resize(num_labels);

  for (int i=0; i<num_labels; i++){
    stats.at(i).mass = 0;
    stats.at(i).ctr_row = 0.;
    stats.at(i).ctr_col = 0.;
    stats.at(i).logical_row = -1;
    stats.at(i).logical_col = -1;
    stats.at(i).r = 0;
    stats.at(i).g = 0;
    stats.at(i).b = 0;
    stats.at(i).valid = false;
    stats.at(i).min_row = std::numeric_limits<int>::max();
    stats.at(i).max_row = 0;
    stats.at(i).min_col = std::numeric_limits<int>::max();
    stats.at(i).max_col = 0;
  }

  for (int row=0; row<labels.getRows(); row++){
    for (int col=0; col<labels.getCols(); col++){
      stats.at(labels(row,col)).mass++;
      stats.at(labels(row,col)).ctr_row += row;
      stats.at(labels(row,col)).ctr_col += col;
      stats.at(labels(row,col)).r += rgb(row,col).r();
      stats.at(labels(row,col)).g += rgb(row,col).g();
      stats.at(labels(row,col)).b += rgb(row,col).b();
      if (row > stats.at(labels(row, col)).max_row){
	stats.at(labels(row, col)).max_row = row;
      }
      if (row < stats.at(labels(row, col)).min_row){
	stats.at(labels(row, col)).min_row = row;
      }
      if (col > stats.at(labels(row, col)).max_col){
	stats.at(labels(row, col)).max_col = col;
      }
      if (col < stats.at(labels(row, col)).min_col){
	stats.at(labels(row, col)).min_col = col;
      }
    }
  }
  
  for (int i=0; i<num_labels; i++){
    stats.at(i).ctr_row /= stats.at(i).mass;
    stats.at(i).ctr_col /= stats.at(i).mass;
    stats.at(i).r /= stats.at(i).mass;
    stats.at(i).g /= stats.at(i).mass;
    stats.at(i).b /= stats.at(i).mass;
  }
  
  return stats;
}

void find_corners(std::vector<blob_stats> &stats, 
		  int &red_id, int &green_id, int &blue_id, int &purple_id){

  //  std::cout << "find corners " << stats.size() << std::endl;
  double most_sat;
#define AVG(x,y) ((x+y)/2)
  //#define AVG(x,y) (max(x,y))
  red_id = 0;
  most_sat = 0;
  for (unsigned int i=1; i<stats.size(); i++){
    double sat = stats.at(i).r / AVG(stats.at(i).g, stats.at(i).b);
    if (sat > most_sat){
      most_sat = sat;
      red_id = i;
    }
  }
  /*
  std::cout << "BEST RED " << red_id << " " << most_sat << std::endl;
  std::cout << stats.at(red_id).r << "," 
	    << stats.at(red_id).g << "," 
	    << stats.at(red_id).b << std::endl;
  std::cout << "MASS " << stats.at(red_id).mass << std::endl;
  */
  green_id = 0;
  most_sat = 0;
  for (unsigned int i=1; i<stats.size(); i++){
    double sat = stats.at(i).g / AVG(stats.at(i).r, stats.at(i).b);
    if (sat > most_sat && (int)i != red_id){
      most_sat = sat;
      green_id = i;
    }
  }
  /*
  std::cout << "BEST GREEN " << green_id << " " << most_sat << std::endl;
  std::cout << stats.at(green_id).r << "," 
	    << stats.at(green_id).g << "," 
	    << stats.at(green_id).b << std::endl;
  std::cout << "MASS " << stats.at(green_id).mass << std::endl;
  */
  blue_id = 0;
  most_sat = 0;
  for (unsigned int i=1; i<stats.size(); i++){
    double sat = stats.at(i).b / max(stats.at(i).r, stats.at(i).g);
    //double satb = stats.at(i).b / stats.at(i).r;
    //double sat = (sata + satb) / 2.0;
    if (sat > most_sat && (int)i != red_id && (int)i != green_id){
      most_sat = sat;
      blue_id = i;
    }
  }
  /*
  std::cout << "BEST BLUE " << blue_id << " " << most_sat << std::endl;
  std::cout << stats.at(blue_id).r << "," 
	    << stats.at(blue_id).g << "," 
	    << stats.at(blue_id).b << std::endl;
  std::cout << "MASS " << stats.at(blue_id).mass << std::endl;
  */
  purple_id = 0;
  most_sat = 0;
  for (unsigned int i=1; i<stats.size(); i++){
    double sat = min(stats.at(i).r, stats.at(i).b)/stats.at(i).g;
    if (sat > most_sat && (int)i != red_id && (int)i != green_id && (int)i != blue_id){
      most_sat = sat;
      purple_id = i;
    }
  }
  /*
  std::cout << "BEST PURPLE " << purple_id << " " << most_sat << std::endl;
  std::cout << stats.at(purple_id).r << "," 
	    << stats.at(purple_id).g << "," 
	    << stats.at(purple_id).b << std::endl;
  std::cout << "MASS " << stats.at(purple_id).mass << std::endl;
  */
}
/*
BEST RED 1164 2.79108
226.053,95.386,66.5965
BEST GREEN 4204 1.16097
134.454,161.205,143.253
BEST BLUE 29577 2.81724
71.2036,89.7844,226.771
BEST PURPLE 29338 1.38573
144.611,104.358,180.969
*/


int SGN(int x) {return (x)<0?-1:(x)>0?1:0; }
// use Bresenham's algorithm to apply a point functor to
//  step along line until we hit the next blob
//  updates r2, c2 to be point where search left off
int FindNextBlob(Image<sRGB> &scan_image, Image<int> &labels,  
		 int &r2, int &c2, int r1, int c1, 
		 int current_label){

  if (r1 == r2 && c1 == c2){
    return -1;
  }

  int row, col, err;
  int dr = r1 - r2;
  int dc = c1 - c2;
  int rstep = SGN(dr);
  int cstep = SGN(dc);
  dr = abs(dr);
  dc = abs(dc);
  if (dr > dc){
    col = c2;
    err = - dr>>1;
    for (row=r2; row!=r1; row+=rstep){
      int label = labels(row,col);
      scan_image(row,col) = sRGB(0,255,0);
      if (label != 0 && label != current_label){
	r2 = row;
	c2 = col;
	return label;
      }

      err += dc;
      if (err > 0){
	col += cstep;
	err -= dr;
      }
    }
  } else {
    row = r2;
    err = - dc>>1;
    for (col=c2; col!=c1; col+=cstep){
      int label = labels(row,col);
      scan_image(row,col) = sRGB(0,255,0);
      if (label != 0 && label != current_label){
	r2 = row;
	c2 = col;
	return label;
      }

      err += dr;
      if (err > 0){
	row += rstep;
	err -= dc;
      }
    }
  }

  return -1;
}

// use Bresenham's algorithm to apply a point functor to
//  step along line finding a sequence of blobs
std::vector<int> FindBlobRow(Image<sRGB> &scan_image, Image<int> &labels,  
			     int r2, int c2, int r1, int c1, 
			     int start_label){
  std::vector<int> sequence;
  sequence.push_back(start_label);
  int current_label = start_label;

  if (r1 == r2 && c1 == c2){
    return sequence;
  }

  int row, col, err;
  int dr = r1 - r2;
  int dc = c1 - c2;
  int rstep = SGN(dr);
  int cstep = SGN(dc);
  dr = abs(dr);
  dc = abs(dc);
  if (dr > dc){
    col = c2;
    err = - dr>>1;
    for (row=r2; row!=r1; row+=rstep){
      int label = labels(row,col);
      scan_image(row,col) = sRGB(255,255,0);
      if (label != 0 && label != current_label){
	sequence.push_back(label);
	current_label = label;
      }

      err += dc;
      if (err > 0){
	col += cstep;
	err -= dr;
      }
    }
  } else {
    row = r2;
    err = - dc>>1;
    for (col=c2; col!=c1; col+=cstep){
      int label = labels(row,col);
      scan_image(row,col) = sRGB(255,255,0);
      if (label != 0 && label != current_label){
	sequence.push_back(label);
	current_label = label;
      }

      err += dr;
      if (err > 0){
	row += rstep;
	err -= dc;
      }
    }
  }

  return sequence;
}

template <class Pixel>
void GaussianBlurImage(Image<Pixel> &image, double sigma){
  double epsilon = 1e-3;
  int half_width = int(ceil(sqrt(-sigma*sigma*log(epsilon))));
  int width = 2*half_width+1;
  double *kernel = new double[width];
  Image<Pixel> temp(image.getRows(), image.getCols());

  double total = 0.;
  for (int i=0;i<width;i++){
    double x = half_width - double(i);
    kernel[i] = exp(-x*x/(sigma*sigma));
    total += kernel[i];
  }
  for (int i=0;i<width;i++){
    kernel[i] /= total;
  }

  // blur horizontally
  // not very efficient
  for (int row=0; row<image.getRows(); row++){
    for (int col=0; col<image.getCols(); col++){
      Pixel sum(0);
      for (int i=0; i<width; i++){
	int c = col + i - half_width;
	if (c < 0) c = 0;
	if (c > image.getCols()-1) c = image.getCols()-1;
	sum += kernel[i] * image(row,c);
      }
      temp(row,col) = sum;
    }
  }

  // blur vertically
  // not very efficient
  for (int col=0; col<image.getCols(); col++){
    for (int row=0; row<image.getRows(); row++){
      Pixel sum(0);
      for (int i=0; i<width; i++){
	int r = row + i - half_width;
	if (r < 0) r = 0;
	if (r > image.getRows()-1) r = image.getRows()-1;
	sum += kernel[i] * temp(r,col);
      }
      image(row,col) = sum;
    }
  }

  delete [] kernel;
}

// fit a quadratic function to image patch to find saddle point in center
void find_patch_center(Image<byte> &input_patch, 
		       double &center_x, double &center_y){
  int center_row = input_patch.getRows()/2;
  int center_col = input_patch.getCols()/2;

  // Gaussian blur image patch (as doubles)
  Image<double> patch(input_patch.getRows(), input_patch.getCols());
  for (int row=0; row<patch.getRows(); row++){
    for (int col=0; col<patch.getCols(); col++){
      patch(row,col) = input_patch(row,col) / 255.;
    }
  }
  double sigma = 4. * input_patch.getRows() / 14.;
  //double sigma = 5. * input_patch.getRows() / 14.;
  GaussianBlurImage(patch, sigma);

  // write blurred patches out for debugging
  for (int row=0; row<patch.getRows(); row++){
    for (int col=0; col<patch.getCols(); col++){
      input_patch(row,col) = int(patch(row,col) * 255.);
    }
  }
  static int filenum;
  char filename[256];
  snprintf(filename, 256, "patch%02d.ppm", filenum++);
  //input_patch.write(filename);

  // fit quadratic to image intensities  
  double **A;
  A = new double*[6];
  A[0] = new double[6*6];
  for (int i=1; i<6; i++){
    A[i] = A[i-1] + 6;
  }

  double b[6];
  for (int i=0; i<6; i++){
    for (int j=0; j<6; j++){
      A[i][j] = 0.;
    }
    b[i] = 0.;
  }

  double r2 = min(patch.getRows()/2., patch.getCols()/2.);
  r2 = r2 * r2;
  for (int row=0; row<patch.getRows(); row++){
    double y = row - center_row;
    double y2 = y * y;
    double y3 = y2 * y;
    double y4 = y2 * y2;
    for (int col=0; col<patch.getCols(); col++){
      double x = col - center_col;
      double x2 = x * x;
      double x3 = x2 * x;
      double x4 = x2 * x2;
      if (x*x+y*y <= r2){ // skip circular patches; they don't work well

        A[0][0] += x4;
        A[0][1] += x3 * y;
        A[1][0] += x3 * y;
        A[0][2] += x2 * y2;
        A[2][0] += x2 * y2;
        A[0][3] += x3;
        A[3][0] += x3;
        A[0][4] += x2 * y;
        A[4][0] += x2 * y;
        A[0][5] += x2;
        A[5][0] += x2;
        
        A[1][1] += x2 * y2;
        A[1][2] += x * y3;
        A[2][1] += x * y3;
        A[1][3] += x2 * y;
        A[3][1] += x2 * y;
        A[1][4] += x * y2;  
        A[4][1] += x * y2;
        A[1][5] += x * y;
        A[5][1] += x * y;

        A[2][2] += y4;
        A[2][3] += x * y2;
        A[3][2] += x * y2;
        A[2][4] += y3;
        A[4][2] += y3;
        A[2][5] += y2;
        A[5][2] += y2;
      
        A[3][3] += x2;
        A[3][4] += x * y;
        A[4][3] += x * y;
        A[3][5] += x;
        A[5][3] += x;
        
        A[4][4] += y2;
        A[4][5] += y;
        A[5][4] += y;
        A[5][5] += 1.;

        double I = patch(row, col);
        b[0] += I * x2;
        b[1] += I * x * y;
        b[2] += I * y2;
        b[3] += I * x;
        b[4] += I * y;
        b[5] += I;
      }
    }
  }

  int ipiv[6];
  int info = clapack_dgesv(CblasRowMajor, 6, 1, A[0], 6, ipiv, b, 6);
  if (info != 0) FATAL_ERROR("dgesv() failure with error %d", info);

  // now, find saddle point
  double M[4];
  double c[2];  
  M[0] = 2. * b[0];
  M[1] = b[1];
  M[2] = b[1];
  M[3] = 2. * b[2];
  c[0] = -b[3];
  c[1] = -b[4];
  info = clapack_dgesv(CblasRowMajor, 2, 1, M, 2, ipiv, c, 2);
  if (info != 0) FATAL_ERROR("dgesv() failure with error %d", info);

  center_x = c[0];
  center_y = c[1];

  delete [] A[0];
  delete [] A;
}

extern "C"{
  int dgesvd_(const char* jobu, const char* jobvt, const int* M, const int* N,
	      double* A, const int* lda, double* S, double* U, const int* ldu,
	      double* VT, const int* ldvt, double* work,const int* lwork, const
	      int* info);
}

class Camera {
public:

  // read parameters from a file (intrinsics, extrinsics, distortion, rows, cols, etc)


  // add function to undo radial distortion in an image here (it calls bilinear)

  void loadCalibration(const char *int_filename, const char *ext_filename){
    FILE *fp = fopen(int_filename, "rt");
    if (NULL == fp){
      FATAL_ERROR("cannot open %s\n", int_filename);
    }
    for (int i=0; i<9; i++){
      fscanf(fp, "%lf", &Intrinsics[i]);
    }
    fclose(fp);

    fp = fopen(ext_filename, "rt");
    if (NULL == fp){
      FATAL_ERROR("cannot open %s\n", ext_filename);
    }
    for (int i=0; i<12; i++){
      fscanf(fp, "%lf", &Extrinsics[i]);
    }
    fclose(fp);

    for (int r=0; r<3; r++){
      for (int c=0; c<4; c++){
        Projection[r*4+c] = 0.;
        for (int k=0;k<3;k++){
          Projection[r*4+c] += Intrinsics[r*3+k] * Extrinsics[k*4+c];
        }
      }
    }
    
    calculate_center();
    calculate_pinvProjection();

  }

  v3d PointFromPixel(int row, int col, double height){
    v3d origin = center;
    v3d point = point_from_pixel(row, col);    
    v3d direction = point - origin; 
    direction.normalize();
    
    const double epsilon = 1e-6;
    if (fabs(direction.z()) < epsilon){
      FATAL_ERROR("View orthogonal to z-axis");
    }
    double l = (height - origin.z()) / direction.z();
    
    return origin + l * direction;
  }

private:
  // note: all stored row-order
  // also note: data is redundant here; make sure it's all in synch
  double u0, v0;
  double rows, cols;
  double Intrinsics[9];
  double Extrinsics[12];
  double Projection[12];
  double pinvProjection[12]; // pseudo-inverse of projection matrix
  v3d center;

  // return a 3d point (not the camera center) on the ray through the
  // given image pixel
  v3d point_from_pixel(int row, int col){
    // !!! do these need to be normalized???
    double x[3] = {(double)col, (double)row, 1.};
    double X[4] = {0., 0., 0., 0.};

    for (int i=0; i<4; i++){
      for (int j=0; j<3; j++){
	X[i] += pinvProjection[i*3+j] * x[j];
      }
    }

    for (int i=0; i<3; i++){
      X[i] /= X[3];
    }

    return v3d(X[0], X[1], X[2]);    
  }
  
  void calculate_pinvProjection(){
    // calculate P P^T
    double PPT[9];
    for (int i=0;i<3; i++){
      for (int j=0;j<3;j++){
	PPT[i*3+j] = 0.;
	for (int k=0;k<4;k++){
	  PPT[i*3+j] += Projection[i*4+k] * Projection[j*4+k];
	}
      }
    }

    // invert to get (P P^T)^-1
    int info;
    int ipiv[3];
    info = clapack_dgetrf(CblasRowMajor, 3, 3, PPT, 3, ipiv);
    if (0 != info){
      FATAL_ERROR("clapack_dgetrf() returned %d", info);
    }    
    info = clapack_dgetri(CblasRowMajor, 3, PPT, 3, ipiv);
    if (0 != info){
      FATAL_ERROR("clapack_dgetri() returned %d", info);
    }

    // mutliply to get P^T (P P^T)^-1, i.e. P^+
    for (int i=0; i<4; i++){
      for (int j=0; j<3; j++){
	pinvProjection[i*3 + j] = 0.;
	for (int k=0; k<3; k++){
	  pinvProjection[i*3 + j] += Projection[i+4*k] * PPT[k*3+j];
	}
      }
    }
  }

  // use SVD to get the nullspace of P; this is the camera center
  void calculate_center(){

    // transpose P into A since LAPACK is FORTRAN (column-major)
    double A[12];
    for (int i=0; i<4; i++){
      for (int j=0; j<3; j++){
	A[i*3 + j] = Projection[j*4 + i];
      }
    }
    
    // call LAPACK FORTRAN SVD routine
    const char *jobu = "A";
    const char *jobvt = "A";
    int M = 3;
    int N = 4;
    int lda = 3;
    double U[9];
    double S[4];
    double VT[16];
    int ldu = 3;
    int ldvt = 4;
    int lwork = max(1, max( 3*min(N,M)+max(M,N), 5*min(N,M)));
    double *work = new double[lwork];
    int info;
    dgesvd_(jobu, jobvt, &M, &N, A, &lda,
	    S, U, &ldu, VT, &ldvt, work, &lwork, &info);
    if (0 != info){
      FATAL_ERROR("dgesvd() returned %d", info);
    }
    // not sure this is the right place to delete... ?
    delete [] work;

    // de-homogenize point
    double x = VT[0*4+3];
    double y = VT[1*4+3];
    double z = VT[2*4+3];
    double w = VT[3*4+3];
    x /= w;
    y /= w;
    z /= w;
    center = v3d(x, y, z);
  }
};

struct point
{
  double row;
  double col;
  bool inlier;
  point(){}
  point(double row, double col){this->row = row; this->col = col;}
  void dump() {std::cout << row << " " << col << std::endl; /*printf("%lf %lf\n", row, col);*/ }
};

// ax + by + c = 0
// note x = col; y = row
struct line
{
  double a;
  double b;
  double c;
  line(){};
  line(double a, double b, double c)
  {
    this->a = a;
    this->b = b;
    this->c = c;
    normalize();
  };
  void normalize()
  {
    double d = sqrt(a*a+b*b);
    a /= d;
    b /= d;
    c /= d;
  };
  void dump() {std::cout << a << " " << b << " " << c << std::endl; /*printf("%lf %lf %lf\n", a, b, c);*/}
};

// construct a line of the form ax + by + c = 0 to two points
// not as easy as it would first appear
line two_point_line(const point& p1, const point &p2)
{
  double a, b, c;
  double x1 = p1.col;
  double y1 = p1.row;
  double x2 = p2.col;
  double y2 = p2.row;
  
  if (x1 == 0){
    if (x2 == 0){
      return line(1.0, 0.0, 0.0); // x = 0
    } else {
      return two_point_line(p2, p1);
    }
  }

  if (y1 == 0 && y2 == 0){
    return line(0.0, 1.0, 0.0); // y = 0
  }

  double denom = (y2 - y1*x2/x1);
  if (denom != 0){
    c = 1.0;
    b = (x2/x1 - 1)/denom;
    a = -(b*y1 + c)/x1;
  } else {
    // line passes through 0,0
    c = 0.0;
    b = 1.0;
    a = -b*y1/x1;
  }

  return line(a,b,c);
}

// assumes normalized line
// aborts if a bad point is found
double check_points(line &l, std::vector<point> &points, double thresh)
{
  std::vector<point>::iterator pt;
  double max_d = 0.;

  for (pt = points.begin(); pt != points.end(); ++pt){
    double d = fabs(l.a * pt->col + l.b * pt->row + l.c);
    //printf("%f\n", d);
    if (d > thresh){
      FATAL_ERROR("Bad point found (dist = %f)", d);
    }
    if (d > max_d) max_d = d;
  }
  return max_d;
} 

struct vector2
{
  double x;
  double y;
  vector2(){};
  vector2(double x, double y){this->x = x; this->y = y;};
  void normalize(){
    double len = sqrt(x*x+y*y);
    x /= len;
    y /= len;
  }
};

// a b
// c d
struct matrix2x2
{
  double a, b, c, d; 
  matrix2x2(double a, double b, double c, double d)
  {
    this->a = a;
    this->b = b;
    this->c = c;
    this->d = d;
  };
  double trace(){return a+d;};
  double det(){return a*d-b*c;};
  double l1() // eigenvalue (not necessarily leading one !!!)
  {
    return trace()/2 + sqrt(trace()*trace()/4 - det());
  };  
  double l2() // eigenvalue
  {
    return trace()/2 - sqrt(trace()*trace()/4 - det());
  };
  vector2 evect1() // eigenvector for l1()
  {
    if (c != 0){
      return vector2(l1()-d, c);
    } else if (b != 0){
      return vector2(b, l1()-d);
    } else {
      return vector2(0.0, 1.0);
    }
  };
  vector2 evect2() // eigenvector for l2()
  {
    if (c != 0){
      return vector2(l2()-d, c);
    } else if (b != 0){
      return vector2(b, l2()-d);
    } else {
      return vector2(1.0, 0.0);
    }
  };  
  matrix2x2 inverse()
  {
    double ia, ib, ic, id;
    double dt = det();
    ia =  d / dt;
    ib = -b / dt;
    ic = -c / dt;
    id =  a / dt;
    return matrix2x2(ia, ib, ic, id);
  };
  vector2 operator*(const vector2 &v)
  {
    vector2 temp;
    temp.x = a * v.x + b * v.y;
    temp.y = c * v.x + d * v.y;
    return temp;
  };
};

line fit_line(std::vector<point> &points)
{
  std::vector<point>::iterator pt;
  vector2 m(0.0, 0.0);
  int n = points.size();

  for (pt = points.begin(); pt != points.end(); ++pt){
    m.x += pt->col;
    m.y += pt->row;
  }
  m.x /= double(n);
  m.y /= double(n);

  double sxx = 0.0;
  double sxy = 0.0;
  double syy = 0.0;
  for (pt = points.begin(); pt != points.end(); ++pt){
    sxx += (pt->col - m.x) * (pt->col - m.x);
    sxy += (pt->col - m.x) * (pt->row - m.y);
    syy += (pt->row - m.y) * (pt->row - m.y);
  }

  matrix2x2 cov(sxx/n, sxy/n, sxy/n, syy/n);
  vector2 v;
  if (fabs(cov.l1()) > fabs(cov.l2())){
    v = cov.evect1(); 
  } else {
    v = cov.evect2(); 
  }
  
  return two_point_line(point(m.y, m.x), point(m.y+v.y, m.x+v.x));
}

// fit lines to each row and column of centers, and test detected centers
//  against this line.
void validate_centers(std::vector<blob_stats> &stats,
                      int logical_rows, int logical_cols,
                      double thresh){

  double max_dist = 0.;
  for (int row=0; row<logical_rows; row++){
    std::vector<point> points;
    for (unsigned int i=0; i<stats.size(); i++){
      if (stats.at(i).valid && stats.at(i).logical_row == row){
        points.push_back(point(stats.at(i).image_y, stats.at(i).image_x));
      }
    }
    line l = fit_line(points);
    double dist = check_points(l, points, thresh);
    if (dist > max_dist) max_dist = dist;
  }

  for (int col=0; col<logical_cols; col++){
    std::vector<point> points;
    for (unsigned int i=0; i<stats.size(); i++){
      if (stats.at(i).valid && stats.at(i).logical_col == col){
        points.push_back(point(stats.at(i).image_y, stats.at(i).image_x));
      }
    }
    line l = fit_line(points);
    double dist = check_points(l, points, thresh);
    if (dist > max_dist) max_dist = dist;
  }

  fprintf(stderr, "max dist = %f\n", max_dist);
}


Image<byte> find_saturated_pixels(Image<sRGB> &rgb){
  Image<byte> saturated(rgb.getRows(), rgb.getCols());
  for (int row=0; row<rgb.getRows(); row++){
    for (int col=0; col<rgb.getCols(); col++){
      int mx = rgb(row,col).max();
      int mn = rgb(row,col).min();
      /*  commented out, unused
      double l = (mx + mn)/2.;
      double sat;
      if (l <= 127){
        sat = (mx - mn) / (2.*l);
      } else {
        sat = (mx - mn) / (2. - 2.*l);
      }
      */
      //      double sat_thresh = 0.1;
      //      if (sat > sat_thresh){
      if (double(mx)/mn > 2){
      //if (double(mx)/mn > 1.5){
        saturated(row,col) = byte(255);
      } else {
        saturated(row,col) = byte(0);
      }
    }
  }
  return saturated;
}

Image<byte> or_image(Image<byte> &a, Image<byte> &b){
  Image<byte> out(b.getRows(), b.getCols());
  for (int row=0; row<b.getRows(); row++){
    for (int col=0; col<b.getCols(); col++){
      out(row, col) = a(row, col) | b(row,col);
    }
  }
  return out;
}


void quick_white_balance(Image<sRGB> &in){
  v3d sum(0., 0., 0.);
  int count =0;
  v3d max_pix(0., 0., 0.);
  for (int row=0; row<in.getRows(); row++){
    for (int col=0; col<in.getCols(); col++){
      sRGB &tmp = in(row,col);
      v3d pix(tmp.r(),tmp.g(),tmp.b());
      //= convertVector3(in(row,col), 0.);
      double score = (pix.max() - pix.min())/(pix.max() + pix.min());
      if (score < 0.25 ){
	sum += pix;
	count++;
      }
      max_pix = max(max_pix, pix);
    }
  }
  sum /= v3d(count);
  sum.normalize();

  double scale = 255./(max_pix/sum).max();

  for (int row=0; row<in.getRows(); row++){
    for (int col=0; col<in.getCols(); col++){
      sRGB &tmp = in(row,col);
      v3d pix(tmp.r(),tmp.g(),tmp.b());
      pix *= scale;
      //v3d pix = scale * convertVector3(in(row,col), 0.);
      pix /= sum;
      //in(row,col) = convertVector3(pix, byte(0));
      tmp.r() = pix.r();
      tmp.g() = pix.g();
      tmp.b() = pix.b();
    }
  }
}

void normalize_colors(Image<sRGB> &in){
  for (int row=0; row<in.getRows(); row++){
    for (int col=0; col<in.getCols(); col++){
      sRGB &tmp = in(row,col);
      v3d pix(tmp.r(),tmp.g(),tmp.b());
      //      v3d pix = convertVector3(in(row,col), 0.);
      //pix = (pix - pix.min())/pix.max();
      pix.normalize();
      //      in(row,col) = convertVector3(255.*pix, byte(0));
      tmp.r() = 255*pix.r();
      tmp.g() = 255*pix.g();
      tmp.b() = 255*pix.b();
   }
  }
}

int main(int argc, char **argv){
  Image<sRGB> input(argv[1]);
  double target_spacing = 0.025; // read from command line

  int logical_rows;
  int logical_cols;
  double inlier_thresh;
  double patch_size_factor;
  double thresh_ratio;
  int window;
  bool adj_edges;

  if (!strcmp(argv[2], "camera")){
    logical_rows = 12;
    logical_cols = 18;
    inlier_thresh = 4.0;
    patch_size_factor = 3.5;
    patch_size_factor = 8.0;
    thresh_ratio = 0.29;
    thresh_ratio = 0.25;
    //window = 25;  // old gigE camera
    window = 50;  // new gigE camera (maybe bigger than necessary)
    adj_edges = false;
  } else if (!strcmp(argv[2], "projector")){
    logical_rows = 12;
    logical_cols = 16;
    inlier_thresh = 6.0;
    patch_size_factor = 3.0;
    thresh_ratio = 0.65;
    window = 100;
    adj_edges = true;
  } else if (!strcmp(argv[2], "slr")){
    logical_rows = 12;
    logical_cols = 18;
    inlier_thresh = 3.0;
    patch_size_factor = 3.9;
    patch_size_factor = 3.5;
    //    thresh_ratio = 0.15;
    thresh_ratio = 0.23;
    window = 100;
    adj_edges = false;
  } else {
    FATAL_ERROR("unknown target type\n");
  }

  Image<byte> gray = sRGBtoGray(input);
  gray.write("gray.pgm");
  Image<byte> thresh = adaptive_thresh(gray, window, thresh_ratio, adj_edges);
  thresh.write("thresh.pgm");
  quick_white_balance(input);
  input.write("white_balanced.ppm");
  normalize_colors(input);
  input.write("normalized.ppm");

  Image<byte> kernel3 = disk_kernel(byte(255), byte(0), 1);
  Image<byte> closed = close_image(thresh, kernel3, byte(255), byte(0));
  closed.write("closed.pgm");
  Image<byte> sat_pixels = find_saturated_pixels(input);
  sat_pixels.write("saturated_pixels.pgm");


  Image<byte> blobs = or_image(closed, sat_pixels);

  blobs = closed;


  blobs.write("blobs.pgm");
  int num_labels;
  Image<int> labels = ConnectedComponents(blobs, num_labels);


  // find stats of targets
  std::vector<blob_stats> stats = collect_stats(input, labels, num_labels);

  MTRand mtrand;
  CreateLabelsImage(labels,"labels.ppm",mtrand);

  // cull tiny  and huge blobs
  int tiny_blob_thresh = 50;
  int huge_blob_thresh = 10000;
  for (int row=0; row<labels.getRows(); row++){
    for (int col=0; col<labels.getCols(); col++){
      if (stats.at(labels(row,col)).mass < tiny_blob_thresh ||
	  stats.at(labels(row,col)).mass > huge_blob_thresh){
	labels(row,col) = 0;
	if (stats.at(labels(row,col)).mass < 100000) {
	  //std::cerr << "discarded blob" << stats.at(labels(row,col)).mass << std::endl;
	}
      } else {

      }
    }
  }
  



  // re-find stats of targets
  stats = collect_stats(input, labels, num_labels);

  // identify the corner targets
  int red_id, green_id, blue_id, purple_id;
  find_corners(stats, red_id, green_id, blue_id, purple_id);

  // debug: show found corner targets
  Image<sRGB> output(input.getRows(), input.getCols());
  for (int row=0; row<labels.getRows(); row++){
    for (int col=0; col<labels.getCols(); col++){
      if (labels(row,col) == red_id){
	output(row,col) = sRGB(255, 0, 0);
      } else if (labels(row,col) == green_id){
	output(row,col) = sRGB(0, 255, 0);
      } else if (labels(row,col) == blue_id){
	output(row,col) = sRGB(0, 0, 255);
      } else if (labels(row,col) == purple_id){
	output(row,col) = sRGB(255, 0, 255);
      } else {
	output(row,col) = input(row,col)/(unsigned char)2;
      }
    }
  }
  output.write("corners.ppm");


  // find smallest corner blob
  int min_mass = min(stats.at(red_id).mass,
		     min(stats.at(green_id).mass,
			 min(stats.at(blue_id).mass, 
			     stats.at(purple_id).mass)));
  // find largest corner blob
  int max_mass = max(stats.at(red_id).mass,
		     max(stats.at(green_id).mass,
			 max(stats.at(blue_id).mass, 
			     stats.at(purple_id).mass)));
  // erase blobs outside these size bounds
  int min_mass_thresh = min_mass / 8;
  int max_mass_thresh = max_mass * 2;
  //  fprintf(stderr, "mass_thresh = %d\n", mass_thresh);
  for (int row=0; row<labels.getRows(); row++){
    for (int col=0; col<labels.getCols(); col++){
      if (stats.at(labels(row,col)).mass < min_mass_thresh ||
	  stats.at(labels(row,col)).mass > max_mass_thresh){
	labels(row,col) = 0;
      }
    }
  }
  // write out culled blobs for debugging
  Image<byte> culled(labels.getRows(), labels.getCols());
  for (int row=0; row<labels.getRows(); row++){
    for (int col=0; col<labels.getCols(); col++){
      if (labels(row, col)){
	culled(row,col) = 0;
      } else {
	culled(row,col) = 255;
      }
    }
  }
  culled.write("culled.pgm");

  int left_id = red_id;
  int right_id = blue_id;
  int left_r1 = int(stats.at(left_id).ctr_row);
  int left_c1 = int(stats.at(left_id).ctr_col);
  int left_r2 = int(stats.at(green_id).ctr_row);
  int left_c2 = int(stats.at(green_id).ctr_col);
  int right_r1 = int(stats.at(right_id).ctr_row);
  int right_c1 = int(stats.at(right_id).ctr_col);
  int right_r2 = int(stats.at(purple_id).ctr_row);
  int right_c2 = int(stats.at(purple_id).ctr_col);

  // intially all blobs are invalid
  for (unsigned int i=0; i<stats.size(); ++i){
    stats.at(i).valid = false;
  }

  // scan along and between edges of blobs to assign logical coordinates
  Image<sRGB> scan_image(input.getRows(), input.getCols());
  for (int row=0; row<scan_image.getRows(); row++){
    for (int col=0; col<scan_image.getCols(); col++){
      scan_image(row,col) = sRGB(culled(row,col)/2,
                                 culled(row,col)/2,
                                 culled(row,col)/2);
    }
  }
  int logical_row = 0;
  do {
    std::vector<int> row_ids = FindBlobRow(scan_image, labels, 
					   int(stats.at(left_id).ctr_row),  
					   int(stats.at(left_id).ctr_col),  
					   int(stats.at(right_id).ctr_row),  
					   int(stats.at(right_id).ctr_col),
					   left_id);
    scan_image.write("line_scan.ppm");

    //std::cerr << "NUMBER OF COLS = " << row_ids.size() << "  should be " << 
    //logical_cols << std::endl;

    if (row_ids.size() != (unsigned int)logical_cols){
      FATAL_ERROR("Wrong number of cols (%u) detected logical_cols (%u)", 
                  unsigned(row_ids.size()),unsigned(logical_cols));
    }
    
    for (unsigned int i=0; i<row_ids.size(); i++){
      stats.at(row_ids.at(i)).valid = true;
      stats.at(row_ids.at(i)).logical_row = logical_row;
      stats.at(row_ids.at(i)).logical_col = i;
    }

    // find blobs bounding next row
    left_id =  FindNextBlob(scan_image, 
                            labels, left_r1, left_c1, left_r2, left_c2, 
			    left_id);
    right_id =  FindNextBlob(scan_image,
                             labels, right_r1, right_c1, right_r2, right_c2, 
			     right_id);

    logical_row++;
  } while (left_id != -1 && right_id != -1);

  if (logical_row != logical_rows){
    FATAL_ERROR("Wrong number of rows detected");
  }

  // mark corners as invalid
  stats.at(red_id).valid = false;
  stats.at(green_id).valid = false;
  stats.at(blue_id).valid = false;
  stats.at(purple_id).valid = false;

  // calculate true coordinates of blob centers
  for (unsigned int i=0; i<stats.size(); ++i){
    if (stats.at(i).valid){
      stats.at(i).true_x = target_spacing * 
	(stats.at(i).logical_col - 0.5 * (logical_cols-1.));
      stats.at(i).true_y = target_spacing * 
	(stats.at(i).logical_row - 0.5 * (logical_rows-1.));
    }
  }

  // debugging image output
  for (int row=0; row<labels.getRows(); row++){
    for (int col=0; col<labels.getCols(); col++){
      if (labels(row,col)){
	int p = 128 + 127 * stats.at(labels(row,col)).logical_col / 8;
	output(row,col) = sRGB(p,p,p);
      } else {
	output(row,col) = sRGB(0,0,0);
      }
    }
  }
  output.write("first_line.ppm");

  // clip image patches
  //  int width = 14; // !!!derive this from minimum, width, height of blobs bboxes
  for (unsigned int i=0; i<stats.size(); ++i){
    if (stats.at(i).valid){
      int width = int(sqrt((double)stats.at(i).mass) / patch_size_factor);
      
      int min_row = stats.at(i).min_row;
      int max_row = stats.at(i).max_row;
      int min_col = stats.at(i).min_col;
      int max_col = stats.at(i).max_col;

      width = std::min(max_row - min_row, max_col - min_col)/3;

      min_row = int(stats.at(i).ctr_row - width);
      max_row = int(stats.at(i).ctr_row + width);
      min_col = int(stats.at(i).ctr_col - width);
      max_col = int(stats.at(i).ctr_col + width);

      Image<byte> patch(2*width+1, 2*width+1);
      if (min_row < 0 ||
	  max_row > input.getRows()-1 ||
	  min_col < 0 ||
	  max_col > input.getCols()-1){
	FATAL_ERROR("Patch outside image");
      }

      for (int row=min_row; row<=max_row; row++){
	for (int col=min_col; col<=max_col; col++){
	  patch(row-min_row, col-min_col) = gray(row,col);
	  gray(row,col) = 255 - gray(row,col);
	}
      }
      
      double x, y;
      find_patch_center(patch, x, y);
      stats.at(i).image_x = x + int(stats.at(i).ctr_col);
      stats.at(i).image_y = y + int(stats.at(i).ctr_row);
    }
  }
  gray.write("patches.pgm");

  // overlay crosses on original image
  for (int row=0; row<labels.getRows(); row++){
    for (int col=0; col<labels.getCols(); col++){
      if (labels(row,col) == red_id){
	output(row,col) = sRGB(255, 0, 0);
      } else if (labels(row,col) == green_id){
	output(row,col) = sRGB(0, 255, 0);
      } else if (labels(row,col) == blue_id){
	output(row,col) = sRGB(0, 0, 255);
      } else if (labels(row,col) == purple_id){
	output(row,col) = sRGB(255, 0, 255);
      } else {
	output(row,col) = input(row,col)/(unsigned char)2;
      }
    }
  }
  for (unsigned int i=0; i<stats.size(); ++i){
    if (stats.at(i).valid){
      int row = int(stats.at(i).image_y + 0.5);
      int col = int(stats.at(i).image_x + 0.5);
#if 1
      int size = 5;
      for (int r=row-size; r<=row+size; r++){
	output(r,col) = sRGB(255,255,0);
      }
      for (int c=col-size; c<=col+size; c++){
	output(row,c) = sRGB(255,255,0);
      }
#else
      int size = stats.at(i).logical_row * logical_cols + stats.at(i).logical_col;
      for (int r=row-size; r<=row+size; r++){
        for (int c=col-size; c<=col+size; c++){
          output(r,c) = sRGB(255,255,0);          
        }
      }
#endif
    }
  }

  output.write("centers.ppm");

  // sort the calibration points by logical grid (gratituitously)
  BlobSorter blob_sorter(logical_cols);
  std::sort(stats.begin(), stats.end(), blob_sorter);

  validate_centers(stats, logical_rows, logical_cols, inlier_thresh);

  // write the calibration points
#if 0
  for (unsigned int i=0; i<stats.size(); ++i){
    if (stats.at(i).valid){
      printf("%+8.5f %+8.5f %+8.2f %+8.2f\n",
	     stats.at(i).true_x, stats.at(i).true_y,
	     stats.at(i).image_x, stats.at(i).image_y);
    }
  }
#endif

  for (unsigned int i=0; i<stats.size(); ++i){
    if (stats.at(i).valid){
      printf("%+8.2f %+8.2f\n",
	     stats.at(i).image_x, stats.at(i).image_y);
    }
  }
  
  return 0;
}
