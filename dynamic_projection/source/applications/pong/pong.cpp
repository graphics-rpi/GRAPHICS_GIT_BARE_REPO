#include "pong.h"
#include "../paint/text.h"
#include <sstream>

// ==========================================================================================
// COLLISION FUNCTIONS
// ==========================================================================================


// check if v is within minimum bounding box of p1 and p2 (taking into account the edge offset and radius)
//bool boundingBoxCheck(Vector2& p1, Vector2& p2, Vector2& v, double radius){
bool boundingBoxCheck(WallPosition& wp, Vector2& v, double radius){
  double offset = radius; // + EDGE_OFFSET;
  WallPosition extPos = wp.getExtendedEndpoints(offset);
  Vector2 ep1 = extPos.getLeft();
  Vector2 ep2 = extPos.getRight();
  return !((std::min(ep1.x, ep2.x) - v.x > EPSILON) || (v.x - std::max(ep1.x, ep2.x) > EPSILON) ||
	   (std::min(ep1.y, ep2.y) - v.y > EPSILON) || (v.y - std::max(ep1.y, ep2.y) > EPSILON));
}

// given incident vector v and normal n return reflected vector
Vector2 reflectedVector(Vector2& n, Vector2& v){
  return v - n * 2 * (v.dot(n));
}

double wierdDotProduct(const Vector2& line_a,
                       const Vector2& line_b,
                       const Vector2& point_c) {
  return
    + line_a.x  * line_b.y
    + line_b.x  * point_c.y
    + point_c.x * line_a.y
    - line_b.x  * line_a.y
    - point_c.x * line_b.y
    - line_a.x  * point_c.y;
}

bool PongGame::collide(const WallPosition& old_pos, 
		       const WallPosition& new_pos, 
		       double tstep, 
		       double& new_t, 
		       Vector2& norm,
		       Vector2& new_ball_pos){
  //bool debug = true; //false;

  int num_steps = 100;
  double dt = tstep / num_steps;
  double t = 0;

  for(int i = 0; i < num_steps; i++, t += dt){
    // get interpolated position of wall and ball
    WallPosition wall_pos = old_pos.lerpTo(new_pos, t);
    Vector2 ball_pos = ball->getPosition() + ball->getVelocity() * tstep * t;

    if(debug) std::cout << "ball : " << ball_pos.x 
			      << " " << ball_pos.y << std::endl;
   
    // test for collision
    // shortest distance between ball pos and wall segment
    Vector2 u = ball_pos - wall_pos.getLeft();
    Vector2 v = wall_pos.getRight() - wall_pos.getLeft();

    if(debug) std::cout << "u : " << u.x << " " << u.y << std::endl;
    if(debug) std::cout << "v : " << v.x << " " << v.y << std::endl;

    if(debug){
      debugRays.clear();
      debugRays.push_back(DebugRay(wall_pos.getLeft(), u, 1, 1, 1));
      debugRays.push_back(DebugRay(wall_pos.getLeft(), v, 1, 1, 0));
      debugRays.push_back(DebugRay(ball_pos, ball->getVelocity() * 10, 1, 0, 0));
    }

    norm = Vector2(v.y, -v.x); // rotate 90 to get norm
    if(debug) std::cout << "norm : " << norm.x << " " << norm.y << std::endl;
    //norm.normalize();
    // project
    double d = u.dot(norm) / norm.magnitude();

    if(debug) std::cout << "d : " << d << std::endl;

    // make sure normal is facing the right way
    if(d < 0){
      d = -d;
      norm = norm * -1;
    }
    norm.normalize();

    // get collision point on wall line and make sure it falls within endpoints
    Vector2 p = ball_pos - norm * d;
    
    if(debug) debugRays.push_back(DebugRay(p, norm * d, 0, 1, 0));

    // find alpha that gives left + alpha * v = p
    double alpha = 0;
    if(abs(v.x) < EPSILON){
      alpha = (p.y - wall_pos.getLeft().y) / v.y;
    }
    else{
      alpha = (p.x - wall_pos.getLeft().x) / v.x;
    }

    if(debug) std::cout << "alpha = " << alpha << std::endl << std::endl;

    if(alpha < 0){
      // if alpha less than 0, clamp nearest point to left endpoint
      d = u.magnitude();
      norm = u;
      p = wall_pos.getLeft();
    }
    else if(alpha > 1){
      // if alpha greater than 0, clamp nearest point to right endpoint
      norm = ball_pos - wall_pos.getRight();
      d = norm.magnitude();
      p = wall_pos.getRight();
    }

    // make sure ball is moving into wall
    Vector2 vel = ball->getVelocity();
    if(norm.dot(vel) >= 0){
      // ignore, since the ball is moving away from the wall
      if(debug) std::cout << "*** ball is moving away from wall" << std::endl;
      //continue;
    }

    if(abs(d) <= ball->radius){
      if(debug) std::cout << "collision found at " << t << std::endl;
      new_t = t;
      norm.normalize();

      // move ball away from norm by radius + EPSILON
      new_ball_pos = p + norm * (2 * ball->radius + EPSILON);
      //ball->setPosition(new_ball_pos);

      glBegin(GL_LINES);
      glVertex2d(0,0);
      glVertex2d(1000,1000);
      glEnd();

      return true;
    }
  }

  return false;
}

/*
bool collide(const Ball& ball, const WallPosition& old_pos, 
	     const WallPosition& new_pos, 
	     double t, double& new_t){
  if(wierdDotProduct(ball.getPosition(), old_pos.getLeft(), old_pos.getRight()) < 0) {
    //std::cout << "Ball starts behind the wall." << std::endl;
    //std::cout << "Conclusion: no collision." << std::endl;
    return false;
  }

  if(wierdDotProduct(ball.getPosition() + ball.getVelocity() * t,
                     new_pos.getLeft(), new_pos.getRight()) >= 0) {
    //std::cout << "Ball ends up in front of the wall." << std::endl;
    //std::cout << "Conclusion: no collision." << std::endl;
    return false;
  }

  Vector2 o_l = old_pos.getLeft();
  Vector2 o_r = old_pos.getRight();
  Vector2 n_l = new_pos.getLeft();
  Vector2 n_r = new_pos.getRight();
  Vector2 pos = ball.getPosition();
  Vector2 vel = ball.getVelocity() * t;

  double a = n_l.y * n_r.x - n_l.x * n_r.y + n_r.y * o_l.x - n_r.x * o_l.y - n_l.y * o_r.x + o_l.y * o_r.x + n_l.x * o_r.y - o_l.x * o_r.y - n_l.y * vel.x + n_r.y * vel.x + o_l.y * vel.x - o_r.y * vel.x + n_l.x * vel.y - n_r.x * vel.y - o_l.x * vel.y + o_r.x * vel.y;
  double b = -n_r.y * o_l.x + n_r.x * o_l.y + n_l.y * o_r.x - 2 * o_l.y * o_r.x - n_l.x * o_r.y + 2 * o_l.x * o_r.y - n_l.y * pos.x + n_r.y * pos.x + o_l.y * pos.x - o_r.y * pos.x+n_l.x * pos.y - n_r.x * pos.y - o_l.x * pos.y + o_r.x * pos.y - o_l.y * vel.x + o_r.y * vel.x + o_l.x * vel.y - o_r.x * vel.y;
  double c = o_l.y * o_r.x - o_l.x * o_r.y - o_l.y * pos.x + o_r.y * pos.x + o_l.x * pos.y - o_r.x * pos.y;

  //std::cerr << "a = " << a << std::endl;
  //std::cerr << "b = " << b << std::endl;
  //std::cerr << "c = " << c << std::endl;

  //std::cerr << EPSILON << std::endl;
  if(a < EPSILON) { //(a == 0) {
    new_t = -c / b;
    //std::cerr << "t = " << new_t << std::endl;
  } else {
    double t1 = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
    double t2 = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
    //std::cerr << "t = " << t1 << " or " << t2 << std::endl;
    new_t = t1;
    if(t2 >= 0 && t2 < t1){
      new_t = t2;
    }
  }
  
  return true;
}
*/

PongGame::PongGame(){
  //collisionSFXFile = "pong_sounds/beep.wav";
  //goalSFXFile = "pong_sounds/arc_beep.wav";
  //endSFXFile = "pong_sounds/cheer2.wav";
  state = START;
  debug = false;
}

PongGame::~PongGame(){
  delete ball;
  for(unsigned int w = 0; w < walls.size(); w++){
    delete walls[w];
  }
}

void PongGame::init(int w, int h){
  width = w;
  height = h;

  leftBound = w * 0.1;
  rightBound = w * 0.9;
  midCourtX = (leftBound + rightBound) / 2;
  topBound = h * 0.1;
  bottomBound = h * 0.9;

  startPos = Vector2(width / 2, height / 2);
  double xVel = 10;
  srand ( time(NULL) );
  if(rand() % 2){
    xVel *= -1;
  }
  startVel = Vector2(xVel, 0);
  //xBound = 15 * FOOT_TO_METER; //4.5;
  //yBound = 15 * FOOT_TO_METER; //4.5;
  ball = new Ball(startPos, startVel);
  ball->radius = 10;
  //radius = 100;
  score1 = score2 = 0;
  running = false; //true;
  restart = false;

  // init walls
  walls.clear();
 
  walls.push_back(new Wall("red_wall"));
  walls.push_back(new Wall("blue_wall"));

  // first set up walls for court bounds
  Vector2 topLeft(leftBound, topBound);
  Vector2 topRight(rightBound, topBound);
  Vector2 bottomLeft(leftBound, bottomBound);
  Vector2 bottomRight(rightBound, bottomBound);
  walls.push_back(new StaticWall("top_edge", WallPosition(topLeft, topRight)));
  walls.push_back(new StaticWall("bottom_edge", WallPosition(bottomRight, bottomLeft)));
  //walls.push_back(new StaticWall("left_edge", WallPosition(bottomLeft, topLeft)));
  //walls.push_back(new StaticWall("right_edge", WallPosition(topRight, bottomRight)));

  // initial wall positions
  std::vector<Vector2> positions;
  positions.push_back(Vector2(width * 0.2, height * 0.8));
  positions.push_back(Vector2(width * 0.2, height * 0.2));
  positions.push_back(Vector2(width * 0.8, height * 0.2));
  positions.push_back(Vector2(width * 0.8, height * 0.8));

  updateWalls(positions);
}

void PongGame::updateWalls(std::vector<Vector2>& positions){
  assert(positions.size() >= 4);
  //  int x = rand() % 2;
  walls[0]->updatePosition(WallPosition(positions[0], // + Vector2(41 * x, 0), 
					positions[1])); // + Vector2(41 * x, 0)));
  walls[1]->updatePosition(WallPosition(positions[2], positions[3]));
}

/*
void PongGame::writeGameData(const char* filename){
  std::ofstream ostr(filename);
  Vector2 pos = ball->getPosition();
  ostr << pos.x << " " << 0 << " " << pos.y << std::endl;
  ostr << xBound << " " << yBound << std::endl;
  ostr << score1 << " " << score2 << std::endl;
}
*/

void PongGame::resetGame(){
  ball->setPosition(startPos);
  ball->setVelocity(startVel);
  score1 = score2 = 0;
  restart = false;
  state = START;
  //running = true;
}

void PongGame::togglePause(){
  running = !running;
}

void PongGame::resetBallAfterGoal(){
  //PlaySound(goalSFXFile.c_str());
  ball->setPosition(startPos);
  if(score1 > 2){
    //score1 = 5;
    //score2 = 6;
    //running = false;
    //restart = true;
    state = END;
  }
  else if(score2 > 2){
    //score1 = 6;
    //score2 = 5;
    //running = false;
    //restart = true;
    state = END;
  }
  else{
    state = RESET_AFTER_GOAL;
    delayCounter = 45;
  }
  Vector2 newVel = ball->getVelocity() * -1;
  ball->setVelocity(newVel);
  //sleep(2);
}

// input the total timestep remaining, return back a fraction of that timestep, between 0 and 1
double PongGame::handleCollision(Wall* wall, Vector2& pos, 
				 Vector2& vel, double timestep){
  double t;
  Vector2 norm;
  if(debug) std::cout << "************** handle collision" << std::endl;
  // find t value (0-1) of collision, if any
  if(!collide(wall->oldPos, wall->newPos, timestep, t, norm, pos)){
    return INFINITY; // no collision
  }
  // calculate ball position at collision
  //pos = pos + vel * t * timestep;
  
  // calculate interpolated position of wall at collision
  //std::cerr << "old pos " << wall->oldPos.getLeft() << " " << wall->oldPos.getRight() << std::endl;
  //std::cerr << "new pos " << wall->newPos.getLeft() << " " << wall->newPos.getRight() << std::endl;
  //WallPosition intPos = wall->interpolatedPosition(t);
  //std::cerr << "int pos " << intPos.getLeft() << " " << intPos.getRight() << std::endl;
  //Vector2 left = intPos.getLeft();
  //Vector2 right = intPos.getRight();
  
  // verify collision within endpts of wall
  //if(!boundingBoxCheck(intPos, pos, ball->radius)){ // + EDGE_OFFSET)){
    //std::cerr << "out of bounds" << std::endl;
  //  return INFINITY; // no collision
  //}

  // compute reflected vector (velocity) and update position a fractional amount
  vel = reflectedVector(norm, vel);
  //pos = pos + vel * EPSILON;
  return t;
}

void PongGame::runFrame(std::vector<Vector2>& positions, double timestep){

  std::cout << "running a frame" << std::endl;
  
  if(restart){
    resetGame();
  }
  
  //if(state == PAUSED) return;

  // first read in the new wall positions and update the walls
  updateWalls(positions);
  
  if(state != RUNNING){
    if(state == RESET_AFTER_GOAL){
      if(delayCounter <= 0){
	state = RUNNING;
      }
      else{
	delayCounter--;
      }
    }
    return;
  }
  
  std::cout << "done updating walls" << std::endl;
  
  //std::cerr << "start frame" << std::endl;
  double time_spent = 0;
  while(time_spent < timestep){
    double t;
    double time_left = timestep - time_spent; // total amount of time left this frame
    double min_t = 1;

    // compute what the new pos and vel of the ball will be without a collision
    Vector2 new_ball_pos = ball->getPosition() + ball->getVelocity() * min_t * time_left;
    Vector2 new_ball_vel = ball->getVelocity();

    //int wall_i = -1;

    
    // now check all walls for collision and handle accordingly
    //debug = true;
    for(unsigned int w = 0; w < walls.size(); w++){
      //if(!walls[w]->inScene) continue;

      if(debug) std::cout << "debug print for wall " << walls[w]->name << std::endl;
      Vector2 pos = ball->getPosition();
      Vector2 vel = ball->getVelocity();
      t = handleCollision(walls[w], pos, vel, time_left);
      
      if(t >= 0 && t < min_t){
	min_t = t;
	new_ball_pos = pos;
	new_ball_vel = vel;

	//	wall_i = w;

      }

      debug=false;
    }
    
    //std::cerr << "update - collision with wall " << wall_i << std::endl;
    
    // update ball
    ball->setPosition(new_ball_pos);
    ball->setVelocity(new_ball_vel);
    
    // update walls (only the first 2 move)
    for(unsigned int w = 0; w < 2; w++){
      //if(!walls[w]->inScene) continue;
      walls[w]->oldPos = walls[w]->interpolatedPosition(min_t);
    }
    
    //std::cerr << "min_t = " << min_t << std::endl;
    time_spent += time_left * min_t;
    
    // check for goal
    
    if(ball->getPosition().x <= leftBound){
      score2++;
      //state = 
      resetBallAfterGoal();
    }
    else if(ball->getPosition().x >= rightBound){
      score1++;
      resetBallAfterGoal();
    }
    
    
    //if(wall_i >= 0){
    //  PlaySound(collisionSFXFile.c_str());
    //}

    /*
      if(wall_i == 0){
      score1++;
      resetBallAfterGoal();
      }
      else if(wall_i == 1){
      score2++;
      resetBallAfterGoal();
      }*/
  }
  //std::cerr << "end frame" << std::endl;

  std::cout << "done running a frame" << std::endl;
}

/*
void PongGame::output_for_visualization(Vector2& ball_pos, double& rad, 
					std::vector<WallPosition>& wall_pos){
  ball_pos = ball->getPosition();
  wall_pos.clear();
  for(int w = 0; w < walls.size(); w++){
    wall_pos.push_back(walls[w]->newPos);
  }
  rad = radius;
}
*/

void PongGame::draw(){

  // draw scores
  std::stringstream ss (std::stringstream::in | std::stringstream::out);
  ss << score1 << "   " << score2;
  drawstring(width/2, height * 0.85, ss.str().c_str(), Vec3f(1,1,1), 100, 50);

  glLineWidth(3);

  glBegin(GL_LINES);

  // draw court
  glColor3f(1,1,1);
  glVertex2d(leftBound, topBound);
  glVertex2d(rightBound, topBound);

  glVertex2d(rightBound, topBound);
  glVertex2d(rightBound, bottomBound);

  glVertex2d(rightBound, bottomBound);
  glVertex2d(leftBound, bottomBound);

  glVertex2d(leftBound, bottomBound);
  glVertex2d(leftBound, topBound);

  glVertex2d(0.5 * (leftBound + rightBound), topBound);
  glVertex2d(0.5 * (leftBound + rightBound), bottomBound);

  glEnd();

  glLineWidth(20);

  glBegin(GL_LINES);

  // draw red wall
  glColor3f(1,0,0);
  glVertex2f(walls[0]->newPos.getLeft().x, walls[0]->newPos.getLeft().y); 
  glVertex2f(walls[0]->newPos.getRight().x, walls[0]->newPos.getRight().y);

  // draw blue wall
  glColor3f(0,1,1);
  glVertex2f(walls[1]->newPos.getLeft().x, walls[1]->newPos.getLeft().y); 
  glVertex2f(walls[1]->newPos.getRight().x, walls[1]->newPos.getRight().y);

  glEnd();

  // draw the ball
  double dTheta = M_PI / 16;
  glBegin(GL_TRIANGLE_FAN);
  //glBegin(GL_LINE_STRIP);
  glColor3f(1,1,1);
  // draw center
  std::cout << "drawing ball : " << ball->getPosition().x
	    << " " << ball->getPosition().y << std::endl;
  glVertex2d(ball->getPosition().x, ball->getPosition().y);

  for(double theta = -dTheta; theta < 2 * M_PI; theta += dTheta){
    double dX = ball->radius * std::cos(theta);
    double dY = ball->radius * std::sin(theta);

    glVertex2d(ball->getPosition().x + dX, 
	       ball->getPosition().y + dY);
  }
  glEnd();

  if(state == START){
    drawstring(width/2, height * 0.7, "GET READY!", Vec3f(1,1,1), 300, 150);
  }
  else if(state == PAUSED){
    drawstring(width/2, height/2, "PAUSED", Vec3f(1,1,1), 300, 150);
  }
  else if(state == END){
    if(score1 == 3){
      drawstring(width/2, height * 0.7, "RED WINS!", Vec3f(1, 0, 0), 300, 150);
    }
    else if(score2 == 3){
      drawstring(width/2, height * 0.7, "BLUE WINS!", Vec3f(0, 1, 1), 300, 150);
    }
  }

  // draw debug rays
  glBegin(GL_LINES);
  for(unsigned int i = 0; i < debugRays.size(); i++){
    glColor3f(debugRays[i].red, debugRays[i].green, debugRays[i].blue);
    glVertex2d(debugRays[i].pos.x, debugRays[i].pos.y);
    Vector2 p = debugRays[i].pos + debugRays[i].dir;
    glVertex2d(p.x, p.y);
  }
  glEnd();
}

void PongGame::debug_output(){
  Vector2 pos = ball->getPosition();
  std::cerr << "Ball : (" << pos.x << " " << pos.y << ")" << std::endl;
}

void PongGame::keypress(){
  if(state == START){
    // start the game
    state = RUNNING;
  }
  else if(state == RUNNING){
    // pause the game
    state = PAUSED;
  }
  else if(state == PAUSED){
    // unpause
    state = RUNNING; 
  }
  else if(state == END){
    state = START;
    restart = true;
  }
}
