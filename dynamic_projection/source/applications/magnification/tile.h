#ifndef __TILE_H__
#define __TILE_H__

#include <vector>

class Tile{
public:
  // x,y refers to upper left.
  Tile(x,y) : x_(x), y_(y), w_(1), h_(1){ }
  Tile(x,y,w,h) : x_(x), y_(y), w_(w), h_(h){ }

  std::vector<float> GetVertexData();

private:
  int x_;
  int y_;
  int w_;
  int h_;
};

#endif
