#pragma once

#include <sutil.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include "Mouse.hpp"

using namespace optix;
//-----------------------------------------------------------------------------
// 
// PinholeCamera -- performs view transformations 
//
//-----------------------------------------------------------------------------

class PinholeCamera {
  typedef optix::float3 float3;
  typedef optix::float2 float2;
public:
  enum AspectRatioMode {
    KeepVertical,
    KeepHorizontal,
    KeepNone
  };

  SUTILAPI PinholeCamera(float3 eye, float3 lookat, float3 up, float hfov=60, float vfov=60,
                         AspectRatioMode arm = KeepVertical);

  SUTILAPI void setup();
  
  SUTILAPI void getEyeUVW(float3& eye, float3& U, float3& V, float3& W);

  SUTILAPI void getEyeLookUpFOV(float3& eye, float3& lookat, float3& up, float& HFOV, float& VFOV);

  SUTILAPI void scaleFOV(float);
  SUTILAPI void translate(float2);
  SUTILAPI void dolly(float);
  SUTILAPI void transform( const optix::Matrix4x4& trans );
  SUTILAPI void setAspectRatio(float ratio);
  
  SUTILAPI void setParameters(float3 eye_in, float3 lookat_in, float3 up_in, float hfov_in, float vfov_in, PinholeCamera::AspectRatioMode aspectRatioMode_in);

  enum TransformCenter {
    LookAt,
    Eye,
    Origin
  };

  float3 eye, lookat, up;
  float hfov, vfov;
private:
  float3 lookdir, camera_u, camera_v;
  AspectRatioMode aspectRatioMode;
};


