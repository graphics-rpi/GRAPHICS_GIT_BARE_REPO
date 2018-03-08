// test of Image class
#include <iostream>
#define DEBUG
#include "Image.h"
#include "ImageOps.h"

// silly test of template-metaprogrammed image operations - divide pixels by some value
template <class Pixel>
struct ImageDarkener {
  typedef Pixel Pixel_t;
  ImageDarkener(double value){
    this->value = value;
  }
  void operator()(Pixel &pixel, int row, int col){
    pixel = Pixel(pixel/value);
  }
private:
  double value;
};

// maybe call "pipeline"?
template <typename T1, typename T2>
// how to parameterize this template??
// also note: this is dereferencing pointers for each operation
// that might be a problem
struct CompositePixelOperation {
  typedef typename T1::Pixel_t Pixel_t;
  CompositePixelOperation(T1 &op1, T2 &op2){
    operation1 = &op1;
    operation2 = &op2;
  }
  void operator()(Pixel_t &pixel, int row, int col){
    (*operation1)(pixel, row, col);
    (*operation2)(pixel, row, col);
  }
private:
  T1 *operation1;
  T2 *operation2;
};

int main(int argc, char **argv){
  /*
  Image<byte> image(argv[1]);
  ImageDarkener<byte> darkener(5.0);
  ImagePointOperation(image, darkener);
  image.write(argv[2]);
  */
  typedef byte im_t;
  Image<im_t> a(100,100);

#if 0
  // run ops sequentially (two calls)
 
  ImageClearer<im_t> clearer(100);
  //typedef typeof(a) a_t;
  //ImageClearer<a_t::Pixel_t> clearer(100);
  ImagePointOperation(a, clearer);
  a(50,50) = 10;
  a(51,51) = 252;

  ImageMax<im_t> min_finder;
  ImagePointOperation(a, min_finder);
#else
  // run ops as a single "composite" operation

  ImageClearer<im_t> clearer(100);
  ImageMax<im_t> min_finder;
  CompositePixelOperation<ImageClearer<im_t>,ImageMax<im_t> >
    pipeline(clearer, min_finder);
  ImagePointOperation(a, pipeline);
  
#endif

  std::cout << int(min_finder.getMin()) << std::endl;
  std::cout << int(min_finder.getMax()) << std::endl;

}
