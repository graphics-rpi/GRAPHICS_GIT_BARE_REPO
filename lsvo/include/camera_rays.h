#include <optixu/optixu_math_namespace.h>

#define ORTHO_MIN .0005f
#define ORTHO_FAR .1f

__host__ __device__ __inline__ void rayStandard(const uint2 launchIndex, const uint2 launchDim, 
                                const float3 eye, const float3 U, const float3 V, const float3 W, 
                                const float3 sceneCenter, const float sceneEpsilon, const float sceneMaxDim,
                                float3& rayOrigin, float3& rayDirection, float& near, float& far)
{
    float2 d = make_float2(launchIndex) / make_float2(launchDim) * 2.f - 1.f;
    rayOrigin = eye;
    rayDirection = normalize(d.x * U + d.y * V + W);
    
    
    const float dist = length(eye - sceneCenter);
    near = max(sceneEpsilon, dist - sceneMaxDim / 2);
    far  = dist + sceneMaxDim / 2;
}

__host__ __device__ __inline__ void rayOrtho(const uint2 launchIndex, const uint2 launchDim, 
                               
                                const float3 sceneCenter, const float sceneEpsilon, const float sceneMaxDim,
                                const float3 cameraDir,
                                const float3 nearPoint1,const float3 nearPoint2,
                                const float3 nearPoint3,const float3 nearPoint4,
                                float3& rayOrigin, float3& rayDirection, float& near, float& far)
{
    float2 d = make_float2(launchIndex) / make_float2(launchDim);
    rayOrigin = nearPoint1 + (nearPoint2-nearPoint1)*(1.f-d.y) + (nearPoint4-nearPoint1)*d.x;
    rayDirection = cameraDir;
    
    //const float dist = length(eye - sceneCenter);
    near = .0005f;//max(sceneEpsilon, dist - sceneMaxDim / 2);
    far  = .1f;//dist + sceneMaxDim / 2;
}


 __device__ __inline__ void rayFisheye(const uint2 launchIndex, const uint2 launchDim,  
                                const float3 eye, const float3 U, const float3 V, const float3 W, 
                                const float3 sceneCenter, const float sceneEpsilon, const float sceneMaxDim,
                                float3& rayOrigin, float3& rayDirection, float& near, float& far)
    {
                                
    //Borrowed from cubemap to fisheye
    float2 d = -1.f + 2*make_float2(launchIndex) / make_float2(launchDim);

    //Finds the distance, r, from the center
    float r=sqrt(d.x*d.x+d.y*d.y);

    //Finds normalized versions of x and y
    float xnorm=d.x/r;
    float ynorm=d.y/r;

    //Finds betaZ, the z coord on a sphere
    float angle=6.*asin(r/2.);
    //float angle=r*3.14159;
//    if(angle>3.14159)
//      far=0;
    float betaZ=cos(angle);
    
    //Finds the scale factor for the x and y co-ordinates to get back to
    //Sphere co-ordinates
    float c=sqrt((1.0-(betaZ*betaZ))/(xnorm*xnorm+ynorm*ynorm));

    //Calculates the sphere co-ordinates
    float xCube=c*xnorm;
    float yCube=c*ynorm;
    float zCube=betaZ;
    
    //End borrowed from cubemap to fisheye
    rayOrigin = eye;
    rayDirection=normalize(U*xCube+V*yCube+W*zCube);
    
    const float dist = length(eye - sceneCenter);
    near = max(sceneEpsilon, dist - sceneMaxDim / 2);
    far  = dist + sceneMaxDim / 2;
    if(angle>3.14159)
      far=0;
}
