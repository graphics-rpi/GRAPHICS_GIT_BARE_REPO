#pragma once
#ifndef __PHOTON_HPP__
#define __PHOTON_HPP__

#include <optixu_math_namespace.h>	

 // using optix::uint;
enum AXIS{X=1,Y=2,Z=3};

struct ClearWindowInfo {
  optix::float3 a;
  optix::float3 b;
  optix::float3 c;
};

struct ClearWindowLaunchInfo {
  optix::float3 start;
  float  area;
  optix::float3 normal;
  float  eps;
  optix::uint   windowIndex;
  int axis;
};

struct AreaLightLaunchInfo {
  optix::float3 start;
  float  area;
  optix::float3 normal;
  float  eps;
  optix::float3 energy;
  optix::uint   windowIndex;
};

struct PhotonRecord {
  static const optix::uint X= 1 << 0;
  static const optix::uint Y= 1 << 1;
  static const optix::uint Z= 1 << 2;
  static const optix::uint LEAF= 1 << 3;
  static const optix::uint NILL= 1 << 4;

  optix::float3 position;
  optix::uint   windowIndex;
  optix::float3 normal;
  float  pada;
  optix::float3 rayDir;
  float  padb;
  optix::float3 energy;
  optix::uint   axis;
};

//const uint PhotonRecord::X    = 1 << 0;
//const uint PhotonRecord::Y    = 1 << 1;
//const uint PhotonRecord::Z    = 1 << 2;
//const uint PhotonRecord::LEAF = 1 << 3;
//const uint PhotonRecord::NILL = 1 << 4;

struct PackedPhotonRecord {
  optix::float4 a; // position, windowIndex
  optix::float4 b; // normal
  optix::float4 c; // rayDir
  optix::float4 d; // energy, axis
};

#endif
