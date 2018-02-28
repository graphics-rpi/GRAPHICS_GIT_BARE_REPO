#include <fstream>
#include <cassert>
#include "mesh.h"
#include "meshmanager.h"
#include "argparser.h"
#include "wall.h"

std::vector<std::pair<std::pair<Vec3f,Vec3f>,bool> > GLOBAL_PUZZLE_CONNECTIONS;

void PuzzleMode(MeshManager *meshes) {

  GLOBAL_PUZZLE_CONNECTIONS.clear();

  std::string &filename =  meshes->args->puzzle_output_file;

  std::vector<bool> lefts_match(5,false);
  std::vector<bool> rights_match(5,false);

  // mark the edge walls
  lefts_match[0] = true;
  rights_match[4] = true;

  Walls* walls = meshes->getWalls();
  assert (walls != NULL);

  std::vector<BasicWall*> my_walls(5);
  std::vector<int> which_face(5);

  my_walls[0] = walls->getWallWithName("left_l_wall");   which_face[0] = 1;
  my_walls[1] = walls->getWallWithName("luan_wall");     which_face[1] = 0;
  my_walls[2] = walls->getWallWithName("big_l_wall");    which_face[2] = 1;
  my_walls[3] = walls->getWallWithName("canvas_wall");   which_face[3] = 0;
  my_walls[4] = walls->getWallWithName("right_l_wall");  which_face[4] = 0;


  for (int j = 0; j < 4; j++) {
    int k = j+1;
    if (my_walls[j] == NULL) continue;
    if (my_walls[k] == NULL) continue;

    int tmp = (which_face[k]+1)%my_walls[k]->numGoodVerts();

    Vec3f A = my_walls[j]->getGoodVert(which_face[j]);
    Vec3f B = my_walls[k]->getGoodVert(tmp);

    double dist = DistanceBetweenTwoPoints(A,B);
    bool flag = (dist < 0.5);
    GLOBAL_PUZZLE_CONNECTIONS.push_back(std::make_pair(std::make_pair(A,B),flag));
    if (flag) {
      rights_match[j] = true;
      lefts_match[k] = true;
    }
  } 

  std::ofstream ostr(filename.c_str());
  assert (ostr);

  for (int i = 0; i < 5; i++) {
    if (lefts_match[i] && rights_match[i]) {
      ostr << "finished_" << i+1 << std::endl;
    } else if (lefts_match[i]) {
      ostr << i+1 << "_left_only" << std::endl;
    } else if (rights_match[i]) {
      ostr << i+1 << "_right_only" << std::endl;
    } else {
      ostr << i+1 << "_neither" << std::endl;
    }
  }

}

