#include "../paint/gl_includes.h"

#include <fstream>
#include <sstream>

#include "../../common/directory_locking.h"
#include "graph.h"

int HandleGLError(std::string foo);


void Graph::addNode(GraphNode*n) { 
  //  std::cout << "addNode " << n->getName() << std::endl;
  assert (!n->isVisible());
  std::string name = n->getName();
  std::pair<std::map<std::string,GraphNode*>::iterator,bool> tmp 
    = nodes_map.insert(std::make_pair(name,n));
  // make sure it wasn't already there!
  assert (tmp.second == true);
}


GraphNode* Graph::SearchForGraphNode(const std::string &name) {
  std::map<std::string,GraphNode*>::iterator tmp = nodes_map.find(name);
  if (tmp == nodes_map.end()) return NULL;
  return tmp->second;
}


void Graph::MakeNodeVisible(GraphNode*n) { 
  assert (!n->isVisible());
  nodes_sorted.push_back(n);
}


void Graph::MakeNodeInvisible(GraphNode*n) { 
  assert (n->isVisible());
  for (unsigned int i = 0; i < nodes_sorted.size(); i++) {
    if (nodes_sorted[i] == n) {
      nodes_sorted[i] = nodes_sorted.back();
      nodes_sorted.pop_back();
      return;
    }
  }
  assert (0);
}


// This function draws the edges of the graph
void Graph::drawedges() const {
  glLineWidth(5);
  glBegin(GL_LINES);
  for (unsigned int i = 0; i < nodes_sorted.size(); i++)  {
    const GraphNode *n = nodes_sorted[i];
    assert (n->isVisible());
    const Pt &p = n->getPosition();
    /*
    double r = n->getR();
    double g = n->getG();
    double b = n->getB();
    */
    const std::vector<GraphNode*>& conn = n->getConnections();
    for (unsigned int j = 0; j < conn.size(); j++) {
      GraphNode *n2 = conn[j];
      if (!n2->isVisible()) continue;
      // don't double draw edges
      if (button_pointer_sorter(n,n2)) continue;
      const Pt &p2 = n2->getPosition();
      /*
      double r2 = n2->getR();
      double g2 = n2->getG();
      double b2 = n2->getB();
      */
      if (n->IsConnected(n2)) {
        glColor3f(n->getColor().r(),n->getColor().g(),n->getColor().b());
	glVertex2f(p.x,p.y);
        glColor3f(n2->getColor().r(),n2->getColor().g(),n2->getColor().b());
	glVertex2f(p2.x,p2.y);
      } // end of if
    } // end of for
  } // end of for
  glEnd();
}



// This functions draws the nodes of the graph
void Graph::drawnodes() {
  HandleGLError("BEFORE draw_buttons()");

  
  std::sort(nodes_sorted.begin(),nodes_sorted.end(),button_pointer_sorter);
  
  int num_pressed = 0;
  int num_painted = 0;
  for (unsigned int i = 0; i < nodes_sorted.size(); i++) {
    if (nodes_sorted[i]->isVisible() == false) continue;
    nodes_sorted[i]->paint(args->background_color);
    num_painted++;
    if (nodes_sorted[i]->isPressed()) {
      num_pressed++;
    }
  }
  //  std::cout << "num painted " << num_painted << std::endl;

/*
  if (num_pressed > (int)actions.size()) {
    for (unsigned int i = 0; i < GLOBAL_buttons.size(); i++) {
      GLOBAL_buttons[i]->paint();
      if (GLOBAL_buttons[i]->isPressed()) {
      }
    }
  }
*/
//  assert (num_pressed <= (int)actions.size());

  HandleGLError("AFTER draw_buttons()");
} // end of draw_buttons







bool tag_sorter(const std::pair<std::string,int> &a, const std::pair<std::string,int> &b) {
  return b.second < a.second;
}

// =========================================================================

#define NUM_TAGS 10



Graph::Graph(const std::vector<std::string> &image_collection_classes,
	     const std::string &image_collection_directory) {

  assert (image_collection_classes.size() >= 1);


  for (unsigned int which = 0; which < image_collection_classes.size(); which++) {

    std::map<std::string,int> tag_freqs;
    std::vector<GraphTerminal*> nodes;


    // open the image collection file
    std::string image_collection_filename = "../mir_images/annotations/" + image_collection_classes[which] + ".txt";
    std::ifstream istr(image_collection_filename.c_str());
    if (!istr) {
      std::cout << "no image collection with name: " << image_collection_classes[which] << std::endl;
      exit(0);
    }
  
    //std::string word = "class "+image_collection_classes[which];
    std::string word = image_collection_classes[which];

    starter_words.push_back(word);
    
    // read in all images from that class
    int i;
    std::string tag;  
    while (istr >> i) {
      // make the id # a string
      std::stringstream ss;   ss << i;   std::string id = ss.str();
      //std::cout << "image " << i << " im" << id+".jpg" << std::endl;
      // create a node for this image


      std::string name = "im"+id+".jpg";
      GraphNode* a2 = SearchForGraphNode(name);
      if (a2 == NULL) { 
	GraphTerminal *a = new GraphTerminal(this,"im"+id+".jpg"); 
      
	// read in all words associated with that image
	std::string tag_filename = image_collection_directory + "/meta/tags/tags" + id + ".txt";
	std::ifstream tags_istr(tag_filename.c_str());
	if (!tags_istr) {
	  std::cout << "cannot open tag file: " << tag_filename << std::endl;
	  exit(0);
	}
	// store tags in node
	while (tags_istr >> tag) {
	  a->addTag(tag);
	  tag_freqs[tag]++;
	}
	nodes.push_back(a);
	a2 = a;
      }
      assert (a2 != NULL);
    } 

    std::vector<std::pair<std::string,int> > tag_freqs_for_sorting;
    
    std::cout << "making top group " << word << std::endl;
    // create a top level node for the image class
    GraphNode *top_group = SearchForGraphNode(word);
    if (top_group == NULL) {
      top_group = new GraphNode(this,word,0);
    } else {
      std::cout << "SET LEVEL FOR " << word << " to 0" << std::endl;
      top_group->setLevel(0);
    }


    // find the top NUM_TAGS tags for this class
    for (std::map<std::string,int>::iterator itr = tag_freqs.begin();
	 itr != tag_freqs.end(); itr++) {
      tag_freqs_for_sorting.push_back(*itr);
    }
    std::sort(tag_freqs_for_sorting.begin(),tag_freqs_for_sorting.end(),tag_sorter);
    for (unsigned int i = 0; i < tag_freqs_for_sorting.size(); i++) {
      int level = 4;
      if (i < NUM_TAGS) {
	level = 2;
      } else {
	continue;
      }

      //std::cout << tag_freqs_for_sorting[i].first << " " << tag_freqs_for_sorting[i].second << std::endl;
      
      // create a top level node for the image class
      std::string tmp = tag_freqs_for_sorting[i].first;
      
      GraphNode *tmp2 = SearchForGraphNode(tmp);
      if (tmp2 == NULL) {
	tmp2 = new GraphNode(this,tmp,level);
      } else {
	if (tmp2->getLevel() > level) {
	  tmp2->setLevel(level);
	}
      }
      
      //tag_nodes[tmp] = tmp2; 
      top_group->AddConnection(tmp2);
    }
    
    // make groups
    for (unsigned int i = 0; i < nodes.size(); i++) {
      GraphTerminal* a = nodes[i];
      for (std::set<std::string>::iterator itr = a->getTags().begin();
	   itr != a->getTags().end(); itr++) {

	GraphNode *tag = SearchForGraphNode(*itr); //image_collection_classes[0]);
	if (tag != NULL) {
	//std::map<std::string,GraphNode*>::const_iterator itr2 = tag_nodes.find(*itr);
	//if (itr2 == tag_nodes.end()) continue;
	  tag->AddConnection(a);
	}
      }
    }
  }
  



  initialize();
}

// =========================================================================

Graph::Graph(const std::string &animals_file) {
  
  starter_words.push_back("animals");
  
  std::ifstream istr(animals_file.c_str());
  std::cout << "load animal data " << animals_file << std::endl;
  assert (istr);

  std::vector<GraphTerminal*> animals;

  // read in all the animals
  std::string line;
  while (getline(istr,line)) {
    std::stringstream ss(line);
    std::string name;
    std::string level;
    std::vector<std::pair<int,std::string> > levels;
    ss >> name;
    int level_num;
    while (ss >> level_num >> level) { 
      assert (level_num >= 0 && level_num <= 4);
      levels.push_back(std::make_pair(level_num,level)); 
    }
    GraphTerminal *a = new GraphTerminal(this,name);
    a->setClassification(levels);
    animals.push_back(a);
  }

  GraphNode *top_group = 
    new GraphNode(this,std::string("animals"),0);

  // make groups
  for (unsigned int i = 0; i < animals.size(); i++) {
    GraphTerminal* a = animals[i];
    int num_levels = a->numLevelsOfClassification();
    assert (num_levels > 0);
    GraphNode* my_group = top_group;
    const std::pair<int,std::string> &level = a->getClassificationLevel(0);
    assert (level.first == 0);
    assert (level.second == "animals");
    for (int j = 1; j < num_levels; j++) {
      GraphNode *node = this->SearchForGraphNode(a->getClassificationLevel(j).second);
      if (node == NULL) {
	node = new GraphNode(this,a->getClassificationLevel(j).second,a->getClassificationLevel(j).first);
	my_group->AddConnection(node);
      } 
      assert (node != NULL && !node->isGraphTerminal());
      my_group = node;
    }
    my_group->AddConnection(a);
  }
  initialize();
}


// =========================================================================
// =========================================================================


// floodfill a bit
void Graph::AddChildren(GraphNode* node, const Pt &parent_position) {
  assert (node != NULL);
  if (node->isVisible()) return;
  if (node->getLevel() > 4) return;
  node->setVisible();
  node->setRandomPosition(parent_position);
  if (!args->start_complete) {
    if (node->getLevel() >= 2 &&
	node->getName() != "vertebrates" && 
	node->getName() != "mammals") {
      return;
    }
  }
  
  const std::vector<GraphNode*>& conn = node->getConnections();
  for (unsigned int i = 0; i < conn.size(); i++) {
    AddChildren(conn[i],node->getPosition());
  }
}



// Helper function to get the associated node
GraphNode* Graph::ClickClosestObject(const Pt &center, double radius) {

  GraphNode *answer = NULL;
  // Get the node affected by the cursor

  double best_dist = radius;
  for (unsigned int j = 0; j < nodes_sorted.size(); j++) {
    GraphNode *n2 = nodes_sorted[j];
    if (n2 == NULL || !n2->isVisible()) continue;
    double my_dist = n2->DistanceFrom( center );
    if (my_dist < best_dist) {
      answer = n2;
      best_dist = my_dist;
    } 
  } 
  
  return answer;
}



void Graph::initialize() {
  assert (this->nodes_map.size() != 0);
  Pt center(args->tiled_display.full_display_width/2.0,
	    args->tiled_display.full_display_height/2.0);
  for (unsigned int i = 0; i < starter_words.size(); i++) {
    //std::cout << "WORDS " << starter_words[i] << std::endl;
    GraphNode *n = this->SearchForGraphNode(this->starter_words[i]);    
    assert (n != NULL);
    AddChildren(n,center);
  }
}


GraphNode* Graph::RandomVisibleNode() {
  unsigned int num_nodes = nodes_sorted.size();
  assert (num_nodes > 0);

  int r = args->mtrand.randInt(num_nodes-1);
  assert (r >= 0 && r < (int)num_nodes);

  return nodes_sorted[r];
}


// return true if further adjustment necessary
bool Graph::AdjustGraph() {

//  std::cout << "adjust center" << SearchForGraphNode(graph,starter_word)->getPosition() << std::endl;
  

  timeval now;
  gettimeofday(&now,NULL);

  double max_movement = 0;

  unsigned int num_nodes = nodes_sorted.size();

  std::vector<std::pair<double,double> > new_positions(num_nodes,std::make_pair(0.0,0.0));
  std::vector<double> weight(num_nodes,0.0);
  std::vector<bool> modified(num_nodes,false);

  for (unsigned int i = 0; i < num_nodes; i++) {
    GraphNode *n1 = nodes_sorted[i]; 
    assert (n1->isVisible());
    if (n1->isGraphTerminal()) {
      bool sized = GraphTerminal::AnimateSize(n1);
      if (sized) max_movement = 1.0; 
    }

    Pt a = n1->getPosition();
    if (!n1->isPressed()) {
      // compute pull from edges

      /*      const std::vector<GraphNode*>& conn = n->getConnections();
      for (unsigned int j = 0; j < conn.size(); j++) {
	GraphNode *n2 = conn[j];
	if (!n2->isVisible()) continue;
	// don't double draw edges
	if (button_pointer_sorter(n,n2)) continue;
      */
      
      for (unsigned int j = 0; j < num_nodes; j++) {
	if (i == j) continue;
	GraphNode *n2 = nodes_sorted[j]; 
	assert (n2->isVisible());
	
	double dist = distance(n1,n2);
	if (n1->IsConnected(n2)) {
	  double x,y;
	  optimal_position(n1,n2,x,y,n1->GetConnectionDistance(n2));
	  double w = 0.5;
	  new_positions[i].first += w*x;
	  new_positions[i].second += w*y;
	  weight[i]+=w;
	  modified[i] = true;
	} else if (dist < 10*IDEAL_DISTANCE) {
	  double x,y;
	  optimal_position(n1,n2,x,y,10*IDEAL_DISTANCE);
	  double w = 0.2;
	  new_positions[i].first += w*x;
	  new_positions[i].second += w*y;
	  weight[i]+=w;
	  modified[i] = true;
	} else if (dist < 20*IDEAL_DISTANCE) {
	  double x,y;
	  optimal_position(n1,n2,x,y,20*IDEAL_DISTANCE);
	  double w = 0.01;
	  new_positions[i].first += w*x;
	  new_positions[i].second += w*y;
	  weight[i]+=w;
	  modified[i] = true;
	}
      }

      /*
      // even if you have tons of connections, limit the influence
      if (weight[i] > 2.0) {
	weight[i] /= 2.0;
	new_positions[i].first /= 2.0;
	new_positions[i].second /= 2.0;
      }
      */

      double motion_memory = 1- n1->getMotionMemoryFactor(now);
      weight[i] *= motion_memory;
      new_positions[i].first *= motion_memory;
      new_positions[i].second *= motion_memory;

    }
   
    // START WITH CURRENT POSITION
    new_positions[i].first += a.x; 
    new_positions[i].second += a.y;
    weight[i] += 1.0;
    modified[i] = true;    
  }

  // MAKE SURE EVERYTHING STAYS ON SCREEN
  for (unsigned int i = 0; i < num_nodes; i++) {
    GraphNode *n = nodes_sorted[i]; 
    assert (n != NULL);
    if (!n->isVisible()) continue;
    assert (weight[i] > 0);
    assert (modified[i] == true);

    //std::cout << "DATA weight=" << weight[i] << std::endl;
    //std::cout << "DATA newpos=" << new_positions[i].first << " " << new_positions[i].second << std::endl;

    Pt a = n->getPosition();
    double oldx = a.x;
    double oldy = a.y;
    double b_width = n->getWidth();
    double b_height = n->getHeight();
    double x = new_positions[i].first /= weight[i];
    double y = new_positions[i].second /= weight[i];

    //std::cout << "aft normalize=" << x << " " << y << std::endl;

    if (1) { //!args->crop_zoom) {
      x = std::min(args->tiled_display.full_display_width-b_width/2.0,std::max(b_width/2.0,x));
      y = std::min(args->tiled_display.full_display_height-b_height/2.0,std::max(b_height/2.0,y));
    }

    //std::cout << n->getName() << " new pos " << x << " " << y << std::endl;
    n->setPosition(x,y);
    max_movement = std::max(max_movement,distance(x,oldx,y,oldy));
  }

  if (max_movement > 0.00001) return true;
  return false;
}

// =========================================================================
// =========================================================================

void Graph::SaveGraphState(const std::string &animal_graph_state_filename) 
{
  static unsigned int graph_frame_counter = 0;
  graph_frame_counter++;
  
  while (!global_app_dirlock.TryLock()) { usleep(1000); }

  { /* SCOPE FOR ostr */
    std::ofstream ostr(animal_graph_state_filename.c_str());
    assert (ostr);
    ostr << "frame " << graph_frame_counter << "\n";
    unsigned int num_nodes = nodes_sorted.size();
    ostr << "num_nodes " << num_nodes << "\n";
    for (unsigned int i = 0; i < num_nodes; i++) {
      GraphNode *n = nodes_sorted[i];
      assert (n != NULL);
      Pt pt(0,0);
      if (n != NULL) {
	pt = n->getPosition();
      }
      int mode = -1;
      double size = -1;
      if (n->isGraphTerminal()) {
	GraphTerminal *a = (GraphTerminal*) n;
	mode = a->getMode();
	size = a->getSize();
      }
      ostr << "node " 
	   << std::setw(3) << i << " " 
	   << std::setw(10) << std::fixed << std::setprecision(3) << pt.x << " "
	   << std::setw(10) << std::fixed << std::setprecision(3) << pt.y << " "
	   << std::setw(10) << std::fixed << std::setprecision(3) << mode << " "
	   << std::setw(10) << std::fixed << std::setprecision(3) << size << "\n";

    }
  } /* SCOPE FOR ostr */
  global_app_dirlock.Unlock();
}

void Graph::LoadGraphState(const std::string &animal_graph_state_filename) {
  static unsigned int last_graph_frame_counter = 0;
  while (!global_app_dirlock.TryLock()) { usleep(1000); }
  //bool success;
  { /* SCOPE FOR istr */
    std::ifstream istr(animal_graph_state_filename.c_str());
    if (!istr) {
      //  success = false;
    } else {
      std::string token;
      unsigned int this_frame;
      istr >> token >> this_frame;
      if (token != "frame" || this_frame == last_graph_frame_counter) {
	//success = false;
      } else {
	if (this_frame < last_graph_frame_counter) {
	  std::cout << "whoops, must have started with a bad frame " << this_frame << " vs. " << last_graph_frame_counter << std::endl;
	}
	last_graph_frame_counter = this_frame;
	unsigned int num_nodes;
	istr >> token >> num_nodes;
	assert (token == "num_nodes");
	assert (num_nodes == nodes_sorted.size());
	for (unsigned int i = 0; i < num_nodes; i++) {
	  GraphNode *n = nodes_sorted[i];
	  int j;
	  double x,y;
	  int mode;
	  double size;
	  istr >> token >> j >> x >> y >> mode >> size;
	  assert (token == "node");
	  assert ((int)i == j);
	  if (n != NULL) {
	    n->setPosition(x,y);
	    if (n->isGraphTerminal()) {
	      GraphTerminal *a = (GraphTerminal*)n;
	      a->setMode(mode);
	      a->setSize(size);
	    }
	  }
	}
	//success = true;
      }
    } /* SCOPE FOR istr */
  }
  global_app_dirlock.Unlock();
}

// =========================================================================
// =========================================================================

void Graph::RandomizeGraph() {
  unsigned int num_nodes = nodes_sorted.size();
  for (unsigned int i = 0; i < num_nodes; i++) {
    GraphNode *n = nodes_sorted[i];
    if (n == NULL || !n->isVisible()) continue;
    double x = args->tiled_display.full_display_width*args->mtrand.rand();
    double y = args->tiled_display.full_display_height*args->mtrand.rand();
    n->setPosition(x,y);
  }
}



// =========================================================================
// =========================================================================

bool Graph::Expand( GraphNode *n) {
  if (n == NULL) { n = SearchForGraphNode(starter_words[0]); }
  if (n == NULL) return false;
  assert (n->isVisible());
  int level = SmallestLevelWithAvailableExpansion(n,true);
  //std::cout << "expand2 =  " << level << std::endl;
  if (level == -1) return false;
  bool answer = ExpandLevel(n,level,true,n->getPosition());
  if (answer != true) {
    //std::cout << "FAILED IN EXPAND2" << std::endl;
  }
  return true;
}

bool Graph::Collapse( GraphNode *n) {
  if (n == NULL) { n = SearchForGraphNode(starter_words[0]); }
  if (n == NULL) return false;
  assert (n->isVisible());
  int level = GreatestLevelWithAvailableCollapse(n,true);
  //std::cout << "HERE in collapse2   level=" << level << std::endl;
  if (level == -1) return false;
  bool answer = CollapseLevel(n,level,true);
  if (answer != true) {
    //std::cout << "FAILED IN COLLAPSE2" << std::endl;
  }
  return true;
}




bool Graph::ExpandLevel(GraphNode *n, int level, bool direct_click, Pt parent_position) {
  if (n->isGraphTerminal()) {
    return ((GraphTerminal*)n)->Expand(level,direct_click,parent_position);
  } else {
    assert (!n->isGraphTerminal());
    if (level == 0) {
      if (!n->isVisible()) {
	n->setVisible();
	n->setRandomPosition(parent_position);
	return true;
      }
      return false;
    } else {
      bool answer = false;
      int count = 0;
      const std::vector<GraphNode*>& conn = n->getConnections();
      for (unsigned int i = 0; i < conn.size(); i++) {
	if (count > 10) break;
        GraphNode *n2 = conn[i];
	assert (n2 != NULL);
	if (n2->getLevel() <= n->getLevel()) continue;
	if (ExpandLevel(n2,level-1,false,n->getPosition()))
          count++;
	  answer = true;
      }
      return answer;
    }
  }
}


bool Graph::CollapseLevel(GraphNode *n, int level, bool direct_click) {
  if (level == 0) {
    if (n->isPressed()) {
      std::cout << "WARNING: cannot collapse, because someone is holding this element" << std::endl;
      return false;
    }
    if (n->isGraphTerminal()) {
      //assert (n->getMode() == 1);
    }
    n->setInvisible();
    return true;
  } else {
    assert (level > 0);
    if (n->isGraphTerminal()) {
      return ((GraphTerminal*)n)->Collapse(level);
    } else {
      assert (!n->isGraphTerminal());
      bool answer = false;      
      const std::vector<GraphNode*>& conn = n->getConnections();
      for (unsigned int i = 0; i < conn.size(); i++) {
	GraphNode *n2 = conn[i];
	assert (n2 != NULL);
	//if (!n2->isVisible()) continue;
	if (n2->getLevel() <= n->getLevel()) continue;
	bool success = CollapseLevel(n2,level-1,false);
	if (success) answer = true;
      }
      return answer;
    }
  }
}


int Graph::SmallestLevelWithAvailableExpansion(GraphNode *n, bool direct_click) { 
  assert (n->isVisible());
  if (n->isGraphTerminal()) {
    assert (n->getMode() > 0);
    if (n->getMode() < 3) 
      return n->getMode();
    return -1;
  }
  assert (!n->isGraphTerminal());
  int answer = -1;
  const std::vector<GraphNode*>& conn = n->getConnections();
  for (unsigned int i = 0; i < conn.size(); i++) {
    GraphNode *n2 = conn[i];
    assert (n2 != NULL); 
    if (n2->getLevel() <= n->getLevel()) continue;
    if (!n2->isVisible()) return 1;
    int answer2 = SmallestLevelWithAvailableExpansion(n2,false);	
    if (answer2 == -1) continue;
    if (answer == -1 || answer2+1 < answer) {
      answer = answer2+1;
    }
  }
  return answer;
}

 
int Graph::GreatestLevelWithAvailableCollapse(GraphNode *n, bool direct_click) { 
  assert (n->isVisible());
  if (n->isGraphTerminal()) {
    GraphTerminal *a = (GraphTerminal*)n;
    int mode = a->getMode();
    assert (mode >= 1);
    return mode-1;
  } 
  assert (!n->isGraphTerminal());
  int answer = 0;
  const std::vector<GraphNode*>& conn = n->getConnections();
  for (unsigned int i = 0; i < conn.size(); i++) {
    GraphNode *n2 = conn[i];
    assert (n2 != NULL);
    if (!n2->isVisible()) continue;
    if (n2->getLevel() <= n->getLevel()) continue;
    int answer2 = GreatestLevelWithAvailableCollapse(n2,false);	
    if (answer2+1 > answer) {
      answer = answer2+1;
    }
  }
  return answer;
}



// =========================================================================
// =========================================================================


bool Graph::Group( std::vector<GraphNode*> &buttonsToGroup )
{
  return false;
} // end of Group

bool Graph::Ungroup( GraphNode *n)
{
  return false;
} // end of Ungroup


void Graph::DropPressedBy(int id) {

  for (unsigned int i = 0; i < nodes_sorted.size(); i++) {
    GraphNode *n = nodes_sorted[i];
    if (n == NULL || !n->isVisible()) continue;
    if (n->isPressed() && n->PressedBy() == id) {
      // keep the button still for 2 (now 4) seconds
      n->release(4000);
    }
  }
}
