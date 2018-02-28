
/*
 * Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "Hit.hpp"
#include "Light.hpp"

using namespace optix;
rtDeclareVariable(float3,     centroid,          attribute centroid,   );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3,        Kd,              ,                           );
rtDeclareVariable(optix::Ray,    currentRay,      rtCurrentRay,               );
rtDeclareVariable(int,     centroidNumber,          attribute centroidNumber,   );
rtDeclareVariable(int,     patch,          attribute patch,   );
rtDeclareVariable(float,     gammaAttr,          attribute gammaAttr,   );
rtDeclareVariable(float,     betaAttr,          attribute betaAttr,   );
rtDeclareVariable(rtObject,      topObject,       ,                           );
rtDeclareVariable(float3,        Ts,              ,                           );
rtDeclareVariable(float,         hitT,            rtIntersectionDistance,     );
rtDeclareVariable(uint,          quickRayType,      ,                           );
rtDeclareVariable(float,         sceneEpsilon,    ,                           );
rtDeclareVariable(float,         sceneMaxDim,     ,                           );
rtDeclareVariable(float3,     rayDirection,          attribute rayDirection,   );


struct PerRayData_radiance
{
  float3 attenuation;
  float3 hitPoint;
  int    centroidNumber;

  float3 direct;
  uint   depth;
  int patch;
  float beta;
  float gamma;
  

};

struct PerRayData_shadow
{
  float3 attenuation;
};

rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow,   prd_shadow,   rtPayload, );


RT_PROGRAM void any_hit_shadow()
{
  //if(fmaxf(Ts) > 0)
  //  rtIgnoreIntersection();

  // this material is opaque, so it fully attenuates all shadow rays
  prd_shadow.attenuation = make_float3(.0);

  rtTerminateRay();
}



RT_PROGRAM void any_hit()
{
  if(dot(geometric_normal,rayDirection)>0)
    rtIgnoreIntersection();
}
RT_PROGRAM void closest_hit()
{

  const float3 direction         = currentRay.direction;
  const float3 hitPoint          = currentRay.origin + hitT * direction;

    if(fmaxf(Ts) > 0) {
    prd_radiance.centroidNumber=-1;
    /*  //currentRay.attenuation *= Ts;
      //++currentRay.depth;
      
      optix::Ray transmissionRay(hitPoint, direction, quickRayType, sceneEpsilon, sceneMaxDim);
      rtTrace(topObject, transmissionRay, prd_radiance);*/
  }
  else
  {
    prd_radiance.centroidNumber=centroidNumber;
  }
    prd_radiance.attenuation =normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
    prd_radiance.patch=patch;
    prd_radiance.beta=betaAttr;
    prd_radiance.gamma=gammaAttr;
    prd_radiance.hitPoint=hitPoint;
//  }
  //prd_radiance.attenuation =make_float3(dot(normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal)),currentRay.direction));
  
  //normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));//make_float3(0,0,1.);//centroid;//Kd*make_float3(dot(normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal)),currentRay.direction)*0.5f + 0.5f);
//prd_radiance.result = make_float3(1,1,1);
  //rtPrintf("Centroid nubmer %d\n", prd_radiance.centroidNumber);
}

RT_PROGRAM void miss() {
  prd_radiance.attenuation= make_float3(0.005f, 0.005f, 1.f);
  prd_radiance.centroidNumber=-1;

}
