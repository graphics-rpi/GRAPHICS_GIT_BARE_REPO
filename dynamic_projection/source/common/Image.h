
#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

// For compile errors related to /usr/include file
#define PNG_SKIP_SETJMP_CHECK

#include <cstdlib>
#include <cstdio>
#include <stdint.h>
#include <cctype>
#include <cassert>
#include <limits>
#include <cmath>
#include <iostream>
#include <typeinfo>
#include <errno.h>
#include <jpeglib.h>
#include <png.h>

// WARNING: If socket reader is needed, it must be compiled with mpicc and -DMPI_DEFINED must be included in the compile line

// #define DEBUG before including this file to turn on (slow) bounds checking

#include "util.h"
#include "ImageGamma.h"


#ifdef MPI_DEFINED
#include "SocketReader.h"
#endif


// traits for PNM images
template <typename Pixel>
struct PixelTraits { 
  typedef Pixel Pixel_t;
  typedef Pixel base_Pixel_t;
  typedef Pixel accum_Pixel_t;
  typedef Pixel base_accum_Pixel_t;

  // "P0" magic is used for generic non-standard types
  const char *magic() const { return "P0"; }
  int pnm_maxval() const { return 0; }
  static Pixel maximum_value() {
    return std::numeric_limits<Pixel>::max(); 
  };
  static Pixel minimum_value() {
    return std::numeric_limits<Pixel>::min();
  };
};



template <>
struct PixelTraits<byte> {
  typedef byte Pixel_t;
  typedef byte base_Pixel_t;
  typedef double accum_Pixel_t;
  typedef double base_accum_Pixel_t;
  const char *magic() const { return "P5"; }
  int pnm_maxval() const { return 255; }
  static byte maximum_value() {
    return 255;
  }
  static byte minimum_value() {
    return 0;
  }
};



template <>
struct PixelTraits<sRGB> {
  typedef Vector3<byte> Pixel_t;
  typedef byte base_Pixel_t;
  typedef v3d accum_Pixel_t;
  typedef double base_accum_Pixel_t;
  const char *magic() const { return "P6"; }
  int pnm_maxval() const { return 255; }
  static sRGB maximum_value() {
    return sRGB(255, 255, 255);
  }
  static sRGB minimum_value() {
    return sRGB(0, 0, 0);
  }
};

template <>
struct PixelTraits<v3d> {
  typedef v3d Pixel_t;
  typedef double base_Pixel_t;
  typedef v3d accum_Pixel_t;
  typedef double base_accum_Pixel_t;
  const char *magic() const { return "P0"; }
  int pnm_maxval() const { return 0; }
  static v3d maximum_value() {
    return v3d(std::numeric_limits<double>::max());
  }
  static v3d minimum_value() {
    return v3d(-std::numeric_limits<double>::max());
  }
};

template <>
struct PixelTraits<unsigned short> {
  typedef unsigned short Pixel_t;
  typedef unsigned short base_Pixel_t;
  typedef double accum_Pixel_t;
  typedef double base_accum_Pixel_t;
  const char *magic() const { return "P5"; }
  int pnm_maxval() const { return 65535; }
  static unsigned short maximum_value() {
    return 65535U;
  }
  static unsigned short minimum_value() {
    return 0;
  }
};


/*
// !!! this is temporary: need a better way to handle these images
template <>
struct PixelTraits<Vector3<unsigned short> > {
  typedef Vector3<unsigned short> Pixel_t;
  typedef unsigned short base_Pixel_t;
  typedef Vector3<unsigned short> accum_Pixel_t;
  typedef unsigned short base_accum_Pixel_t;
  const char *magic(){ return "P6"; }
  int pnm_maxval(){ return 65535; }
};
*/


template <>
struct PixelTraits<int> {
  typedef int Pixel_t;
  typedef int base_Pixel_t;
  typedef int accum_Pixel_t;
  typedef int base_accum_Pixel_t;
  const char *magic() const { return "P0"; }
  int pnm_maxval() const { return 65535; }
  static unsigned short maximum_value() {
    return (unsigned short)std::numeric_limits<int>::max();
  }
  static unsigned short minimum_value() {
    return (unsigned short)std::numeric_limits<int>::min();
  }
};



//
// generic image class
//
template <typename Pixel>
class Image
{
public:
  typedef Pixel Pixel_t;
  typedef typename PixelTraits<Pixel_t>::base_Pixel_t base_Pixel_t;
  typedef typename PixelTraits<Pixel_t>::accum_Pixel_t accum_Pixel_t;
  typedef typename PixelTraits<Pixel_t>::base_accum_Pixel_t base_accum_Pixel_t;


  /*
  Image(){
    rows = 0;
    cols = 0;
    min_x = max_x = min_y = max_y = 0.0;
    data = NULL;
    //fake_pixel = Pixel();
    initialized = false;
  }
  */

  Image(int rows_=0, int cols_=0) {
    rows = rows_;
    cols = cols_;
    min_x = 0.;
    max_x = cols-1;
    min_y = 0.;
    max_y = rows-1;
    //fake_pixel = Pixel();
    if (rows == 0 || cols == 0) {
      data = NULL;
    } else {
      data = new Pixel[rows*cols];
    }
    //    for (int i = 0; i < rows*cols; i++) { data[i] = fake_pixel; }
    initialized = false;
  }

  //  void setInit

  
  // for visualizing the depth buffer
  // prooooobably shouldn't exist
  Image(int rows_, int cols_, const float* p) {
    //std::cout << "IMAGE CONSTRUCTOR THAT SHOULD BE DEPRECATED???" << std::endl;
    rows = rows_;
    cols = cols_;
    min_x = 0.;
    max_x = cols-1;
    min_y = 0.;
    max_y = rows-1;
    data = new Pixel[rows*cols];
    for (int i = 0; i < rows*cols; i++) { 
      //fake_pixel = p[i];
      data[i] = p[i];
      //fake_pixel; 
    }
    //    fake_pixel = Pixel();
  //  initialized = false;
  }
   

 /* Image(int rows_, int cols_, const Pixel &p) {
    rows = rows_;
    cols = cols_;
    min_x = 0.;
    max_x = cols-1;
    min_y = 0.;
    max_y = rows-1;
    fake_pixel = 0;
    data = new Pixel[rows*cols];
    for (int i = 0; i < rows*cols; i++) {
      data[i] = p;
    }
  }*/

  Image(int rows_, int cols_, const Pixel *p) {
    assert (rows_ != 0 && cols_ != 0);
    rows = rows_;
    cols = cols_;
    min_x = 0.;
    max_x = cols-1;
    min_y = 0.;
    max_y = rows-1;
    //fake_pixel = Pixel();
    data = new Pixel[rows*cols];
    for (int i = 0; i < rows*cols; i++) {
      data[i] = p[i];
    }
    initialized = true;
  }

  Image(int rows_, int cols_, double min_x_, double min_y_, double max_x_, double max_y_){
    assert (rows_ != 0 && cols_ != 0);
    rows = rows_;
    cols = cols_;
    min_x = min_x_;
    max_x = max_x_;
    min_y = min_y_;
    max_y = max_y_;
    //fake_pixel = Pixel();
    data = new Pixel[rows*cols];
    //    for (int i = 0; i < rows*cols; i++) { data[i] = fake_pixel; }
    initialized = false;
  }

  Image(const std::string& filename) { 
    //    fake_pixel = Pixel();
    try {
      load(filename); 
      //      assert (initialized == true);
    } catch (...) {
      std::cerr << "EXCEPTION" << std::endl;
    }
  }

  void load(const std::string& filename) { 
    //  void load_from_file(const std::string& filename) { 

    //    std::cout << "trying to load " << filename << std::endl;
    // ===============================================================
    // LOAD PPM FILE
    if (filename.substr(filename.size()-4,4) == ".ppm" ||
	filename.substr(filename.size()-4,4) == ".pgm") {
      load_ppm(filename);
    } 
    // ===============================================================
    // LOAD JPG FILE
    else if (filename.substr(filename.size()-4,4) == ".jpg" ||
	     filename.substr(filename.size()-5,5) == ".jpeg") {
      load_jpg(filename);
    } 
    // ===============================================================
    // LOAD PNG FILE
    else if (filename.substr(filename.size()-4,4) == ".png") {
      load_png(filename);
    } 
    // ===============================================================
    // ELSE PANIC
    else {
      std::cerr << "DON'T KNOW HOW TO LOAD THIS TYPE OF IMAGE: " << filename << std::endl;
      throw -1;
    }
    // ===============================================================
  }
  /*
  Image(FILE *fp){
    fake_pixel = 0;
    load_ppm_from_fp(fp);
  }
  */
  
#ifdef MPI_DEFINED
  Image(SocketReader & sr){
    //fake_pixel = Pixel();
    load_from_sr(sr);
  }
#endif

  // copy constructor
  Image(const Image &im) { copy(im); }
  // assignment operator
  Image& operator= (const Image &im) {
    // check for self assignment
    if (this != &im){
      destroy();
      copy(im);
    }
    return *this;
  }
  // destructor
  ~Image() { destroy();}


private:
  void copy(const Image &im) {
    rows = im.rows;
    cols = im.cols;
    min_x = im.min_x;
    min_y = im.min_y;
    max_x = im.max_x;
    max_y = im.max_y;
    if (rows == 0 && cols == 0) {
      data = NULL;
      //      assert (im.initialized == false);
      initialized = false;
    } else {
      //      assert (im.initialized == true);
      assert (min_x >= 0 && max_x <= cols-1);
      assert (min_y >= 0 && min_y <= rows-1);
      data = new Pixel[rows*cols];
      for (int j = min_x; j <= max_x; j++) {
	for (int i = min_y; i <= max_y; i++) {
	  data[linear_index(i,j)] = im.data[linear_index(i,j)];
	}
      }
      initialized = true;
    }
    //fake_pixel = im.fake_pixel;
    traits = im.traits;
  }
  void destroy() {
    delete[] data;
  }


public:

  void write(const std::string &filename) const {
    //void write(const std::string &filename, const std::string &comment="") const {
    //void write(const char *filename, const char *comment = NULL){
    //assert (comment == "");
    //std::cout << "IN WRITE: " << filename << std::endl;
    //fflush(stdout);
    //    assert (initialized == true);
    assert (filename != "");
    assert (filename.size() > 4);
    if (filename.substr(filename.size()-4,4) == ".ppm" ||
	filename.substr(filename.size()-4,4) == ".pgm") {
      save_ppm(filename);
    } 
    // ===============================================================
    // SAVE JPG FILE
    else if (filename.substr(filename.size()-4,4) == ".jpg" ||
	     filename.substr(filename.size()-5,5) == ".jpeg") {
      save_jpg(filename);
    } 
    // ===============================================================
    // SAVE PNG FILE
    else if (filename.substr(filename.size()-4,4) == ".png") {
      save_png(filename);
    } 
    // ===========================================
    else {
      std::cerr << "DON'T KNOW HOW TO SAVE THIS TYPE OF IMAGE: " << filename << std::endl;
      throw -1;
    }
  }
 
  void save_ppm(const std::string &filename) const;
  void save_png(const std::string &filename) const;
  void save_jpg(const std::string &filename) const;



  int getRows() const {return rows;}
  int getCols() const {return cols;}

  int linear_index(int row, int col){
#ifdef DEBUG
    assert (row >= 0 && row < rows && col >= 0 && col < cols);
#endif
    return row*cols+col;
  }

  Pixel& operator()(int row, int col){
    //    assert (initialized == true);
#ifdef DEBUG
    assert (row >= 0 && row < rows && col >= 0 && col < cols);
    if (row>=0 && row<rows && col>=0 && col<cols){
#endif
      return data[row*cols+col];
#ifdef DEBUG
    } else {
      WARNING("Invalid pixel (%d, %d) addressed", //, accessing fake pixel",
	      row, col);
      std::cerr << "Invalid pixel (" << row << "," << col << ") addressed" << std::endl;
      exit(1); //return Pixel(); //fake_pixel;
    }
#endif
  }

  Pixel operator()(int row, int col) const {
    //    assert (initialized == true);
#ifdef DEBUG
    if (col >= cols) {
      //std::cout << "checking " << row << " " << col << " (" << rows << " " << cols << ")" << std::endl;
    }
    assert (row >= 0 && row < rows && col >= 0 && col < cols);
    if (row>=0 && row<rows && col>=0 && col<cols){
#endif
      return data[row*cols+col];
#ifdef DEBUG
    } else {
      WARNING("Invalid pixel (%d, %d) addressed", //, accessing fake pixel",
	      row, col);
      return Pixel();
      //return fake_pixel;
    }
#endif
  }

  Pixel operator()(int linear_index) const {
    //    assert (initialized == true);
#ifdef DEBUG
    assert (linear_index >= 0 && linear_index < rows*cols);
    if (linear_index >=0 && linear_index < rows*cols){
#endif
      return data[linear_index];
#ifdef DEBUG
    } else {
      WARNING("Invalid pixel (%d) addressed", //, accessing fake pixel",
	      linear_index);
      return Pixel();
      //return fake_pixel;
    }
#endif
  }

  Pixel &operator()(int linear_index) {
    //    assert (initialized == true);
#ifdef DEBUG
    assert (linear_index >= 0 && linear_index < rows*cols);
    if (linear_index >=0 && linear_index < rows*cols){
#endif
      return data[linear_index];
#ifdef DEBUG
    } else {
      WARNING("Invalid pixel (%d) addressed",//, accessing fake pixel",
	      linear_index);
      //return fake_pixel;
      return Pixel();
    }
#endif
  }
  
  // use bilinear interpolation to get values at non-integer coords
  // !!!note: this only works for scalar images now; modify for use on vectors
  Pixel bilinear(double row, double col){
    //    assert (initialized == true);
    int rmin = int(floor(row));
    if (rmin < 0) rmin = 0;
    if (rmin > rows - 1){
      rmin = rows - 1;
    }
    int rmax = rmin + 1;
    if (rmax < 0) rmax = 0;
    if (rmax > rows - 1){
      rmax = rows - 1;
    }
    row -= rmin;
    int cmin = int(floor(col));
    if (cmin < 0) cmin = 0;
    if (cmin > cols - 1){
      cmin = cols - 1;
    }
    int cmax = cmin + 1;
    col -= cmin;
    if (cmax < 0) cmax = 0;
    if (cmax > cols - 1){
      cmax = cols - 1;
    }
    // note: calculations done in doubles
#if 0
    accum_Pixel_t i1 = convertVector3((*this)(rmin, cmin), 
				      base_accum_Pixel_t(0));
    accum_Pixel_t i2 = convertVector3((*this)(rmin, cmax),
				      base_accum_Pixel_t(0));
    accum_Pixel_t i12 = (1. - col) * i1 + col * i2;
    accum_Pixel_t i3 = convertVector3((*this)(rmax, cmin),
			      base_accum_Pixel_t(0));
    accum_Pixel_t i4 = convertVector3((*this)(rmax, cmax),
				      base_accum_Pixel_t(0));
    accum_Pixel_t i34 = (1. - col) * i3 + col * i4;
    accum_Pixel_t i = (1. - row) * i12 + row * i34;
    return convertVector3(i, base_Pixel_t(0));
#else
    accum_Pixel_t i1 = (*this)(rmin, cmin);
    accum_Pixel_t i2 = (*this)(rmin, cmax);
    accum_Pixel_t i12 = (1. - col) * i1 + col * i2;
    accum_Pixel_t i3 = (*this)(rmax, cmin);
    accum_Pixel_t i4 = (*this)(rmax, cmax);
    accum_Pixel_t i34 = (1. - col) * i3 + col * i4;
    accum_Pixel_t i = (1. - row) * i12 + row * i34;

    Pixel_t tmp = i;    
    Pixel_t tmp2;
    tmp2 = tmp;
    return tmp2;

#endif
  }


  int row(double y){
    return int(rows*(max_y-y)/(max_y-min_y));    
  }

  int col(double x){
    return int(cols*(x-min_x)/(max_x-min_x));
  }

  Pixel* getData(){  
    //assert (initialized == true); 
return data; }

  // use Bresenham's algorithm to apply a point functor to
  // all pixels along a line in the image
  void setLinePixels(int r2, int c2, int r1, int c1, Pixel new_value ){
    //    assert (initialized == true);
    if (r1 == r2 && c1 == c2){
      (*this)(r1,c1) = new_value;
      return;
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
      for (row=r2; row!=(r1+rstep); row+=rstep){
        (*this)(row,col) = new_value;
	err += dc;
        if (err > 0){
          col += cstep;
          err -= dr;
        }
      }
    } else {
      row = r2;
      err = - dc>>1;
      for (col=c2; col!=(c1+cstep); col+=cstep){
        (*this)(row,col) = new_value;        
	err += dr;
        if (err > 0){
          row += rstep;
          err -= dc;
        }
      }
    }
  }

 private:

  // IMAGE REPRESENTATION
  int rows;
  int cols;
  double min_x, min_y;
  double max_x, max_y;
  Pixel *data;
  bool initialized;
  //Pixel fake_pixel;
  PixelTraits<Pixel> traits;
  int SGN(int x) {return (x)<0?-1:(x)>0?1:0; }
  
  // private helper function
  void load_jpg(const std::string &filename); 
  void load_png(const std::string &filename); 
  void load_ppm(const std::string &filename); 

#ifdef MPI_DEFINED
  void load_from_sr(SocketReader & sr);
#endif

};

// defined in color_balance.cpp
void ColorBalance(Image<sRGB> &image, double lower_percentile = 0.2, double upper_percentile = 0.8, bool verbose=false);

//
// convert one image type to another via pixel-wise casts
//
template <class T1, class T2>
void convertImage(Image<T1> &src, Image<T2> &dst){
  assert(src.getRows() == dst.getRows());
  assert(src.getCols() == dst.getCols());
  for (int row=0; row<src.getRows(); row++){
    for (int col=0; col<src.getCols(); col++){
      dst(row,col) = T2(src(row,col));
    }
  }
}

// apply a supplied point-operator functor to each pixel
template <typename Pixel, typename Operator >
void ImagePointOperation(Image<Pixel> &image, Operator &operation){
  Pixel *pixel = image.getData();
  for (int row=0; row<image.getRows(); row++){
    for (int col=0; col<image.getCols(); col++){
      operation(*pixel++, row, col);
    }
  }
}

#include "Image.tpp"

#endif // #ifndef IMAGE_H_INCLUDED
