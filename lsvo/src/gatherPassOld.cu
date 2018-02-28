#include <optix.h>
#include <optix_math.h>

#include "Hit.hpp"
#include "Photon.hpp"

rtBuffer<uint>               windowHitBuffer;
rtBuffer<PackedPhotonRecord> photonMapBuffer;
rtBuffer<PackedHitRecord, 2> eyeHitBuffer;
rtBuffer<float4,          2> outputBuffer;

rtDeclareVariable(uint,          totalPhotonsFired, ,                           );
rtDeclareVariable(uint2,         launchIndex,       rtLaunchIndex,              );

__device__ __inline__ 
void accumulatePhoton(
  const PackedPhotonRecord& photon,
  const float3& hitNormal,
  const float3& hitKd,
  uint& numNewPhotons, float3& totalFlux
) {
  float3 photonEnergy = make_float3(photon.d);
  float3 photonNormal = make_float3(photon.b);
  float  cos = dot(photonNormal, hitNormal);
  if(cos > 0.01f) { // Fudge factor for imperfect cornell box geom
    windowHitBuffer[__float_as_int(photon.a.w)] = 1;
    float3 photonDir = make_float3(photon.c);
    float3 flux = photonEnergy * hitKd * -dot(photonDir, hitNormal);
    ++numNewPhotons;
    totalFlux += flux;
  }
}

RT_PROGRAM void gatherPass() {
  PackedHitRecord hit = eyeHitBuffer[launchIndex];
  const float3 hitPosition  = make_float3(hit.a);
  const float  hitRadiusSq  = hit.a.w;
  const float3 hitNormal    = make_float3(hit.b);
  const float  hitPhotonCnt = hit.b.w;
  const float3 hitKd        = make_float3(hit.c);
  const float3 hitFlux      = make_float3(hit.d);
  const float3 hitDirect    = make_float3(hit.e);
  const uint   hitFlags     = __float_as_int(hit.e.w);

  // TODO: Confirm this does what we think we want it to do
  if(!(hitFlags & HitRecord::HIT)) {
    outputBuffer[launchIndex] = make_float4(hitKd, 1);
    return;
  }

#define MAX_DEPTH 20
  unsigned int stack[MAX_DEPTH];
  unsigned int stackSize = 0;
  unsigned int node      = 0;

#define pushNode(N) stack[stackSize++] = (N)
#define popNode()   stack[--stackSize]

  float3 flux = make_float3(0);

  pushNode(0);
  unsigned int numNewPhotons = 0;
  unsigned int iterations    = 0;

  do {
    const PackedPhotonRecord& photon = photonMapBuffer[node];

    unsigned int axis = __float_as_int(photon.d.w);
    if(!(axis & PhotonRecord::NILL)) {
      float3 photonPosition = make_float3(photon.a);
      float3 diff = hitPosition - photonPosition;
      float  distanceSq = dot(diff, diff);

      if(distanceSq <= hitRadiusSq) {
        accumulatePhoton(photon, hitNormal, hitKd, numNewPhotons, flux);
      }

      if(!(axis & PhotonRecord::LEAF)) {
        float d;
        if     (axis & PhotonRecord::X) d = diff.x;
        else if(axis & PhotonRecord::Y) d = diff.y;
        else                            d = diff.z;

        int child = d < 0 ? 0 : 1;
        if(d * d < hitRadiusSq) {
          pushNode((node << 1) + 2 - child);
        }
        node = (node << 1) + 1 + child;
      } else {
        node = popNode();
      }
    } else {
      node = popNode();
    }
    ++iterations;
  } while(node);

#define ALPHA 0.6

  const float n = hitPhotonCnt;
  const float m = static_cast<float>(numNewPhotons);

  const float nHat      = n + ALPHA * m;
  const float reduction = m > 0 ? nHat / (n + m) : 1;
  const float rHatSq    = hitRadiusSq * reduction;

  const float3 tauNHat  = (hitFlux + flux) * reduction;

  hit.a.w = rHatSq;
  hit.b.w = nHat;

  hit.d   = make_float4(tauNHat);
  eyeHitBuffer[launchIndex] = hit;

  float3 indirectFlux = 1.0 / (M_PI * hitRadiusSq) * (tauNHat / totalPhotonsFired);
  outputBuffer[launchIndex] = make_float4(hitDirect + indirectFlux * 1000000);
}
