#ifndef _RENDER_H_
#define _RENDER_H_

#include "meshmanager.h"

class Mesh;
class ArgParser;

class Render {

public:

  static void RenderAll(MeshManager *meshes);
  static void renderForSelect(MeshManager *meshes);

private:

  static void RenderLines(MeshManager *meshes);

  static void RenderTriangles(MeshManager *meshes);
  static void RenderArrangement(MeshManager *meshes, int flag);
  static void RenderTriangleVertices(MeshManager *meshes);
  static void RenderTriangleEdges(MeshManager *meshes);

  static void RenderClusterBoundaries(MeshManager *meshes);
  static void RenderNonManifoldEdges(MeshManager *meshes);
  static void RenderCreaseEdges(MeshManager *meshes);

  static void RenderArrangementBoundaries(MeshManager *meshes);
  static void RenderBadTriangles(MeshManager *meshes);
  static void RenderFlippedEdges(MeshManager *meshes);
  static void RenderShortEdges(MeshManager *meshes);
  static void RenderConcaveAngles(MeshManager *meshes);
  static void RenderGroundPlane(MeshManager *meshes);

  static void RenderPointSampledEnclosure(MeshManager *meshes); 
};

#endif
