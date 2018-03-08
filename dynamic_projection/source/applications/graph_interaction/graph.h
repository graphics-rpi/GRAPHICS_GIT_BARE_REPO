#include <map>
#include <string>
#include <vector>
#include <iostream>

#include "node.h"


/*
#if 1
// ON 2K PROJECTOR
#define IDEAL_DISTANCE 25
#define ANIMAL_BUTTON_WIDTH 100
#define ANIMAL_BUTTON_HEIGHT 40
#else
// ON 4K projector
#define IDEAL_DISTANCE 40
#define ANIMAL_BUTTON_WIDTH 150
#define ANIMAL_BUTTON_HEIGHT 45
#endif
*/


#define IDEAL_DISTANCE 20
#define ANIMAL_BUTTON_WIDTH 70
#define ANIMAL_BUTTON_HEIGHT 30


#define MAX_ANIMAL_SIZE 3


extern ArgParser *args;
extern DirLock global_app_dirlock;

class GraphNode;

class Graph {

public:
  
  // CONSTRUCTORS
  Graph(const std::vector<std::string> &image_collection_classes,
	     const std::string &image_collection_directory);
  Graph(const std::string &animals_file);	     


  // ACTION RESPONSE
  GraphNode* ClickClosestObject(const Pt &center, double radius);
  // these are recursive
  bool Expand(GraphNode *n=NULL);
  bool Collapse(GraphNode *n=NULL);
  void RandomizeGraph();

  // Functions for Grouping and Ungrouping
  bool Group(std::vector<GraphNode*> &buttonsToGroup );
  bool Ungroup(GraphNode* n );

  GraphNode* SearchForGraphNode(const std::string &name);
  void DropPressedBy(int id);

  // VISUALIZATION
  void drawedges() const;
  void drawnodes();
  // returns true if further adjustment is necessary
  bool AdjustGraph();

  // I/O
  void SaveGraphState(const std::string &animal_graph_state_filename);
  void LoadGraphState(const std::string &animal_graph_state_filename);

  GraphNode* RandomVisibleNode();
  
private:

  // don't use these!
  Graph() { assert(0); exit(0); }
  Graph(const Graph &g) { assert(0); exit(0); }
  Graph& operator=(const Graph &g) { assert(0); exit(0); }


  // constructor helpers
  void addNode(GraphNode*n);
  void MakeNodeVisible(GraphNode*n);
  void MakeNodeInvisible(GraphNode*n);
  friend class GraphNode;  // can call addNode & visible/invisible!
  friend class GraphTerminal;  // can call addNode & visible/invisible!
  void initialize();
  void AddChildren(GraphNode* node, const Pt &parent_position);


  // expand/collapse helpers
  int SmallestLevelWithAvailableExpansion(GraphNode *n, bool direct_click);
  int GreatestLevelWithAvailableCollapse(GraphNode *n, bool direct_click);
  bool ExpandLevel(GraphNode *n, int level, bool direct_click, Pt parent_position);
  bool CollapseLevel(GraphNode *n, int level, bool direct_click);



  // REPRESENTATION
  std::vector<GraphNode*> nodes_sorted;
  std::map<std::string,GraphNode*> nodes_map;

  std::vector<std::string> starter_words;

};


// ==============================================
// helper functions

//int HandleGLError(std::string foo);
double distance(double x1, double x2, double y1, double y2);
double distance(GraphNode *n1, GraphNode *n2);
void optimal_position(GraphNode *n1, GraphNode *n2, double &x, double &y, double optimal_distance);
 

