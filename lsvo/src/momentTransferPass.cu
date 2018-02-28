#include <optix.h>
#include <optixu/optixu_math_namespace.h>



rtDeclareVariable(uint,        curMoment,             ,                           );
rtBuffer<float4,          2> outputBuffer;
rtBuffer<float4,          1> intermediateBuffer;
rtBuffer<float> MomentBuffer;

rtDeclareVariable(uint,          numFramesPerMoment,      ,                           );
rtDeclareVariable(uint,          numTriangles,      ,                           );
rtDeclareVariable(uint2,         launchIndex,       rtLaunchIndex,              );

RT_PROGRAM void momentTransferPass()
{
  //uint2 index=make_uint2(launchIndex.x+curMoment*numFramesPerMoment,0);
  //float4 temp = intermediateBuffer[index];
  //float4 temp = intermediateBuffer[launchIndex.x+curMoment*numFramesPerMoment];
  float4 temp = intermediateBuffer[launchIndex.x];
  float tempfloat=0.2989*temp.x + 0.5870*temp.y+0.1140*temp.z;
  //rtPrintf("numTriangles %d curMomeent %d \n", numTriangles, curMoment);
  MomentBuffer[curMoment*numTriangles+launchIndex.x]= tempfloat;
}
