#include "canvas.h"
#include <iostream>
#include <cstdlib>

#include "argparser.h"
#include "mesh.h"

int main(int argc, char* argv[]){
  ArgParser* args = new ArgParser(argc, argv);
  Mesh* mesh = new Mesh(args);
  glutInit(&argc, argv);
  Canvas::Initialize(args, mesh);

  delete args;
  delete mesh;
  return 0;
}

void TryToPressObj(int){}
void TryToReleaseObj(int){}
void TryToMoveObj(int){}
