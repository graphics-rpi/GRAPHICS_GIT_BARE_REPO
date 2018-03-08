#ifndef _IMAGE_GAMMA_H_
#define _IMAGE_GAMMA_H_

#include "Vector3.h"

typedef uint8_t byte;
typedef Vector3<byte> sRGB;

//
// simple function to clamp values into (0,1)
//
inline double sRGBclamp(double c){
  if (c < 0.0) return 0.0;
  if (c > 1.0) return 1.0;
  return c;
}

#include <iostream>

inline std::ostream& operator<< (std::ostream &ostr, const sRGB &c) {
  ostr << "{" << (int)c.r() << "," << (int)c.g() << "," << (int)c.b() << "}";
  return ostr;
}

//
// sRGB gamma function
// data and algorithm from:
// http://en.wikipedia.org/wiki/SRGB_color_space
//
inline double sRGBgamma(double c){
  if (c < 0.0031308){
    return 12.92 * c;
  } else {
    return 1.055 * pow(c, 1/2.4) - 0.055;
  }
}

//
// BT REC709 gamma function
//
inline double REC709gamma(double c){
  if (c <= 0.018){
    return 4.5 * c;
  } else {
    return (1.099 * pow(c, 0.45)) - 0.099;
  }
}

//
// BT REC709 inverse gamma function
//
inline double REC709degamma(double c){
  if (c <= 0.081){
    return c / 4.5;
  } else {
    return pow((c+0.099)/1.099, 1./0.45);
  }
}

//
// scale 0-1 rgb data to 0-255 with proper rounding
//
inline double sRGB255(double c){
  c = floor(255. * c + 0.5);
  if (c > 255.) return 255.;
  return c;
}

//
// sRGB de-gamma function
// data and algorithm from:
// http://en.wikipedia.org/wiki/SRGB_color_space
//
inline double sRGBdegamma(double c){
  if (c < 0.04045){
    return c / 12.92;
  } else {
    return pow((c + 0.055)/1.055, 2.4);
  }
}

#endif
