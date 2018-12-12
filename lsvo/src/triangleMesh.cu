
/*
 * Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

#include <optix.h>
#include <optix_math.h>
#include <optixu/optixu_matrix.h>
#include <optixu/optixu_aabb.h>

// This is to be plugged into an RTgeometry object to represent
// a triangle mesh with a vertex buffer of triangle soup (triangle list)
// with an interleaved position, normal, texturecoordinate layout.

rtBuffer<float3> vertexBuffer;
rtBuffer<float3> normalBuffer;
rtBuffer<float2> texcoordBuffer;
rtBuffer<int3>   vertexIndexBuffer;    // position indices
rtBuffer<int3>   normalIndexBuffer;    // normal indices
rtBuffer<int3>   texcoordIndexBuffer;  // texcoord indices
rtBuffer<int>   centroidIndexBuffer;  // texcoord indices
rtBuffer<int>   patchIndexBuffer;  // texcoord indices
rtBuffer<uint>   materialBuffer;       // per-face material index

rtDeclareVariable(float3,     texcoord,          attribute texcoord,         );
rtDeclareVariable(float3,     geometric_normal,  attribute geometric_normal, );
rtDeclareVariable(float3,     shading_normal,    attribute shading_normal,   );

rtDeclareVariable(float3,     centroid,          attribute centroid,   );
rtDeclareVariable(float3,     rayDirection,          attribute rayDirection,   );
rtDeclareVariable(int,        patch,             attribute patch,   );
rtDeclareVariable(int,        centroidNumber,    attribute centroidNumber,   );
rtDeclareVariable(int,        triNumber,    attribute triNumber,   );
//rtDeclareVariable(int,        isGatherPass , ,   );
rtDeclareVariable(float,        betaAttr , attribute betaAttr,   );
rtDeclareVariable(float,        gammaAttr , attribute gammaAttr,   );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay,                             );

RT_PROGRAM void meshIntersect(int primIdx) {
  //rtPrintf("intersected with %i \n", primIdx);
  int3 v_idx = vertexIndexBuffer[primIdx];

  float3 p0 = vertexBuffer[v_idx.x];
  float3 p1 = vertexBuffer[v_idx.y];
  float3 p2 = vertexBuffer[v_idx.z];
	// centroid= (p0+p1+p2)/3.;
	// centroidNumber=centroidIndexBuffer[primIdx];
	// patch=patchIndexBuffer[primIdx];
	// triNumber=primIdx;
  //centroid=centroid/3.;
  // Intersect ray with triangle
  float3 e0 = p1 - p0;
  float3 e1 = p0 - p2;
  float3 n  = cross(e0, e1);

  float v   = dot(n, ray.direction);

  float r   = 1.0f / v;

  float3 e2 = p0 - ray.origin;
  float va  = dot(n, e2);
  float t   = r * va;

  if(t < ray.tmax && t > ray.tmin) {
    float3 i   = cross(e2, ray.direction);
    float v1   = dot(i, e1);
    float beta = r * v1;
    if(beta >= 0.0f){
      float v2 = dot(i, e0);
      float gamma = r * v2;
      if((v1 + v2) * v <= v * v && gamma >= 0.0f) {
        if(rtPotentialIntersection(t)) {
					rayDirection=ray.direction;
					centroid= (p0+p1+p2)/3.;
					centroidNumber=centroidIndexBuffer[primIdx];
					patch=patchIndexBuffer[primIdx];
					triNumber=primIdx;

          int3 n_idx = normalIndexBuffer[primIdx];

          if(normalBuffer.size() == 0 || n_idx.x < 0 || n_idx.y < 0 || n_idx.z < 0) {
            shading_normal = -n;
          } else {
            float3 n0 = normalBuffer[n_idx.x];
            float3 n1 = normalBuffer[n_idx.y];
            float3 n2 = normalBuffer[n_idx.z];
            shading_normal = normalize(n1 * beta + n2 * gamma + n0 * (1.0f - beta - gamma));
          }
          geometric_normal = -n;

          int3 t_idx = texcoordIndexBuffer[primIdx];
          if(texcoordBuffer.size() == 0 || t_idx.x < 0 || t_idx.y < 0 || t_idx.z < 0) {
            texcoord = make_float3(0.0f, 0.0f, 0.0f);
          } else {
            float2 t0 = texcoordBuffer[t_idx.x];
            float2 t1 = texcoordBuffer[t_idx.y];
            float2 t2 = texcoordBuffer[t_idx.z];
            texcoord = make_float3(t1 * beta + t2 * gamma + t0 * (1.0f - beta - gamma));
          }
          //rtPrintf("Centroid nubmer %d\n", centroidNumber);
          //if(!isGatherPass||v>0)

          betaAttr=beta;
          gammaAttr=gamma;
          rtReportIntersection(materialBuffer[primIdx]);
        }
      }
    }
  }
}

RT_PROGRAM void meshBounds(int primIdx, float result[6]) {
  int3 v_idx = vertexIndexBuffer[primIdx];

  float3 v0 = vertexBuffer[v_idx.x];
  float3 v1 = vertexBuffer[v_idx.y];
  float3 v2 = vertexBuffer[v_idx.z];

  optix::Aabb* aabb = (optix::Aabb*)result;
  aabb->m_min = fminf(fminf(v0, v1), v2);
  aabb->m_max = fmaxf(fmaxf(v0, v1), v2);
}
