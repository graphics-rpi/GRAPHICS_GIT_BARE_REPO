#include <fstream>
#include <sstream>
#include "../../common/MersenneTwister.h"
#include "node.h"
#include <utility>
#include <algorithm>
#include "argparser.h"
#include "../../common/directory_locking.h"
#include "graph.h"

#include <fstream>
#include <string>
#include <map>
#include <iomanip>



 



// =========================================================================


// =========================================================================
// =========================================================================

GraphNode::GraphNode(Graph *graph_, const std::string &n, int lev) : 
  ClickableObject(  Pt(0,0),
                    ANIMAL_BUTTON_WIDTH, ANIMAL_BUTTON_HEIGHT,
                    Vec3f(0,0,0)),
  Button(  Pt(0,0),
           ANIMAL_BUTTON_WIDTH, ANIMAL_BUTTON_HEIGHT,Vec3f(0,0,0),n), 
  name(n), 
  my_level(lev),
  mode(0)
{
  graph = graph_;
  assert (graph != NULL);
  graph->addNode(this);
  int level = getLevel();
  assert (level <= 5);

  Pt position;
  
  position.x += args->tiled_display.full_display_width*0.3*(args->mtrand.rand()-0.5);
  position.y += args->tiled_display.full_display_height*0.3*(args->mtrand.rand()-0.5);

  Move(position); 
  setColor(getNodeColor()); ////  setColor(r,g,b);
  
  setTransparency(0.7);
  setBoldText(true);
  //  buttons_sorted.push_back(this); 
  
  // To start, it doesn't have a parent object group
  parentObjectGroup = NULL;
  // Inherited from ClickableObject
  is_object_group = false;
}

Vec3f GraphNode::getNodeColor() const {

  if (my_level == 0) {
    return Vec3f(1,0,0);
  } else if (my_level == 1) {
    return Vec3f(1,1,0);
  } else if (my_level == 2) {
    return Vec3f(0,1,0);
  } else if (my_level == 3) {
    return Vec3f(0,0,1);
  } else if (my_level == 4) {
    return Vec3f(1,0,1);
  } else if (my_level == 5) {
    return Vec3f(1,1,1);
  } else {
    assert(0);
  }
}

void GraphNode::setRandomPosition(Pt parent_pt) {
  Pt position = parent_pt;
  position.x += args->tiled_display.full_display_width*0.3*(args->mtrand.rand()-0.5);
  position.y += args->tiled_display.full_display_height*0.3*(args->mtrand.rand()-0.5);
  MoveNoDamping(position);
}

void GraphNode::setup_texture() {
  if (args->animal_example) {
    specify_texture("","../source/applications/graph_interaction/cropped_images/"+getName()+".jpg");
  } else {
    specify_texture("",args->image_collection_directory+"/"+getName());
  }
  // note: texture initially disabled
}



GraphNode::~GraphNode() {
   std::cout << "only call at exit" << std::endl; exit(0); 
   /*
  for (std::vector<Button*>::iterator itr = GLOBAL_buttons.begin();
       itr != GLOBAL_buttons.end(); itr++) {
    if (*itr == this) { //button) {
      GLOBAL_buttons.erase(itr);
      delete button;
      return;
    }
  }
   */
}


// =========================================================================
// =========================================================================

GraphTerminal::GraphTerminal(Graph *graph_, const std::string &n) : GraphNode(graph_,n,5), my_current_size(1) {
  setup_texture(); 
}

void GraphTerminal::setSize(double s) {
  my_current_size = s;
  if (mode == 0) {
  } else if (mode == 1) {
    setDimensions(ANIMAL_BUTTON_WIDTH,ANIMAL_BUTTON_HEIGHT);
  } else {
    double w = ANIMAL_BUTTON_WIDTH;
    double h = ANIMAL_BUTTON_WIDTH;
    assert (my_current_size > 0);
    // ANIMATED SIZING
    if (mode != 2) {
      w *= my_current_size;
      h *= my_current_size;
    }
    assert (w > 0 && h > 0);
    // ASPECT RATIO
    double w_=1;
    double h_=1;
    if (getImageLarge() != NULL) {
      w_ = getImageLarge()->getCols();
      h_ = getImageLarge()->getRows();
    }
    assert (w_ > 0 && h_ > 0);
    if (w_ > h_) {
      h *= h_/double(w_);
    } else if (h_ > w_) {
      w *= w_/double(h_);
    }
    assert (w > 0 && h > 0);
    setDimensions(w,h);
  }
}


void GraphNode::setVisible() { 
  if (mode == 0) { graph->MakeNodeVisible(this); mode = 1; } 
}
void GraphNode::setInvisible() {
  if (mode != 0) { graph->MakeNodeInvisible(this); mode = 0; }
}



void GraphTerminal::setMode(int i) {
  if (i == mode) return;
  int old_mode = mode;

  if (old_mode == 0) {
    graph->MakeNodeVisible(this);    
  } 
  if (i == 0) {
    graph->MakeNodeInvisible(this);    
  }

  mode = i;
  if (i == 0) {
    assert (old_mode == 1);
    assert (!is_texture_enabled());
  } else if (i == 1) {
    my_current_size = 1;
    if (old_mode == 0) {
      setSize(my_current_size);
    } else {
      assert (old_mode == 2);
      assert (is_texture_enabled());
      disable_texture();
      assert (!is_texture_enabled());
      setSize(my_current_size);
    }
  } else if (i == 2) {
    if (old_mode == 1) {
      assert (!is_texture_enabled());
      enable_texture();
      assert (is_texture_enabled());
      setSize(my_current_size);
    } else {
      assert (old_mode == 3);
      assert (is_texture_enabled());
    }
  } else {
    assert (i == 3); 
    assert (old_mode == 2);
    assert (is_texture_enabled());
  }
}

// =========================================================================
// =========================================================================


double distance(double x1, double x2, double y1, double y2) {
  return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

double distance(GraphNode *n1, GraphNode *n2) {
  Pt a = n1->getPosition();
  Pt b = n2->getPosition();
  return distance(a.x,b.x,a.y,b.y); 
}

void optimal_position(GraphNode *n1, GraphNode *n2, double &x, double &y, double optimal_distance) {
  Pt a = n1->getPosition();
  Pt b = n2->getPosition();
  double dx = a.x-b.x;
  double dy = a.y-b.y;
  double dist = distance(a.x,b.x,a.y,b.y); 
  if (dist > 0.0001*optimal_distance) {
    dx /= dist;
    dy /= dist;
    x = b.x + dx*optimal_distance;
    y = b.y + dy*optimal_distance; 
    x = 0.5 * x + 0.5 * a.x;
    y = 0.5 * y + 0.5 * a.y;
  } else {
    //std::cout << "STUCK TOGETHER " << n1->getName() << " " << n2->getName() << std::endl;
    //std::cout << "BEFORE " << a << " " << b << std::endl;
    x = a.x + 10*(args->mtrand.rand() - 0.5);
    y = a.y + 10*(args->mtrand.rand() - 0.5);
    //std::cout << "after " << x << " " << y << std::endl;
  }
}

double GraphNode::GetConnectionDistance(GraphNode *n) {
  if (n->isGraphTerminal()) {
    return IDEAL_DISTANCE*2+n->getHeight();
  } else if (n->isGraphTerminal()) {
    return IDEAL_DISTANCE*2+n->getHeight();
  }
  double d1 = IDEAL_DISTANCE*(10-this->getLevel());
  double d2 = IDEAL_DISTANCE*(10-n->getLevel());
  double d = 0.5*(d1+d2);
  return d;
}

// =========================================================================
// =========================================================================




//TODO add to drawnnode
bool GraphTerminal::AnimateSize(GraphNode *n) {
  assert (n != NULL);
  assert (n->isGraphTerminal()); 
  GraphTerminal *a = (GraphTerminal*)n;
  assert (a != NULL);
  Button *b = n; 
  assert (b != NULL);
  assert (a->mode != 0);
  if (a->mode == 1) {
    return false; 
  }
  if (a->mode == 2) {
    if (a->my_current_size < 1.00001) {
      // no animation necessary
      return false;
    } else {
      // decrease size
      a->my_current_size = std::max(a->my_current_size * 0.95, 1.0);
      a->setSize(a->my_current_size);
      //n->setDimensions(ANIMAL_BUTTON_WIDTH*a->my_current_size,ANIMAL_BUTTON_WIDTH*a->my_current_size);
      return true;
    }
  } else {
    assert (a->mode == 3);
    if (a->my_current_size > MAX_ANIMAL_SIZE - 0.0001) {
      // no animation necessary
      return false;
    } else {
      // increase size
      a->my_current_size = std::min(a->my_current_size * 1.05, double(MAX_ANIMAL_SIZE));
      a->setSize(a->my_current_size);
      //      n->setDimensions(ANIMAL_BUTTON_WIDTH*a->my_current_size,ANIMAL_BUTTON_WIDTH*a->my_current_size);
      return true;
    }
  }
}


bool GraphTerminal::Expand(int level, bool direct_click, const Pt &parent_position) {
  if (getMode() == 0) {
    setMode(1);
    setMode(2);
    //    setMode(3);
    //    getMode < 3) {
    //setMode(3);
    return true;
  }
  return false;

  /*
  level+=1;
  assert (level >= 0 && level <= 3);
  bool answer;
  if (getMode() == level) answer = false;
  else if (getMode()+1 > level) answer = false; 
  else {
    assert (getMode()+1 == level);
    if (direct_click || level != 3) {
      if (getMode() == 0) {
	setRandomPosition(parent_position);
      }
      setMode(level);
      assert (isVisible());
      answer = true;
    } else {
      answer = false; 
    }
  }
  assert (isVisible());
  return answer;
  */
}


bool GraphTerminal::Collapse(int level) {

  int mode = this->getMode();
  assert (level+1 >= mode);

  if (mode == 0) 
    return false; 
  
  if (mode == 1) {
    if (level == 0) {
      this->setMode(0);
      return true;
    }
  } else if (mode == 2) {
    if (level == 1) {
      this->setMode(1);
      return true;
    }
  } else {
    assert (mode == 3);
    if (level == 2) {
      this->setMode(2);
      return true;
    }
  }
  return false;
}

