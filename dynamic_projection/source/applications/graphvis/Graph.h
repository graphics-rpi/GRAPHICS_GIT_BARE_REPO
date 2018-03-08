/*
 * graph.h
 *
 *  Created on: Jun 17, 2011
 *      Author: phipps
 */

#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <algorithm>

#include "../paint/ButtonManager.h"
#include "../paint/PathManager.h"

#include "Layer.h"
#include "Hash.h"
#include "Spring.h"
#include "QuadTree/QuadTree.h"
#include "QuadTree/QTNode.h"

#include <glm/glm.hpp>

#ifndef GRAPH_H_
#define GRAPH_H_

class Metadata;

class Graph{

    friend std::ostream& operator<<(std::ostream &ostr, const Graph &g);

    public:

        // CONSTRUCTORS, etc.
        Graph();
        Graph(ArgParser* args);
        Graph(double _x1, double _y1, double _x2, double _y2, ArgParser* args); 
        ~Graph();

        // ACCESSORS
        std::vector<Spring>::iterator FindSpring(int index);
        std::vector<Spring>::iterator FindSpring(QTNode *a, QTNode *b);
        std::vector<Spring>::iterator FindSpring(QTNode *a, Pt b);
        QTEdge *GetConnection(QTNode *a, QTNode *b);
        void SetMode(int mode);
        bool IsConnected(QTNode *a, QTNode *b);
        unsigned int Size(){return nodeindex-1;}
        std::vector<std::pair<QTElement*,double> > FindClosest(double x, double y, double windowsize);
        std::vector<std::pair<QTElement*,double> > FindByID(std::string &message);

        // MODIFIERS
        void AddNode(QTNode* a); //, int layerID);
        void AddConnection(QTNode *a, QTNode *b, QTEdge* edge );
        void AddLayer(std::string name, const Vec3f &color);
        // bool AddToLayer(QTNode *a, int layerID);
        // bool AddToLayer(int _ID, int layerID);
        bool AddSpring(QTNode *a, QTNode *b, double optimal, double w);
        bool AddSpring(QTNode *a, Pt anchor, double optimal, double w);
        bool RemoveSpring(int index);
        bool RemoveFromLayer(QTNode *a, int layerID);
        bool RemoveFromLayer(int _ID, int layerID);
        bool RemoveLayer(int layerID);
        bool RemoveNode(QTNode *a);
        bool RemoveNode(int _ID, int layerID);
        bool RemoveConnection(QTNode *a, QTNode *b);
        bool RemoveConnection(int ID1, int ID2, int layerID);
        bool RemoveConnections(QTNode *a);
        bool RemoveConnections(int ID, int layer);
        int  GetMode(){return viewmode;}
        void Resize(double _x1, double _y1, double _x2, double _y2);


        //Current expand and collapse functionality, collapses 1 ring neighbor
        //void CollapseNode(QTNode *a, int desiredepth = 0, int depth = 0, QTNode *b = NULL);
        //void ExpandNode(QTNode *a, int desiredepth = 0, int depth = 0, QTNode *b = NULL);

        //Simply hides and unhides a given layer not the same nor used in the same way
        //as the above two functions
        //void CollapseLayer(int layerID);
        //void ExpandLayer(int layerID);


        void Draw(glm::mat4 model_view, glm::mat4 projection);
        void ScaleElements(double sf);

        void Clear();
        bool Adjust();
        void Randomize();
        void InitSprings();

        QTNode* GetNodePointer(int ID, int layerID);

        void SetDegreeThreshold(int val){degreethreshold = val;}

        void ToggleSprings(){drawsprings = !drawsprings;}
        void ToggleSpringCalc(){calcsprings = !calcsprings;}

        int GetLayerID(const std::string &name) const;
        Layer* GetLayerByID(int ID) const;

        void resetVisited();

        void writeToDataFile(char *filename);

        void generateRandomData();
        void resetAllNodes();
        void resetAllStatus();
        void setupInitialState();
        void updateGraph();

        QuadTree<QTElement> *GetTree(){return qtree;}

        const std::vector<Layer*> & GetLayers(){return layers;}
        const std::vector<QTNode*> & GetAllNodes(){return all_nodes_for_sorting;}
        const std::vector<QTEdge*> & GetAllEdges(){return all_edges_for_sorting;}

        bool HasLinks(const std::string &layername) const;
        bool HasCrossLinks(const std::string &layername) const;

        unsigned int NumNodes() const { return all_nodes_for_sorting.size(); }

    private:

        int viewmode;
        int nodeindex;
        int edgeindex;
        int layerindex;
        int springindex;
        int degreethreshold;
        bool drawsprings;
        bool calcsprings;

        double x1, y1, x2, y2;
        std::vector<Layer*> layers;

        std::vector<QTNode*> all_nodes_for_sorting;

        std::vector<QTEdge*> all_edges_for_sorting;
        std::vector<Spring> springs;
        edgeshashtype edges;
        QuadTree<QTElement> *qtree;

        GLuint edge_vao[2];
        GLuint edge_vbo[5];

        ButtonManager m_button_manager;
        PathManager m_path_manager;
};


double distance(double x1, double x2, double y1, double y2);
void optimal_position(QTNode *n1, QTNode *n2, double &x, double &y, double optimal_distance);
QTNode* otherend(QTNode *a, QTEdge *edge);

#endif /* GRAPH_H_ */
