#pragma once
#ifndef __HIT_HPP__
#define __HIT_HPP__
#include <optixu_math.h>	
struct HitRecord {
  static const unsigned int MISS= 1 << 0;
  static const optix::uint HIT= 1 << 1;

  float3 position;
  float  radiusSquared;

  float3 normal;
  float  photonCount;

  float3 Kd;
  float  minRdsSquared;

  float3 flux;
  float  attenuation;

  float3 direct;
  optix::uint   flags;
};

//const optix::uint HitRecord::MISS = 1 << 0;
//const optix::uint HitRecord::HIT  = 1 << 1;

struct PackedHitRecord {
  float4 a; // position, radiusSquared
  float4 b; // normal,   photonCount
  float4 c; // Kd,       minRdsSquared
  float4 d; // flux,     padb
  float4 e; // direct,   flags
};

#endif
