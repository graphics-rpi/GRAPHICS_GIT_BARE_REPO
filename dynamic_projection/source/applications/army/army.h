#ifndef ARMY_H
#define ARMY_H

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <list>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "Vector3.h"
#include "Image.h"
#include "ImageOps.h"
#include "CalibratedCamera.h"

#include "munkres.h"
#include "node.h"
#include "gridreader.h"

// hardcode for now


const unsigned int MAX_SOLDIER_MOVE = 50;
const unsigned int SOLDIER_GRID_SIZE = 4 * MAX_SOLDIER_MOVE * MAX_SOLDIER_MOVE;

const unsigned int MAX_SHOT_RANGE = 100;
const unsigned int SOLDIER_SHOT_HEIGHT = 50;

const float BASE_HIT_CHANCE = 0.333333333;
const double ADVANTAGE_THRESH = 40;

const double TABLE_RADIUS = 0.5334; // 21 inches in meters
const double TABLE_DIAMETER = 2 * TABLE_RADIUS; // 42 inches in meters

//const char* temp_out_file = "overlay_temp.ppm";
const char* temp_out_file = "/home/grfx/images/army.ppm";

//const char* out_file = "overlay.ppm";
const char* out_file = "/home/grfx/images/surface_camera_floor.ppm";


enum Color{
  RED,
  GREEN,
  NONE
};

enum GameState{
  TERRAIN_INIT,
  SOLDIER_INIT,
  RED_MOVE,
  PRE_RED_COMBAT,
  RED_COMBAT,
  POST_RED_COMBAT,
  GREEN_MOVE,
  PRE_GREEN_COMBAT,
  GREEN_COMBAT,
  POST_GREEN_COMBAT,
  END_GAME,
  BLANK
};

template <typename Pixel>
class PixelSetter: public ImagePointFunctor<Pixel>
{
 public:
  PixelSetter(Pixel val){
    this->val = val;
  }
  void operator()(Pixel& pixel, int row, int col){
    pixel = val;
  }
 private:
  Pixel val;
};

class v2d{
public:
  v2d(){ x = y = 0;}
  
  v2d(const v2d& other){
    x = other.x;
    y = other.y;
  }

  v2d(float xval, float yval){
    x = xval; 
    y = yval;
  }
  
  v2d operator=(const v2d& other){
    x = other.x;
    y = other.y;
    return other;
  }

  v2d operator+(const v2d& other) const{
    return v2d(x + other.x, y + other.y);
  }

  v2d operator-(const v2d& other) const{
    return v2d(x - other.x, y - other.y);
  }

  float length() const{
    return std::sqrt(x*x + y*y);
  }

  float squared_length() const{
    return x*x + y*y;
  }

  void normalize(){
    float len = length();
    x /= len;
    y /= len;
  }
  
  float x, y;
};

v2d operator*(const v2d& v, float s){
  return v2d(v.x * s, v.y * s);
}


v2d operator*(float s, const v2d& v){
  return v2d(v.x * s, v.y * s);
}

/*
struct Point{
  Point(){row = col = 0;}
  Point(int r, int c){
    row = r;
    col = c;
  }
  int row, col;
};
*/

struct Target{
  int id;
  float hitChance;
};

bool compareTargets(const Target& t1, const Target& t2){
  return t1.hitChance < t2.hitChance;
}

struct Soldier{
public:

  Soldier(){
    //movementGrid = 0;
    //movementPaths = 0;
  }

  Soldier(const v2d& p, float r, int i, bool color){
    pos = new_pos = p;
    id = i;
    radius = r;
    red = color;
    //movementGrid = 0;
    //movementPaths = 0;
    found = true;
    dead = false;
  }

  Soldier(const Soldier& other){
    pos = other.pos;
    new_pos = other.new_pos;
    id = other.id;
    red = other.red;
    found = other.found;
    dead = other.dead;
    
    movementGrid = other.movementGrid;
    movementPaths = other.movementPaths;
  }

  ~Soldier(){
    /*
    std::cout << "cleaning up soldier" << std::endl;
    if(movementGrid){
      std::cout << "deallocating movementGrid" << std::endl;
      delete [] movementGrid;
    }
    if(movementPaths){
      std::cout << "deallocating movementPaths" << std::endl;
      delete [] movementPaths;
    }
    */
  }

  const Soldier& operator=(const Soldier& other){
    pos = other.pos;
    new_pos = other.new_pos;
    id = other.id;
    red = other.red;
    found = other.found;
    dead = other.dead;
    
    movementGrid = other.movementGrid;
    movementPaths = other.movementPaths;
    
    return other;
  }

  v2d pos;
  v2d new_pos;
  float radius; // might replace this later with a constant value
  //std::vector<Target > targets;
  //std::vector<int> targets;
  //std::vector<float> chanceToHit;
  int numAttackers;
  int id;
  bool red;
  bool found;
  bool dead;

  //float *movementGrid; //[FIELD_SIZE];
  //int *movementPaths;
  std::vector<float> movementGrid;
  std::vector<int> movementPaths;
  // FIX: this wastes a lot of space, given that no soldier will ever need that much space
};

class ArmyGame{
 public:
  ArmyGame();

 // Node* getCell(int r, int c) const;
  bool setGridFromImageFile(const char* filename);
  bool initFromFrame(const char* init_file);

  bool loadFile(const char* filename, std::vector<Soldier>& r_soldiers, std::vector<Soldier>& g_soldiers);

  bool updateSoldierMoves(Soldier& s);
  bool updateSoldierMoves(std::vector<Soldier>& soldiers);

  bool computeShotMatrices();
  void runCombatRound();

  double computeMovementCost(const Soldier& s, const v2d& dest);
  bool lineOfSight(const v2d& pos1, const v2d& pos2);
 
  bool updateFromFrame(const char* update_file, bool updateColor, std::vector<Soldier>& soldiers);

  void blankOverlay();

  void drawCircle(const v2d& center, sRGB& color, float radius);

  void drawMoveOptions(std::vector<Soldier>& soldiers);
  void drawMoveOptions(Soldier& s);

  void drawMovePaths(std::vector<Soldier>& soldiers);

  void drawShotLines();
  void drawShotLine(int r, int g);
  void drawShotArrow(int r, int g, Color target);
  void drawShotArrow(v2d from, v2d to, sRGB targetColor);
  sRGB advantageColor(int r, int g);

  void markSoldiers(std::vector<Soldier>& soldiers, sRGB liveColor, sRGB deadColor);
  void markDeadSoldiers();

  void drawRedWins();
  void drawGreenWins();
  void drawTie();
  void drawCheckerboard(sRGB color);

  void logOverlay();
  void writeOverlay(const char* filename);

  void displayCurrentBoard(Color  moveOptionsDisplayed, Color movePathsDisplayed,
                           bool shotLinesDisplayed, bool soldiersDisplayed);

  void run();

  int numSoldiersAlive(const std::vector<Soldier>& soldiers);
  bool correctRemoval();
  void removeDeadSoldiers();
  void changeState(GameState newState);
  
  void startLoggingImages(const char* dir);
  
 private:
  GameState state;
  Node* grid; // 2D grid representing world heightfield
  GridReader* gridreader;
  Image<sRGB> overlay; // overlay image that gets output as the floor texture
  std::vector<Soldier> red_soldiers;
  std::vector<Soldier> green_soldiers;

  // probability matrices used for combat round
  float* red_shot_matrix;
  float* green_shot_matrix;
  bool gameOver;
  bool validState;
  int round;
  int subStep;
  bool soldierDrawn(Soldier& s);
  void soldierInit(bool receivedOK);
  void drawDeadSoldiers();
  void drawDeadSoldier(Soldier& soldier);

  bool combatShouldHappen;

  // place to log images
  std::string logImageDirectory;
  bool logImages;
};

#endif
