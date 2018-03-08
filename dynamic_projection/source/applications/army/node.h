#ifndef NODE_H
#define NODE_H

struct Node{
  int height;
  float edges[8]; // edge weights clockwise, starting with upward neighbor

  Node(){
    height = 0;
    for(int i = 0; i < 8; i++){
      edges[i] = INFINITY;
    }
  }

  int numNeighbors(){
    int n = 0;
    for(int i = 0; i < 8; i++){
      if(edges[i] < INFINITY){
	n++;
      }
    }
    return n;
  }
};

#endif
