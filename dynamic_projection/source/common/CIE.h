#ifndef CIE_H_INCLUDED_
#define CIE_H_INCLUDED_
#include "Vector3.h"
#include "Matrix3.h"
#include "Image.h"
#include <math.h>

extern Matrix3<double> XYZ_to_sRGB;
extern Matrix3<double> sRGB_to_XYZ;
const int CIEMinWavelength = 380;
const int CIEMaxWavelength = 780;
const int CIENumWavelengths = (CIEMaxWavelength - CIEMinWavelength + 1);
const double CIE_XYZ_PEAK = 1.782968200000;
extern Vector3<double> lambda_to_XYZ[CIENumWavelengths];
extern double lambda_to_Vm[CIENumWavelengths];
extern double lambda_to_D65[81];
extern double lambda_to_ExViewHAD[601];

#ifndef CIE_CPP_
v3d XYZ_to_xyY(v3d XYZ){
  double sum = XYZ.x() + XYZ.y() + XYZ.z();
  v3d xyY(XYZ.x()/sum, XYZ.y()/sum, XYZ.y());
  return xyY;
}

// lambda in nm
v3d wavelength_to_XYZ(double lambda){
#ifdef OLD_wavelength_code
  int idx = int(lambda - 380.);
  if (idx < 0) idx = 0;
  if (idx > 400) idx = 400;
  return lambda_to_XYZ[idx];
#else
  // interpolate
  int idx1 = int(lambda - 380.);
  int idx2 = idx1 + 1;
  if (idx1 < 0 || idx2 >= 400){
    return v3d(0., 0., 0.);
  }
  double a = lambda - 380. - idx1;
  return (1.-a) * lambda_to_XYZ[idx1] + a * lambda_to_XYZ[idx2];
#endif
}

double wavelength_to_Vm(double lambda){
  // interpolate
  int idx1 = int(lambda - 380.);
  int idx2 = idx1 + 1;
  if (idx1 < 0 || idx2 >= 400){
    return 0.;
  }
  double a = lambda - 380. - idx1;
  return (1.-a) * lambda_to_Vm[idx1] + a * lambda_to_Vm[idx2];
}

double wavelength_to_D65(double lambda){
  // interpolate
  int idx1 = int(lambda - 380.)/5.;
  int idx2 = idx1 + 1;
  if (idx1 < 0 || idx2 >= 81){
    return 0.;
  }
  double a = (lambda - 380.)/5. - idx1;
  return ((1.-a) * lambda_to_D65[idx1] + a * lambda_to_D65[idx2])/109.354;
}

double wavelength_to_ExViewHAD(double lambda){
  // interpolate
  int idx1 = int(lambda - 400.);
  int idx2 = idx1 + 1;
  if (idx1 < 0 || idx2 >= 600){
    return 0.;
  }
  double a = lambda - 400. - idx1;
  return (1.-a) * lambda_to_ExViewHAD[idx1] + a * lambda_to_ExViewHAD[idx2];
}


static double lab_f_(double t){
  const double thresh = 0.00885645167903563; // (6/29)^3
  if (t > thresh){
    return pow(t, 1./3.);
  } else {
    // 4/29 + (29/6)^2 * t / 3
    return 0.137931034482759 + 7.78703703703704 * t;
  }
}

v3d XYZ_to_Lab(v3d XYZ){
  double L = 116. * lab_f_(XYZ.y()) - 16;
  double a = 500. * (lab_f_(XYZ.x()) - lab_f_(XYZ.y()));
  double b = 200. * (lab_f_(XYZ.y()) - lab_f_(XYZ.z()));
  v3d Lab(L, a, b);
  return Lab;
}

v3d Lab_to_XYZ(v3d Lab){
  double d = 6./29.;
  const double d32 = 3.*d*d;
  const double g =  16./116.;
  double fy = (Lab(0) + 16.)/116.;
  double fx = (fy + Lab(1))/500.;
  double fz = (fy - Lab(2))/200.;
  double x, y, z;
  if (fy > d){
    y = fy*fy*fy;
  } else {
    y = d32*(fy - g);
  }
  if (fx > d){
    x = fx*fx*fx;
  } else {
    x = d32*(fx - g);
  }
  if (fz > d){
    z = fz*fz*fz;
  } else {
    z = d32*(fz - g);
  }

  v3d XYZ(x, y, z);
  return XYZ;
}

v3d sRGB_to_Lab(sRGB rgb){
  v3d linear_rgb = v3d(rgb)/v3d(255., 255., 255.);
  linear_rgb.apply(sRGBdegamma);
  v3d xyz = sRGB_to_XYZ * linear_rgb;
  return XYZ_to_Lab(xyz);
}

sRGB Lab_to_sRGB(v3d lab){
  v3d xyz = Lab_to_XYZ(lab);
  v3d rgb = XYZ_to_sRGB * xyz;
  rgb.apply(sRGBclamp);
  rgb.apply(sRGBgamma);
  rgb.apply(sRGB255);
  return sRGB(rgb);
}

#endif

#endif //#ifndef CIE_H_INCLUDED_
