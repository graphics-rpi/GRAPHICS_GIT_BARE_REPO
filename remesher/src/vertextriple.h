#ifndef _VERTEX_TRIPLE_H_
#define _VERTEX_TRIPLE_H_

#include <cstdio>

// ==========================================================

class VertexParent {
public:
  VertexParent(int i1, int i2, int o) {
    in1 = i1; in2 = i2; out = o; }
  ~VertexParent() {}
  int getIn1() { return in1; }
  int getIn2() { return in2; }
  int getOut() { return out; }
  void Print() { 
    printf ("vertexparent %d %d %d -----------\n",in1,in2,out); }
  
private:
  int in1, in2, out;
};

// ==========================================================

#endif
