#define _USE_MATH_DEFINES
#include <optix.h>
#include <optix_math.h>
#include <math.h>
#include "Light.hpp"
#include "Photon.hpp"
#include "random.hpp"
#include "random_float3.hpp"


struct ShadowPayload {
  float3 attenuation;
  uint   lightIndex;
};

struct WindowPayload {
  uint2 seed;
  uint  windowIndex;
};

struct AreaLightPayload {
  float3 energy;
  uint2  seed;
};

struct SkyPayload {
  float3 energy;
  float3 attenuation;
  float2 angles;
  uint   sky;
};

struct PhotonPayload {
  float3 energy;
  uint2  seed;
  uint   pmIndex;
  uint   numDeposits;
  uint   rayDepth;
  uint   windowIndex;
};
rtBuffer<char,    1>     screenBuffer;
rtBuffer<uint2>                 randBuffer;
rtBuffer<DirectionalLight>      dirLightBuffer;
rtBuffer<PhotonRecord>          skyPhotonBuffer;
rtBuffer<PhotonRecord>          areaLightPhotonBuffer;
rtBuffer<ClearWindowLaunchInfo> windowBuffer;
rtBuffer<AreaLightLaunchInfo>   areaLightBuffer;
rtBuffer<float3,    1>     normalBuffer;

rtDeclareVariable(rtObject,         topObject,       ,                           );
rtDeclareVariable(rtObject,         topShadower,     ,                           );

rtDeclareVariable(PhotonPayload,    photonPayload,   rtPayload,                  );
rtDeclareVariable(WindowPayload,    windowPayload,   rtPayload,                  );
rtDeclareVariable(AreaLightPayload, lightPayload,    rtPayload,                  );
rtDeclareVariable(SkyPayload,       skyPayload,      rtPayload,                  );
rtDeclareVariable(uint,             launchIndex,     rtLaunchIndex,              );
rtDeclareVariable(float3,           shadingNormal,   attribute shading_normal,   ); 
rtDeclareVariable(float3,           geometricNormal, attribute geometric_normal, );
rtDeclareVariable(float3,           centroid,        attribute centroid,         );

rtDeclareVariable(uint,             maxPhotonCount,  ,                           );
rtDeclareVariable(uint,             maxDepth,        ,                           );
rtDeclareVariable(float2,           sunAngles,       ,                           );

rtDeclareVariable(uint,             shadowRayType,   ,                           );
rtDeclareVariable(uint,             windowRayType,   ,                           );
rtDeclareVariable(uint,             lightRayType,    ,                           );
rtDeclareVariable(uint,             skyRayType,      ,                           );
rtDeclareVariable(uint,             photonRayType,   ,                           );
rtDeclareVariable(float,            sceneEpsilon,    ,                           );
rtDeclareVariable(float,            sunBrightness,    ,                           );
rtDeclareVariable(float3,            sunDirection,    ,                           );
rtDeclareVariable(int,              normalInversion, ,                           );
rtDeclareVariable(int,          bounce, ,                           );
// Material properties
rtDeclareVariable(float3,        Kd,              ,                           );
rtDeclareVariable(float3,        Ks,              ,                           );
rtDeclareVariable(float3,        Td,              ,                           );
rtDeclareVariable(float3,        Ts,              ,                           );

rtDeclareVariable(optix::Ray,    currentRay,      rtCurrentRay,               );
rtDeclareVariable(float,         hitT,            rtIntersectionDistance,     );
rtDeclareVariable(int,        screen,              ,                           );
rtDeclareVariable(int,        screenWidth,              ,                           );
rtDeclareVariable(int,        screenHeight,              ,                           );
rtDeclareVariable(int,        skyType,              ,                           );

rtDeclareVariable(int,        centroidNumber,    attribute centroidNumber,   );

RT_PROGRAM void photonPassGenerator() {
  uint2 seed = randBuffer[launchIndex];
}

RT_PROGRAM void areaLightPassGenerator() {
  //Initially zeroes out all photons
  for(unsigned int i = 0; i < maxPhotonCount; ++i) {
    areaLightPhotonBuffer[launchIndex + i].energy = make_float3(0);
  }

  
  const AreaLightLaunchInfo& info = areaLightBuffer[launchIndex];
  optix::Ray ray(info.start, info.normal, lightRayType, info.eps, 3 * info.eps);

  // WindowPayload payload;
  // payload.seed        = randBuffer[launchIndex];
  // payload.windowIndex = info.windowIndex;
  // payload.launchIndex = launchIndex;
  AreaLightPayload payload;
  payload.seed   = randBuffer[launchIndex];
  payload.energy = info.energy;
  rtTrace(topObject, ray, payload);
}
RT_PROGRAM void areaLightPassClosestHit() {
  const float3 direction = currentRay.direction;
  const float3 hitPoint  = currentRay.origin + hitT * direction;

  // Perp values will be used later, so might as well calc them
  float3 perpX = cross(direction, make_float3(1, 0, 0));
  if(length(perpX) < 1e-6) {
    perpX = cross(direction, make_float3(0, 1, 0));
  }
  perpX=normalize(perpX);
  const float3 perpZ = normalize(cross(direction, perpX));

  float3 dir;

  float  directOrSky = rnd(lightPayload.seed.x);

  const float u1 = rnd(lightPayload.seed.x);
  const float u2 = rnd(lightPayload.seed.y);
  const float r  = sqrt(u1);
  const float theta = 2 * M_PI * u2;

  const float x = r * cos(theta);
  const float z = r * sin(theta);
  //What distribution is this?
  dir = x * perpX + z * perpZ + sqrt(1 - u1) * normalInversion * direction;

  PhotonPayload photonPayload;
  photonPayload.seed        = lightPayload.seed;
  photonPayload.pmIndex     = launchIndex;
  photonPayload.numDeposits = 0;
  photonPayload.rayDepth    = 1;
  photonPayload.energy      = lightPayload.energy;

  optix::Ray photon(hitPoint, -dir, photonRayType, sceneEpsilon);
  rtTrace(topObject, photon, photonPayload);
}

RT_PROGRAM void windowPassGenerator() {
  for(unsigned int i = 0; i < maxPhotonCount; ++i) {
    skyPhotonBuffer[launchIndex + i].energy = make_float3(0);
  }

   const ClearWindowLaunchInfo& info = windowBuffer[launchIndex];

   optix::Ray ray(info.start, info.normal, windowRayType, info.eps, 3 * info.eps);
   WindowPayload payload;
   payload.seed        = randBuffer[launchIndex];
   payload.windowIndex = info.windowIndex;
   // payload.launchIndex = launchIndex; 
   rtTrace(topObject, ray, payload);

}

// This function fires rays from a random point on the window
// This point has been determined already inside of the CPU
// 
//                |
//        •       |  <---  •
//                |
//       end    window   start
// 
// This function is called when it hits the window, which then
// picks a random direction (hopefully) towards the sky to fire.
// It then reverses the direction of the ray, and fires it back
// inside.
RT_PROGRAM void windowPassClosestHit() {
  const float3 direction = currentRay.direction;
  const float3 hitPoint  = currentRay.origin + hitT * direction;
  int thisRayCounts=1;
  
  
        float3 normal=normalBuffer[centroidNumber];
      float cosX=abs(normal.x);
      float cosY=abs(normal.y);
      float cosZ=abs(normal.z);
        float3 xVec,yVec;
      float maxVal=0;
      if (cosX>cosY)
      {
       
        maxVal=cosX;
        xVec=make_float3(0,1,0);
        yVec=make_float3(0,0,1);

      }
      else
      {
       
        maxVal=cosY;
        xVec=make_float3(1,0,0);
        yVec=make_float3(0,0,1);
      }
      if(cosZ>maxVal)
      {
       
        maxVal=cosZ;
        xVec=make_float3(1,0,0);
        yVec=make_float3(0,1,0);

      }
      int xCoord=(int)20*dot(hitPoint,xVec)*screenWidth;
      int yCoord=(int)20*dot(hitPoint,yVec)*screenHeight;
      xCoord%=screenWidth;
      yCoord%=screenHeight;
  
  if (screen&&screenBuffer[yCoord*screenWidth+xCoord]=='X')
       thisRayCounts=0;
  
  // Perp values will be used later, so might as well calc them
  float3 perpX = normalize(cross(direction, make_float3(1, 0, 0)));
  if(length(perpX) < 1e-6) {
    perpX = normalize(cross(direction, make_float3(0, 1, 0)));
  }
  const float3 perpZ = normalize(cross(direction, perpX));
  float3 dir;

  float  directOrSky = rnd(windowPayload.seed.x);
  SkyPayload payload;


  if(directOrSky < 0.5) {
    // SKY
    const float u1 = rnd(windowPayload.seed.x);
    const float u2 = rnd(windowPayload.seed.y);
    const float theta = 2 * M_PI * u2;
    const float r  = sqrt(u1);

    const float x = r * cos(theta);
    const float z = r * sin(theta);
    dir = x * perpX + z * perpZ + sqrt(1 - u1) * normalInversion * -direction;

    float3 ndir = normalize(dir);
    //Dir Zenith is from the straight out angle.  (Opposite of CIE paper)
    float  dirZenith  = acos(ndir.y);
    float  dirAzimuth = atan2(ndir.x, ndir.z);

    payload.sky = 1;
    payload.attenuation = make_float3(1);
    payload.angles = make_float2(dirZenith, dirAzimuth);
    //For indirect sun, no sky
    if(bounce==2||bounce==6)
      thisRayCounts=0;
    //  payload.attenuation=make_float3(0);
  } else {
    // DIRECT SUN
    const DirectionalLight& light = dirLightBuffer[0];
    dir = -light.direction;
    payload.sky = 0;
    payload.attenuation = make_float3(dot(dir, direction));
    //For direct sky and indirect sky, no sun.
    if(bounce>3&&bounce!=6)
        thisRayCounts=0;
//        payload.attenuation = make_float3(0);
  }

  payload.energy = make_float3(0);
  // payload.seed   = windowPayload.seed;
  // payload.depth  = 0;
  optix::Ray ray(hitPoint, dir, skyRayType, sceneEpsilon);
  rtTrace(topObject, ray, payload);
  payload.energy *= payload.attenuation*thisRayCounts;

  if(fmaxf(payload.energy) > 0) {
    PhotonPayload photonPayload;
    photonPayload.seed        = windowPayload.seed;
    photonPayload.pmIndex     = launchIndex;
    photonPayload.numDeposits = 0;
    photonPayload.rayDepth    = payload.sky;
    photonPayload.windowIndex = windowPayload.windowIndex;

    const float avgTd = (Td.x + Td.y + Td.z) / 3;
    const float avgTs = (Ts.x + Ts.y + Ts.z) / 3;
    const float sumT  = avgTd + avgTs;
    const float roll  = rnd(photonPayload.seed.y);

    if(roll < sumT) {
      if(roll < avgTs) {
        photonPayload.energy = payload.energy * Ts / avgTs;
        optix::Ray photon(hitPoint, -dir, photonRayType, sceneEpsilon);
        rtTrace(topObject, photon, photonPayload);
      } else {
        photonPayload.energy = payload.energy * Td / avgTd;

        const float u1 = rnd(photonPayload.seed.x);
        const float u2 = rnd(photonPayload.seed.y);
        const float r  = sqrt(u1);
        const float theta = 2 * M_PI * u2;

        const float x = r * cos(theta);
        const float z = r * sin(theta);
        const float3 newRayDir = x * perpX + z * perpZ + sqrt(1 - u1) * normalInversion * direction;



        optix::Ray photon(hitPoint, newRayDir, photonRayType, sceneEpsilon);
        rtTrace(topObject, photon, photonPayload);
      }
    }
  }
}

// Photon hit logic
RT_PROGRAM void photonClosestHit() {
  //rtPrintf("photon hit\n");
  const float3 direction         = currentRay.direction;
  const float3 hitPoint          = currentRay.origin + hitT * direction;
  const float3 wsShadingNormal   = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shadingNormal));
  const float3 wsGeometricNormal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometricNormal));
  const float3 ffnormal          = faceforward(wsShadingNormal, -direction, wsGeometricNormal);

  if(fmaxf(Kd) > 0) {
    // Diffuse, so store a photon
    if((photonPayload.rayDepth > 0||bounce==6) &&
     (bounce!=5||photonPayload.rayDepth!=1))
    //If it was sky the ray depth is already 1
    //If it's sun we disregard because direct light was already calculated
     {
      PhotonRecord& rec = skyPhotonBuffer[photonPayload.pmIndex + photonPayload.numDeposits];
      rec.position    = hitPoint;
      rec.windowIndex = photonPayload.windowIndex;
      rec.normal      = ffnormal;
      rec.rayDir      = currentRay.direction;
      rec.energy      = photonPayload.energy;
      ++photonPayload.numDeposits;
    }
  }

  if(photonPayload.numDeposits >= maxPhotonCount ||
     photonPayload.rayDepth    >= maxDepth||
     bounce==4) {
    return;
  }

  ++photonPayload.rayDepth;

  // Russian roulette the bounce
  float4 pr;
  const float avgKd = (Kd.x + Kd.y + Kd.z) / 3;
  const float avgTd = (Td.x + Td.y + Td.z) / 3;
  const float avgKs = (Ks.x + Ks.y + Ks.z) / 3;
  const float avgTs = (Ts.x + Ts.y + Ts.z) / 3;
  //rtPrintf("Kd %f Td %f Ks %f Ts %f \n", avgKd, avgTd, avgKs, avgTs);
  pr.x = avgKd;
  pr.y = pr.x + avgTd;
  pr.z = pr.y + avgKs;
  pr.w = pr.z + avgTs;

  float3 rayDir;
  const float roulette = rnd(photonPayload.seed.x);
  //Kd or Td
  if(roulette < pr.y) {
    // Diffuse
    
    //Random number
    //const float u1 = rnd(photonPayload.seed.x);
    //const float u2 = rnd(photonPayload.seed.y);
 
    rayDir=diffuse_sample_point(ffnormal,photonPayload.seed.x,photonPayload.seed.y);
    //rayDir =  specular_sample_point(ffnormal, direction, .02, 
    //          photonPayload.seed.x,photonPayload.seed.y);
    //reflect(direction, ffnormal);
    
    //Kd
    if(roulette < pr.x) {
      // Reflection
      photonPayload.energy *= (Kd / avgKd);
    } 
    //Td
    else 
    {
      // Transmission
      rayDir = -rayDir;
      photonPayload.energy *= (Td / avgTd);
    }
  } 
  // Ks or Ts
  else if(roulette < pr.w) 
  {
    
    // Specular ks
    if(roulette < pr.z) {
      // Reflection
      rayDir = reflect(direction, ffnormal);
      photonPayload.energy *= (Ks / avgKs);
    } 
    // Ts
    else 
    {
      // Transmission
      // Because we're assuming no refraction or complex BTDFs, we do this
      // to not deposit photons in certain cases
      --photonPayload.rayDepth;
      rayDir = direction;
      photonPayload.energy *= (Ts / avgTs);
    }
  } else {
    // Absorb
    return;
  }
  //float3 hacked_start_point = make_float3(0,.1,0);
  optix::Ray bounceRay(hitPoint, rayDir, photonRayType, sceneEpsilon);
  rtTrace(topObject, bounceRay, photonPayload);
}

RT_PROGRAM void skyPassMiss() {
  if(skyPayload.sky) {
    // Sky illumination

    if(skyPayload.angles.x < M_PI / 2) {
      float chi  = acos(cos(sunAngles.x) * cos(skyPayload.angles.x) +
                        sin(sunAngles.x) * sin(skyPayload.angles.x) * cos(abs(skyPayload.angles.y - sunAngles.y)));
      // float phi0 = 1 + a * exp(b);
      float phiZ;
      float fChi;
      float a,b,c,d,e;
      if(skyType==0)
      {
        a = -1.0;
        b = -0.32;
        c = 10;
        d = -3.0;
        e = 0.45;        
        phiZ = 1 + a * exp(b / cos(skyPayload.angles.x));
        fChi = .91 + c * (exp(d * chi) /*- exp(d * M_PI / 2)*/) + e * cos(chi) * cos(chi);
        skyPayload.energy = make_float3(fChi * phiZ);
      }
      else if(skyType==1)
      {
        a = -1.0;
        b = -0.32;
        c = 16;
        d = -3.0;
        e = 0.3;
        phiZ = 1 + a * exp(b / cos(skyPayload.angles.x));
        fChi = .856 + c * (exp(d * chi) /*- exp(d * M_PI / 2)*/) + e * cos(chi) * cos(chi);
        skyPayload.energy = make_float3(fChi * phiZ);
      }
      else if(skyType==2)
      {
        float x=M_PI/2.f-sunAngles.x;
        float y=M_PI/2.f-skyPayload.angles.x;
        a=1.35*(sin(3.59*y-.009)+2.31f)*
          (sin(2.6f*x+.316)+y+4.799f)/2.326;
        b=-.563f*((y+1.059)*(x-.008)+.812);
        skyPayload.energy = make_float3(a*exp(chi*b));
      }
      else if(skyType==3)
      {
        skyPayload.energy = make_float3((1.f+ 2.f * sin(M_PI/2.f-skyPayload.angles.x))/3.f);
      }
      else
      {
        phiZ = 1.;
        fChi = 1.;
      }


    }
  } else {
    // SUN
    const DirectionalLight& light = dirLightBuffer[0];
    skyPayload.energy = light.color * light.intensity;
  }
}

RT_PROGRAM void skyPassClosestHit() {
  // TODO: Figure out if this is necessary
  const float3 direction         = currentRay.direction;
  const float3 hitPoint          = currentRay.origin + hitT * direction;
  const float3 wsShadingNormal   = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shadingNormal));
  const float3 wsGeometricNormal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometricNormal));
  const float3 ffnormal          = faceforward(wsShadingNormal, -direction, wsGeometricNormal);

  if(fmaxf(Kd) > 0) {
    // Compute direct lighting
    int numLights = dirLightBuffer.size();
    while(numLights--) {
      const DirectionalLight& light = dirLightBuffer[numLights];
      float nDL = dot(ffnormal, -sunDirection);

      if(nDL > 0) {
        ShadowPayload shadowPRD;
        shadowPRD.attenuation = make_float3(1);
        shadowPRD.lightIndex  = numLights;
        optix::Ray shadowRay(hitPoint, -sunDirection, shadowRayType, sceneEpsilon);
        rtTrace(topShadower, shadowRay, shadowPRD);

        skyPayload.energy += Kd * nDL * /*light.color*/sunBrightness * 1/*light.intensity*/ * shadowPRD.attenuation;
      }
    }


  }
}
