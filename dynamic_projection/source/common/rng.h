#ifndef RNG_H_INCLUDED_
#define RNG_H_INCLUDED_
#include "MersenneTwister.h"

class RandomNumberGenerator {
public:
  double operator() () {
    return mtrand.rand53();
  }
private:
  MTRand mtrand;
};
RandomNumberGenerator UniformDeviate;

// generate a uniform random normalized vector (direction)
Vector3<double> RandomDirection(){
  Vector3<double> direction; 
  double length;
  do {
    direction = Vector3<double>(2.0*UniformDeviate()-1.0,
				2.0*UniformDeviate()-1.0,
				2.0*UniformDeviate()-1.0);	
    length = direction.length();
  } while (length > 1.0 || length < 0.0001);
  direction /= length;
  return direction;
}

#endif // #ifndef RNG_H_INCLUDED_
