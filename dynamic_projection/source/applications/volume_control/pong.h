#ifndef PONG_H_
#define PONG_H_

#include "../paint/gl_includes.h"

#include "vector2.h"
#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <limits>

//const double INF = 10000;
const double EPSILON = std::numeric_limits<double>::epsilon(); //0.000000000001;
//const double EDGE_OFFSET = 0.1524;

enum GameState {
  START,
  RUNNING,
  PAUSED,
  RESET_AFTER_GOAL,
  END
};

// ==========================================================================================
// Ball class
// ==========================================================================================

class Ball {
public:
  Ball(Vector2 pos, Vector2 vel)
    : radius(1), 
    mPosition(pos),
    mVelocity(vel)
  {}

  Vector2 getPosition() const {
    return mPosition;
  }

  Vector2 getVelocity() const {
    return mVelocity;
  }

  void setPosition(Vector2& pos){
    mPosition = pos;
  }

  void setVelocity(Vector2& vel){
    mVelocity = vel;
  }


  double radius;

protected:
  Vector2 mPosition;
  Vector2 mVelocity;

private:
  Ball() {}
};


// ==========================================================================================
// WallPosition class
// ==========================================================================================

class WallPosition {
public:
  WallPosition() 
    : mLeft(Vector2(0,0)),
      mRight(Vector2(0,0))
  {}

  WallPosition(Vector2 left, Vector2 right)
    : mLeft(left),
      mRight(right)
  {}

  Vector2 getLeft() const {
    return mLeft;
  }

  Vector2 getRight() const {
    return mRight;
  }

  WallPosition getExtendedEndpoints(double offset){
    Vector2 d = mRight - mLeft;
    offset /= d.magnitude();
    d.normalize();
    Vector2 ep1 = mLeft - d * offset;
    Vector2 ep2 = mRight + d * offset;
    return WallPosition(ep1, ep2);
  }

  WallPosition lerpTo(const WallPosition& other, double t) const{
    WallPosition res(mLeft * (1-t) + other.mLeft * t,
		     mRight * (1-t) + other.mRight * t);
    return res;
  }

protected:
  Vector2 mLeft;
  Vector2 mRight;
};

// ==========================================================================================
// COLLISION FUNCTIONS
// ==========================================================================================

bool boundingBoxCheck(WallPosition& wp, Vector2& v, double radius);
Vector2 reflectedVector(Vector2& p1, Vector2& p2, Vector2& w);
double wierdDotProduct(const Vector2& line_a,
                       const Vector2& line_b,
                       const Vector2& point_c);
bool collide(const Ball& ball, const WallPosition& old_pos, 
	     const WallPosition& new_pos, 
	     double t, double& new_t);

// ==========================================================================================
// Wall class
// ==========================================================================================


class Wall{
public:
  Wall(){}
  
  Wall(std::string n){
    name = n;
    //inScene = false;
  }

  virtual void updatePosition(WallPosition pos){
    oldPos = newPos;
    newPos = pos;
  }

  // t is in range 0-1 and represents a percent of the time between the old and new positions
  // This function uses t to estimate where the wall was at t % of the timestep
  virtual WallPosition interpolatedPosition(double t){
    //Vector2 left = oldPos.getLeft() * t + newPos.getLeft() * (1-t);
    //Vector2 right = oldPos.getRight() * t + newPos.getRight() * (1-t);
    //return WallPosition(left, right);
    return oldPos.lerpTo(newPos, t);
  }

  std::string name;
  //bool inScene;
  WallPosition oldPos;
  WallPosition newPos;
};

class StaticWall: public Wall{
public:
  StaticWall(std::string n, WallPosition pos){
    name = n;
    oldPos = newPos = pos;
    //inScene = true;
  }

  void updatePosition(WallPosition& pos){
    std::cerr << "Error: Cannot update static wall position." << std::endl;
  }
  
  WallPosition interpolatedPosition(double t){
    return oldPos;
  }
};


// debug class
class DebugRay{
 public:
  DebugRay(){
    red = green = blue = 1;
  }

  DebugRay(Vector2 p, Vector2 d, float r, float g, float b){
    pos = p;
    dir = d;
    red = r;
    green = g;
    blue = b;
  }

  Vector2 pos;
  Vector2 dir;
  float red, green, blue;
}; 


// ==========================================================================================
// PongGame class
// ==========================================================================================


class PongGame{
public:
  PongGame();
  
  ~PongGame();

  //void readWalls(const char* filename);
  void init(int w, int h);

  void updateWalls(std::vector<Vector2>& positions);
  //void readWallPositions(const char* filename);
  
  //void writeGameData(const char* filename);

  void resetGame();

  void togglePause();

  void resetBallAfterGoal();

  bool collide(const WallPosition& old_pos, 
	       const WallPosition& new_pos, 
	       double tstep, 
	       double& new_t, 
	       Vector2& norm,
	       Vector2& new_ball_pos);
  
  double handleCollision(Wall* wall, Vector2& pos, 
			 Vector2& vel, double timestep);

  void runFrame(std::vector<Vector2>& positions, double timestep);

  void draw();

  //void output_for_visualization(Vector2& ball_pos, double& rad, 
  //			std::vector<WallPosition>& wall_pos);

  void debug_output();

  int getLeftBound() const { return leftBound; }
  int getRightBound() const { return rightBound; }
  int getTopBound() const { return topBound; }
  int getBottomBound() const { return bottomBound; }
  int getMidCourtX() const { return midCourtX; }

  void keypress();

private:
  Vector2 startPos; 
  Vector2 startVel;
  
  //double xBound, yBound;

  int width, height;
  int leftBound, rightBound, topBound, bottomBound, midCourtX;

  Ball* ball;
  double radius;
  std::vector<Wall*> walls;
  int score1, score2;
  
  std::vector<DebugRay> debugRays;

  //std::string collisionSFXFile;
  //std::string goalSFXFile;
  //std::string endSFXFile;

  int delayCounter;

  bool running;
  bool restart;

  bool debug;

  GameState state;
};

#endif
