#ifndef GRIDREADER_H
#define GRIDREADER_H

// shift height field to better align with table coords
#define SHIFTING_HACK

const unsigned int FIELD_WIDTH = 512;
const unsigned int FIELD_HEIGHT = 512;
const unsigned int FIELD_SIZE = 512 * 512;


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

//This class is a singleton as we should never have more than one grid.
class GridReader
{
  public:
    static GridReader* Inst(Node* node_ptr);
    bool readFromFile(const char* filename);

  protected:
    GridReader(Node* node_ptr); //Constructor

  private:
    static GridReader* instance;
    Node* grid;
    Node* getCell(int r, int c) const;

};


GridReader* GridReader::instance=NULL;

GridReader* GridReader::Inst(Node* node_ptr)
{
      if(instance==NULL)
        instance=new GridReader(node_ptr);
      return instance;
}

GridReader::GridReader(Node* node_ptr)
{
  grid=node_ptr;
}


Node* GridReader::getCell(int r, int c) const{
  return grid + (FIELD_WIDTH - 1 - c) * FIELD_WIDTH + r;
}

bool GridReader::readFromFile(const char* filename)
{
  std::cout << "reading file " << filename << std::endl;

  Image<sRGB> heightImage(filename);
  if(heightImage.getRows() != (int)FIELD_HEIGHT || heightImage.getCols() != (int)FIELD_WIDTH){
    std::cerr << "Error: heightfield size mismatch." << std::endl;
    return false;
  }

  //int minHeight = 255;

  Image<bool> floorMask(FIELD_WIDTH, FIELD_HEIGHT);
  Image<bool> floorMaskDilated(FIELD_WIDTH, FIELD_HEIGHT);
  Image<bool> floorMaskFinal(FIELD_WIDTH, FIELD_HEIGHT);
  Image<sRGB> floorMaskDebug(FIELD_WIDTH, FIELD_HEIGHT);


  // rotate by 90
  for(unsigned int i = 0, c = FIELD_WIDTH - 1; c >= 0 && c < FIELD_WIDTH; c--){
    for(unsigned int r = 0; r < FIELD_HEIGHT; r++, i++){
      //std::cout << "pixel " << r << " " << c << std::endl;
      grid[i].height = heightImage(r,c).x(); // height stored in red value
    }
  }
  
#ifdef SHIFTING_HACK
  const int shift_cols = -19; //5; //0;//-6;
  const int shift_rows = 18; // 10; //0;//-3;

  int scratch[FIELD_WIDTH * FIELD_HEIGHT]; 

  for(unsigned int i = 0; i < FIELD_WIDTH * FIELD_HEIGHT; i++){
    scratch[i] = grid[i].height;
  }

  for(unsigned int i = 0; i < FIELD_WIDTH * FIELD_HEIGHT; i++){
    int j = ((int)i) + shift_cols + ((int)FIELD_WIDTH) * shift_rows;
    if(j >= 0 && j < FIELD_WIDTH * FIELD_HEIGHT){
      grid[i].height = scratch[j];
    }
    else{
      grid[i].height = 0;
    }
  }
#endif

  int i = 0;
  unsigned int c = FIELD_WIDTH - 1;
  for(; c >= 0 && c <= FIELD_WIDTH; c--){
    for(unsigned int r = 0; r < FIELD_HEIGHT; r++, i++){
      floorMask(r,c) = (grid[i].height > 0);
      floorMaskDilated(r,c) = false;
      floorMaskFinal(r,c) = false;
      floorMaskDebug(r,c) = sRGB(255 * (grid[i].height > 0));
      //minHeight = std::min(grid[i].height, minHeight);
    }
  }

  floorMaskDebug.write("floor_start.ppm");

  // std::cout << "floor height = " << minHeight << std::endl;

  // now fill in the gaps (dilate then erode)
  // generate a disk kernel
  const unsigned int KERNEL_SIZE = 11;
  bool useSquare = false;
  Image<bool> kernel(KERNEL_SIZE, KERNEL_SIZE);
  for(unsigned int r = 0; r < KERNEL_SIZE/2 + 1; r++){
      for(unsigned int c = 0; c < KERNEL_SIZE/2 + 1; c++){
	float dr = (KERNEL_SIZE/2 - r);
	float dc = (KERNEL_SIZE/2 - c);
	bool val = useSquare || (dr*dr + dc*dc <= KERNEL_SIZE*KERNEL_SIZE/4.0);
	kernel(r,c) = val;
	kernel(KERNEL_SIZE - 1 - r,c) = val;
	kernel(r,KERNEL_SIZE - 1 - c) = val;
	kernel(KERNEL_SIZE - 1 - r, KERNEL_SIZE - 1 - c) = val;
      }
  }

  // dilate floorMask
  for(unsigned int r = 0; r < FIELD_HEIGHT; r++){
    for(unsigned int c = 0; c < FIELD_WIDTH; c++){
      int start_i = -std::min(r, KERNEL_SIZE/2);
      int stop_i = std::min((int)FIELD_HEIGHT - r, KERNEL_SIZE/2);
      int start_j = -std::min(c, KERNEL_SIZE/2);
      int stop_j = std::min((int)FIELD_WIDTH - c, KERNEL_SIZE/2);
      bool done = false;
      floorMaskDebug(r,c) = sRGB(0,0,0);
      for(unsigned int i = r + start_i, ki = start_i + KERNEL_SIZE/2; !done && i <= r + stop_i; i++, ki++){
	for(unsigned int j = c + start_j, kj = start_j + KERNEL_SIZE/2; !done && j <= c + stop_j; j++, kj++){
	  if(kernel(ki,kj) && floorMask(i,j)){
	    // debug
	    if(!floorMask(r,c)){
	      floorMaskDebug(r,c) = sRGB(0,255,0);
	    }
	    else{
	      floorMaskDebug(r,c) = sRGB(255);
	    }
	    // include (r,c)
	    floorMaskDilated(r,c) = true;
	    floorMaskFinal(r,c) = true;
	    done = true;
	  }
	}
      }
    }
  }

  floorMaskDebug.write("floor_dilated.ppm");

  // erode dilated floorMask
  for(unsigned int r = 0; r < FIELD_HEIGHT; r++){
    for(unsigned int c = 0; c < FIELD_WIDTH; c++){
      int start_i = -std::min(r, KERNEL_SIZE/2);
      int stop_i = std::min((int)FIELD_HEIGHT - r, KERNEL_SIZE/2);
      int start_j = -std::min(c, KERNEL_SIZE/2);
      int stop_j = std::min((int)FIELD_WIDTH - c, KERNEL_SIZE/2);
      bool done = false;
      floorMaskDebug(r,c) = sRGB(255);
      for(unsigned int i = r + start_i, ki = start_i + KERNEL_SIZE/2; !done && i <= r + stop_i; i++, ki++){
	for(unsigned int j = c + start_j, kj = start_j + KERNEL_SIZE/2; !done && j <= c + stop_j; j++, kj++){
	  if(kernel(ki,kj) && !floorMaskDilated(i,j)){
	    // debug
	    //if(!floorMask(r,c)){
	      floorMaskDebug(r,c) = sRGB(0,0,0);
	      //}
	    // exclude (r,c)
	    floorMaskFinal(r,c) = false;
	    done = true;
	  }
	}
      }
    }
  }

  floorMaskDebug.write("floor_final.ppm");
  std::cout << "done computing floor mask" << std::endl;

  // now assign new height values to the pixels that are active in the final floor mask but not in the original
  for(unsigned int r = 0; r < FIELD_HEIGHT; r++){
    for(unsigned int c = 0; c < FIELD_WIDTH; c++){
      if(floorMaskFinal(r,c) && !floorMask(r,c)){
	std::cout << "starting height search for pixel " << r << " " << c << std::endl;
	// search for the nearest pixel active in floorMask and use its height
	int newHeight = 0;
	int searchDist = 1;
	while(!newHeight && searchDist < 50){
	  int min_i = std::max(0, (int)r - searchDist);
	  int max_i = std::min((int)FIELD_HEIGHT - 1, (int)r + searchDist);
	  int min_j = std::max(0, (int)c - searchDist);
	  int max_j = std::min((int)FIELD_WIDTH - 1, (int)c + searchDist);
	  if(c - searchDist >= 0){
	    // check down column at min_j
	    for(int i = min_i; !newHeight && i <= max_i ; i++){
	      if(floorMask(i, min_j)){
		// found a closest pixel (for now just use it and don't worry about bias)
		newHeight = getCell(i,min_j)->height;
	      }
	    }
	  }
	  if(!newHeight && c + searchDist < FIELD_WIDTH ){
	    // check down column at max_j
	    for(int i = min_i; !newHeight && i <= max_i ; i++){
	      if(floorMask(i, max_j)){
		// found a closest pixel (for now just use it and don't worry about bias)
		newHeight = getCell(i,max_j)->height;
	      }
	    }
	  }
	  if(!newHeight && r - searchDist >= 0){
	    // check along row at min_i
	    for(int j = min_j; !newHeight && j <= max_j ; j++){
	      if(floorMask(min_i, j)){
		// found a closest pixel (for now just use it and don't worry about bias)
		newHeight = getCell(min_i, j)->height;
	      }
	    }
	  }
	  if(!newHeight && r + searchDist < FIELD_HEIGHT){
	    // check along row at min_i
	    for(int j = min_j; !newHeight && j <= max_j ; j++){
	      if(floorMask(max_i, j)){
		// found a closest pixel (for now just use it and don't worry about bias)
		newHeight = getCell(max_i, j)->height;
	      }
	    }
	  }
	  searchDist++;
	}
	
	// assign new height
	std::cout << "assigning new height : " << newHeight << std::endl;
	getCell(r,c)->height = newHeight;
      }
    }
  }

  // solve grid connectivity
  int tl, tr, br, bl;

  tl = 0;
  tr = 1;
  bl = FIELD_WIDTH;
  br = FIELD_WIDTH + 1;

  const double DIFF_THRESH = 20;
  const double ROOT2 = std::sqrt(2.0);
  
  for(unsigned int r = 0; r < FIELD_HEIGHT - 1; r++){
    for(unsigned int c = 0; c < FIELD_WIDTH - 1; c++, tl++, tr++, bl++, br++){
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
  for(unsigned int c = 0; c < FIELD_WIDTH - 1; c++, tl++, tr++){
    if(std::fabs(grid[tl].height - grid[tr].height) < DIFF_THRESH){
      grid[tl].edges[RIGHT] = grid[tr].edges[LEFT] = 1;
    }
  }


  return true;

}


#endif
