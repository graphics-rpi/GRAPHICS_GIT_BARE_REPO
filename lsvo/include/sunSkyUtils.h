#include "contants.h"
#include <optixu/optixu_math_namespace.h>

__host__ __device__ __inline__ double calcSolarBrightness(
  unsigned int skyType, float3 sunDirection)
{

  if (skyType!=OVERCAST && skyType!=SViewConfig::eCIE_Uniform && altitude > 0.0)
  {
    solarBrightness = 1.5e9/SUNEFFICACY * 
      (1.147 - .147/(sunDirection.y>.16?sunDirection.y:.16));
    if (skyType == SViewConfig::eCIE_Intermediate)
  	  solarBrightness *= 0.15;	/* fudge factor! */
  }
  else
    solarBrightness = 0;

  return solarBrightness;
}
