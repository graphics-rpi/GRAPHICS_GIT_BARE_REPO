#ifndef _UNIDENTIFIED_H_
#define _UNIDENTIFIED_H_

#include "object.h"

// ======================================================================
/// UNIDENTIFIED
// ======================================================================

class UnidentifiedObject : public Object {
public:
  UnidentifiedObject(Image<byte> &component_image, std::vector<Point> &points){
    center_row = 0;
    center_col = 0;
    // estimate image-space center and radius for overlay image
    for(unsigned int i = 0; i < points.size(); i++){
      center_row += points[i].row;
      center_col += points[i].col;
    }
    assert (points.size() > 0);
    center_row /= points.size();
    center_col /= points.size();
  }

  virtual void project(CalibratedCamera&) {}
  virtual void write(FILE*) {}

  void draw(Image<sRGB> &image){
    int rows = image.getRows();
    int cols = image.getCols();
    for (int i=-10; i<11; i++){
      if (center_row+i >= 0 && center_row+i < rows && 
	  center_col   >= 0 && center_col   < cols) {
	image(center_row + i, center_col) = sRGB(255, 255, 255);
      }
      if (center_row   >= 0 && center_row   < rows && 
	  center_col+i >= 0 && center_col+i < cols) {
	image(center_row, center_col + i) = sRGB(255, 255, 255);
      }
    }
  }

private:
  double center_row;
  double center_col;
};

#endif
