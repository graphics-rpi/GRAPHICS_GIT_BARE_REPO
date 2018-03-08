
#include <cmath>
#include <iostream>
#include <list>
#include <fstream>

enum Direction{
  UP,
  UPRIGHT,
  RIGHT,
  DOWNRIGHT,
  DOWN,
  DOWNLEFT,
  LEFT,
  UPLEFT
};

struct Node{
  float height;
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

int main(int argc, char** argv){

  const float ROOT2 = std::sqrt(2.0);
  const float DIFF_THRESH = 0.1;

  int width = 15;
  int height = 15;
  int size = width * height;
  Node* grid;

  // if input file is specified, read height-field from it
  if(argc == 2){
    std::ifstream fin(argv[1]);
    fin >> width >> height;
    int size = width * height;
    grid = new Node[size];
    
    for(int i = 0; i < size; i++){
      fin >> grid[i].height;
    }
    fin.close();
  }
  else{
    grid = new Node[size];
  }

  int tl, tr, br, bl;

  tl = 0;
  tr = 1;
  bl = width;
  br = width + 1;

  for(int r = 0; r < height - 1; r++){
    for(int c = 0; c < width - 1; c++, tl++, tr++, bl++, br++){
      // link between tl and tr
      if(std::fabs(grid[tl].height - grid[tr].height) < DIFF_THRESH){
	grid[tl].edges[RIGHT] = grid[tr].edges[LEFT] = 1;
      }
      // link between tl and bl
      if(std::fabs(grid[tl].height - grid[bl].height) < DIFF_THRESH){
	grid[tl].edges[DOWN] = grid[bl].edges[UP] = 1;
      }
      // link between bl and tr
      if(std::fabs(grid[tr].height - grid[bl].height) < DIFF_THRESH){
	grid[tr].edges[DOWNLEFT] = grid[bl].edges[UPRIGHT] = ROOT2;
      }
      // link between tl and br
      if(std::fabs(grid[tl].height - grid[br].height) < DIFF_THRESH){
	grid[tl].edges[DOWNRIGHT] = grid[br].edges[UPLEFT] = ROOT2;
      }
    }

    // right-most column, handle UP-DOWN link
    if(std::fabs(grid[tl].height - grid[bl].height) < DIFF_THRESH){
      grid[tl].edges[DOWN] = grid[bl].edges[UP] = 1;
    }
    tl++; bl++; tr++; br++;
  }

  // handle LEFT-RIGHT links for bottom row
  for(int c = 0; c < width - 1; c++, tl++, tr++){
    if(std::fabs(grid[tl].height - grid[tr].height) < DIFF_THRESH){
      grid[tl].edges[RIGHT] = grid[tr].edges[LEFT] = 1;
    }
  }
  

  // print the grid
  int i = 0;
  for(int r = 0; r < height; r++){
    for(int c = 0; c < width; c++, i++){
      if(grid[i].height == 0){
	std::cout << grid[i].numNeighbors();
      }
      else{
	std::cout << 'X';
      }
      //std::cout << (grid[i].edges[DOWNRIGHT] < INFINITY);
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;


  // now perform a BFS to get shortest path distances for each node, 
  // starting at center;
  int node = size / 2;
  int MOVE_DOWN = width;
  int MOVE_UP = -width;
  int MOVE_LEFT = -1;
  int MOVE_RIGHT = 1;

  const float MAX_MOVE = 6;

  int moves[] = {MOVE_UP, MOVE_UP + MOVE_RIGHT, MOVE_RIGHT, 
		 MOVE_RIGHT + MOVE_DOWN, MOVE_DOWN, MOVE_DOWN + MOVE_LEFT, 
		 MOVE_LEFT, MOVE_LEFT + MOVE_UP};

  float* dists = new float[size];
  float d;
  for(int j = 0; j < size; j++){
    dists[j] = INFINITY;
  }

  dists[node] = 0;

  int next;
  std::list<int> fringe;
  fringe.push_back(node);

  while(!fringe.empty()){
    node = fringe.front();
    fringe.pop_front();

    for(int j = 0; j < 8; j++){
      next = node + moves[j];
      if(grid[node].edges[j] < INFINITY){
	d = dists[node] + grid[node].edges[j];
	if(d < MAX_MOVE && d < dists[next]){
	  dists[next] = d;
	  fringe.push_back(next);
	}
      }
    }
  }

  // print the dists
  i = 0;
  for(int r = 0; r < height; r++){
    for(int c = 0; c < width; c++, i++){
      std::cout << dists[i] << "\t";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;

  delete [] grid;
  delete [] dists;

  return 0;
}
