#ifndef _RANDOM_H_
#define _RANDOM_H_

#include "mtrand.h"

// ==========================================================
// ==========================================================

// This class can be used instead of drand48 when you want 
// multiple identical streams of random floating point numbers.

// To do this, create multiple instances and seed them with 
// the same number.


// ==========================================================
// A linear congruential generator
//
// It is fast, simple, and (if instantiated with the right
// constants) gives reasonable pseudorandom numbers. The nth random
// number is a function of the (n-1)st random number:
//
// R_n = (a*R_n-1 + c) mod m
// a = 1366
// c = 150889
// m = 714025
//

#include "vectors.h"


class Random {

public:

 Random(int seed = 0) : mtrand(seed) { }
  ~Random() {}

  // returns a random floating point between 0 & 1
  double next() {
    return mtrand.rand();
    /*last = (1366*last + 150889) % 714025;
    double answer = last / double (714025);
    return answer; 
    */
  }
    
  // return a random vector with each component from -1 -> 1
  Vec3f randomVector() {
    double x = next()*2 - 1;
    double y = next()*2 - 1;
    double z = next()*2 - 1;
    return Vec3f(x,y,z);
  }

  // return a random vector with each component from -1 -> 1
  Vec3f randomColor() {
    double x,y,z;
    while (1) {
      x = next(); if (x<0) x=0;
      y = next(); if (y<0) y=0;
      z = next(); if (z<0) z=0;
      if (x > 0.1 || y > 0.1 || z > 0.1) break;
    }
    Vec3f v(x,y,z);
    v.Normalize();
    return v; 
  }

private:
  MTRand mtrand;
  //  int last;
};

// ==========================================================

#endif
