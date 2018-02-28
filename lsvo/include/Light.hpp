#ifndef __LIGHT_HPP__
#define __LIGHT_HPP__

struct PointLight {
  float3 pos;
  float3 color;
  float  attenuation;
  optix::uint   castsShadow;
  optix::uint   intensity;
};

struct DirectionalLight {
  float3 direction;
  float3 color;
  float  intensity;
};

#endif
