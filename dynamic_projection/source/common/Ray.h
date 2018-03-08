#ifndef RAY_H_INCLUDED_
#define RAY_H_INCLUDED_

#include <cassert>
#include <Vector3.h>
#include <Photon.h>
#include <math.h>
#include "rng.h"

class Ray 
{
public:
  Ray(){};
  Ray(const Vector3<double> &origin, 
      const Vector3<double> &direction, 
      const Photon &p, double length = 0.): photon(p)
  {
    this->origin = origin;
    this->direction = direction;
    this->direction.normalize();
    this->length = length;
  }
  
  // accessors
  Vector3<double> getOrigin() const {return origin;}
  Vector3<double> getDirection() const {return direction;}
  double getLength() const {return length;}
  void setOrigin(const Vector3<double> &new_origin){
    origin = new_origin;
  }
  void setDirection(const Vector3<double> &new_dir) {
    direction = new_dir;
    direction.normalize();
  }
  void setLength(double new_length){
    length = new_length;
  }
  Photon getPhoton() const {return photon;};
  Photon& getPhoton() {return photon;};
  
  // attenuate the photon's intensity
  void attenuate(double factor){
    assert(factor >= 0.0 && factor <= 1.0);
    photon.setIntensity(photon.getIntensity() * factor);
  }

  // advance ray along its direction, updating its origin
  void advance(double distance){
    origin += distance * direction;
  }

  // reflect ray from surface
  // note: this routine expects that the ray's origin has
  // already been advanced to the reflection point by advance()
  void reflect(const Vector3<double> &normal){
    double cos_th = -normal.dot(direction);
    direction += 2.0 * cos_th * normal;
    epsilon_magic();
  }

  // refract ray at refractive index discontinuity
  // algorithm from:
  // http://en.wikipedia.org/wiki/Snell's_law
  void refract(Vector3<double> normal, double n1, double n2){
    double costh1 = -normal.dot(direction);
    double discriminant = 1.0 - (n1/n2)*(n1/n2)*(1-costh1*costh1);

    if (discriminant >= 0.0){
      double costh2 = sqrt(discriminant);
      if (costh1 >= 0.0){
        direction = (n1/n2) * direction + (costh1*n1/n2 - costh2) * normal;
      } else {
        direction = (n1/n2) * direction - (costh1*n1/n2 + costh2) * normal;
      }
      epsilon_magic();
#define CHECK_SNELL
#ifdef CHECK_SNELL
      // check Snell's law:
      double sinth1 = sqrt(1-costh1*costh1);
      double sinth2 = sqrt(1-pow(normal.dot(direction), 2.0));
      if (fabs(n2/n1 - sinth1/sinth2) > 0.0001){
        printf("snell mismatch %f = %f\n", n2/n1, sinth1/sinth2);
      }
#endif
    } else {
      // total internal reflection
      reflect(normal);
    }
  }

  // interacts probabilistically with interface
  // nb: maybe should have media objects instead of n1, n2.  Could have
  //     other parameters stored.
  void interact(Vector3<double> normal, double n1, double n2){
    double cs_th_i = -normal.dot(direction);
    double discriminant = 1. - (n1/n2)*(n1/n2)*(1.-cs_th_i*cs_th_i);
    if (discriminant >= 0.0){
      double cs_th_t = sqrt(discriminant);
      double Rs = (n1 * cs_th_i - n2 * cs_th_t) / (n1 * cs_th_i + n2 * cs_th_t);
      Rs = Rs * Rs;
      double Rp = (n1 * cs_th_t - n2 * cs_th_i) / (n1 * cs_th_t + n2 * cs_th_i);
      Rp = Rp * Rp;
      double R = 0.5 * (Rs + Rp);
      double T = 1. - R;
      // for now, choose one director or the other randomly.  Don't adjust
      // intensities
      // note these can be optimized inline since the math is now done twice
      if (UniformDeviate() > T){
	// reflect only
	reflect(normal);
      } else {
	// refract only
        refract(normal, n1, n2);
      }
    } else {
      // total internal reflection
      reflect(normal);
    }
  }

private:
  // move the ray a little bit along its direction to avoid
  // self-intersections in raytracers
  // NOTE: epsilon value may need to be tweaked
  void epsilon_magic(){
    advance(epsilon);
  }
  static double epsilon;
  Vector3<double> origin;
  Vector3<double> direction;
  double length;
  Photon photon;
};

double Ray::epsilon = 1e-6; //!!! tune this parameter

#endif // #ifndef RAY_H_INCLUDED_
