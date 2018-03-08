#include <Image.h>
#include <ImageOps.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include "util.h"
#include "Vector3.h"
#include "Image.h"
#include <SDL/SDL.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL/SDL_opengl.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>
#ifdef FIREWIRE_CAMERA
#include <PointGreyCamera.hpp>
#endif  // #ifdef FIREWIRE_CAMERA

#include <X11/Xlib.h>

#ifdef GIGE_CAMERA
// Status is a #define in Xlib.h
#undef Status
#include <GigEVisionCamera.hpp>
#endif // #ifdef GIGE_CAMERA
#include <CalibratedCamera.h>


//-------------------------Point Class-----------------------------------------------
struct point
{
  double row;
  double col;
  point(){}
  point(double row, double col){this->row = row; this->col = col;}
  point &operator+= (const point &a){
    row += a.row;
    col += a.col;
    return *this;
  }
  point &operator/= (const double &a){
    row /= a;
    col /= a;
    return *this;
  }
  double distance(point p){
    return sqrt((p.row-row)*(p.row-row)+(p.col-col)*(p.col-col));
  }
  void dump() {printf("%lf %lf\n", row, col);}
};
//-------------------------End of Point Class--------------------------------------------


//-------------------------Vector2 struct-------------------------------------------------
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
//-------------------------End of Vector2 struct-------------------------------------------------


//-------------------------2 by 2 Matrix Struct--------------------------------------------------
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
    const double eps = 1e-9;
    if (fabs(c) > eps){
      return vector2(l1()-d, c);
    } else if (fabs(b) > eps){
      return vector2(b, l1()-a);
    } else {
      if (fabs(a) > fabs(d)){
	return vector2(1.0, 0.0);
      } else {
	return vector2(0.0, 1.0);
      }
    }
  };
  vector2 evect2() // eigenvector for l2()
  {
    const double eps = 1e-9;
    if (fabs(c) > eps){
      return vector2(l2()-d, c);
    } else if (fabs(b) > eps){
      return vector2(b, l2()-a);
    } else {
      if (fabs(a) > fabs(d)){
	return vector2(1.0, 0.0);
      } else {
	return vector2(0.0, 1.0);
      }
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
//-------------------------end of 2 by 2 Matrix struct-------------------------------------------------

//A line struct is defined
// ax + by + c = 0
// note x = col; y = row
//-------------------------Line struct----------------------------------------------------------------
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
  }

  line(std::vector<point> &points, double inlier_thresh = 0.){
    fit(points, inlier_thresh, false);
    if (inlier_thresh > 0.){
      fit(points, inlier_thresh, true);
    }
  }

  void refit(std::vector<point> &points, double inlier_thresh = 0.){
    fit(points, inlier_thresh, true);
  }

  void fit (std::vector<point> &points, double inlier_thresh = 0., bool 
            init = false){
    vector2 m(0.0, 0.0);
    int n = 0;
    
    std::vector<point>::iterator pt;
    for (pt = points.begin(); pt != points.end(); ++pt){
      if (!init || point_error(*pt) < inlier_thresh){
        m.x += pt->col;
        m.y += pt->row;
        n++;
      }
    }
    m.x /= double(n);
    m.y /= double(n);
    
    double sxx = 0.0;
    double sxy = 0.0;
    double syy = 0.0;
    for (pt = points.begin(); pt != points.end(); ++pt){
      if (!init || point_error(*pt) < inlier_thresh){
        sxx += (pt->col - m.x) * (pt->col - m.x);
        sxy += (pt->col - m.x) * (pt->row - m.y);
        syy += (pt->row - m.y) * (pt->row - m.y);
      }
    }
    
    matrix2x2 cov(sxx/n, sxy/n, sxy/n, syy/n);
    vector2 v;
    if (fabs(cov.l1()) > fabs(cov.l2())){
      v = cov.evect1(); 
    } else {
      v = cov.evect2(); 
    }
    a = -v.y;
    b = v.x;
    c = -(a * m.x + b * m.y); 
    normalize();
    //printf("%u %f %f %f\n", points.size(), a, b, c);
  }

  void normalize()
  {
    double d = sqrt(a*a+b*b);
    a /= d;
    b /= d;
    c /= d;
  }

  bool side(point &p){
    double d = a*p.col + b*p.row + c;
    if (d < 0.){
      return true;
    } else {
      return false;
    }
  }

  // +1 one side
  // -1 other side
  //  0 to close to call (within tolerance of line)
  int side_or_line(point &p, double tolerance = 0.){
    double d = a*p.col + b*p.row + c;
    if (d > tolerance){
      return +1;
    } else if (d < -tolerance){
      return -1;
    }
    return 0;
  }

  double point_error(point &p){
    return  fabs(a * p.col + b * p.row + c);
  }

  double point_error(std::vector<point> &points){
    double err_sum = 0.;
    for (unsigned i=0; i<points.size(); i++){
      err_sum += point_error(points[i]);
    }
    return err_sum / points.size();
  }

  int points_closer_than(std::vector<point> &points, double inlier_thresh){
    int count = 0;
    for (unsigned i=0; i<points.size(); i++){
      if(point_error(points[i]) < inlier_thresh){
        count++;
      }
    }
    return count;
  }

  void dump() {printf("%lf %lf %lf\n", a, b, c);};
};
//-------------------------End of line struct-------------------------------------------------


//-------------------------Pair of line structs-----------------------------------------------
// a pair of lines?
struct linepair {
  linepair(const line &l1, const line &l2){
    this->l1 = l1;
    this->l2 = l2;
    dot = l1.a * l2.a + l1.b * l2.b;
    matrix2x2 m(l1.a, l1.b, l2.a, l2.b);
    vector2 v = m.inverse() * vector2(-l1.c, -l2.c);  
    intersection = point(v.y, v.x);    
  }
  line l1;
  line l2;
  double dot;
  point intersection;
};
//-------------------------end of Pair of line structs-----------------------------------------------

//It appears this function 
Image<byte> CorrectRadialDistortion(Image<byte> &in, double u0, double v0,
                                    double k1, double k2, double a0){
  Image <byte> out(in.getRows(), in.getCols());
  double denom = a0;
  for (int row=0; row<out.getRows(); row++){
    double y = (row - v0) / denom;
    for (int col=0; col<out.getCols(); col++){
      double x = (col - u0) / denom;

      double r2 = (x*x + y*y);
      double r4 = r2 * r2;
      double u = col + (col - u0)*(k1 * r2 + k2 * r4);
      double v = row + (row - v0)*(k1 * r2 + k2 * r4);
      out(row, col) = in.bilinear(v, u);
    }
  }
  return(out);      
}

//Conversions to and from gray numbers
//Gray numbers are special in that they only vary in one bit from the number before them
unsigned short binaryToGray(unsigned short num)
{
  return (num>>1) ^ num;
}

unsigned short grayToBinary(unsigned short num)
{
  unsigned short temp = num ^ (num>>8);
  temp ^= (temp>>4);
  temp ^= (temp>>2);
  temp ^= (temp>>1);
  return temp;
}

//-------------------------Binary Calibrator Class-----------------------------------------------
class BinaryCalibratorGenerator {
public:
  
  //Constructor, just sets variables as it should
  BinaryCalibratorGenerator (int rows, int cols,
                             int point_rows, int point_cols){
    this->rows = rows;
    this->cols = cols;
    this->point_rows = point_rows;
    this->point_cols = point_cols;
    row_bits = int(ceil(log(point_rows+1)/log(2.)));
    col_bits = int(ceil(log(point_cols+1)/log(2.)));
    point_image = Image<point>(point_rows, point_cols);
    code_image = Image<PointIdx>(rows, cols);
    fill_image(code_image);
  }

  //Accessor
  int getNumImages(){
    return nimages;
  }
  

  Image<byte> getMaskImage(int n){
    Image <byte> image(code_image.getRows(), code_image.getCols());
    for (int row=0; row<image.getRows(); row++){
      for (int col=0; col<image.getCols(); col++){
        if (n%2) {
          image(row, col) = 255;
        } else {
          image(row, col) = 0;
        }
      }
    }
    return image;
  }

  Image<byte> getImage(int n){
    Image <byte> image(code_image.getRows(), code_image.getCols());
    bool detecting_rows = n < 2*row_bits;
    int bit;
    if (detecting_rows){
      bit = 1 << (n / 2);
    } else {
      bit = 1 << ((n-2*row_bits) / 2);
    }
    int polarity = (n & 1)?1:0;

    for (int row=0; row<image.getRows(); row++){
      for (int col=0; col<image.getCols(); col++){
	PointIdx point_codes = code_image(row, col);
	int code;
	if (detecting_rows){
	  code = point_codes.row;
	} else {
	  code = point_codes.col;
	}
        if (code >= 0){
          if (((code & bit)?0:1) ^ polarity){
            image(row, col) = 255;
          } else {
            image(row, col) = 0;
          }
        } else {
          image(row, col) = 127;
        }
      }
    }

    return image;
  }
  
  //Accessor- uses point_image (
  Image<point> getPointImage(){
    Image<point> image(point_rows-1, point_cols-1);
    for (int row=0; row<image.getRows(); row++){
      for (int col=0; col<image.getCols(); col++){
	image(row, col) = point_image(row, col);
      }
    }
    return image;
  }

private:
  struct PointIdx {
    int row, col;
    PointIdx(){
      row = col = -1;
    }
    PointIdx(int r, int c){
      row = r;
      col = c;
    }
  };

  int rows;
  int cols;
  int point_rows;
  int point_cols;
  int row_bits;
  int col_bits;
  int nimages;
  Image<PointIdx> code_image;
  Image<point> point_image;
  
  //Appears to create a bar code pattern (using gray numbers)
  void fill_image(Image<PointIdx> &image){
    int rstep = rows / (point_rows);
    int cstep = cols / (point_cols);
    
    for (int row=0; row<image.getRows(); row++){
      for (int col=0; col<image.getCols(); col++){
        code_image(row,col) = PointIdx(-1, -1);
      }
    }

    int point_row = 0;
    //Downsampling?
    for (int row0=0; row0<=rows-rstep; row0+=rstep){
      int point_col = 0;
      for (int col0=0; col0<=cols-cstep; col0+=cstep){
//This line is key: points are determined to be valid or invalid based on these values
	point_image(point_row, point_col) = point(row0+rstep+0.5, 
						  col0+cstep+0.5);
	point_col++;
        for (int row=row0; row<row0+rstep; row++){
          for (int col=col0; col<col0+cstep; col++){
            code_image(row, col) = 
	      PointIdx(binaryToGray(point_row), binaryToGray(point_col));
          }
        }
      }
      point_row++;
    }
    nimages = 2 * (row_bits + col_bits);
  }
};
  
//-------------------------End of Binary Calibrator Class-----------------------------------------------

//-------------------------Histogram Class-----------------------------------------------
class Histogram {
public:
  Histogram(int nbins){
    this->nbins = nbins;
    hist.resize(nbins);
    for (int i=0; i<nbins; i++){
      hist[i] = 0;
    }
  }

  int &operator()(int idx){
    assert(idx >= 0);
    assert(idx < nbins);
    return hist[idx];
  }

  int peak_idx(){
    int best_idx = 0;
    int max_count = hist[0];
    for (unsigned i=1; i<hist.size(); i++){
      if (hist[i] > max_count){
        max_count = hist[i];
        best_idx = i;
      }
    }
    return best_idx;
  }

  int median(){
    int sum = 0;
    for (unsigned i=0; i<hist.size(); i++){
      sum += hist[i];
    }
    int medval = sum / 2;
    sum = 0;
    int last_i = 0;
    for (unsigned i=0; i<hist.size(); i++){
      if (hist[i]){
        sum += hist[i];
        if (sum > medval){
          return (i+last_i)/2;
        }
        last_i = i;
      }
    }
    return hist.size();
  }
  
  int mean (){
    int sum = 0;
    for (unsigned i=0; i<hist.size(); i++){
      sum += hist[i];
    }
    return sum/hist.size();
  }

  int range (){
    return max() - min();
  }

  int min(){
    for (unsigned i=0; i<hist.size(); i++){
      if (hist[i]){
        return i;
      }
    }
    return 0;
  }

  int max(){
    for (unsigned i=hist.size()-1; i>=0; i--){
      if (hist[i]){
        return i;
      }
    }
    return 0;
  }
  
  void dump(){
    for (unsigned i=0; i<hist.size(); i++){
      printf("%3d %10d\n", i, hist[i]);
    }
  }
    
private:
  int nbins;
  std::vector<int> hist;
};

//-------------------------End of Histogram class-----------------------------------------------



//-------------------------Binary Calibrator Recognizer class-----------------------------------------------
class BinaryCalibratorRecognizer {
public:
  BinaryCalibratorRecognizer(int rows, int cols,
			     int point_rows, int point_cols){
    this->rows = rows;
    this->cols = cols;
    this->point_rows = point_rows;
    this->point_cols = point_cols;
    row_bits = int(ceil(log(point_rows+1)/log(2.)));
    col_bits = int(ceil(log(point_cols+1)/log(2.)));
    n_images = 2*(row_bits+col_bits);
    images = Image<std::vector<byte> >(rows, cols);
    id_image = Image<PointIdx>(rows, cols);
    image_num = 0;
    mask_image = Image<byte>(rows, cols);
  }

  void addImage(Image<byte> &image){
    assert(image.getRows() == rows);
    assert(image.getCols() == cols);
    for (int row=0; row<rows; row++){
      for (int col=0; col<cols; col++){
        images(row, col).push_back(image(row,col));
      }
    }
    image_num++;
  }

  void clearImages(){
    for (int row=0; row<rows; row++){
      for (int col=0; col<cols; col++){
        images(row, col).clear();
      }
    }
    image_num = 0;
  }

  Image<point> getPointImage(){
    Image<point> image(point_rows-1, point_cols-1);
    for (int row=0; row<image.getRows(); row++){
      for (int col=0; col<image.getCols(); col++){
	image(row, col) = point_image(row, col);
      }
    }
    return image;
  }

  //This function usees a histogram to try to decipher where the pieces of the mask are
  void findMask(){
    Histogram range_hist(256);
    for (int row=0; row<rows; row++){
      for (int col=0; col<cols; col++){
        //The below is done for each pixel
        assert((images(row, col).size() % 2) == 0); // must have even number
        Histogram hist(256);
        //The histogram (for this pixel) is filled with the corresponding pixel from all the images
        for (unsigned i=0; i<images(row, col).size(); i++){
          hist(images(row,col)[i])++;
        }

        //Apparently there is  range histogram too?
	      range_hist(hist.range())++;
	      int range_thresh = 20;

        //If there is a large range it means presumably that the projectors are both on and off
        // for some values and this part of the mask should be activated
	      if (hist.range() > range_thresh){
          mask_image(row, col) = 255;
        } else {
          mask_image(row, col) = 0;
        }
      }
    }

    // Presumably only one configuous part of the mask is the plane, so we throw out others 
    // assuming they are outliers

    // keep only the largest connected component
    KeepLargestComponent(mask_image);
    // range_hist.dump();

    //
    // this is broken: always calculates the range_thresh specified above
    // !!! need a better way to estimate min_range
    //

    // # of pixels in the mask
    // This has to be done here because the mask changed in KeepLargest
    int pixel_count = 0;
    for (int row=0; row<mask_image.getRows(); row++){
      for (int col=0; col<mask_image.getCols(); col++){
        if (mask_image(row, col)){
	        pixel_count++;
	      }
      }
    }

    // find a "good" range threshold based on the range histogram
    //  - find the smallest threshold that accounts for the number of
    //    pixels in the mask
    min_range = 255;
    int sum = 0;
    while (min_range > 0 && sum < pixel_count){
      sum += range_hist(min_range--);
    }
    fprintf(stderr, "min_range = %d\n", min_range);
    //    mask_image.write("mask.pgm");
  }

  // sort the pixel values
  // highest ones are 1's, lowest ones are zeros....
  void findIds(){
    if (image_num != n_images){
      fprintf(stderr, "Incorrect number of images %d:%d\n",
	      image_num, n_images);
      assert(0);
    }
    for (int row=0; row<rows; row++){
      for (int col=0; col<cols; col++){
        assert((images(row, col).size() % 2) == 0); // must have even number
        Histogram hist(256);
        for (unsigned i=0; i<images(row, col).size(); i++){
          hist(images(row,col)[i])++;
        }
        if (!mask_image(row, col) || hist.range() < min_range){
          id_image(row, col) = PointIdx(-1, -1);
        } else {
          byte thresh = hist.median();
          int row_id = 0;
	  int i;
	  for (i=0; i<2*row_bits; i+=2){
            int sum = 0;
            for (unsigned polarity=0; polarity<2; polarity++){
              byte val = images(row,col)[i+polarity];
              int b = (val > thresh) ? 1:0;
              if (polarity) b = 1-b;
              sum += b;
            }
            if (0 == sum){
              row_id |= (1<<(i/2));
            }
	  }
	  int col_id = 0;
	  for (; i<(int)images(row,col).size(); i+=2){
            int sum = 0;
            for (unsigned polarity=0; polarity<2; polarity++){
              byte val = images(row,col)[i+polarity];
              int b = (val > thresh) ? 1:0;
              if (polarity) b = 1-b;
              sum += b;
            }
            if (0 == sum){
              col_id |= (1<<((i-2*row_bits)/2));
            }
	  }
          id_image(row, col) = PointIdx(grayToBinary(row_id),
					grayToBinary(col_id)-1);
        }
      }
    }

    // keep only the largest contiguous block of each row index
    for (int row_idx = 0; row_idx < point_rows; row_idx++){
      Image<byte> image(rows, cols);
      for (int row=0; row<image.getRows(); row++){
	for (int col=0; col<image.getCols(); col++){
	  if (id_image(row,col).row == row_idx){
	    image(row, col) = 255;
	  } else {
	    image(row, col) = 0;
	  }
	}
      }
      KeepLargestComponent(image);
      for (int row=0; row<image.getRows(); row++){
	for (int col=0; col<image.getCols(); col++){
	  if (id_image(row,col).row == row_idx && !image(row, col)){
	    id_image(row, col) = PointIdx(-1, -1);
	  }
	}
      }
    }

    // keep only the largest contiguous block of each row index
    for (int col_idx = 0; col_idx < point_cols; col_idx++){
      Image<byte> image(rows, cols);
      for (int row=0; row<image.getRows(); row++){
	for (int col=0; col<image.getCols(); col++){
	  if (id_image(row,col).col == col_idx){
	    image(row, col) = 255;
	  } else {
	    image(row, col) = 0;
	  }
	}
      }
      KeepLargestComponent(image);
      for (int row=0; row<image.getRows(); row++){
	for (int col=0; col<image.getCols(); col++){
	  if (id_image(row,col).col == col_idx && !image(row, col)){
	    id_image(row, col) = PointIdx(-1, -1);
	  }
	}
      }
    }
  }



  //-------------------------Bounds class---------------------------------------------------------------------
  class Bounds {
  public:
    Bounds(){
      min_row = std::numeric_limits<int>::max();
      max_row = std::numeric_limits<int>::min();
      min_col = std::numeric_limits<int>::max();
      max_col = std::numeric_limits<int>::min();
    }
    void update(int row, int col){
      if (row < min_row) min_row = row;
      if (row > max_row) max_row = row;
      if (col < min_col) min_col = col;
      if (col > max_col) max_col = col;
    }
    int min_row;
    int max_row;
    int min_col;
    int max_col;
  };

  //-------------------------End of Bounds class------------------------------------------------------------

  Image<byte> findCenters(){
    valid = true;
    point_image = Image<point>(point_rows, point_cols);

    Image<byte> mask(rows, cols);
    Image<byte> test(rows, cols);
    for (int row=0; row<test.getRows(); row++){
      for (int col=0; col<test.getCols(); col++){
        if (id_image(row, col).row >=0){
          test(row, col) = id_image(row, col).row * 16 +
            id_image(row, col).col;
        } else {
          test(row, col) = 0;
        }
      }
    }

    // find centroids of ID blocks
    Image<point> centroid_image(point_rows, point_cols);
    Image<int> count_image(point_rows, point_cols);
    for (int row=0; row<centroid_image.getRows(); row++){
      for (int col=0; col<centroid_image.getCols(); col++){
	centroid_image(row, col) = point(0., 0.);
	count_image(row, col) = 0;
      }
    }
    for (int row=0; row<id_image.getRows(); row++){
      for (int col=0; col<id_image.getCols(); col++){
	int row_id = id_image(row,col).row;
	int col_id = id_image(row,col).col;
	if (row_id >= 0 && row_id < point_rows &&
	    col_id >= 0 && col_id < point_cols){
	  centroid_image(row_id, col_id) += point(row, col);
	  count_image(row_id, col_id)++;
	}
      }
    }
    for (int row=0; row<centroid_image.getRows(); row++){
      for (int col=0; col<centroid_image.getCols(); col++){
	centroid_image(row, col) /= count_image(row, col);
      }
    }
    
    // merge block centroids to get estimated centers
    Image<point> center_image(point_rows, point_cols);
    for (int row=0; row<center_image.getRows()-1; row++){
      for (int col=0; col<center_image.getCols()-1; col++){
	center_image(row, col) = point(0., 0.);
	center_image(row, col) += centroid_image(row, col);
	center_image(row, col) += centroid_image(row+1, col);
	center_image(row, col) += centroid_image(row, col+1);
	center_image(row, col) += centroid_image(row+1, col+1);
	center_image(row, col) /= 4.;
      }
    }


    Image<std::vector<point> > horiz_points(point_rows, point_cols);
    Image<std::vector<point> > vert_points(point_rows, point_cols);
    for (int row=1; row<id_image.getRows()-1; row++){
      for (int col=1; col<id_image.getCols()-1; col++){
	int row_id = id_image(row,col).row;
	int col_id = id_image(row,col).col;
        for (int i=-1; i<=1; i++){
          for (int j=-1; j<=1; j++){
            if (0 == i && 0 == j) continue;
            int other_row_id = id_image(row+i, col+j).row;
            int other_col_id = id_image(row+i, col+j).col;
            if ((other_row_id >= 0 && other_row_id < point_rows &&
                 other_col_id >= 0 && other_col_id < point_cols) ||
                (row_id >= 0 && row_id < point_rows && 
                 col_id >= 0 && col_id < point_cols)){
              if (abs(row_id - other_row_id) == 1 &&
                  col_id == other_col_id){
		vert_points(std::max(std::min(row_id, other_row_id), 0),
			    col_id).push_back(point(row, col));
              }
              if (abs(col_id - other_col_id) == 1 &&
                  row_id == other_row_id){
		horiz_points(row_id, std::max(std::min(col_id, other_col_id), 0)
			     ).push_back(point(row, col));
              }
            }
          }
        }
      }
    }

    for (int point_row=0; point_row<point_rows; point_row++){
      for (int point_col=0; point_col<point_cols; point_col++){
        for (unsigned i=0; i<horiz_points(point_row,point_col).size(); i++){
          test(horiz_points(point_row,point_col)[i].row, 
               horiz_points(point_row,point_col)[i].col) = 127;
        }
        for (unsigned i=0; i<vert_points(point_row,point_col).size(); i++){
          test(vert_points(point_row,point_col)[i].row, 
               vert_points(point_row,point_col)[i].col) = 127;
        }
      }
    }

    double dist_thresh = 10.; // maximum distance to estimated centers
    for (int point_row=0; point_row<point_rows-1; point_row++){
      for (int point_col=0; point_col<point_cols-1; point_col++){    
        double inlier_thresh = 1.5;
	std::vector<point> hpoints;
	std::vector<point> vpoints;
	hpoints = horiz_points(point_row, point_col);
	hpoints.insert(hpoints.end(), 
		       horiz_points(point_row+1, point_col).begin(),
		       horiz_points(point_row+1, point_col).end());
	vpoints = vert_points(point_row, point_col);
	vpoints.insert(vpoints.end(), 
		       vert_points(point_row, point_col+1).begin(),
		       vert_points(point_row, point_col+1).end());

	point_distance_pred pred(center_image(point_row, point_col),
				 dist_thresh);
	// eliminate points too far from estimated centers
	hpoints.erase(remove_if(hpoints.begin(), hpoints.end(), 
				pred), hpoints.end());
	vpoints.erase(remove_if(vpoints.begin(), vpoints.end(), 
				pred), vpoints.end());

        line vert_line(vpoints, inlier_thresh);
        line horiz_line(hpoints, inlier_thresh);
        linepair center(vert_line, horiz_line);

	if (center.intersection.row >= 0 &&
	    center.intersection.row < test.getRows() &&
	    center.intersection.col >= 0 &&
	    center.intersection.row < test.getCols() &&
            mask_image(center.intersection.row,
                       center.intersection.col)){
	  point_image(point_row, point_col) = center.intersection;
	  for (int idx=0; idx<100; idx++){
	    double th = (2.*3.1415927*idx)/100;
	    int r = int(center.intersection.row + 10.*sin(th));
	    int c = int(center.intersection.col + 10.*cos(th));
	    if (r >=0 && r < test.getRows() && c >= 0 && c < test.getCols()){
	      test(r, c) = 255;
	    }
	  }
	  int r = int(center.intersection.row);
	  int c = int(center.intersection.col);
	  if (r >=0 && r < test.getRows() && c >= 0 && c < test.getCols()){
	    test(r, c) = 255;
	  }
	}
      }
    }
    
    return test;
  }

  bool isValid(){
    return valid;
  }
private:
  struct PointIdx {
    int row, col;
    PointIdx(){
      row = col = -1;
    }
    PointIdx(int r, int c){
      row = r;
      col = c;
    }
  };




  //-------------------------Point Distance pred struct-----------------------------------------------
  struct point_distance_pred {
    point_distance_pred(const point &p, double thresh){
      this->p = p;
      this->thresh = thresh;
    }
    bool operator()(const point &q){
      if (p.distance(q) > thresh){
	return true;
      }
      return false;
    }
    point p;
    double thresh;
  };
  //-------------------------end of Point Distance pred struct-----------------------------------------------

  int rows;
  int cols;
  int row_bits;
  int col_bits;
  int point_rows;
  int point_cols;
  int n_images;
  int image_num;
  int min_range;
  bool valid;
  //An Image of vectors of bytes
  Image<std::vector<byte> > images;
  Image<PointIdx> id_image;
  Image<point> point_image;
  Image<byte> mask_image;
  
  // keep only the largest connected component
  void KeepLargestComponent(Image<byte> &image){
    int n_labels = 0;
    Image<int> labels = ConnectedComponents(image, n_labels);
    std::vector<int> mass(n_labels);
    for (unsigned i=0; i<mass.size(); i++){
      mass[i] = 0;
    }
    for (int row=0; row<image.getRows(); row++){
      for (int col=0; col<image.getCols(); col++){
	mass[labels(row,col)]++;
      }
    }
    int largest_mass = 0;
    int best_index = 0;
    for (unsigned i=1; i<mass.size(); i++){
      if (mass[i] > largest_mass){
	largest_mass = mass[i];
	best_index = i;
      }
    }
    for (int row=0; row<image.getRows(); row++){
      for (int col=0; col<image.getCols(); col++){
	if (labels(row, col) != best_index){
	  image(row, col) = 0;
	}
      }
    }
  }

};

//-------------------------end of Binary Calibrator Recognizer class-----------------------------------------------
void getRootWindowSize(const char *name, int &rows, int &cols){
  Display *disp = XOpenDisplay(name);
  int screen_number = XDefaultScreen(disp);
  rows = DisplayHeight(disp, screen_number); 
  cols = DisplayWidth(disp, screen_number); 
}

void draw_8bit_image(SDL_Surface *screen, Image<byte> image, int level){
  if (screen->format->BytesPerPixel != 4){
    FATAL_ERROR("wrong screen format\n");
  }
  if ( SDL_MUSTLOCK(screen) ) {
    if ( SDL_LockSurface(screen) < 0 ) {
      FATAL_ERROR("can't lock screen\n");
      return;
    }
  }
  
  for (int row=0; row<image.getRows(); row++){
    for (int col=0; col<image.getCols(); col++){
      Uint32 *bufp = (Uint32 *)screen->pixels + row*screen->pitch/4 + col;
      byte v = int((image(row, col)?level:0) * (1-0.3*row/image.getRows()));
      // partally correct for varying projector distance
      *bufp= SDL_MapRGB(screen->format, v,v, v);
    }
  } 

  if ( SDL_MUSTLOCK(screen) ) {
    SDL_UnlockSurface(screen);
  }
 
  //  SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
  SDL_Flip(screen);
}

void cleanup(){
  SDL_Quit();
}

bool file_exists(const char *filename){
  FILE *fp = fopen(filename, "rt");
  if (NULL == fp){
    return false;
  } else {
    fclose(fp);
    return(true);
  }
}

//Writes points out to file if they are above a certain threshold
// where are these points from?
void WritePointPairs(const char *filename, Image<point> &OutputPoints,
                     Image<point> &InputPoints){
  assert(OutputPoints.getRows() == InputPoints.getRows());
  assert(OutputPoints.getCols() == InputPoints.getCols());
  FILE *fp = fopen(filename, "wt");
  if (NULL==fp){
    FATAL_ERROR("Can't open file '%s' for writing\n", filename);
  }

  for (int row=0; row<OutputPoints.getRows(); row++){
    for (int col=0; col<OutputPoints.getCols(); col++){
      if (fabs(InputPoints(row,col).row) > 0.5 &&
	  fabs(InputPoints(row,col).col) > 0.5){
	fprintf(fp, "%f %f %f %f\n", 
		OutputPoints(row,col).row, OutputPoints(row,col).col,
		InputPoints(row,col).row, InputPoints(row,col).col);
printf("good point %f %f %f %f\n", 
		OutputPoints(row,col).row, OutputPoints(row,col).col,
		InputPoints(row,col).row, InputPoints(row,col).col);
      }
      else{
	printf("b");
      }
    }
  }

  fclose(fp);
  printf("Data saved as '%s'\n", filename);
}

// Straightforward conversion to the brightest channel of RGB
Image <byte> rgb_to_gray(const Image<sRGB> &in){
  Image <byte> out(in.getRows(), in.getCols());
  for (int row=0; row<in.getRows(); row++){
    for (int col=0; col<in.getCols(); col++){
      out(row, col) = in(row, col).max();
    }
  }
  return out;
}

Image<byte> get_image(CameraAPI<sRGB> *camera){
  
  // delay until image appears on projector
#ifdef FIREWIRE_CAMERA
  SDL_Delay(100);
#endif

#ifdef GIGE_CAMERA
  SDL_Delay(66);
#endif

  Image<sRGB> raw;
 
  // toss a few images to make sure we have a fresh one
  //for (int i=0; i<3; i++){
    raw = camera->GetLatestFrame();
  //}
  
  Image<byte> result = rgb_to_gray(raw);

  return result;
}


// find the largest gray level that saturates no pixels in the camera
int set_projector_level (CameraAPI<sRGB> *camera, int rows, int cols,
			 SDL_Surface *screen){
  int level = 255;
  int step = 128;
  Image<byte> out_image(rows, cols);

  // create a uniform (white) image
  for (int row=0; row<out_image.getRows(); row++){
    for (int col=0; col<out_image.getCols(); col++){
      out_image(row, col) = 255;
    }
  }

  do {
    // draw image at current intensity level
    draw_8bit_image(screen, out_image, level);

    // check for saturated pixels
    bool found_saturated = false;
    Image<byte> in_image = get_image(camera);
    for (int row=0; row<in_image.getRows(); row++){
      for (int col=0; col<in_image.getCols(); col++){
	if (255 == in_image(row, col)){
	  found_saturated = true;
	}
      }
    }

    if (found_saturated){
      level -= step;
    } else {
      level += step;
    }
    step /= 2;
  
  } while (step > 0);

  return level;
}

int main(int argc, char **argv){
  setenv("__GL_SYNC_TO_VBLANK", "1", 1);
  if (argc != 5){
    fprintf(stderr,
	    "\nstructured light correspondence generator \n"
	    "%s <Xdisplay> <output file> <camera_calibration.dat>\n"
	    "example:\n"
	    "  %s :1.0 output.dat camera_images/camera_calibration.dat\n",
	    argv[0], argv[0]);
    return -1;
  }

  setenv("DISPLAY", argv[1], 1);
  char *filename = argv[2];
  
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }
  atexit(cleanup);

  // get root window size
  int rows, cols;
  getRootWindowSize(NULL, rows, cols);
  printf("Screen size = %d, %d\n", rows, cols);

  SDL_Surface *screen = NULL;

  if (NULL == (screen = SDL_SetVideoMode(cols, rows, 0, 
                                         SDL_HWSURFACE|
                                         SDL_FULLSCREEN|
                                         SDL_DOUBLEBUF))){
    printf("Can't set OpenGL mode: %s\n", SDL_GetError());
    SDL_Quit();
    exit(1);
  }

  // turn off mouse/keyboard grabbing
  SDL_WM_GrabInput(SDL_GRAB_OFF);

  // hide mouse cursor
  SDL_ShowCursor(0);


#ifdef FIREWIRE_CAMERA
  PointGreyCamera<sRGB> camera(0,0,1392, 1032);
  camera.SetShutter(900);
  camera.SetGain(56);
  camera.SetGamma(1024);
  camera.SetWhiteBalance(605, 615); // blue, red
  camera.SetBrightness(0);
#endif

#ifdef GIGE_CAMERA
  GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_BAYER8> camera;
  int shutter = 12000;
  int gain = 0;
  int cam_rows = 960;
  int cam_cols = 960;
  int row_offset = 0;
  int col_offset = 145;
  camera.SetExposure(shutter);
  camera.SetGain(gain);
  camera.SetWhiteBalance(105,400);
  camera.SetBinning(1, 1);
  camera.SetROI(row_offset, col_offset, cam_rows, cam_cols);
  camera.InitMultipleFrameCapture();
#endif

  int point_rows = 9;
  int point_cols = 12;
  BinaryCalibratorGenerator calgen(rows, cols, point_rows, point_cols);
  int n = calgen.getNumImages();

  printf("n = %d\n", n);

  Image<byte> temp = get_image(&camera);//Gets image in grayscale
  int image_rows = temp.getRows();
  int image_cols = temp.getCols();
  printf("Image size = %d, %d\n", image_rows, image_cols);
  BinaryCalibratorRecognizer calrec(image_rows, image_cols, 
				    point_rows, point_cols);


  int level = set_projector_level (&camera, rows, cols, screen);
  fprintf(stderr, "level = %d\n", level);

  CalibratedCamera cal_cam;
  cal_cam.loadCalibration(argv[3]);
  char *mask_filename = argv[4];
  Image<byte> EMPAC_mask=Image<byte>(mask_filename);
  double a0 = cal_cam.getA0();
  double u0 = cal_cam.getCx();
  double v0 = cal_cam.getCy();
  double k1 = cal_cam.getK1();
  double k2 = cal_cam.getK2();

  int n_masks = 8;
  for (int i=0; i<n_masks; i++){
    Image<byte> mask_image = calgen.getMaskImage(i);
    draw_8bit_image(screen, mask_image, level);    
    Image<byte> in_image = get_image(&camera);
    Image<byte> undistorted = CorrectRadialDistortion(in_image, 
						      u0, v0, k1, k2, a0);
    calrec.addImage(undistorted);
  }

  calrec.findMask();
  calrec.clearImages();

  for (int i=0; i<n; i++){
    Image<byte> out_image =  calgen.getImage(i);
    draw_8bit_image(screen, out_image, level);
    Image<byte> in_image = get_image(&camera);
    Image<byte> undistorted = CorrectRadialDistortion(in_image, 
						      u0, v0, k1, k2, a0);
    calrec.addImage(undistorted);
  } 

  fprintf(stderr,"Finding ids...\n");
  calrec.findIds();
  
  fprintf(stderr,"Finding centers...\n");
  Image<byte> check_image = calrec.findCenters();
  check_image.write("test.pgm");

  // to-do : add validity test (check for points which are not "near" grid)
  if (!calrec.isValid()){
    printf("Invalid point detected, not saving\n");
    return -1;
  }

  //LCD points are the points which determine if a point is valid
  Image<point> lcd_points = calgen.getPointImage();
  Image<point> cam_points = calrec.getPointImage();

  //points are decided to be valid or not based on the values of lcd_points being greater than .5 (or some threshold)
  WritePointPairs(filename, lcd_points, cam_points); 
  cleanup();
  return 0;
}
