#include <optix.h>
#include <optix_math.h>

#include "Hit.hpp"
#include "Photon.hpp"
#include "Light.hpp"
#include "num_neighbors.h"
#include "camera_rays.h"
#define ALPHA 0.5
//#define OMIT_FIRST

struct EyePayload {
  float3 attenuation;
  float3 hitPoint;
  int centroidNumber;
  float3 direct;
  uint   depth;
  int patch;
  float beta;
  float gamma;

};
rtBuffer<float> MomentBuffer;
rtBuffer<PackedHitRecord, 2> eyeHitBuffer;
rtBuffer<float4,          2> outputBuffer;
rtBuffer<float4,          1> patchValueBuffer;
rtBuffer<float3,          1> centroidBuffer;
rtBuffer<float,           1> patchAreaBuffer;
rtBuffer<int,             1> patchSizesBuffer;
rtBuffer<int,             1> patchStartIndexesBuffer;
rtBuffer<float,           1> triAreasBuffer;
rtBuffer<int,             1> neighborsBuffer;
rtBuffer<float,           1> neighborWeightsBuffer;
rtBuffer<float3,          2> directBuffer;
rtBuffer<float4,          1> intermediateBuffer;

rtDeclareVariable(uint,          numTriangles,      ,                         );
rtDeclareVariable(uint2,         launchDim,       rtLaunchDim,                );
rtDeclareVariable(rtObject,      topObject,       ,                           );

rtDeclareVariable(uint2,         launchIndex,       rtLaunchIndex,            );
rtDeclareVariable(int,           centroidNumber,          attribute centroidNumber, );
rtDeclareVariable(int,           numPatches,          ,                             ) ;
rtDeclareVariable(float3,        sceneCenter,     ,                           );
rtDeclareVariable(float,         sceneEpsilon,    ,                           );
rtDeclareVariable(float,         sceneMaxDim,     ,                           );
rtDeclareVariable(uint,          quickRayType,      ,                           );
rtDeclareVariable(int,           res,      ,                           );
rtDeclareVariable(int,           NUM_MOMENTS,      ,                           );
rtDeclareVariable(EyePayload,    eyePayload,      rtPayload,                  );


rtDeclareVariable(float3,     shading_normal,    attribute shading_normal,   );

// Camera
rtDeclareVariable(float3,        eye,             ,                           );
rtDeclareVariable(float3,        U,               ,                           );
rtDeclareVariable(float3,        V,               ,                           );
rtDeclareVariable(float3,        W,               ,                           );
rtDeclareVariable(float3,        cameraDir,               ,                   );
rtDeclareVariable(float3,        nearPoint1,              ,                   );
rtDeclareVariable(float3,        nearPoint2,              ,                   );
rtDeclareVariable(float3,        nearPoint3,              ,                   );
rtDeclareVariable(float3,        nearPoint4,              ,                           );
rtDeclareVariable(uint,          useOrthoCamera,                ,                   );
rtDeclareVariable(float,         toobright,                ,                   );
rtDeclareVariable(float,         toodim,                ,                   );
rtDeclareVariable(int,         greyscale,                ,                   );
rtDeclareVariable(int,          cameraType,                ,                   );




RT_PROGRAM void preRegatherPass() {
  // rtPrintf("regather pass\n");

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
  int cent=prd.centroidNumber;



}

RT_PROGRAM void patchGatherPass()
{
   int patchNum=launchIndex.x;
   float4 tempor=make_float4(0);
    //rtPrintf("num_tris %d \n", num_tris);
   int num_tris=patchSizesBuffer[patchNum];
   int triNum=patchStartIndexesBuffer[patchNum];
   float triArea;
   float totalArea=0.f;
   for (int i=0; i<num_tris; i++,triNum++)
    {
      triArea=triAreasBuffer[triNum];
      tempor+=intermediateBuffer[triNum]*triArea/patchAreaBuffer[patchNum];
    }
    patchValueBuffer[patchNum]=tempor;
}

RT_PROGRAM void regatherPass() {

    float4 outputVal=make_float4(directBuffer[launchIndex],1);

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
      rayFisheye(
            launchIndex, launchDim,
            eye, U, V, W,
            sceneCenter, sceneEpsilon, sceneMaxDim,
            rayOrigin, rayDirection, near, far);
      else
      rayStandard(
            launchIndex, launchDim,
            eye, U, V, W,
            sceneCenter, sceneEpsilon, sceneMaxDim,
            rayOrigin, rayDirection, near, far);
    }
    optix::Ray ray(rayOrigin, rayDirection, quickRayType, sceneEpsilon, far);

    EyePayload prd;
    prd.attenuation = make_float3(1);
    prd.direct      = make_float3(0.005f, 0.005f, 0.01f);
    prd.depth       = 0;

    rtTrace(topObject, ray, prd);
    int cent=prd.centroidNumber;



  if(cent==-1) //This is very bad
  {
          bool dark=false;
        bool bright=false;
        if(outputVal.x>toobright)
          bright=true;
        if(outputVal.x<toodim)
          dark=true;
        if(bright||dark)
        {
          PackedHitRecord hit = eyeHitBuffer[launchIndex];
          float3 hitPosition  = prd.hitPoint+make_float3(500);//make_float3(hit.a);
          //float hit_sum=(hitPosition.x+hitPosition.y+hitPosition.z)*50;
          //int hit_sum_int=(int)hit_sum;
          int count=0;
          if( abs(hitPosition.x*50.f-(int)(hitPosition.x*50))>.5 )
              count++;
          if( abs(hitPosition.y*50.f-(int)(hitPosition.y*50))>.5 )
              count++;
          if( abs(hitPosition.z*50.f-(int)(hitPosition.z*50))>.5 )
              count++;

          if(count%2==0)
          {
            if(bright)
              outputVal=make_float4(outputVal.x*1.6,
                                    outputVal.y*1.4,
                                    outputVal.z*0.4,
                                    outputVal.w);
            else //dark
                          outputVal=make_float4(outputVal.x*0.6666,
                                    outputVal.y*0.6666,
                                    outputVal.z*1.5,
                                    outputVal.w);

          }//end ifcount
          //else
          //  outputBuffer[launchIndex]=make_float4(0,1,0,1);
        }//end if bright||dark
      //outputBuffer[launchIndex] = outputVal; //make_float4(0.0,1.0,0,1);//   intermediateBuffer[eyePayload.centroidNumber];
      outputBuffer[launchIndex] = make_float4(0.0,0.0,0.0,1);//   intermediateBuffer[eyePayload.centroidNumber];
  }
  else
  {

    if (res==0)
    {
      //outputBuffer[launchIndex] =  intermediateBuffer[launchIndex];
    }
    else if(res==1)
    {
        float4 tempor=make_float4(0);
        int num_tris=patchSizesBuffer[prd.patch];
        int triNum=patchStartIndexesBuffer[prd.patch];
        outputBuffer[launchIndex] =  patchValueBuffer[prd.patch];
    }

    else
    {

       if(res==3)//hybrid
       {
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
            //if(!(hitFlags & HitRecord::HIT))
            {
              outputBuffer[launchIndex] = make_float4(.5*directBuffer[launchIndex], 1);
            }
            //else
            //  outputBuffer[launchIndex]=make_float4(0,1,0,1);
       }//end if hybrid
       else//Just tris
       {
          outputBuffer[launchIndex]=make_float4(0,0,0,1);
       }

        float beta=prd.beta;
        float gamma=prd.gamma;
        //outputBuffer[launchIndex] =  intermediateBuffer[prd.centroidNumber];// make_float
        int index = prd.centroidNumber;
        float4 val0=make_float4(0);
        float4 val1=make_float4(0);
        float4 val2=make_float4(0);
        float neighborWeightSum0=0;
        float neighborWeightSum1=0;
        float neighborWeightSum2=0;
        int num_neighbors=NUM_NEIGHBORS;
        //If we are doing non-interpolated tris;
        if(res==5)
        {
          num_neighbors=1;
          beta=gamma=1.f/3.f;
        }

        //Triangle interpolation
        for(int i =0; i< num_neighbors; i++)
        {
            //int index=launchIndex.x*8*3;
            int index0=(3*index+0)*NUM_NEIGHBORS+i;
            int index1=(3*index+1)*NUM_NEIGHBORS+i;
            int index2=(3*index+2)*NUM_NEIGHBORS+i;
            float neighborWeight0=neighborWeightsBuffer[index0];
            float neighborWeight1=neighborWeightsBuffer[index1];
            float neighborWeight2=neighborWeightsBuffer[index2];
            #ifdef OMIT_FIRST
            if(i!=0)
            #endif
            {
                neighborWeightSum0+=neighborWeight0;
                neighborWeightSum1+=neighborWeight1;
                neighborWeightSum2+=neighborWeight2;
                val0 += intermediateBuffer[neighborsBuffer[index0]] * neighborWeight0;
                val1 += intermediateBuffer[neighborsBuffer[index1]] * neighborWeight1;
                val2 += intermediateBuffer[neighborsBuffer[index2]] * neighborWeight2;
            }

        }//endfor
        float4 outputVal=outputBuffer[launchIndex]
                                     + (1.0f - beta - gamma) *  val0/ neighborWeightSum0
                                     +  gamma              *  val2 / neighborWeightSum2
                                     +  beta               * val1 / neighborWeightSum1 ;
        if(greyscale==1)
        {
          //rtPrintf("greyscaling\n");
          float temp=(outputVal.x+outputVal.y+outputVal.z)/3.f;
          float tempa=outputVal.w;
          if(temp>1) temp=1;
          temp=.2+.6*temp;
          outputVal=make_float4(temp,temp,temp, tempa);
        }//end ifgreyscale
        bool dark=false;
        bool bright=false;
        if(outputVal.x>toobright)
          bright=true;
        if(outputVal.x<toodim)
          dark=true;
        if(bright||dark)
        {
          PackedHitRecord hit = eyeHitBuffer[launchIndex];
          float3 hitPosition  = prd.hitPoint+make_float3(500);//make_float3(hit.a);
          //float hit_sum=(hitPosition.x+hitPosition.y+hitPosition.z)*50;
          //int hit_sum_int=(int)hit_sum;
          int count=0;
          if( abs(hitPosition.x*50.f-(int)(hitPosition.x*50))>.5 )
              count++;
          if( abs(hitPosition.y*50.f-(int)(hitPosition.y*50))>.5 )
              count++;
          if( abs(hitPosition.z*50.f-(int)(hitPosition.z*50))>.5 )
              count++;

          if(count%2==0)
          {
            if(bright)
              outputVal=make_float4(outputVal.x*1.5,
                                    outputVal.y*0.6666,
                                    outputVal.z*0.6666,
                                    outputVal.w);
            else //dark
                          outputVal=make_float4(outputVal.x*0.6666,
                                    outputVal.y*0.6666,
                                    outputVal.z*1.5,
                                    outputVal.w);

          }//end ifcount
          //else
        }//end if bright||dark
        outputBuffer[launchIndex]=outputVal;
      }//end not res 0 or res 1
    }//end big else


}

RT_PROGRAM void momentGatherPass() {

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
    int cent=prd.centroidNumber;


  if(cent==-1) //This is very bad
  {
      outputBuffer[launchIndex] =   make_float4(0.0,0.0,0.0,1);//   intermediateBuffer[eyePayload.centroidNumber];
  }
  else
  {
        int index = prd.centroidNumber;
        float max=0;
        float temp;

        for(int i=0; i<NUM_MOMENTS; i++)
        {
          temp=MomentBuffer[i*numTriangles+index];
          if (temp>max)
            max=temp;
        }

        outputBuffer[launchIndex] =  make_float4(max);


    }


}
