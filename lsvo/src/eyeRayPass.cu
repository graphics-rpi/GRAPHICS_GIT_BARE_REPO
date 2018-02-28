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
rtBuffer<float3, 2>    directBuffer;

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

rtDeclareVariable(uint,          eyeRayType,      ,                           );
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
rtDeclareVariable(int,          cameraType,                ,                   );
rtDeclareVariable(uint,          useOrthoCamera,                ,                   );
rtDeclareVariable(float3,     shading_normal,    attribute shading_normal,    );



RT_PROGRAM void eyePassCamera() {
    //rtPrintf("in EPC\n");
    float3 rayOrigin;
    float3 rayDirection;
    float near, far;
    if(useOrthoCamera==1)
        rayOrtho(launchIndex, launchDim,
            sceneCenter, sceneEpsilon, sceneMaxDim,
            cameraDir,
            nearPoint1, nearPoint2, nearPoint3, nearPoint4,
            rayOrigin, rayDirection, near, far);
    else
    {
      if(cameraType==1)
      {
        rayFisheye(
        //rayStandard(
              launchIndex, launchDim,
              eye, U, V, W,
              sceneCenter, sceneEpsilon, sceneMaxDim,
              rayOrigin, rayDirection, near, far);
       }
       else
       {
          rayStandard(
              launchIndex, launchDim,
              eye, U, V, W,
              sceneCenter, sceneEpsilon, sceneMaxDim,
              rayOrigin, rayDirection, near, far);
       }
    }
    optix::Ray ray(rayOrigin, rayDirection, eyeRayType, sceneEpsilon, far);

    EyePayload prd;
    prd.attenuation = make_float3(1);
    prd.direct      = make_float3(0.005f, 0.005f, 0.01f);
    prd.depth       = 0;

    rtTrace(topObject, ray, prd);

}

// This is the camera for orthographic views (Such as rendering a wall
//   on the tabletop.  Necessary for both eye pass and importon pass
/*RT_PROGRAM void orthoEyePassCamera() {

    float3 rayOrigin;
    float3 rayDirection;
    float near, far;
    rayOrtho(launchIndex, launchDim,
            eye, U, V, W,
            sceneCenter, sceneEpsilon, sceneMaxDim,
            rayOrigin, rayDirection, near, far);
    optix::Ray ray(rayOrigin, rayDirection, eyeRayType, near, far);

    EyePayload prd;
    prd.attenuation = make_float3(1);
    prd.direct      = make_float3(0.005f, 0.005f, 0.01f);
    prd.depth       = 0;

    rtTrace(topObject, ray, prd);

}*/ 

RT_PROGRAM void eyePassCamera1D() {

  float3 centroid=centroidBuffer[launchIndex.x];
  float3 normal=normalBuffer[launchIndex.x];
  float3 rayDirection;
  float3 rayOrigin;

  rayOrigin =centroid+.1*normalize(normal);
  rayDirection = -1*normalize(normal);

  //Should we scale these better?
  const float dist =length(normal);
  const float near = max(sceneEpsilon, dist - sceneMaxDim / 2);
  const float far  =dist + sceneMaxDim;// / 2;

  optix::Ray ray(rayOrigin, rayDirection, eyeRayType, sceneEpsilon, far);

  EyePayload prd;
  prd.attenuation = make_float3(1);
  prd.direct      = make_float3(0.005f, 0.005f, 0.01f);
  prd.depth       = 0;
  prd.centroidNumber=launchIndex.x;

  rtTrace(topObject, ray, prd);
}

//This has the logic for screens embedded in it.
RT_PROGRAM void shadowAnyHit() {
  const float3 direction = currentRay.direction;
  const float3 hitPoint  = currentRay.origin + hitT * direction;

  if(fmaxf(Ts) > 0) {
      float3 normal=normalBuffer[centroidNumber];
      if (screen)
      {
        int2 coords= screenCoord(hitPoint,normal, screenWidth,screenHeight);
        //rtPrintf("coords %d %d \n", coords.x, coords.y);
        if(screenBuffer[coords.y*screenWidth+coords.x]=='X')

            shadowPayload.attenuation=make_float3(0);
            //rtPrintf("blocked by screen\n");
      }
      //else
      //  rtPrintf("no  screen\n");
      shadowPayload.attenuation *= Ts;
      
      rtIgnoreIntersection();
      

  }
  // TODO: Fix logic
  else if(fmaxf(Kd) > 0) {
    shadowPayload.attenuation = make_float3(0);

    rtTerminateRay();
  }

}




//Importon hits (where we gather photons).  Direct light is going to be
//factored into a separate pass
RT_PROGRAM void eyeRayPassClosestHit() {
 //   rtPrintf("in eyePassClosestHit\n");
  float3 direction         = currentRay.direction;
  float3 hitPoint          = currentRay.origin + hitT * direction;
  float3 wsShadingNormal   = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shadingNormal));
  float3 wsGeometricNormal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometricNormal));
  float3 ffnormal          = faceforward(wsShadingNormal, -direction, wsGeometricNormal);

  float3 directIllumination = make_float3(0);

  if(fmaxf(Kd) > 0) {
    // Compute direct lighting
    int numLights = dirLightBuffer.size();
    while(numLights--)
    {
      const DirectionalLight& light = dirLightBuffer[numLights];


      float  nDL   = dot(ffnormal, -light.direction);


      if(nDL > 0)
      {
        ShadowPayload shadowPRD;
        shadowPRD.attenuation = make_float3(1);
        shadowPRD.lightIndex  = numLights;
        optix::Ray shadowRay(hitPoint, -light.direction, shadowRayType, sceneEpsilon);
        rtTrace(topShadower, shadowRay, shadowPRD);

        directIllumination =Kd * nDL * light.color* shadowPRD.attenuation*light.intensity;
        //if(directIllumination.x>0)
        //rtPrintf("direct Illum %f \n", directIllumination.x);
      } //end if nDl

    } //End while numlights--

    float distanceToEye = length(hitPoint - eye);

    //Just computing direct light... no hit
    float3 direct        = directIllumination;
    directBuffer[launchIndex]=direct;

    // eyePayload.attenuation = hit.Kd;
    eyePayload.direct = directIllumination;
  } // end if kd >0
  else
  {
    if(fmaxf(Ts) > 0)
    {

            

      eyePayload.attenuation=make_float3(0);
      eyePayload.attenuation *= Ts;
      
      //BEGIN HACK
      eyePayload.attenuation=make_float3(1);
      eyePayload.direct =make_float3(1);
      directBuffer[launchIndex]=make_float3(1);
      
      //++eyePayload.depth;

      //optix::Ray transmissionRay(hitPoint, direction, eyeRayType, sceneEpsilon, sceneMaxDim);
      //rtTrace(topObject, transmissionRay, eyePayload);


    } //end if fmaxf(Ts) > 0
  } //end else if kd >0
}


// We ignore backfacing polygons
RT_PROGRAM void eyeRayPassAnyHit() {
  if(backfaceCulling) {
    const float3 direction         = currentRay.direction;
    const float3 wsGeometricNormal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometricNormal));

    if(dot(direction, wsGeometricNormal) > 0) {
      rtIgnoreIntersection();
    }
  }
}

RT_PROGRAM void eyePassMiss()
{
  //rtPrintf("in eye pass miss\n");
  directBuffer[launchIndex]=make_float3(0,1,0);
  //eyePayload.direct=make_float3(30000);
}
