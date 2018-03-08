/*
 * Layer.h
 *
 *  Created on: Jun 20, 2011	
 *      Author: phipps
 */



#ifndef LAYER_H_
#define LAYER_H_


#include "QuadTree/QTNode.h"
#include "Hash.h"
#include <string.h>
#include <string>

class QTNode;

class Layer{

  friend std::ostream &operator<<(std::ostream &ostr, const Layer l);

public:
  Layer();
  Layer(int _ID, std::string _name);
  Layer(const Layer &e);

  void AddNode(QTNode *a) { nodes.push_back(a); } 
  QTEdge* GetConnection(QTNode *a, QTNode *b);
  void AddConnection(QTNode *a, QTNode *b, QTEdge* edge);
  
  bool RemoveConnection(QTNode *a, QTNode *b);
  //  bool RemoveNode(QTNode *a);
  void SetName(std::string _name){name = _name;}
  const std::string& GetName(){return name;}
  QTNode* GetNodePointer(unsigned int ID);
  edgeshashtype* GetEdgeHash(){return &edges;}
  void SetColor(float r, float g, float b);
  
  void ToggleNodes(){ drawNodes = !drawNodes; } 
  void ToggleEdges(){ drawEdges = !drawEdges; }
  void ToggleCrossEdges(){ drawCrossEdges = !drawCrossEdges; }
  bool GetNodeShow(){ return drawNodes; }
  bool GetEdgeShow(){ return drawEdges; }
  bool GetCrossEdgeShow(){ return drawCrossEdges; }
  std::vector<QTNode*> *getNodes(){return &nodes;}
  std::vector<QTNode*>::iterator GetLayerNodeIterator(){return nodes.begin();}
  std::vector<QTNode*>::iterator GetLayerNodeIteratorEnd(){return nodes.end();}
  edgeshashtype::iterator GetLayerEdgeIterator(){return edges.begin();}
  
  void DrawEdges();
  void DrawLittleNodes();
  void DrawBigNodes();
  
  int GetNextIndex(){ return edgeindex++; }
  
private:
  int ID;
  int edgeindex;
  float mr, mg, mb;
  bool drawNodes, drawEdges, drawCrossEdges;
  
  std::string name;
  std::vector <QTNode*> nodes;
  edgeshashtype edges;
};


#endif /* LAYER_H_ */
