#ifndef _NODE_H_
#define _NODE_H_

#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <set>
#include <map>

#include "argparser.h"
#include "../paint/button.h"
#include "object_group.h"

class Graph;
extern ArgParser *args;

//=========================================================


class GraphNode : public Button {
 public:

  GraphNode(Graph *graph, const std::string &n, int lev); //=5);
  ~GraphNode();

  void setup_texture();

  void AddConnection(GraphNode *n) { 
    assert (n != NULL);
    if (!IsConnected(n)) 
      connections.push_back(n);
    if (!n->IsConnected(this))
      n->connections.push_back(this);
  }
  void RemoveConnection(GraphNode *n) {
    exit(0);
    assert (n != NULL);
    assert (IsConnected(n));
    for (std::vector<GraphNode*>::iterator itr = connections.begin(); itr != connections.end(); itr++) {
      if ((*itr) == n) { itr = connections.erase(itr); return; }
    }
    assert (0);
  }
  void RemoveAllConnections() {
    for (std::vector<GraphNode*>::iterator itr = connections.begin(); itr != connections.end(); itr++) {
      GraphNode *n = *itr;
      n->RemoveConnection(this);
    }
    connections.clear();
  }
  bool IsConnected(GraphNode *n) const {
    for (unsigned int i = 0; i < connections.size(); i++) {
      if (connections[i] == n) return true;
    }
    return false;
  }
  double GetConnectionDistance(GraphNode *n);
  Pt getPosition() const { return getCentroid(); }
  void setPosition(double x, double y) { 
    MoveNoDamping(Pt(x,y));
  }
  void setDimensions(double w, double h) {
    Pt centroid = getCentroid();
    Button::setDimensions(w,h);
    setPosition(centroid.x,centroid.y);
  }

  bool isVisible() const { return (mode != 0); }
  void setVisible();
  void setInvisible();

  void setRandomPosition(Pt parent_pt);

  Vec3f getNodeColor() const;
  //  double getR() const { return r; }
  //double getG() const { return g; }
  //double getB() const { return b; }

  //  void setColor(double _r, double _g, double _b) { r = _r; g = _g; b = _b; }

  //  virtual bool isGroup() const { return true; }
  virtual bool isGraphTerminal() const { return false; }
  const std::string& getName() const { return name; }
  int getLevel() const { return my_level; } 

  void setLevel(int level) { my_level = level; setColor(getNodeColor());  } 

  const std::vector<GraphNode*>& getConnections() const { return connections; }

  int getMode() const { return mode; }
  virtual void setMode(int i) { assert(0); }

private:
protected:
  
  //GraphNode() { assert(0); exit(0); }
  // GraphNode(const GraphNode &) { assert(0); exit(0); }
  GraphNode& operator=(const GraphNode &) { assert(0); exit(0); }

  Graph *graph;

  std::vector<GraphNode*> connections;
  //double r,g,b;
  std::string name;
  int my_level;  
  int mode;  // 0 = invisible, 1 = text, 2 = small, 3 = large
};

//=========================================================

class GraphTerminal : public GraphNode {

 public:

  GraphTerminal(Graph *graph, const std::string &n);
  
  void setClassification(const std::vector<std::pair<int,std::string> > &c) {
    classification = c; }

  int numLevelsOfClassification() const { return classification.size(); }
  const std::pair<int,std::string>& getClassificationLevel(int i) const { 
    assert (i <= numLevelsOfClassification());
    return classification[i];
  }

  static bool AnimateSize(GraphNode *n);
  bool Expand(int level, bool direct_click, const Pt &parent_position);
  bool Collapse(int level);

  bool isGraphTerminal() const { return true; }

  void addTag(const std::string &tag) { tags.insert(tag); }
  bool hasTag(const std::string &tag) { return tags.find(tag) != tags.end(); }

  const std::set<std::string>& getTags() { return tags; }

  void setSize(double s); // used only by tiled display
  double getSize() const { return my_current_size; } // used only by tiled display

  void setMode(int i);


 private:
  
  GraphTerminal();
  GraphTerminal(const GraphTerminal &);
  std::vector<std::pair<int,std::string> > classification;
  std::set<std::string> tags;
  double my_current_size;

};


// ==============================================================================================






#endif

