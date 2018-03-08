#ifndef _COLORTOKEN_H_
#define _COLORTOKEN_H_

#include <cmath>
#include "circle.h"
#include "object.h"
#include "histogram.h"
#include "color.h"
#include "quad.h"

class ColorToken : public Object {
public:
  ColorToken (std::vector<Point> &edge_points, 
	      std::vector<Point> &points, 
	      Image<byte> &component_image,
	      Image<sRGB> &color_image,
	      Quad quad, Histogram &histogram);

  virtual bool isColorToken() { return true; }

  void draw(Image<sRGB> &image){
    //const double r = 3.5;
    //double r2 = r*r;
    const double inner_r = 9.;
    //const double outer_r = 12.;
    const double outer_r = 20.;
    double inner_r2 = inner_r * inner_r;
    double outer_r2 = outer_r * outer_r;
    for (int row = int(center.row-outer_r-1); 
         row <= int(center.row+outer_r+1); row++){
      for (int col = int(center.col-outer_r-1);
           col <= int(center.col+outer_r+1); col++){
        double rad2 = ((row-center.row)*(row-center.row) +
                       (col-center.col)*(col-center.col));
        if (/*rad2 < r2 ||*/ (rad2 > inner_r2 && rad2 < outer_r2)){
          image(row, col) = color; //sRGB(128, 128, 128);
        }
      }
    }

/*
    for (int i=0;i<200; i++){
      double theta = 2*M_PI*i/200.;
      int row = int(center.row + sample_radius*sin(theta));
      int col = int(center.col + sample_radius*cos(theta));
      image(row, col) = color; //sRGB(192, 192, 192);
    }
*/
  }


  void project(CalibratedCamera &camera){
    /*
    double height = 0.0508; // 0.0508 meters = 2 inches
    v3d unpadded_points[4];
    for (int i=0; i<4; i++){
      unpadded_points[i] = camera.PointFromPixel(corner_points[i].row,
						 corner_points[i].col,
						 height);
    }

    // add padding
    const double platform_padding = 0.00635; // 0.00635 meters = 0.25 inches
    for (int i=0; i<4; i++){
      v3d v1 = unpadded_points[i] - unpadded_points[(i+1)%4];
      v3d v2 = unpadded_points[i] - unpadded_points[(i+3)%4];
      v1.normalize();
      v2.normalize();
      world_points[i] = unpadded_points[i] + (v1 + v2) * platform_padding;
    }

    //    std::cout << "platform ";
    for (int i=0; i<4; i++){
      v3d v1 = world_points[i] - world_points[(i+1)%4];
      //std::cout << "  " << v1.length() * 39.3700787;
      //printf("distance = %f\n", v1.length() * 39.3700787);
    }
    //    std::cout << "\n";
    */

  }

  //  const Point& getCenter() { assert (0); exit(0); }
  const Point& getCenter() const { return center; }
  const sRGB& getColor() const { return color; }
  

  void write(FILE *fp){
    //    fprintf (fp, "wall_material %f %f %f\n", color.r()/255.0, color.g()/255.0, color.b()/255.0);
    //    fprintf (fp, "   center %f %f\n", center.row, center.col);
  }

  int type;  //enum {GLOBAL_WALL, SPECIFIC_WALL, FLOOR, CEILING} type;
  sRGB color;
  Point center;
  //double sample_radius;
  //  bool valid;
};

#endif
