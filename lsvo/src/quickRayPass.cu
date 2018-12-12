#include <optix.h>
#include <optix_math.h>

#include "Hit.hpp"
#include "Light.hpp"

struct EyePayload {
  float3 attenuation;
  float3 direct;
  uint   depth;
  int    centroidNumber;
};

struct ShadowPayload {
  float3 attenuation;
  uint   lightIndex;
};

// rtBuffer<PointLight>       lightBuffer;
rtBuffer<DirectionalLight> dirLightBuffer;
rtBuffer<HitRecord, 2>     eyeHitBuffer;
rtBuffer<float4,    2>     outputBuffer;

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
rtDeclareVariable(float3,     centroid,          attribute centroid,   );
rtDeclareVariable(int,     centroidNumber,          attribute centroidNumber,   );

rtDeclareVariable(uint,          quickRayType,      ,                           );
rtDeclareVariable(uint,          quickShadowRayType,   ,                           );
rtDeclareVariable(float3,        sceneCenter,     ,                           );
rtDeclareVariable(float,         sceneEpsilon,    ,                           );
rtDeclareVariable(float,         sceneMaxDim,     ,                           );
rtDeclareVariable(uint,          backfaceCulling, ,                           );

// Material properties
rtDeclareVariable(float3,        Kd,              ,                           );
rtDeclareVariable(float3,        Ks,              ,                           );
rtDeclareVariable(float3,        Td,              ,                           );
rtDeclareVariable(float3,        Ts,              ,                           );

rtDeclareVariable(optix::Ray,    currentRay,      rtCurrentRay,               );
rtDeclareVariable(float,         hitT,            rtIntersectionDistance,     );

RT_PROGRAM void shadowAnyHit() {
  const float3 direction = currentRay.direction;
  const float3 hitPoint  = currentRay.origin + hitT * direction;

  // TODO: Fix logic
  if(fmaxf(Kd) > 0) {
    shadowPayload.attenuation = make_float3(0);

    rtTerminateRay();
  } else {
    if(fmaxf(Ts) > 0) {
      shadowPayload.attenuation *= Ts;
      rtIgnoreIntersection();
    }
  }
}

RT_PROGRAM void shadowMiss() {
  // const PointLight& light = dirLightBuffer[shadowPayload.lightIndex];
  // const float attenuatedDistance = max(hitT / light.attenuation, 1.0);
  // shadowPayload.attenuation = make_float3(1 / (attenuatedDistance * attenuatedDistance));
  // shadowPayload.attenuation = make_float3(0);
}

RT_PROGRAM void quickPass() {
  float2 d = make_float2(launchIndex) / make_float2(launchDim) * 2 - 1;
  float3 rayOrigin = eye;
  float3 rayDirection = normalize(d.x * U + d.y * V + W);

  const float dist = length(eye - sceneCenter);
  const float near = max(sceneEpsilon, dist - sceneMaxDim / 2);
  const float far  = dist + sceneMaxDim / 2;

  optix::Ray ray(rayOrigin, rayDirection, quickRayType, sceneEpsilon, far);

  EyePayload prd;
  prd.attenuation = make_float3(1);
  prd.direct      = make_float3(0.005f, 0.005f, 0.01f);
  prd.depth       = 0;

  rtTrace(topObject, ray, prd);
    rtPrintf("centroid number :-P %d", prd.centroidNumber);

   outputBuffer[launchIndex] = make_float4(prd.direct, 1.0);
}

RT_PROGRAM void quickPassMiss() {
  HitRecord hit;
  hit.flags = HitRecord::MISS;
  hit.Kd    = make_float3(0.005f, 0.005f, 0.01f);
  hit.flux  = make_float3(0);
  eyeHitBuffer[launchIndex] = hit;
}

RT_PROGRAM void eyePassAnyHit() {
  if(backfaceCulling) {
    const float3 direction         = currentRay.direction;
    const float3 wsGeometricNormal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometricNormal));

    if(dot(direction, wsGeometricNormal) < 0) {
      rtIgnoreIntersection();
    }
  }
}

RT_PROGRAM void eyePassClosestHit() {
  const float3 direction         = currentRay.direction;
  const float3 hitPoint          = currentRay.origin + hitT * direction;
  const float3 wsShadingNormal   = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shadingNormal));
  const float3 wsGeometricNormal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometricNormal));
  const float3 ffnormal          = faceforward(wsShadingNormal, -direction, wsGeometricNormal);

  float3 directIllumination = make_float3(0);

  if(fmaxf(Kd) > 0) {
    // Compute direct lighting
    int numLights = dirLightBuffer.size();
    while(numLights--) {
      const DirectionalLight& light = dirLightBuffer[numLights];

      // float3 L     = light.pos - hitPoint;
      // float  Ldist = length(L);
      // L /= Ldist;
      float  nDL   = dot(ffnormal, -light.direction);

      if(nDL > 0) {
        ShadowPayload shadowPRD;
        shadowPRD.attenuation = make_float3(1);
        shadowPRD.lightIndex  = numLights;
        optix::Ray shadowRay(hitPoint, -light.direction, quickShadowRayType, sceneEpsilon);
        rtTrace(topShadower, shadowRay, shadowPRD);

        directIllumination += Kd * nDL * light.color * shadowPRD.attenuation;
      }
    }

    float distanceToEye = length(hitPoint - eye);

    HitRecord hit;
    hit.position      = hitPoint;
    hit.normal        = ffnormal;
    hit.Kd            = Kd * eyePayload.attenuation;
    hit.flags         = HitRecord::HIT;
    // hit.radiusSquared = powf(distanceToEye * tan(0.000272707696 * 10), 2);
    // hit.radiusSquared = 1. / (100 * 100);
    // hit.radiusSquared = 1. / (4 * 4);
    // hit.radiusSquared = powf(2, 3);
    hit.radiusSquared = powf(2, 13);
    // TODO: Unmagify this number
    // hit.minRdsSquared = powf(distanceToEye * tan(0.000204530772), 2); // 45 degrees / 1920 / 2 in radians
    hit.minRdsSquared = powf(distanceToEye * tan(0.000136353848), 2); // 30 degrees / 1920 / 2 in radians
    // hit.minRdsSquared = powf(distanceToEye * tan(0.000227256413) * 0.25, 2); // 50 degrees / 1920 / 2 in radians
    // hit.minRdsSquared = powf(distanceToEye * tan(0.000272707696 * 1), 2); // 30 degrees / 1920 in radians
    hit.photonCount   = 0;
    hit.flux          = make_float3(0);
    hit.direct        = directIllumination;

    eyeHitBuffer[launchIndex] = hit;
    // eyePayload.attenuation = hit.Kd;
    eyePayload.direct = directIllumination;
  } else {
    if(fmaxf(Ts) > 0) {
      eyePayload.attenuation *= Ts;
      ++eyePayload.depth;

      optix::Ray transmissionRay(hitPoint, direction, quickRayType, sceneEpsilon, sceneMaxDim);
      rtTrace(topObject, transmissionRay, eyePayload);
//        eyePayload.centroidNumber=centroidNumber;


    }
  }
}
