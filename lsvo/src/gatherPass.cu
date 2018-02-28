#define _USE_MATH_DEFINES
#include <optix.h>
#include <optix_math.h>
#include <math.h>

#include "Hit.hpp"
#include "Photon.hpp"
#define DEBUG
#define ALPHA 0.5
#define MAGIC_PHOTON_FACTOR .017
#define MAGIC_NONPHOTON_FACTOR 1.4
#define OVEREXPOSURE_THRESHOLD .03

rtBuffer<PackedPhotonRecord> photonMapBuffer;
rtBuffer<PackedHitRecord, 2> eyeHitBuffer;
rtBuffer<float4,          2> outputBuffer;
rtBuffer<float3,          2> directBuffer;
rtBuffer<float4,          1> intermediateBuffer;
rtDeclareVariable(uint2,         launchDim,       rtLaunchDim,                );
rtDeclareVariable(uint,          totalPhotonsFired, ,                           );
rtDeclareVariable(uint,          photonsPerPass,    ,                           );
rtDeclareVariable(uint2,         launchIndex,       rtLaunchIndex,              );
rtDeclareVariable(float,         windowCorrection,  ,                           );
rtDeclareVariable(int,          bounce, ,                           );
rtDeclareVariable(int,          exPer, ,                           );
rtDeclareVariable(float,          factor, ,                           );
rtDeclareVariable(int,          res,      ,                           );

RT_PROGRAM void gatherPass() {
  //if(launchIndex.x==0&&launchIndex.y==0)
  //rtPrintf("gather pass\n");
  PackedHitRecord hit = eyeHitBuffer[launchIndex];
  const float3 hitPosition  = make_float3(hit.a);
  float        hitRadiusSq  = hit.a.w;

  const float3 hitNormal    = make_float3(hit.b);
  float        hitPhotonCnt = hit.b.w;
  const float3 hitKd        = make_float3(hit.c);
  const float  hitMinRds    = hit.c.w;
  float3       hitFlux      = make_float3(hit.d);
  const float3 hitDirect    = make_float3(hit.e);
  const uint   hitFlags     = __float_as_int(hit.e.w);
    
  //BUG: Find out what this does!!!!
  // TODO: Confirm this does what we think we want it to do
  // If there isn't a hit?
  if(!(hitFlags & HitRecord::HIT)) {
    if (launchDim.y>1)
    {
      outputBuffer[launchIndex] = make_float4(hitKd, 1);
    }
    //else if(res==3)
    //{
    //  intermediateBuffer[launchIndex.x] = make_float4(0,0,0, 1);
    //} 
    else
    {
      intermediateBuffer[launchIndex.x] = make_float4(hitKd, 1);
    }
    return;
  }

#define MAX_DEPTH 100
  unsigned int stack[MAX_DEPTH];
  unsigned int stackSize = 0;
  unsigned int node      = 0;

#define pushNode(N) stack[stackSize++] = (N)
#define popNode()   stack[--stackSize]

  float3 flux = make_float3(0);
  // float  compensationFactor = 1;

  // uint m = 0;

  pushNode(0);
  // unsigned int numNewPhotons = 0;
  unsigned int iterations    = 0;
  
  // Do while navigating kd-tree
  do {
    const PackedPhotonRecord& photon = photonMapBuffer[node];
  
    //Use the axis of the photon
    unsigned int axis = __float_as_int(photon.d.w);
    
    //If the photon record of specified axis is NILL
    if(!(axis & PhotonRecord::NILL)) {
      float3 photonPosition = make_float3(photon.a);
      float3 diff = hitPosition - photonPosition;
      float  distanceSq = dot(diff, diff);

      //If the photon is in the specified distance
      if(distanceSq <= hitRadiusSq) {
        float3 photonEnergy = make_float3(photon.d);
        float3 photonNormal = make_float3(photon.b);
        float  cosTerm      = dot(photonNormal, hitNormal);
        //rtPrintf("cosTerm %f \n", cosTerm);
        //if(cosTerm > 0.001f) { // Fudge factor for imperfect cornell box geom
        //Corrected from eric's.  We should want almost identical vectors (Dot product of 1).
        if(cosTerm > 0.99f) { // Fudge factor for imperfect cornell box geom
          // ++m;
          // flux += hitKd * photonEnergy * cosTerm;

          // float g  = (hitPhotonCnt * ALPHA + ALPHA) / (hitPhotonCnt * ALPHA + 1);
          // compensationFactor *= g;
          // hitRadiusSq *= g;
          // ++hitPhotonCnt;
          // flux      = (flux + hitKd * photonEnergy * cosTerm) * g;
          // hitFlux      = hitFlux + (hitKd * photonEnergy * cosTerm) * g;

          float a    = ALPHA;
          float g    = (hitPhotonCnt + a) / (hitPhotonCnt + 1);
          //g = hitPhotonCt +.5 / hitPhotonCnt + 1 
          float rHat = hitRadiusSq * g;
          if(g>1)
          {
            rtPrintf("shouldnt be here");
            while(1);
          }
          if(rHat < hitMinRds) {
            a = (hitMinRds / hitRadiusSq) * (hitPhotonCnt + 1) - hitPhotonCnt;
            g = (hitPhotonCnt + a) / (hitPhotonCnt + 1);
            rHat = hitMinRds;
            rtPrintf("shouldnt be here");
            while(1);

          }
          hitPhotonCnt += a;
          hitRadiusSq   = rHat;
          //if(hitRadiusSq>100)
            //rtPrintf("hrs %f \n", hitRadiusSq);

          hitFlux       = (hitFlux + hitKd * photonEnergy * cosTerm) * g;
        }// end if cosTerm
      }//end if distanceSq

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

  hit.a.w = hitRadiusSq;
  hit.b.w = hitPhotonCnt;
  hit.d   = make_float4(hitFlux);

  eyeHitBuffer[launchIndex] = hit;

  float3 indirectFlux = 1.0 / (M_PI * hitRadiusSq) * (hitFlux / totalPhotonsFired / photonsPerPass);

  //By default this one (below)
  float3 total ;
  if(factor>0)
  {
   float indirectFactor=1.f;
   float directFactor=1.f;
   
   if(res==3)//Hybrid rendering
      directFactor=0.f;
   
   if(bounce==1)
      indirectFactor=0.f;
   else if(bounce>1)
      directFactor=0.f;
   //total = .5*hitDirect*directFactor
   total = .5*directBuffer[launchIndex]*directFactor*MAGIC_NONPHOTON_FACTOR  
         + .5*indirectFlux * 100 * windowCorrection*factor*indirectFactor*MAGIC_PHOTON_FACTOR;// *distrFactor; 
 /*  if(bounce==0)
      total = .5*hitDirect + .5*indirectFlux * 100 * windowCorrection*factor;// *distrFactor;
   else if(bounce==1)
      total = .5*hitDirect;
   else
      total = .5*indirectFlux * 100 * windowCorrection*factor;// *distrFactor;*/
  }
  else total=make_float3(0);
  // float3 total =  indirectFlux * 100 * windowCorrection;
  // float3 total = hitDirect + indirectFlux * 15 * windowCorrection;
  // float3 total = indirectFlux * 800000;
  // float3 total = hitDirect + indirectFlux * 15000;
  // float a = 0.2125 * total.x + 0.7154 * total.y + 0.0721 * total.z + 1;
  total = .01*make_float3(
    powf(total.x, 1 / 2.4),
    powf(total.y, 1 / 2.4),
    powf(total.z, 1 / 2.4)
  );
  // outputBuffer[launchIndex] = make_float4(total / a);
  if (launchDim.y>1) //If full res (not tri or patches)
  {
   //if(total.x>OVEREXPOSURE_THRESHOLD)
   //  outputBuffer[launchIndex] =  make_float4(1,0,0,1);
   //else
     outputBuffer[launchIndex] = make_float4(exPer*total);
  }
  //else //If not at full res (e.g triangles, patches)
  //{   if(total.x>OVEREXPOSURE_THRESHOLD)
  //   intermediateBuffer[launchIndex.x] =  make_float4(1,0,0,1);
  // else
     intermediateBuffer[launchIndex.x] = make_float4(exPer*total);
  //}
    // intermediateBuffer[launchIndex.x] = make_float4(exPer*total);
  //if(launchIndex.x%100==0)
   // rtPrintf("output %f\n", outputBuffer[launchIndex].x);
}
