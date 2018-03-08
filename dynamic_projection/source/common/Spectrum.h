#ifndef SPECTRUM_H_INCLUDED
#define SPECTRUM_H_INCLUDED

#include <cstdio>
#include <stdint.h>
#include <math.h>
#include "Vector3.h"
#include "CIE.h"
#include "Image.h"
#include "Photon.h"

double max(double a, double b){
  return a > b ? a : b;
}

class Spectrum
{
public:
  const static int MinLambda = 380;
  const static int MaxLambda = 780;
  const static int LambdaStep = 2;
  const static int NumSpectralSamples = ((MaxLambda - MinLambda + 1) / 
					 LambdaStep);

  Spectrum(){
    for (int i=0; i<NumSpectralSamples; i++){
      intensity[i] = 0.0;
    }
  }

  void clear(){
    for (int i=0; i<NumSpectralSamples; i++){
      intensity[i] = 0.0;
    }    
  }

  void operator+=(const Photon &photon){
    if (photon.getLambda() >= MinLambda && photon.getLambda() <= MaxLambda){
      int idx = int((photon.getLambda() - MinLambda)/LambdaStep);
      if (idx >= 0 && idx < NumSpectralSamples){
        intensity[idx] += photon.getIntensity();
      }
    }
  }

  void operator+=(const Spectrum &s){
    for (int i=0; i<NumSpectralSamples; i++){
      intensity[i] += s.intensity[i];
    }    
  }

  void filter(double (*filter_function)(double)){
    for (int i=0; i<NumSpectralSamples; i++){
      double lambda = MinLambda + i * LambdaStep;
      intensity[i] *= filter_function(lambda);
    }
  }

  Spectrum &operator*= (double a){
    for (int i=0; i<NumSpectralSamples; i++){
      intensity[i] *= a;
    }
    return *this;
  }

  operator Vector3<double>() const {
    Vector3<double> XYZ(0.0, 0.0, 0.0);
    for (int i=0; i<NumSpectralSamples; i++){
      int l = MinLambda + i * LambdaStep;
      if (l >= CIEMinWavelength && l <= CIEMaxWavelength){
        int idx = l - CIEMinWavelength;
        XYZ += intensity[i] * lambda_to_XYZ[idx];
      }
    }
    Vector3<double> rgb = XYZ_to_sRGB * XYZ;

    // experimental camera calibration step
    m3d wb(0.7483453102,        0.0000000000,        0.0000000000, 
	   0.0000000000,        0.5643086428,        0.0000000000, 
	   0.0000000000,        0.0000000000,        0.3680657119);
 
    m3d full(0.7926959925,       -0.0621104653,       -0.0267108641, 
	     0.0137148773,        0.5552201746,        0.0044866426, 
	     0.0113777478,       -0.0126055872,        0.3686788024); 


    //rgb = wb * rgb / 0.7483453102;
    //rgb = full * rgb / 0.7926959925;

    // clip negative values instead of moving to gamut edge
    //rgb.r() = max(rgb.r(), 0.);
    //rgb.g() = max(rgb.g(), 0.);
    // rgb.b() = max(rgb.b(), 0.);
    
    // move out-of-gamut colors onto gamut edge by moving towards
    // gray point of equal Y.  Method from:
    // http://mintaka.sdsu.edu/GF/explain/optics/rendering.html#CIEdiag
    double m = rgb.min();
    if (m < 0.0){
      double f = XYZ.y()/(XYZ.y() - m);
      rgb = Vector3<double>(XYZ.y(), XYZ.y(), XYZ.y()) + 
        f * (rgb - Vector3<double>(XYZ.y(), XYZ.y(), XYZ.y()));
    }
    
    return rgb;
  }
  
  operator sRGB() const {
    Vector3<double> rgb = Vector3<double>(*this);
    rgb.apply(sRGBclamp);
    rgb.apply(sRGBgamma);
    rgb *= 255.0;
    return sRGB(rgb);
  }

  void read(char *filename){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      fprintf(stderr, "cannot open %s for reading\n", filename);
      assert(0);
    }

    for (int i=0; i<NumSpectralSamples; i++){
      fscanf(fp, "%lf", &intensity[i]);
    }

    fclose(fp);
  }

  void write(char *filename){
    FILE *fp = fopen(filename, "wt");
    if (NULL == fp){
      fprintf(stderr, "cannot open %s for writing\n", filename);
      assert(0);
    }

    for (int i=0; i<NumSpectralSamples; i++){
      fprintf(fp, "%lf\n", intensity[i]);
    }

    fclose(fp);
  }
private:
  double intensity[NumSpectralSamples];
};


#endif // #ifndef SPECTRUM_H_INCLUDED

