#include "army.h"
#include "gridreader.h"
#include <algorithm>

#define pair_vector std::vector<std::pair<unsigned int,unsigned int> >

ArmyGame::ArmyGame(){
  grid = new Node[FIELD_SIZE];
  state = BLANK;
  round = 0;
  logImages = false;
  combatShouldHappen = false;

  gridreader= GridReader::Inst(grid);
  changeState(TERRAIN_INIT);
}


void ArmyGame::changeState(GameState newState)
{
  if(newState==RED_MOVE)
    ++round;
  std::ofstream fout_iter("round.txt");
  fout_iter<<round;
  fout_iter.close();

  state=newState;
  std::ofstream fout_state("state.txt");
  fout_state<<newState;
  fout_state.close();
  
  subStep = 0;
}

void ArmyGame::startLoggingImages(const char* dir){
  std::cout << "START LOGGING IMAGES" << std::endl;
  logImageDirectory = std::string(dir);
  logImages = true;
} 

bool file_exist(const char* str)
{

  std::ifstream fin(str);

  if(!fin)
  {
    return false;
  }
  else
  {
    fin.close();
    return true;
  }  
  
}

bool ArmyGame::setGridFromImageFile(const char* filename){

  // for now, simply init overlay to be the same as the heightfield
  if(gridreader->readFromFile(filename)==true)
  {
    overlay = Image<sRGB>(FIELD_HEIGHT, FIELD_WIDTH);
    for(unsigned int i = 0, r = 0; r < FIELD_HEIGHT; r++)
      for(unsigned int c = 0; c < FIELD_WIDTH; c++, i++)
        overlay(r,c) = sRGB(grid[i].height);
    return true;
  }
  return false;
}


bool ArmyGame::loadFile(const char* filename, std::vector<Soldier>& r_soldiers, std::vector<Soldier>& g_soldiers) {
  
  printf("loading %s \n", filename);
  std::ifstream fin(filename);
  if(!fin){
    printf("failed to load %s \n", filename);
    return false;
  }

  g_soldiers.clear();
  r_soldiers.clear();

  std::string s;
  v2d v;
  float r;
  //bool red = true;
  while(fin >> s){

    if (s == "soldier") {
      
      // rotate by 90
      fin >> v.y >> v.x >> r >> s;
      // convert world coords to pixel coords
      v.x = (v.x / TABLE_DIAMETER + 0.5) * FIELD_WIDTH;
      v.y = (-v.y / TABLE_DIAMETER + 0.5) * FIELD_HEIGHT;
      int height = grid[((int)v.y) * FIELD_WIDTH + (int)v.x].height;
      std::cout << "Init : soldier pos = " << v.y << " " << v.x << " " << height << std::endl;
      if(s == "red") {
	r_soldiers.push_back(Soldier(v,r,r_soldiers.size(),true));
	std::cout << "got a red soldier" << std::endl;
      }
      else{
	assert (s == "green");
	g_soldiers.push_back(Soldier(v,r,g_soldiers.size(),false));
	std::cout << "got a green soldier" << std::endl;
      }
    }
  }

  return true;
}



bool ArmyGame::initFromFrame(const char* init_file){

  // read in initial game configuration
  return loadFile(init_file,red_soldiers,green_soldiers);

}

bool ArmyGame::updateSoldierMoves(std::vector<Soldier>& soldiers){
  
  bool retVal=true;
  std::cout << "updating soldier moves" << std::endl;
  for(unsigned int i = 0; i < soldiers.size(); i++){
    if(updateSoldierMoves(soldiers[i])==false);
      retVal=false;
  }
  return retVal;
}

bool ArmyGame::updateSoldierMoves(Soldier& s){
  s.pos = s.new_pos; // finalize position

  // do a BFS to build a shortest distance grid
  int node = ((int)s.pos.y) * FIELD_WIDTH + (int)s.pos.x;

  if(s.movementGrid.empty()){
    std::cout << "allocating soldier movement grid" << std::endl;
    s.movementGrid.resize(FIELD_SIZE, INFINITY);
    std::cout << "done allocating soldier movement grid" << std::endl;
  }

  if(s.movementPaths.empty()){
    std::cout << "allocating soldier movement paths" << std::endl;
    s.movementPaths.resize(FIELD_SIZE, -1);
  }

  // initialize all distances in movement grid to infinity
  for(unsigned int j = 0; j < FIELD_SIZE; j++){
    s.movementGrid[j] = INFINITY;
    s.movementPaths[j] = -1;
  }

  int MOVE_DOWN = FIELD_WIDTH;
  int MOVE_UP = -FIELD_WIDTH;
  int MOVE_LEFT = -1;
  int MOVE_RIGHT = 1;

  int moves[] = {MOVE_UP, MOVE_UP + MOVE_RIGHT, MOVE_RIGHT, 
		 MOVE_RIGHT + MOVE_DOWN, MOVE_DOWN, MOVE_DOWN + MOVE_LEFT, 
		 MOVE_LEFT, MOVE_LEFT + MOVE_UP};

  int next;
  std::list<int> fringe;
  s.movementGrid[node] = 0;
  s.movementPaths[node] = node;
  fringe.push_back(node);

  while(!fringe.empty()){
    node = fringe.front();
    fringe.pop_front();
    
    //for the 8 directions
    for(int j = 0; j < 8; j++)
    {
      next = node + moves[j];
      if(grid[node].edges[j] < INFINITY)
      {
	      float d = s.movementGrid[node] + grid[node].edges[j];
	      if(d < MAX_SOLDIER_MOVE && d < s.movementGrid[next])
        {
      	  s.movementGrid[next] = d;
	        s.movementPaths[next] = node;
	        fringe.push_back(next);
	      }
      }
    }
  }
  return true;
}

// returns true if at least one pair of combatants will exchange fire, false otherwise
bool ArmyGame::computeShotMatrices(){
  bool matrixNonzero = false;
  for(unsigned int i = 0, r = 0; r < red_soldiers.size(); r++){
    for(unsigned int g = 0; g < green_soldiers.size(); g++, i++){
      if(lineOfSight(red_soldiers[r].new_pos, green_soldiers[g].new_pos)){
	matrixNonzero = true;
	
	// compute difference for height advantages
	double redHeight = grid[((int)red_soldiers[r].new_pos.y) * FIELD_WIDTH 
				+ (int)red_soldiers[r].new_pos.x].height;
	double greenHeight = grid[((int)green_soldiers[g].new_pos.y) * FIELD_WIDTH 
				  + (int)green_soldiers[g].new_pos.x].height;
	red_shot_matrix[i] = green_shot_matrix[i] = BASE_HIT_CHANCE;
	if(redHeight - greenHeight > ADVANTAGE_THRESH){
	  red_shot_matrix[i] = BASE_HIT_CHANCE * 2;
	}
	else if(greenHeight - redHeight > ADVANTAGE_THRESH){
	  green_shot_matrix[i] = BASE_HIT_CHANCE * 2;
	}
      }
      else{
	red_shot_matrix[i] = green_shot_matrix[i] = 0;
      }
    }
  }
  
  return matrixNonzero;

  /*
  std::cout << "red shot matrix " << std::endl;
  for(unsigned int i = 0, r = 0; r < red_soldiers.size(); r++){
    for(unsigned int g = 0; g < green_soldiers.size(); g++, i++){
      std::cout << red_shot_matrix[i] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl << "green shot matrix " << std::endl;
  for(unsigned int i = 0, r = 0; r < red_soldiers.size(); r++){
    for(unsigned int g = 0; g < green_soldiers.size(); g++, i++){
      std::cout << green_shot_matrix[i] << " ";
    }
    std::cout << std::endl;
  }
  */
}

void ArmyGame::drawDeadSoldier(Soldier& soldier){

  assert(soldier.dead);
  sRGB deadColor = sRGB(255);
	drawCircle(soldier.pos, deadColor, 5);


  for(int j = -5; j <= 5; j++){
	  overlay(soldier.pos.y + j, soldier.pos.x) = deadColor;
	  overlay(soldier.pos.y, soldier.pos.x + j) = deadColor;
  }
  
}

void ArmyGame::drawDeadSoldiers(){

  for(unsigned int g = 0; g < green_soldiers.size(); g++ ){
    if(green_soldiers[g].dead==true){
      drawDeadSoldier(green_soldiers[g]);
    }
  }

  for(unsigned int r = 0; r < red_soldiers.size(); r++){
    if(red_soldiers[r].dead==true){  
      drawDeadSoldier(red_soldiers[r]);
    }
  }
}

void ArmyGame::runCombatRound(){
  pair_vector shots;
  float rnd;
  double sleepSeconds = 0.75;
  for(unsigned int i = 0, r = 0; r < red_soldiers.size(); r++){
    for(unsigned int g = 0; g < green_soldiers.size(); g++, i++){
      if((red_shot_matrix[i] == 0) && (green_shot_matrix[i] == 0)){
	      continue;
      }
      shots.push_back(std::pair<unsigned int,unsigned int>(r,g));
    }
  }
  std::random_shuffle(shots.begin(), shots.end());
  for(pair_vector::iterator it = shots.begin(); it!=shots.end(); ++it){
    int r = it->first;
    int g = it->second;
    int i = r*green_soldiers.size() + g;
    rnd = (rand() / (float)RAND_MAX);
    if(rnd < red_shot_matrix[i]){
      green_soldiers[g].dead = true;
    }

    blankOverlay();
    drawDeadSoldiers();
    drawShotArrow(r,g, GREEN);

    writeOverlay("/home/grfx/images/army.ppm");
    rename("/home/grfx/images/army.ppm", out_file);
    usleep(sleepSeconds * 1000000);

    rnd = (rand() / (float)RAND_MAX);
    if(rnd < green_shot_matrix[i]){
	    red_soldiers[r].dead = true;
    } 

    blankOverlay();
    drawDeadSoldiers();
    drawShotArrow(r,g, RED);

    writeOverlay("/home/grfx/images/army.ppm");
    rename("/home/grfx/images/army.ppm", out_file);
    usleep(sleepSeconds * 1000000);


 
  }
}

double ArmyGame::computeMovementCost(const Soldier& s, const v2d& dest){
  int pos = ((int)dest.y) * (int)FIELD_WIDTH + (int)dest.x;
  if(s.movementGrid[pos] == INFINITY){
    return 1000000;
  }
  return s.movementGrid[pos];
}

bool ArmyGame::lineOfSight(const v2d& pos1, const v2d& pos2){
  v2d dir = (pos2 - pos1);
  double dist = dir.length();
  double dt = 1 / dist;
  if(dist > MAX_SHOT_RANGE){
    return false;
  }

  double z1 = grid[((int)pos1.y) * FIELD_WIDTH + (int)pos1.x].height + SOLDIER_SHOT_HEIGHT;
  double z2 = grid[((int)pos2.y) * FIELD_WIDTH + (int)pos2.x].height + SOLDIER_SHOT_HEIGHT;
  double dz = z2 - z1;

  for(double t = dt; t < 1; t += dt){
    v2d p = pos1 + t * dir;    

    if(grid[((int)p.y) * FIELD_WIDTH + (int)p.x].height > z1 + t * dz){
      // hit an obstacle
      return false;
    }
  }

  return true;
}

bool ArmyGame::updateFromFrame(const char* update_file, bool updateColorRed, std::vector<Soldier>& soldiers){
  
  std::vector<Soldier> r_soldiers;
  std::vector<Soldier> g_soldiers;

  bool success =  loadFile(update_file,r_soldiers,g_soldiers);
  if (!success) return false;

  std::vector<Soldier> new_soldiers;
  
  if (updateColorRed) { // red
    printf("RED soldiers size %d new soldiers size %d \n", (int)soldiers.size(), (int)r_soldiers.size());
    new_soldiers = r_soldiers;
  } else {
    printf("GREEN soldiers size %d new soldiers size %d \n", (int)soldiers.size(), (int)g_soldiers.size());
    new_soldiers = g_soldiers;
  }  

  std::cout << "outputting munkres matrix" << std::endl;

  Matrix<double> move_matrix(soldiers.size(), new_soldiers.size());
  for(int i = 0; i < move_matrix.rows(); i++){
    soldiers[i].found = false;
    for(int j = 0; j < move_matrix.columns(); j++){
      move_matrix(i,j) = computeMovementCost(soldiers[i], new_soldiers[j].pos);
      std::cout << move_matrix(i,j) << " ";
    }
    std::cout << std::endl;
  }
  
  Munkres m;
  m.solve(move_matrix);
  
  std::cout << "solved : " << std::endl;
  for(int i = 0; i < move_matrix.rows(); i++){
    for(int j = 0; j < move_matrix.columns(); j++){
      std::cout << move_matrix(i,j) << " ";
    }
    std::cout << std::endl;
  }

  for(int i = 0; i < move_matrix.rows(); i++){
    for(int j = 0; j < move_matrix.columns(); j++){
      if(move_matrix(i,j) == 0){
	// soldier matched, but need to make sure we can actually reach this spot
	int node = ((int)new_soldiers[j].pos.y) * FIELD_WIDTH + (int)new_soldiers[j].pos.x;
	soldiers[i].new_pos = new_soldiers[j].pos;
	std::cout << "Soldier moved from (" << soldiers[i].pos.y << ", " << soldiers[i].pos.x
		  << ") to (" << soldiers[i].new_pos.y << ", " << soldiers[i].new_pos.x << ")" << std::endl;
	if(soldiers[i].movementPaths[node] != -1){
	  soldiers[i].found = true;
	}
      }
    }
  }

  for(unsigned int i=0; i<soldiers.size(); i++)
  {
    if(soldiers[i].found == false)
    {
      return false;
    }
  }

  //for(unsigned int i=0; i<soldiers.size(); i++)
  //{
  //  if(soldierDrawn(soldiers[i]) == false)
  //  {
  //    return false;
  //  }
  //}

  return true;
}

bool ArmyGame::soldierDrawn(Soldier& s)
{

  int node = ((int)s.new_pos.y) * FIELD_WIDTH + (int)s.new_pos.x;

    if(s.movementPaths[node] != node)
        return true;

  else 
    return true;
}

void ArmyGame::blankOverlay(){
  for(unsigned int r = 0; r < FIELD_HEIGHT; r++){
    for(unsigned int c = 0; c < FIELD_WIDTH; c++){
      overlay(r,c) = sRGB(0,0,0);
    }
  }
}

void ArmyGame::drawCircle(const v2d& center, sRGB& color, float radius){
  int n_pts = 300;
  for (int p=0; p<n_pts; p++){
    double th = (2.* M_PI * p)/n_pts;
    int r = int(center.y) + radius * cos(th);
    int c = int(center.x) + radius * sin(th);
    if (r >= 0 && r < (int)FIELD_HEIGHT &&
	c >= 0 && c < (int)FIELD_WIDTH){
      overlay(r, c) = color;
    }
  }
}

void ArmyGame::drawMoveOptions(std::vector<Soldier>& soldiers){
  for(unsigned int i = 0; i < soldiers.size(); i++){
    drawMoveOptions(soldiers[i]);
  }
}

void ArmyGame::drawMoveOptions(Soldier& s){
  int MOVE_DOWN = FIELD_WIDTH;
  int MOVE_UP = -FIELD_WIDTH;
  int MOVE_LEFT = -1;
  int MOVE_RIGHT = 1;

  int moves[] = {MOVE_UP, MOVE_UP + MOVE_RIGHT, MOVE_RIGHT, 
		 MOVE_RIGHT + MOVE_DOWN, MOVE_DOWN, MOVE_DOWN + MOVE_LEFT, 
		 MOVE_LEFT, MOVE_LEFT + MOVE_UP};
  
  // overlay stuff
  for(unsigned int i = 0, r = 0; r < FIELD_HEIGHT; r++){
    for(unsigned int c = 0; c < FIELD_WIDTH; c++, i++){
      if(s.movementGrid[i] == INFINITY){
	continue;
      }
      sRGB oldColor = overlay(r,c);
      if(oldColor == sRGB(0,0,0)){
	overlay(r,c) = (s.red ? sRGB(200, 50, 50) : sRGB(50, 200, 50));
	//overlay(r,c) = (s.red ? sRGB(255, 175, 175) : sRGB(175, 255, 175));
      }
      else if(s.red && oldColor != sRGB(255, 255, 255)){ //sRGB(255,0,0)){
	overlay(r,c) = sRGB(std::min(255, oldColor.r() + 5), 
			    std::min(200, oldColor.g() + 25), 
			    std::min(200, oldColor.b() + 25));
	//overlay(r,c) = sRGB(255, std::max(50, oldColor.g() - 25), std::max(50, oldColor.b() - 25));
      }
      else if((!s.red) && oldColor != sRGB(255, 255, 255)){ //sRGB(0,255,0)){
	overlay(r,c) = sRGB(std::min(200, oldColor.r() + 25), 
			    std::min(255, oldColor.g() + 5),
			    std::min(200, oldColor.b() + 25));
	//overlay(r,c) = sRGB(std::max(50, oldColor.r() - 25), 255, std::max(50, oldColor.b() - 25));
      }
      // figure out if this is position is on the "edge" of movement (has an unreachable neighbor)
      for(int j = 0; j < 8; j++){
	if(grid[i].edges[j] == INFINITY || s.movementGrid[i + moves[j] ] == INFINITY){
	  // found an edge
	  overlay(r,c) = (s.red ? sRGB(255, 225, 225) : sRGB(225, 255, 225));
	  //overlay(r,c) = (s.red ? sRGB(255, 0, 0) : sRGB(0, 255, 0));
	  break;
	}
      }
    }
  }

}

void ArmyGame::drawMovePaths(std::vector<Soldier>& soldiers){
  std::cout << "drawing movement paths" << std::endl;
  // draw soldier movement paths
  for(unsigned int i = 0; i < soldiers.size(); i++){
    
    std::cout << "\tdrawing pos" << std::endl;

    // draw a circle
    sRGB color = sRGB(255); //(soldiers[i].red ? sRGB(255, 0, 0) : sRGB(0, 255, 0));
    drawCircle(soldiers[i].pos, color, 5);

    if(!soldiers[i].found){  
      continue;
    }
    
    // too short a move - don't bother displaying path
    if((soldiers[i].new_pos - soldiers[i].pos).squared_length() < 25){
      continue;
    }
    
    std::cout << "\tdrawing path" << std::endl;

    int node = ((int)soldiers[i].new_pos.y) * FIELD_WIDTH + (int)soldiers[i].new_pos.x;
    int count = 0;
    std::cout << "\tstarting at node : " << node << " with path value " 
	      << soldiers[i].movementPaths[node] << std::endl;
    while(soldiers[i].movementPaths[node] != node && count < 10000){
      int r = node / FIELD_WIDTH;
      int c = node % FIELD_WIDTH;
      overlay(r,c) = sRGB(255);
      node = soldiers[i].movementPaths[node];

      count++;
    }

    std::cout << "\tdone drawing path" << std::endl;
    


  }
}

sRGB ArmyGame::advantageColor(int r, int g)
{
    int i = r * green_soldiers.size() + g;
    int diff = (red_shot_matrix[i] - green_shot_matrix[i]) * 500;
    std::cout << "diff = " << diff << std::endl;
    sRGB color(255, 255, 0);
    if(diff > 0){
      // red advantage
      color = sRGB(255, 0, 0); //255 - std::min(255, diff), 0);
    }
    else if(diff < 0){
      // green advantage
      color = sRGB(0, 255, 0); //255 + std::max(-255, diff), 255, 0);
    }
    return color;
}

void ArmyGame::drawShotLine(int r, int g){
  int i = r * green_soldiers.size() + g;
  if(red_shot_matrix[i] > 0 || green_shot_matrix[i] > 0){
    //std::cout << "drawing line between red " << r << " and green " << g << std::endl;
    // determine line color based on advantage
    int diff = (red_shot_matrix[i] - green_shot_matrix[i]) * 500;
    std::cout << "diff = " << diff << std::endl;
    sRGB color(255, 255, 0);
    if(diff > 0){
      // red advantage
      color = sRGB(255, 0, 0); //255 - std::min(255, diff), 0);
    }
    else if(diff < 0){
      // green advantage
      color = sRGB(0, 255, 0); //255 + std::max(-255, diff), 255, 0);
    }
    
    //std::cout << "color = " << (int)color.r() << " " << (int)color.g() 
	  //    << " " << (int)color.b() << std::endl;
    // now draw a line
    PixelSetter<sRGB> setPixel(color);
    overlay.applyFunctorOnLine(setPixel, 
			       int(red_soldiers[r].new_pos.y),
			       int(red_soldiers[r].new_pos.x),
			       int(green_soldiers[g].new_pos.y),
			       int(green_soldiers[g].new_pos.x));
    //std::cout << "done drawing line" << std::endl;
  }
}

void ArmyGame::drawShotArrow(int r, int g, Color target){
  sRGB originColor, adColor;
  adColor=advantageColor(r,g);
  if(target==RED){
    originColor=sRGB(0,255,0);
    drawShotArrow( green_soldiers[g].new_pos, red_soldiers[r].new_pos, adColor);
  }
  else{
    originColor=sRGB(255,0,0);
    drawShotArrow( red_soldiers[r].new_pos, green_soldiers[g].new_pos, adColor);
  }
  
  
      
}

void ArmyGame::drawShotArrow(v2d from, v2d to, sRGB originColor)
{
  float m = ( to.y - from.y ) / 
            ( to.x - from.x );

  //The slope of the line perpendicular to the segment between the two soldiers
  float perpm;
  if(m==0)
      perpm=999;
  else
      perpm=-1./m;
  float theta=atan(perpm);
  float y_diff=sin(theta);
  float x_diff=cos(theta);
  v2d diffvec=to-from;
  float difflength = sqrt( diffvec.x*diffvec.x+diffvec.y*diffvec.y);

  //if(red_shot_matrix[i] > 0 || green_shot_matrix[i] > 0){
    //std::cout << "drawing line between red " << r << " and green " << g << std::endl;
    // determine line color based on advantage
 
    
    // now draw a line
    PixelSetter<sRGB> setPixel(originColor);
    overlay.applyFunctorOnLine(setPixel, 
			       int(to.y),
			       int(to.x),
			       int(from.y),
			       int(from.x));
    overlay.applyFunctorOnLine(setPixel, 
			       int(to.y-10*y_diff-diffvec.y*10/difflength),
			       int(to.x-10*x_diff-diffvec.x*10/difflength),
			       int(to.y),
			       int(to.x));

    overlay.applyFunctorOnLine(setPixel, 
			       int(to.y),
			       int(to.x),
			       int(to.y+10*y_diff-diffvec.y*10/difflength),
			       int(to.x+10*x_diff-diffvec.x*10/difflength));

}

void ArmyGame::drawShotLines(){
  //std::cout << "drawing shot lines" << std::endl;
  for(unsigned int r = 0; r < red_soldiers.size(); r++){
    for(unsigned int g = 0; g < green_soldiers.size(); g++){
      if(red_soldiers[r].found && green_soldiers[g].found){
	drawShotLine(r,g);
      }
    } 
  }
  //std::cout << "done drawing shot lines" << std::endl;
}

void ArmyGame::markSoldiers(std::vector<Soldier>& soldiers, sRGB liveColor, sRGB deadColor){
  for(unsigned int i = 0; i < soldiers.size(); i++){
    if(!soldiers[i].found){
      if(!soldiers[i].dead){
	// invalid
	sRGB color = liveColor;
	drawCircle(soldiers[i].new_pos, color, 10);
	drawCircle(soldiers[i].new_pos, color, 11);
	for(int j = -10; j <= 10; j++){
	  overlay(soldiers[i].new_pos.y + j, soldiers[i].new_pos.x) = color;
	  //overlay(soldiers[i].new_pos.y, soldiers[i].new_pos.x + j) = color;
	}
      }
    }
    else{
      sRGB color = liveColor;
      if(soldiers[i].dead){
	drawCircle(soldiers[i].new_pos, deadColor, 5);
	color = deadColor;
      }
      for(int j = -5; j <= 5; j++){
	overlay(soldiers[i].new_pos.y + j, soldiers[i].new_pos.x) = color;
	overlay(soldiers[i].new_pos.y, soldiers[i].new_pos.x + j) = color;
      }
    }
  }
}

void ArmyGame::drawRedWins(){
  drawCheckerboard(sRGB(255,0,0));
}

void ArmyGame::drawGreenWins(){
  drawCheckerboard(sRGB(0,255,0));
}

void ArmyGame::drawTie(){
  drawCheckerboard(sRGB(255,255,255));
}

void ArmyGame::drawCheckerboard(sRGB color){
  std::cout << "drawing checkerboard" << std::endl;
  unsigned int squareWidth = 50;
  unsigned int doubleWidth = 2 * squareWidth;
  for(unsigned int r = 0; r < FIELD_HEIGHT; r++){
    for(unsigned int c = 0; c < FIELD_WIDTH; c++){
      if(((r % doubleWidth) < squareWidth) == ((c % doubleWidth) < squareWidth)){
	overlay(r,c) = sRGB(0,0,0);
      }
      else{
	overlay(r,c) = color;
      }
    }
  }
}

void ArmyGame::logOverlay(){
  // logImageDirectory/overlay_<round>_<state>
  std::stringstream file;
  file << logImageDirectory << "/overlay_" << round << "_" << state << "_" << subStep << ".ppm";
  subStep++;
  std::cout << "logging file : " << file.str().c_str() << std::endl;
  overlay.write(file.str().c_str());
}

void ArmyGame::writeOverlay(const char* filename){
  std::cout << "writing overlay" << std::endl;
  
  overlay.write(filename);
}

void ArmyGame::soldierInit(bool receivedOK){

  //If it has been initialized and approved  
  if(validState && receivedOK){

    if(red_soldiers.size() > 0){
      changeState(RED_MOVE);
    }
    else{
      changeState(GREEN_MOVE);
    }
    
    validState = false;
    
    // allocate combat matrices
    int num_soldiers = red_soldiers.size() * green_soldiers.size();
    red_shot_matrix = new float[num_soldiers];
    green_shot_matrix = new float[num_soldiers];
    
    computeShotMatrices();	
    
    // compute moves for finalized soldier positions
    updateSoldierMoves(red_soldiers);
    updateSoldierMoves(green_soldiers);
    
    // show movement options
    displayCurrentBoard(RED, NONE, true, true);
    std::cout << "soldiers initialized : going to state RED_MOVE" << std::endl;
  }  //end if(validState && receivedOK)

  //If we need to initialize or if users want to re-initialize
  else if(initFromFrame("soldiers.army")){
    rename("soldiers.army", "soldiers.init");
    validState = true;
    
    // show soldier locations
    displayCurrentBoard(NONE, NONE, false, true);    
    std::cout << "in state SOLDIER_INIT : soldier positions updated" << std::endl;
  } 
}

void ArmyGame::displayCurrentBoard(Color  moveOptionsDisplayed, Color movePathsDisplayed, 
                                   bool shotLinesDisplayed, bool soldiersDisplayed)
{

  //Blanks output
  blankOverlay();

  //Draws requested movement paths
  if(movePathsDisplayed==GREEN)
    drawMovePaths(green_soldiers);
  else if(movePathsDisplayed==RED)
    drawMovePaths(red_soldiers);

  //Draws move options if specified
  if(moveOptionsDisplayed==GREEN)
    drawMoveOptions(green_soldiers);
  else if(moveOptionsDisplayed==RED)
    drawMoveOptions(red_soldiers);

  //Draws shot lines if requested
	if(shotLinesDisplayed)
    drawShotLines();

  //Draws soldiers if requested otherwise just draws them black
  if(soldiersDisplayed)
  {
	  markSoldiers(red_soldiers, sRGB(255, 0, 0), sRGB(255));
    markSoldiers(green_soldiers, sRGB(0, 255, 0), sRGB(255));
  }
  else
  {
    markSoldiers(red_soldiers, sRGB(255), sRGB(255));
    markSoldiers(green_soldiers, sRGB(255), sRGB(255));
  }

  //Writes out to the temp file
  if(logImages){
    logOverlay();
  }
  writeOverlay(temp_out_file);

  //moves file (so entire file is replaced at once
  rename(temp_out_file, out_file);

}

int ArmyGame::numSoldiersAlive(const std::vector<Soldier>& soldiers){
  int alive = 0;
  for(unsigned int i = 0; i < soldiers.size(); i++){
    if(!soldiers[i].dead){
      alive++;
    }
  }
  return alive;
}

//Returns true if correct soldiers were removed from game, false otherwise
bool ArmyGame::correctRemoval()
{

  for(unsigned int i = 0;  i < red_soldiers.size(); i++){
    if(red_soldiers[i].dead == red_soldiers[i].found){
      std::cout << "Dead army men were not correctly removed." << std::endl;
      return false;
    }
  }
  for(unsigned int i = 0;  i < green_soldiers.size(); i++){
    if(green_soldiers[i].dead == green_soldiers[i].found){
      std::cout << "Dead army men were not correctly removed." << std::endl;
      return false;
    }
  }
  return true;
}

//Removes dead soldiers from the appropriate vectors
void ArmyGame::removeDeadSoldiers()
{
  // remove dead soldiers
  std::cout << "num red soldiers = " << red_soldiers.size() << std::endl;
  std::vector<Soldier>::iterator itr = red_soldiers.begin();
  while(itr != red_soldiers.end()){
    std::cout << "red soldier : " << itr->dead << std::endl;
    if(itr->dead){
      std::cout << "removing" << std::endl;
      itr = red_soldiers.erase(itr);
      std::cout << "done removing" << std::endl;
    }
    else{
      itr++;
    }
  }//endwhile
  
  std::cout << "in state POST_COMBAT : cleaned up red" << std::endl;
  
  std::cout << "num green soldiers = " << green_soldiers.size() << std::endl;
  itr = green_soldiers.begin();
  while(itr != green_soldiers.end()){
    std::cout << "green soldier : " << itr->dead << std::endl;
    if(itr->dead){
      std::cout << "removing" << std::endl;
      itr = green_soldiers.erase(itr);
      std::cout << "done removing" << std::endl;
    }
    else{
      itr++;
    }
  }//endwhile
}

//The running of the game
void ArmyGame::run(){
  // first initialize terrain
  while(state == TERRAIN_INIT){
    // read in initial heightmap
    if(setGridFromImageFile("army_floorplan.ppm")){
      std::cout << "terrain initialized : going to state SOLDIER_INIT" << std::endl;
      // write initial overlay
      displayCurrentBoard(NONE, NONE, false, false);
      changeState( SOLDIER_INIT );
    }
  }
  
  // write out initial overlay
  //writeOverlay(temp_out_file);
  //rename(temp_out_file, out_file);

  gameOver = false;
  validState = false;

  while(!gameOver){

    // check for an OK
    bool receivedOK = false;
    std::ifstream okCheck("army_OK");
    if(okCheck){
      receivedOK = true;
      remove("army_OK");
    }

    if(state == SOLDIER_INIT){ //State1
      soldierInit(receivedOK);
      
    }//end if(state == SOLDIER_INIT)
    else if(state == RED_MOVE || state == GREEN_MOVE){//State2
      printf("state 2\n");
      if(validState && receivedOK)//&& 
	{//State2.2
	  printf("2.2\n");
	  validState = false;

	  if(state == RED_MOVE){
	    updateSoldierMoves(red_soldiers);
	    if(combatShouldHappen){
	      std::cout << "red soldiers moved : going to state RED_PRE_COMBAT" << std::endl;
	      // show movement options for red
	      changeState( PRE_RED_COMBAT );
	      displayCurrentBoard(NONE, NONE, true, true);
	    }
	    else{
	      std::cout << "red soldiers moved : no combat, going to green's move" << std::endl;
	      changeState( GREEN_MOVE );
	      displayCurrentBoard(GREEN, NONE, true, true);
	    }

	  } // end red_move
	  else{ //green move
	    updateSoldierMoves(green_soldiers);
	    if(combatShouldHappen){
	      std::cout << "green soldiers moved : going to state PRE_GREEN_COMBAT" << std::endl;
	      changeState( PRE_GREEN_COMBAT );
	      displayCurrentBoard(NONE, NONE, true, true);
	    }
	    else{
	      std::cout << "green soldiers moved : no combat, going to red's move" << std::endl;
	      changeState( RED_MOVE );
	      displayCurrentBoard(RED, NONE, true, true);
	    }
	    
	  } //end green move
	}// end validState && receivedOK

      //(If not valid state or haven't received ok then need to update)
      else if(file_exist("soldiers.army"))
      {
        printf("2.1\n");   
     
        validState=false;
        printf("valid state false\n");
        validState = updateFromFrame("soldiers.army", (state == RED_MOVE),
				     (state == RED_MOVE) ? red_soldiers : green_soldiers);
        //Supposed to do this if only if there is a valid army configuration
        //if()
        //  {
        std::cout << "*** update call done" << std::endl;
        rename("soldiers.army", (state == RED_MOVE) ? "soldiers.redmove" : "soldiers.greenmove");
	      std::cout << "*** rename call done" << std::endl; 
	      //validState = true;
        printf("valid state true\n");

        combatShouldHappen = computeShotMatrices();

	// show paths and movement options
	if(state == RED_MOVE){
          displayCurrentBoard(RED, RED, true, true);
	}
	else{
	  displayCurrentBoard(GREEN, GREEN, true, true);
	}
	std::cout << "in state " << ((state == RED_MOVE) ? "RED_MOVE" : "GREEN_MOVE") 
		  << " : soldier positions updated" << std::endl;
        
      }//end updating frame
    }
    else if(state == PRE_GREEN_COMBAT||state == PRE_RED_COMBAT){//State 3
      if(receivedOK){
        if(state==PRE_GREEN_COMBAT)
        {
	      std::cout << "in state GREEN_PRE_COMBAT : going to state GREEN_COMBAT" << std::endl;
  	      changeState( GREEN_COMBAT );
        }
        else
        {
	      std::cout << "in state RED_PRE_COMBAT : going to state RED_COMBAT" << std::endl;
  	      changeState( RED_COMBAT );
        }
      }
    }//end precombat
    else if(state == GREEN_COMBAT||state==RED_COMBAT){//State 4
      std::cout << "in state COMBAT : starting combat simulation" << std::endl;

      runCombatRound();

      std::cout << "in state COMBAT : done with combat simulation" << std::endl;

      // check if the game is over
      bool redDefeat = numSoldiersAlive(red_soldiers) == 0;
      bool greenDefeat = numSoldiersAlive(green_soldiers) == 0;
      
      if(redDefeat || greenDefeat){
	// game over
	changeState( END_GAME );
      }
      else{
	displayCurrentBoard(NONE,NONE, false, true);

	std::cout << "in state COMBAT : going to state POST_COMBAT" << std::endl;
	if(state==GREEN_COMBAT){
	  std::cout << "in state POST_GREEN_COMBAT" << std::endl;
	  changeState( POST_GREEN_COMBAT );
	}
	else{
	  std::cout << "in state POST_RED_COMBAT" << std::endl;
	  changeState( POST_RED_COMBAT );
	}
      }
    }//end combat
    else if(state == POST_GREEN_COMBAT || state == POST_RED_COMBAT){//State 5
      if(validState && receivedOK){
	validState = false;
	 
        std::cout << "in state POST_COMBAT : cleaning up dead soldiers" << std::endl;

        removeDeadSoldiers();
	 
	std::cout << "in state POST_COMBAT : cleaned up" << std::endl;

        if (state == POST_GREEN_COMBAT)
        {
  	      changeState(  RED_MOVE );
  	      // show movement options for red
          displayCurrentBoard(RED,NONE, false, true);
        }
        else
        {
          changeState( GREEN_MOVE );
          // show movement options for green
          displayCurrentBoard(GREEN,NONE, false, true);
       
        }
      }//end validState and receivedOK
      
      //Combat is complete, need to update game
      else if(file_exist("soldiers.army")){
	  
	updateFromFrame("soldiers.army", true, red_soldiers);
	updateFromFrame("soldiers.army", false, green_soldiers);
	
	rename("soldiers.army", "soldiers.postcombat");
	
	// check to make sure that all dead army men were removed
	validState = correctRemoval();
	displayCurrentBoard( NONE,NONE, false, true);
      }
    }// end postcombat
    else if(state == END_GAME){
      int redAlive = numSoldiersAlive(red_soldiers);
      int greenAlive = numSoldiersAlive(green_soldiers);

      //Blanks output
      blankOverlay();

      if(redAlive > greenAlive){
	// red wins!
	drawRedWins();
      }
      else if(redAlive < greenAlive){
	// green wins!
	drawGreenWins();
      }
      else{
	// tie!
	drawTie();
      }

      //Writes out to the temp file
      if(logImages){
	logOverlay();
      }
      writeOverlay(temp_out_file);
      
      //moves file (so entire file is replaced at once
      rename(temp_out_file, out_file);

      changeState(BLANK);

    }// end endgame
  }// main game loop (while !gameOver)
}

int main(int argc, char** argv){

  ArmyGame ar;

  for(int argi = 1; argi < argc; argi++){
    if(std::string(argv[argi]) == "--log-images"){
      argi++;
      if(argi < argc){
	std::cout << "SETTING LOG DIRECTORY: " << argv[argi] << std::endl;
	ar.startLoggingImages(argv[argi]);
      }
    }
    else{
      std::cerr << "Unknown argument: " << argv[argi] << std::endl;
    }
  }

  srand(time(0));
  
  ar.run();
  
  return 0;
}

