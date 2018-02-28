#include "camera_utils.hpp"
#include "PinholeCamera.hpp"


//-----------------------------------------------------------------------------
// 
// PinholeCamera definition 
//
//-----------------------------------------------------------------------------

PinholeCamera::PinholeCamera(float3 eye, float3 lookat, float3 up, float hfov, float vfov, AspectRatioMode arm)
  : eye(eye)
  , lookat(lookat)
  , up(up)
  , hfov(hfov)
  , vfov(vfov)
  , aspectRatioMode(arm)
{
  setup();
}

void PinholeCamera::setAspectRatio(float ratio)
{
  float realRatio = ratio;

  const float* inputAngle = 0;
  float* outputAngle = 0;
  switch(aspectRatioMode) {
  case PinholeCamera::KeepHorizontal:
    inputAngle = &hfov;
    outputAngle = &vfov;
    realRatio = 1.f/ratio;
    break;
  case PinholeCamera::KeepVertical:
    inputAngle = &vfov;
    outputAngle = &hfov;
    break;
  case PinholeCamera::KeepNone:
    return;
    break;
  }

  *outputAngle = RtoD(2.0f*atanf(realRatio*tanf(DtoR(0.5f*(*inputAngle)))));

  setup();
}

void PinholeCamera::setParameters(float3 eye_in, float3 lookat_in, float3 up_in, float hfov_in, float vfov_in, PinholeCamera::AspectRatioMode aspectRatioMode_in)
{
  eye = eye_in;
  lookat = lookat_in;
  up = up_in;
  hfov = hfov_in;
  vfov = vfov_in;
  aspectRatioMode = aspectRatioMode_in;
  
  setup();
}

void PinholeCamera::setup()
{
  lookdir = assignWithCheck( lookdir, lookat-eye );  // do not normalize lookdir -- implies focal length
  float lookdir_len = length( lookdir );
  up = assignWithCheck( up, normalize(up));
  camera_u = assignWithCheck( camera_u, normalize( cross(lookdir, up) ) );
  camera_v = assignWithCheck( camera_v, normalize( cross(camera_u, lookdir) ) );
  float ulen = lookdir_len * tanf(DtoR(hfov*0.5f));
  camera_u = assignWithCheck( camera_u, camera_u * ulen );
  float vlen = lookdir_len * tanf(DtoR(vfov*0.5f));
  camera_v = assignWithCheck( camera_v, camera_v * vlen );
}

void PinholeCamera::getEyeUVW(float3& eye_out, float3& U, float3& V, float3& W)
{
  eye_out = eye;
  U = camera_u;
  V = camera_v;
  W = lookdir;
}

void PinholeCamera::getEyeLookUpFOV(float3& eye_out, float3& lookat_out, float3& up_out, float& HFOV_out, float& VFOV_out)
{
  eye_out = eye;
  lookat_out = lookat;
  up_out = up;
  HFOV_out = hfov;
  VFOV_out = vfov;
}

void PinholeCamera::scaleFOV(float scale)
{
  const float fov_min = 0.0f;
  const float fov_max = 120.0f;
  float hfov_new = RtoD(2*atanf(scale*tanf(DtoR(hfov*0.5f))));
  hfov_new = Clamp(hfov_new, fov_min, fov_max);
  float vfov_new = RtoD(2*atanf(scale*tanf(DtoR(vfov*0.5f))));
  vfov_new = Clamp(vfov_new, fov_min, fov_max);

  hfov = assignWithCheck( hfov, hfov_new );
  vfov = assignWithCheck( vfov, vfov_new );

  setup();
}

void PinholeCamera::translate(float2 t)
{
  float3 trans = camera_u*t.x + camera_v*t.y;

  eye = assignWithCheck( eye, eye + trans );
  lookat = assignWithCheck( lookat, lookat + trans );

  setup();
}


// Here scale will move the eye point closer or farther away from the
// lookat point.  If you want an invertable value feed it
// (previous_scale/(previous_scale-1)
void PinholeCamera::dolly(float scale)
{
  // Better make sure the scale isn't exactly one.
  if (scale == 1.0f) return;
  float3 d = (lookat - eye) * scale;
  eye  = assignWithCheck( eye, eye + d );

  setup();
}

void PinholeCamera::transform( const Matrix4x4& trans )
{
  float3 cen = lookat;         // TODO: Add logic for various rotation types (eg, flythrough)

  Matrix4x4 frame = initWithBasis( normalize(camera_u),
                                         normalize(camera_v),
                                         normalize(-lookdir),
                                         cen );
  Matrix4x4 frame_inv = inverse( frame );

  Matrix4x4 final_trans = frame * trans * frame_inv;
  float4 up4     = make_float4( up );
  float4 eye4    = make_float4( eye );
  eye4.w         = 1.0f;
  float4 lookat4 = make_float4( lookat );
  lookat4.w      = 1.0f;


  up     = assignWithCheck( up, make_float3( final_trans*up4 ) );
  eye    = assignWithCheck( eye, make_float3( final_trans*eye4 ) );
  lookat = assignWithCheck( lookat, make_float3( final_trans*lookat4 ) );

  setup();
}
