//importon pass
#include <optix.h>
#include <optix_math.h>

#include "Hit.hpp"
#include "Light.hpp"
#include "screen_utils.h"
#include "camera_rays.h"

struct EyePayload {
    int    centroidNumber;
  float3 attenuation;
  float3 direct;
  uint   depth;

};

struct ShadowPayload {
  float3 attenuation;
  uint   lightIndex;

};

rtBuffer<DirectionalLight> dirLightBuffer;
rtBuffer<HitRecord, 2>     eyeHitBuffer;
rtBuffer<float3,    1>     centroidBuffer;
rtBuffer<float3,    1>     normalBuffer;
rtBuffer<char,    1>     screenBuffer;

// Scene graph
rtDeclareVariable(rtObject,      topObject,       ,                           );
rtDeclareVariable(rtObject,      topShadower,     ,                           );

// Camera
rtDeclareVariable(float3,        eye,             ,                           );
rtDeclareVariable(float3,        U,               ,                           );
rtDeclareVariable(float3,        V,               ,                           );
rtDeclareVariable(float3,        W,               ,                           );

rtDeclareVariable(EyePayload,    eyePayload,      rtPayload,                  );
rtDeclareVariable(ShadowPayload, shadowPayload,   rtPayload,                  );
rtDeclareVariable(uint2,         launchIndex,     rtLaunchIndex,              );
rtDeclareVariable(uint2,         launchDim,       rtLaunchDim,                );
rtDeclareVariable(float3,        shadingNormal,   attribute shading_normal,   );
rtDeclareVariable(float3,        geometricNormal, attribute geometric_normal, );

rtDeclareVariable(uint,          importonType,    ,                           );
rtDeclareVariable(uint,          shadowRayType,   ,                           );
rtDeclareVariable(float3,        sceneCenter,     ,                           );
rtDeclareVariable(float,         sceneEpsilon,    ,                           );
rtDeclareVariable(float,         sceneMaxDim,     ,                           );
rtDeclareVariable(uint,          backfaceCulling, ,                           );

// Material properties
rtDeclareVariable(float3,        Kd,              ,                           );
rtDeclareVariable(float3,        Ks,              ,                           );
rtDeclareVariable(float3,        Td,              ,                           );
rtDeclareVariable(float3,        Ts,              ,                           );

rtDeclareVariable(float3,        nearPoint1,              ,                   );
rtDeclareVariable(float3,        nearPoint2,              ,                   );
rtDeclareVariable(float3,        nearPoint3,              ,                   );
rtDeclareVariable(float3,        nearPoint4,              ,                           );
rtDeclareVariable(float3,        cameraDir,               ,                   );
rtDeclareVariable(int,        screenWidth,                ,                   );
rtDeclareVariable(int,        screenHeight,               ,                   );
rtDeclareVariable(int,        screen,                     ,                   );

rtDeclareVariable(optix::Ray,    currentRay,      rtCurrentRay,               );
rtDeclareVariable(float,         hitT,            rtIntersectionDistance,     );
rtDeclareVariable(int,        centroidNumber,    attribute centroidNumber,    );
//debugging
rtDeclareVariable(int,          viewpoint,                ,                   );
rtDeclareVariable(float3,     shading_normal,    attribute shading_normal,    );
rtDeclareVariable(uint,          useOrthoCamera,                ,                   );
rtDeclareVariable(int,          cameraType,                ,                   );

//This file shouldn't need any knowledge of screens (no actual Lighting should be done)



//If we're shooting importons per triangle or patch
RT_PROGRAM void importonPassCamera1D() {

  float3 centroid=centroidBuffer[launchIndex.x];
  float3 normal=normalBuffer[launchIndex.x];

  float3 rayDirection;
  float3 rayOrigin;

  //If we're using the normal approach to finding the light on a patch
  if(viewpoint==0)
  {
      rayOrigin =centroid+.1*normalize(normal);
      rayDirection = -1*normalize(normal);
  }

  //if we're instead using rays shot from the eye to the centroid
  else
  {
      rayOrigin =eye;
      rayDirection = centroid-eye;
  }

  //Should we scale these better?
  const float dist =length(normal);
  const float near = max(sceneEpsilon, dist - sceneMaxDim / 2);
  const float far  =dist + sceneMaxDim;// / 2;

  optix::Ray ray(rayOrigin, rayDirection, importonType, sceneEpsilon, far);

  EyePayload prd;
  prd.attenuation    = make_float3(1);
  prd.direct         = make_float3(0.005f, 0.005f, 0.01f);
  prd.depth          = 0;
  prd.centroidNumber = launchIndex.x;

  rtTrace(topObject, ray, prd);
}

// This camera is for the placement of importons (or the points where we will
//   be gathering photons from.
RT_PROGRAM void importonPassCamera() {

    float3 rayOrigin;
    float3 rayDirection;
    float near;
    float far;

    if(useOrthoCamera==1)
        rayOrtho(launchIndex, launchDim,
            sceneCenter, sceneEpsilon, sceneMaxDim,
            cameraDir,
            nearPoint1, nearPoint2, nearPoint3, nearPoint4,
            rayOrigin, rayDirection, near, far);
    else
    {
      if(cameraType==1)
        rayFisheye(launchIndex, launchDim,
            eye, U, V, W,
            sceneCenter, sceneEpsilon, sceneMaxDim,
            rayOrigin, rayDirection, near, far);
      else
        rayStandard(launchIndex, launchDim,
            eye, U, V, W,
            sceneCenter, sceneEpsilon, sceneMaxDim,
            rayOrigin, rayDirection, near, far);
    }
    optix::Ray ray(rayOrigin, rayDirection, importonType, sceneEpsilon, far);

    EyePayload prd;
    prd.attenuation = make_float3(1);
    prd.direct      = make_float3(0.005f, 0.005f, 0.01f);
    prd.depth       = 0;

    rtTrace(topObject, ray, prd);

}


//When importons miss (we just display black for now).
RT_PROGRAM void importonPassMiss() {

  HitRecord hit;
  hit.flags = HitRecord::MISS;

  hit.Kd    = make_float3(0.000f, 0.000f, 0.00f);
  hit.flux  = make_float3(0);
  eyeHitBuffer[launchIndex] = hit;
  float3 normal=normalBuffer[launchIndex.x];

}

// We ignore backfacing polygons
RT_PROGRAM void importonPassAnyHit() {
  if(backfaceCulling) {
    const float3 direction         = currentRay.direction;
    const float3 wsGeometricNormal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometricNormal));

    if(dot(direction, wsGeometricNormal) > 0) {
      rtIgnoreIntersection();
    }
  }
}

//Importon hits (where we gather photons).  Direct light is going to be
//factored into a separate pass
RT_PROGRAM void importonPassClosestHit() {
  float3 direction         = currentRay.direction;
  float3 hitPoint          = currentRay.origin + hitT * direction;
  float3 wsShadingNormal   = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shadingNormal));
  float3 wsGeometricNormal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometricNormal));
  float3 ffnormal          = faceforward(wsShadingNormal, -direction, wsGeometricNormal);

  float3 directIllumination = make_float3(0);

  if(fmaxf(Kd) > 0) {

    //No light sampling needed

    float distanceToEye = length(hitPoint - eye);

    HitRecord hit;
    hit.position      = hitPoint;
    hit.normal        = ffnormal;
    hit.Kd            = Kd * eyePayload.attenuation;
    hit.flags         = HitRecord::HIT;
    hit.radiusSquared = powf(sceneMaxDim/4., 2);

    // TODO: Look for  reason for this number
    hit.minRdsSquared = powf(distanceToEye * tan(0.000136353848), 2);
    hit.photonCount   = 0;
    hit.flux          = make_float3(0);
    hit.direct        = directIllumination;

    eyeHitBuffer[launchIndex] = hit;
    //Direct light needs to be pulled out
    // eyePayload.attenuation = hit.Kd;
    //eyePayload.direct = directIllumination;
  } // end if kd >0
  else
  {
    if(fmaxf(Ts) > 0)
    {

      eyePayload.attenuation=make_float3(0);
      eyePayload.attenuation *= Ts;
      ++eyePayload.depth;

      optix::Ray transmissionRay(hitPoint, direction, importonType, sceneEpsilon, sceneMaxDim);
      rtTrace(topObject, transmissionRay, eyePayload);


    } //end if fmaxf(Ts) > 0
  } //end else if kd >0
}
