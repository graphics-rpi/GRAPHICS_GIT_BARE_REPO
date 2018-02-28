#ifndef _WALL_CHAIN_H_
#define _WALL_CHAIN_H_

#include <vector>

struct ChainNeighbor {
  ChainNeighbor() { start_neighbor = -1; end_neighbor = -1; }
  int start_neighbor;
  bool flip_start_neighbor;
  double start_fit;
  int end_neighbor;
  bool flip_end_neighbor;
  double end_fit;
};

struct WallChain {
  std::vector<ConvexQuad> quads;
  int sided;
};

#endif
