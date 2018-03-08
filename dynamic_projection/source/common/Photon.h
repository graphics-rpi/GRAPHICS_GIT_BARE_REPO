#ifndef PHOTON_H_INCLUDED
#define PHOTON_H_INCLUDED
#include <math.h>
#include <util.h>

// really a "packet" of photons at a specific wavelength
class Photon 
{
public:
  enum spectrum_t {SPECTRUM_ASTM_E490};
  Photon(){}
  Photon(double lambda, double intensity){
    this->lambda = lambda;
    this->intensity = intensity;
  }
  // intensity set by Planck's radiation law
  Photon(double lambda){
    this->lambda = lambda;
    this->intensity = BBRadiance(lambda);
  }
  // set intensity by ASTM E490 extra-terrestrial irradiance model
  Photon(double lambda, spectrum_t spect){
    switch(spect){
    case SPECTRUM_ASTM_E490:
      this->lambda = lambda;
      this->intensity = E490_lookup(lambda);
      break;
    default:
      FATAL_ERROR("unknown spectrum type");
      break;
    }
  }

  double getLambda() const {return lambda;}
  double getIntensity() const {return intensity;}
  void setIntensity(double intensity) {this->intensity = intensity;}
  static Photon BlackBody(double lambda, double temperature){
    Photon photon;
    photon.lambda = lambda;
    photon.intensity = BBRadiance(lambda, temperature);
    return photon;
  }
private:
  double lambda;
  double intensity;

  static double E490_lookup(double lambda){
    extern double ASTM_E490_table[401];
    int idx = int(lambda - 380.);
    if (idx < 0) idx = 0;
    if (idx > 400) idx = 400;
    return ASTM_E490_table[idx];
  }

  //  Calculate, by Planck's radiation law, the emittance of a black body
  //  of temperature bbTemp at the given wavelength (in nm).
  //
  // algorithm from: http://www.fourmilab.ch/documents/specrend/specrend.c
  //
  static double BBRadiance(double wavelength, double bbTemp = 5778.0){
    double wlm = wavelength * 1e-9;   /* Wavelength in meters */
    
    return ( (3.74183e-16 * pow(wlm, -5.0)) /
	     (exp(1.4388e-2 / (wlm * bbTemp)) - 1.0) ) / 9.1983e13;
  }
};

#endif // #ifndef PHOTON_H_INCLUDED

