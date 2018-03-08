#ifndef IMAGE_OPS_H_
#define IMAGE_OPS_H_

#include "Matrix3.h"
#include "Vector3.h"

template<typename T>
T min(const T &a, const T&b){
  return a<b ? a : b;
}

template<typename T>
T max(const T &a, const T&b){
  return a>b ? a : b;
}

/* moved to Vector3.h
template <typename pixel_t>
Vector3<pixel_t> min(const Vector3<pixel_t> &x, const Vector3<pixel_t> &y){
  return Vector3<pixel_t>(min(x.r(), y.r()),
			  min(x.g(), y.g()),
			  min(x.b(), y.b()));
}

template <typename pixel_t>
Vector3<pixel_t> max(const Vector3<pixel_t> &x, const Vector3<pixel_t> &y){
  return Vector3<pixel_t>(max(x.r(), y.r()),
			  max(x.g(), y.g()),
			  max(x.b(), y.b()));
}
*/

//!!! note: could also keep track of coordinates of min, max
template <typename Pixel>
struct ImageExtrema {
  typedef Pixel Pixel_t;
  ImageExtrema(){
    min_value = PixelTraits<Pixel_t>::maximum_value();
    max_value = PixelTraits<Pixel_t>::minimum_value();
  }
  void operator()(const Pixel_t &pixel, int row, int col){
    min_value = min(pixel, min_value);
    max_value = max(pixel, max_value);
  }
  Pixel_t getMin() { return min_value; }
  Pixel_t getMax() { return max_value; }
private:
  Pixel_t min_value;
  Pixel_t max_value;
};

#define ImageMin ImageExtrema
#define ImageMax ImageExtrema

template <typename Pixel>
struct ImageClearer {
  typedef Pixel Pixel_t;
  ImageClearer(Pixel_t val){
    this->val = val;
  }
  void operator()(Pixel_t &pixel, int row, int col){
    pixel = val;
  }
private:
  Pixel_t val;
};


template <typename label_t>
class CCLookup {
 public:
  CCLookup(int block_size){
    this->block_size = block_size;
    max_label = block_size;
    table = new label_t[max_label];
    for (int i=0; i<max_label; ++i){
      table[i] = label_t(i);
    }
    current_label = label_t(0);
  }

  CCLookup& operator=(const CCLookup &l){
    FATAL_ERROR("unimplemented%s","");
  }

  CCLookup(const CCLookup &l){
    FATAL_ERROR("unimplemented%s","");
  }

  ~CCLookup(){
    delete [] table;
  }

  label_t next_label(){
    current_label++;
    if (current_label > max_label-1){
      int old_max_label = max_label;
      while (current_label > max_label-1){
	max_label += block_size;      
      }
      label_t *new_table = new label_t[max_label];
      int i;
      for (i=0; i<old_max_label; ++i){
	new_table[i] = table[i];
      }
      for (;i<max_label; ++i){
	new_table[i] = label_t(i);
      }
      delete [] table;
      table = new_table;
    }
    return current_label;
  }

  label_t& operator()(int index){
    return table[index];
  }

  // find minimum label, ignoring zeros
  label_t nzmin(label_t a, label_t b){
    if (label_t(0) == a){
      return b;
    } else if (label_t(0) == b){
      return a;
    }
    return a<b ? a : b;
  }

  // merge two labels
  void merge(label_t a, label_t b){
    if (label_t(0) != a && label_t(0) != b){
      label_t c = nzmin(find(a), find(b));
      table[a] = c;
      table[b] = c;
    }
  }

  // find minimum equivalent label
  label_t find(label_t a){
    if (table[a] == a){
      return a;
    } else {
      table[a] = find(table[a]);
      return table[a];
    }
  }
  
  // "reduce" labels to minimum equivalent
  void reduce(){
    int num_labels = 0;
    for (int i=0;i<max_label;++i){
      if (table[i] == i){
	table[i] = num_labels++;
      } else {
	table[i] = table[int(table[i])];
      }
    }
  }

 private:
  label_t current_label;
  int max_label;
  int block_size;
  label_t *table;

  template <typename p_t, typename l_t>
  friend Image<l_t> ConnectedComponents(const Image<p_t>, l_t&);
};

template <typename pixel_t, typename label_t>
Image<label_t> ConnectedComponents(const Image<pixel_t> &image,
				   label_t &num_labels){

  Image<label_t> answer_labels(image.getRows(), image.getCols());
  ImageClearer<label_t> clearer(label_t(0));
  ImagePointOperation(answer_labels, clearer);
  
  // simple heuristic for lookup table block allocation
  int block_size = image.getRows() + image.getCols();
  CCLookup<label_t> lookup(block_size);

  if (image.getRows() < 1 || image.getCols() < 2){
    FATAL_ERROR("Image too small for connected components%s","");
  }

  //
  // intial labeling pass through image
  //

  // first pixel
  if (pixel_t(0) != image(0,0)){
    answer_labels(0,0) = lookup.next_label();
  }
  // first row
  for (int col=1; col<image.getCols(); ++col){
    if (pixel_t(0) != image(0, col)){
      if (label_t(0) != answer_labels(0, col-1)){
	answer_labels(0, col) = answer_labels(0, col-1);
      } else {
	answer_labels(0, col) = lookup.next_label();
      }
    }
  }
  // rest of rows
  for (int row=1; row<image.getRows(); ++row){
    // first col
    if (pixel_t(0) != image(row, 0)){
      label_t min_label = answer_labels(row-1, 0);
      min_label = lookup.nzmin(min_label, answer_labels(row-1, 1));      
      if (label_t(0) == min_label){
	answer_labels(row, 0) = lookup.next_label();
      } else {
	min_label = lookup.find(min_label);
	answer_labels(row, 0) = min_label;
	lookup.merge(answer_labels(row-1, 0), min_label);
	lookup.merge(answer_labels(row-1, 1), min_label);
      }
    }
    // middle cols
    int col;
    for (col=1; col<image.getCols()-1; ++col){
      if (pixel_t(0) != image(row, col)){
      	label_t min_label = answer_labels(row, col-1);
	min_label = lookup.nzmin(min_label, answer_labels(row-1, col-1));
	min_label = lookup.nzmin(min_label, answer_labels(row-1, col));      
	min_label = lookup.nzmin(min_label, answer_labels(row-1, col+1));      
	if (label_t(0) == min_label){
	  answer_labels(row, col) = lookup.next_label();
	} else {
	  min_label = lookup.find(min_label);
	  answer_labels(row, col) = min_label;
	  lookup.merge(answer_labels(row, col-1), min_label);
	  lookup.merge(answer_labels(row-1, col-1), min_label);
	  lookup.merge(answer_labels(row-1, col), min_label);
	  lookup.merge(answer_labels(row-1, col+1), min_label);
	}
      }
    }
    // last col
    if (pixel_t(0) != image(row, col)){
      label_t min_label = answer_labels(row, col-1);
      min_label = lookup.nzmin(min_label, answer_labels(row-1, col-1));
      min_label = lookup.nzmin(min_label, answer_labels(row-1, col));      
      if (label_t(0) == min_label){
	answer_labels(row, col) = lookup.next_label();
      } else {
	min_label = lookup.find(min_label);
	answer_labels(row, col) = min_label;
	lookup.merge(answer_labels(row, col-1), min_label);
	lookup.merge(answer_labels(row-1, col-1), min_label);
	lookup.merge(answer_labels(row-1, col), min_label);
      }
    }
  }

  //
  // reverse merge pass
  //
  for (int row=answer_labels.getRows()-2; row>0; row--){
    for (int col=answer_labels.getCols()-2; col>0; col--){
      if (pixel_t(0) != image(row,col)){
	label_t min_label = answer_labels(row, col);
	min_label = lookup.nzmin(min_label, answer_labels(row-1, col-1));
	min_label = lookup.nzmin(min_label, answer_labels(row-1, col));      
	min_label = lookup.nzmin(min_label, answer_labels(row-1, col+1));      
	min_label = lookup.nzmin(min_label, answer_labels(row, col-1));
	min_label = lookup.nzmin(min_label, answer_labels(row, col+1));
	min_label = lookup.nzmin(min_label, answer_labels(row+1, col-1));
	min_label = lookup.nzmin(min_label, answer_labels(row+1, col));      
	min_label = lookup.nzmin(min_label, answer_labels(row+1, col+1));    
	min_label = lookup.find(min_label);
	lookup.merge(answer_labels(row-1, col-1), min_label);
	lookup.merge(answer_labels(row-1, col), min_label);
	lookup.merge(answer_labels(row-1, col+1), min_label);  
	lookup.merge(answer_labels(row, col-1), min_label);
	lookup.merge(answer_labels(row, col), min_label);
	lookup.merge(answer_labels(row, col+1), min_label);
	lookup.merge(answer_labels(row+1, col-1), min_label);
	lookup.merge(answer_labels(row+1, col), min_label);
	lookup.merge(answer_labels(row+1, col+1), min_label);
      }
    }
  }

  // remove numbering gaps
  lookup.reduce();

  //
  // pass through image to re-number pixel answer_labels
  //
  num_labels = 0;
  for (int row=0; row<answer_labels.getRows(); ++row){
    for (int col=0; col<answer_labels.getCols(); ++col){
      answer_labels(row,col) = lookup(answer_labels(row,col));
      if (answer_labels(row,col) > num_labels) num_labels = answer_labels(row,col);
    }
  }
  num_labels++;

  return answer_labels;
}

template <typename pixel_t>
Image<pixel_t> sRGBtoGray(const Image<Vector3<pixel_t> > &in){
  Image<pixel_t> out(in.getRows(), in.getCols());
  for (int row=0; row<in.getRows(); ++row){
    for (int col=0; col<in.getCols(); ++col){
      out(row, col) = pixel_t(0.299 * in(row,col)(0) +
			      0.587 * in(row,col)(1) +
			      0.114 * in(row,col)(2));
    }
  }
  return out;
}

// BARB: had to put this in so it would compile on Mac
// complains that default template typenames are not allowed
#ifdef __APPLE__
template <typename pixel_in_t, typename pixel_out_t>
#else
template <typename pixel_in_t, typename pixel_out_t = pixel_in_t>
#endif
Image<pixel_out_t> ThresholdImage(const Image<pixel_in_t> &in, 
				  pixel_in_t threshold){
  PixelTraits<pixel_out_t> traits;
  Image<pixel_out_t> out(in.getRows(), in.getCols());
  for (int row=0; row<in.getRows(); row++){
    for (int col=0; col<in.getCols(); col++){
      if (in(row,col) > threshold){
	out(row,col) = traits.maximum_value();
      } else {
	out(row,col) = traits.minimum_value();
      }
    }
  }
  return out;
}

namespace ImageEdge {
  enum edgetype {SOBEL, SCHARR};
}

template <typename pixel_t>
Image<pixel_t> FindEdges(const Image<pixel_t> &input, 
			 ImageEdge::edgetype edge_type){
  m3d kernel;

  switch (edge_type){
  case ImageEdge::SOBEL:
    kernel = m3d(1., 2., 1.,
		 0., 0., 0.,
		 -1., -2., -1.);
    break;
  case ImageEdge::SCHARR:
    kernel = m3d(3., 10., 3.,
		 0., 0., 0.,
		 -3., -10., -3.);
    break;
  default:
    FATAL_ERROR("Unknown edge type%s","");
  }

  m3d kernel_h = kernel;
  m3d kernel_v = kernel.transpose();

  Image<pixel_t> EdgeImage(input.getRows(), input.getCols());
  for (int row=0; row<input.getRows(); row++){
    for (int col=0; col<input.getCols(); col++){
      pixel_t h_sum = pixel_t(0);
      pixel_t v_sum = pixel_t(0);
      int count = 0;
      for (int r_off = -1; r_off <= 1; r_off++){
	int r = row+r_off;
	if (r >=0 && r < input.getRows()){
	  for (int c_off = -1; c_off <= 1; c_off++){
	    int c = col+c_off;
	    if (c >=0 && c < input.getCols()){
	      h_sum += kernel_h(r_off+1, c_off+1) * input(r, c);
	      v_sum += kernel_v(r_off+1, c_off+1) * input(r, c);
	      count++;
	    }
	  }
	}
      }
      pixel_t edge = sqrt(h_sum * h_sum + v_sum * v_sum) / count;
      EdgeImage(row, col) = edge;
    }
  }

  return EdgeImage;
}

template<typename pixel_t>
Image<pixel_t> operator-(const Image<pixel_t> &a, const Image<pixel_t> &b){
  if (a.getRows() != b.getRows() ||
      a.getCols() != b.getCols()){
    assert(0);
  }
  Image<pixel_t> result(a.getRows(), a.getCols());
  for (int row=0; row<a.getRows(); row++){
    for (int col=0; col<a.getCols(); col++){
      result(row, col) = a(row, col) - b(row, col);
    }
  }
  return result;
}

template<typename pixel_t>
Image<pixel_t> &operator-=(Image<pixel_t> &a, const Image<pixel_t> &b){
  if (a.getRows() != b.getRows() ||
      a.getCols() != b.getCols()){
    assert(0);
  }
  for (int row=0; row<a.getRows(); row++){
    for (int col=0; col<a.getCols(); col++){
      a(row, col) = a(row, col) - b(row, col);
    }
  }
  return a;
}


template<typename pixel_t>
Image<pixel_t> operator+(const Image<pixel_t> &a, const Image<pixel_t> &b){
  if (a.getRows() != b.getRows() ||
      a.getCols() != b.getCols()){
    assert(0);
  }
  Image<pixel_t> result(a.getRows(), a.getCols());
  for (int row=0; row<a.getRows(); row++){
    for (int col=0; col<a.getCols(); col++){
      result(row, col) = a(row, col) + b(row, col);
    }
  }
  return result;
}

template<typename pixel_t>
Image<pixel_t> &operator+=(Image<pixel_t> &a, const Image<pixel_t> &b){
  if (a.getRows() != b.getRows() ||
      a.getCols() != b.getCols()){
    assert(0);
  }
  for (int row=0; row<a.getRows(); row++){
    for (int col=0; col<a.getCols(); col++){
      a(row, col) = a(row, col) + b(row, col);
    }
  }
  return a;
}


// write a histogram stretcher which gets constructed from either min, max values
// or directly from an ImageExtrema class to stretch fully.

//!!! how to write a generic template-based efficient neighborhood-op function ???
#endif // #ifndef IMAGE_OPS_H_
