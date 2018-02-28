#include "random.hpp"

__device__ __inline__ float3 random_point_on_unit_sphere(float u1, float u2 )
{

    //http://mathworld.wolfram.com/SpherePointPicking.html
    
    float theta, phi;
    
    theta = u1*2.f*M_PI;
    
    phi   = u2*M_PI;
    
    float u=cos(phi);
    
    float sqrt1MinusUSquared=sqrt(1.f-(u*u));
    
    float x=sqrt1MinusUSquared*cos(theta);
    
    float y=sqrt1MinusUSquared*sin(theta);
    
    float z=u;
    
    return normalize(make_float3(x,y,z));
  
}

__device__ __inline__ float3 random_point_on_unit_sphere_gauss(unsigned int& prev1, unsigned int& prev2)
{
  float rand1=rnd(prev1);
  float rand2=rnd(prev1);
  float rand3=rnd(prev2);
  float rand4=rnd(prev2);  
  float gauss_rnd1=sqrt(-2.f*log(rand1))*cos(2*M_PI*rand2);
  float gauss_rnd2=sqrt(-2.f*log(rand1))*sin(2*M_PI*rand2);
  float gauss_rnd3=sqrt(-2.f*log(rand3))*cos(2*M_PI*rand4);
  
  float3 normalized_vec=normalize(make_float3(gauss_rnd1,gauss_rnd2,gauss_rnd3));
  //rtPrintf("NV %f %f %f \n", normalized_vec.x,  normalized_vec.y,  normalized_vec.z);
  return normalized_vec;

}

__device__ __inline__ float3 diffuse_sample_point(float3 normal,unsigned int& prev1, unsigned int& prev2 )
{

    float3 perpX = cross(normal, make_float3(1, 0, 0));
    //try adjusting threshold
    if(length(perpX) < 1e-2) {
      perpX = cross(normal, make_float3(0, 1, 0));
    }
    perpX=normalize(perpX);
    
    //The component of the surface orthogonal to Z
    const float3 perpZ = normalize(cross(normal, perpX));
    float3 randPoint=random_point_on_unit_sphere_gauss(prev1,prev2);
    //The right?? way
    //float3 retVal=normalize(normal
    //                +randPoint.x*perpX
    //                +randPoint.y*perpZ
    //                +randPoint.z*normal);
    //return retVal;
    //rtPrintf("rv %f %f %f \n",  retVal.x,   retVal.y,   retVal.z);
    
    //Mirror negative z to positive z to get only a single hemisphere
    if (randPoint.z<0)
      randPoint.z=-randPoint.z;
    
    return normalize(
                    +randPoint.x*perpX
                    +randPoint.y*perpZ+
                    +randPoint.z*normal);
    //return randPoint;
    
    
}

//Specular is made by adding some percentage of a random point to the reflected vector
//
__device__ __inline__ float3 specular_sample_point(float3 normal, float3 incoming, float factor, unsigned int& prev1, unsigned int& prev2 )
{
  float3 reflected=normalize(reflect(incoming, normal));
  float3 rand_direction=random_point_on_unit_sphere_gauss(prev1,prev2);
  float3 new_dir=normalize((rand_direction*factor+reflected*(1.f-factor))/2.f);
  if(dot(new_dir, normal)<0)
    return reflected;
  else
    return new_dir;
  
}


