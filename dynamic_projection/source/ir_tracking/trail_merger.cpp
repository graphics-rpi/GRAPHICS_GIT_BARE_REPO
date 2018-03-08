#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <unistd.h>
#include <algorithm>
#include <sys/time.h>
#include "../common/directory_locking.h"


#define IR_STATE_DIRECTORY          "../../state/ir_tracking"
#define FOUND_IR_POINTS_FILENAME           "../../state/ir_tracking/found_ir_points.txt"
DirLock dirlock(IR_STATE_DIRECTORY);

int frame_count = 0;

class Pt {
public:
  Pt(double _x, double _y) : x(_x), y(_y) {}
  double x;
  double y;
};

class Trail {
public:
  Trail() {
    current = 0;
  }
  const Pt& next() { 
    assert (pts.size() > 0);
    assert (current < pts.size());
    current++;
    if (current == pts.size())
      current = 0;
    return pts[current];
  }
  void add(const Pt &p) { pts.push_back(p); }
  std::vector<Pt> pts;
  int current;
};


int main(int argc, char* argv[]) {

  std::vector<Trail> trails;
  
  if (argc == 1) {
    std::cout << "WARNING: no files specified" << std::endl;
  }
  //assert (argc > 1);

  for (int i = 1; i < argc; i++) {
    std::ifstream istr(argv[i]);
    double x,y;
    Trail t;
    while (istr >> x >> y) {
      t.add(Pt(x,y));
    }
    std::cout << "trail has " << t.pts.size() << " points " << std::endl;
    trails.push_back(t);
  }

  while (1) { 
    {

      timeval tval;  
      int foo = gettimeofday(&tval,NULL);
      unsigned int sec = tval.tv_sec;
      unsigned int usec = tval.tv_usec;

#if 1
      while (!dirlock.TryLock()) {
      	std::cout << "miss";
	flush(std::cout);
	//CRAP MISSED THE LOCK" << std::endl;
	usleep(1000);
      }
      
      //std::cout << "GOT THE LOCK" << std::endl;
#else
      dirlock.Lock();
#endif

      std::ofstream ostr(FOUND_IR_POINTS_FILENAME);
      if (!ostr) {
	std::cout << "ERROR OPENING IR POINTS FILE FOR WRITING" << std::endl;
	exit(1);
      }
      ostr << "FRAME " << frame_count << "\n";
      frame_count++;
      ostr << "TIME " << sec << " " << usec << "\n";

      std::random_shuffle(trails.begin(),trails.end());

      std::vector<Pt> FOO;

      for (int i = 0; i < trails.size(); i++) {
	Pt p = trails[i].next();
	if (p.x > 0) {
	  FOO.push_back(p);
	}
      }

      ostr << "NUM_POINTS " << FOO.size() << "\n";

      for (int i = 0; i < FOO.size(); i++) {
	Pt p = FOO[i];
	ostr << "11111 " << p.x << " " << p.y << " 500 3.5 -1.00" << "\n";
      }
      
      dirlock.Unlock();

    }



    // ~ 33 fps
    usleep(30000);
    // ~ 5 fps
    //usleep(200000);
    //std::cout << "out" << std::endl;
	
  }


}
