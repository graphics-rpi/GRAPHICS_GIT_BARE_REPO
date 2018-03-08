/*
 * graph.cpp
 *
 *  Created on: Jun 17, 2011
 *      Author: phipps
 */

#include "../paint/gl_includes.h"


#include <algorithm>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include "Graph.h"
#include "../../calibration/planar_interpolation_calibration/planar_calibration.h"
#include "../../multi_cursor/interaction.h"
#include <sstream>

using namespace std;

int GLOBAL_REMOVE_LATER = 0;

double distance(double x1, double x2, double y1, double y2)
{
    return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

void optimal_position(QTNode *n1, QTNode *n2, double &x, double &y, double optimal_distance)
{

    //Pt a = n1->getPosition();
    //	Pt b = n2->getPosition();

    Pt centroid_a = n1->getCentroid();
    Pt centroid_b = n2->getCentroid();

    double dx = centroid_a.x-centroid_b.x;
    double dy = centroid_a.y-centroid_b.y;
    double dist = distance(centroid_a.x,centroid_b.x,centroid_a.y,centroid_b.y);

    dx /= dist;
    dy /= dist;
    x = centroid_b.x + dx*optimal_distance;
    y = centroid_b.y + dy*optimal_distance;

    x = 0.5 * x + 0.5 * centroid_a.x;
    y = 0.5 * y + 0.5 * centroid_a.y;

}


Graph::Graph(double _x1, double _y1, double _x2, double _y2, ArgParser* args) : m_button_manager(args), m_path_manager(args){
    x1 = _x1;
    y1 = _y1;
    x2 = _x2;
    y2 = _y2;
    edgeindex = 0;
    nodeindex = 0;
    //edgeshow = true;
    springindex = 0;
    layerindex = 1;
    viewmode = 0;
    degreethreshold = 1;
    drawsprings = false;
    calcsprings = false;
    //hidenodes = false;
    qtree = new QuadTree<QTElement>(Pt(x1, y1), Pt(x2, y2)); //, 50, 10);
}

Graph::~Graph()
{
    /*
       for(unsigned int i = 0; i < nodes.size(); ++i)
       {
       delete nodes[i];
       }
       */
    for(unsigned int i = 0; i < layers.size(); ++i)
    {
        delete layers[i];
    }


    /*
       for(unsigned int i = 0; i < all_edgelist.size(); ++i)
       {
       delete edgelist[i];
       }
       */

    delete qtree;

}

void Graph::AddNode(QTNode *a) {
    assert (a->getBox().Overlap(qtree->GetBounds()));
    qtree->Add(a);

    // Insert to button manager for drawing 
    m_button_manager.AddButton( a );

    all_nodes_for_sorting.push_back(a);
}


void Graph::AddConnection(QTNode* a, QTNode* b, QTEdge* edge ){
    //std::cout << "ADDCONNECTION " << std::endl; 

    QTNode *first = a->getID() < b->getID() ? a:b;
    QTNode *second = a->getID() < b->getID() ? b:a;

    edges[std::make_pair(first, second)] = edge;
    m_path_manager.AddPath(edge);

    all_edges_for_sorting.push_back(edge);
    qtree->Add(edge);
}
/*
   QTEdge* Graph::AddConnection(QTNode *a, QTNode *b, Metadata d, int layerID)
   {
   assert (layerID >= 0 && layerID < (int)layers.size());
   QTEdge *temp = layers[layerID]->AddConnection(a, b, d);

   if(temp != NULL)
   {
   QTNode *first = NULL;
   QTNode *second = NULL;

   first = a->getID() < b->getID() ? a:b;
   second = a->getID() < b->getID() ? b:a;

   edges[std::make_pair(first, second)] = temp;
   edgelist.push_back(temp);
   qtree->Add(temp);
   }

   return temp;

   }

   QTEdge* Graph::AddConnection(int ID1, int ID2, Metadata d, int layerID)
   {
   return AddConnection(GetNodePointer(ID1, layerID), GetNodePointer(ID2, layerID), d, layerID);
   }
   */
void Graph::AddLayer(std::string name, const Vec3f &/*color*/)
{
    std::cout << "add layer " << name << std::endl;
    Layer *temp = new Layer(layerindex, name); //,color);
    layers.push_back(temp);
    ++layerindex;
}

/*
   bool Graph::AddToLayer(QTNode *a, int layerID)
   {
   if(a == NULL || layerID > layerindex)
   return false;

   assert (layerID >= 0 && layerID < (int)layers.size());
   layers[layerID]->AddNode(a);

   return true;
   }

   bool Graph::AddToLayer(int _ID, int layerID)
   {

   return AddToLayer(GetNodePointer(_ID, layerID), layerID);

   }
   */

bool Graph::AddSpring(QTNode *a, QTNode *b, double optimal, double w)
{

    springs.push_back(Spring(a, b, optimal, w, springindex++));
    return true;
}

bool Graph::AddSpring(QTNode *a, Pt anchor, double optimal, double w)
{

    springs.push_back(Spring(a, anchor, optimal, w, springindex++));
    return true;
}

bool Graph::RemoveSpring(int index)
{
    std::vector<Spring>::iterator it = FindSpring(index);

    if(it != springs.end())
    {
        springs.erase(it);
        return true;
    }

    return false;
}

std::vector<Spring>::iterator Graph::FindSpring(int index)
{
    for(std::vector<Spring>::iterator it = springs.begin(); it != springs.end(); ++it)
    {
        if(it->GetIndex() == index)
        {
            return it;
        }
    }

    return springs.end();
}

std::vector<Spring>::iterator Graph::FindSpring(QTNode *a, QTNode *b)
{

    for(std::vector<Spring>::iterator it = springs.begin(); it != springs.end(); ++it)
    {
        if( (it->GetStart()->getID() == a->getID()) && (it->GetEnd()->getID() == b->getID()) )
            return it;
    }

    return springs.end();
}

std::vector<Spring>::iterator Graph::FindSpring(QTNode *a, Pt b)
{

    for(std::vector<Spring>::iterator it = springs.begin(); it != springs.end(); ++it)
    {
        if( (it->GetStart()->getID() == a->getID())	&& (it->GetAnchor() == b) )
            return it;
    }

    return springs.end();
}


QTEdge *Graph::GetConnection(QTNode *a, QTNode *b)
{
    if(a == NULL || b == NULL)
      return NULL;

    QTNode *first = NULL;
    QTNode *second = NULL;

    //Ensure that the pair is structured so first is smaller than second
    first = a->getID() < b->getID() ? a:b;
    second = a->getID() < b->getID() ? b:a;

    //First step is to find the edge
    edgeshashtype::const_iterator result = edges.find(std::make_pair(first,second));
    if( result == edges.end() )
        return NULL;
    else
        return result->second;
}


bool Graph::RemoveConnection(QTNode *a, QTNode *b)
{
    if(a == NULL || b == NULL)
      return false;

    QTNode *first = NULL;
    QTNode *second = NULL;

    //Ensure that the pair is structured so first is smaller than second
    first = a->getID() < b->getID() ? a:b;
    second = a->getID() < b->getID() ? b:a;

    QTEdge *connection = edges[std::make_pair(first, second)];

    if(connection == NULL)
        return false;

    //Remove the connection from both of the nodes
    first->RemoveConnection(connection);
    second->RemoveConnection(connection);

    //Find the edge in the edgelist, once found remove it from
    //both the hashmap and the vector of edges

    /*

       for(std::vector<QTEdge*>::iterator itr = all_edges_for_sorting.begin();
       itr != all_edges_for_sorting.end(); ++itr)
       {
       if((*itr)->getID() == connection->getID())
       {
       edgelist.erase(itr);
       edges.erase(std::make_pair(first,second));
       delete connection;
       break;
       }
       }

*/

    return true;

}


bool Graph::RemoveConnection(int ID1, int ID2, int layerID)
{

    return RemoveConnection(GetNodePointer(ID1, layerID), GetNodePointer(ID2, layerID));
}

bool Graph::RemoveConnections(QTNode* /*a*/)
{

    assert (0);

    /*
       QTNode *node = a;
       QTNode *tempnode = NULL;
       Edge *connection = NULL;

       if(node == NULL)
       return false;

       for(unsigned int i = 0; i < nodes.size(); ++i)
       {
       tempnode = nodes[i];

       if(tempnode != NULL)
       {
    //Ensure that the pair is made in the form <smaller, larger>
    //And then erase the edge from the edgehash
    if(tempnode->getID() < node->getID())
    {
    connection = edges[std::make_pair(tempnode, node)];
    edges.erase(std::make_pair(tempnode, node));
    }
    else
    {
    connection = edges[std::make_pair(node, tempnode)];
    edges.erase(std::make_pair(node, tempnode));
    }

    if(connection != NULL)
    {
    //Now find it and remove it from the vector list
    //it is important that both a and b on the edge remove the
    //connection from their own lists
    for(std::vector<Edge*>::iterator itr = edgelist.begin();
    itr != edgelist.end(); ++itr)
    {
    if((*itr)->GetID() == connection->GetID())
    {
    edgelist.erase(itr);
    node->RemoveConnection(connection);
    tempnode->RemoveConnection(connection);
    delete connection;
    break;
    }
    }
    }
    }
    }
    */

    return true;
}


bool Graph::RemoveConnections(int id, int layerID)
{
    return RemoveConnections(GetNodePointer(id, layerID));
}

void Graph::Resize(double _x1, double _y1, double _x2, double _y2)
{
    x1 = _x1;
    y1 = _y1;
    x2 = _x2;
    y2 = _y2;
}

void Graph::SetMode(int mode)
{
    viewmode = mode;
    if(viewmode == 1)
        InitSprings();
}


//TODO Needs to be adjusted for spatial data structure

std::vector<std::pair<QTElement*,double> > Graph::FindClosest(double x, double y, double windowsize)
{
    std::cout << "\nQUERY... ";
    BoundingBox2f tmp(Pt(x-windowsize, y-windowsize),
            Pt(x+windowsize, y+windowsize));
    std::vector<std::pair<QTElement*,double> > els = qtree->Query(tmp, true);
    std::cout << "   found " << els.size() << " elements " << std::endl;
    return els;
}


std::vector<std::pair<QTElement*,double> > Graph::FindByID(std::string &message) 
{
    std::vector<std::pair<QTElement*,double> > answer;
    std::cout << "WANT TO CLICK " << message << std::endl;
    if (message.size() < 2) {
        std::cout << "DON'T UNDERSTAND MESSAGE " << message << std::endl;
    }

    // =============================================
    // PARSE THE MESSAGE
    std::string token;
    std::string token2;
    unsigned int id;

    unsigned int i = 0;
    while (i < message.size()) {
        if (isspace(message[i])) { i++; }
        else if (isalpha(message[i])) { token.push_back(tolower(message[i])); i++; }
        else break;
    }
    while (i < message.size()) {
        if (isspace(message[i])) { i++; }
        else if (isdigit(message[i])) { token2.push_back(message[i]); i++; }      
        else break;
    }
    if (token.size() == 0 || token2.size() == 0) return answer;
    id = atoi(token2.c_str());

    std::cout << "token " << token << " id " << id << std::endl;

    // =============================================
    // SEARCHING FOR A NODE
    if (token == "n" || token == "node") {
        bool found = false;
        for(unsigned int i = 0; i < layers.size(); i++){
            std::vector<QTNode*> *nodes = layers[i]->getNodes();
            std::cout << "search for node " << nodes->size() << std::endl;
            for (unsigned int i = 0; i < nodes->size(); i++) {
                QTNode *n = (*nodes)[i];
                if (n->getID() == id) {
                    std::cout << "found node " << id << std::endl;
                    answer.push_back(std::make_pair(n,0));
                    found = true;
                    break;
                }
            }
        }
        if (!found) { std::cout << "ERROR: couldn't find node " << id << std::endl; }
    } 
    // =============================================
    // SEARCHING FOR A ARC/EDGE
    else if (token == "e" || token == "edge" ||
            token == "a" || token == "arc") {	     

        std::cout << "searching for an edge" << all_edges_for_sorting.size() << std::endl;
        bool found = false;
        for (unsigned int i = 0; i < all_edges_for_sorting.size(); i++) {
            QTEdge *e = (all_edges_for_sorting)[i];
            if (e->getID() == id) {
                std::cout << "found edge " << id << std::endl;
                answer.push_back(std::make_pair(e,0));
                found = true;
                break;	
            }
        }
        if (!found) { std::cout << "ERROR: couldn't find edge " << id << std::endl; }
    } 

    else {
        std::cout << "DON'T UNDERSTAND MESSAGE " << message << std::endl;
    }
    return answer;
}




void Graph::Clear(){

    /*
       std::vector<QTNode*>::iterator itr = nodes.begin();

       while(itr != nodes.end()){
       RemoveNode(*itr);
       itr = nodes.begin();
       }
       */

    std::vector<Layer*>::iterator itr2 = layers.begin() + 1;

    while(itr2 != layers.end() && itr2 != layers.begin()){

        delete *itr2;
        layers.erase(itr2);
    }

}

#define IDEAL_DISTANCE 30
bool Graph::Adjust(){

    if(!calcsprings)
        return false;
    /*
       double max_movement = 0;

       std::vector<std::pair<double,double> > new_positions(nodes.size(),std::make_pair(0.0,0.0));
       std::vector<double> weight(nodes.size(),0.0);

       Pt temp, a;
       double w;

       for(unsigned int i = 0; i < springs.size(); ++i)
       {
       if(springs[i].GetStart()->getButton()->isPressed())
       continue;

       if(springs[i].GetStart()->IsCollapsed() >= 0)
       {
    //TODO Fix to go over all layers.
    Pt p = GetNodePointer(springs[i].GetStart()->IsCollapsed(), 0)->getButton()->getCentroid(); //Position();
    weight[springs[i].GetStart()->getID()] = 1;

    new_positions[springs[i].GetStart()->getID()].first = p.x;
    new_positions[springs[i].GetStart()->getID()].second = p.y;
    continue;
    }

    temp = springs[i].GetNewPos();
    w = springs[i].GetW();
    new_positions[springs[i].GetStart()->getID()].first += w*temp.x;
    new_positions[springs[i].GetStart()->getID()].second += w*temp.y;
    weight[springs[i].GetStart()->getID()] += springs[i].GetW();
    }

    for (unsigned int i = 0; i < nodes.size(); i++)
    {
    a = nodes[i]->getButton()->getCentroid();//getPosition();
    new_positions[i].first += a.x;
    new_positions[i].second += a.y;
    weight[i] += 1.0;
    }

    // MAKE SURE EVERYTHING STAYS ON SCREEN
    for (unsigned int i = 0; i < nodes.size(); i++)
    {
    if(nodes[i] == NULL)
    continue;

    assert (weight[i] > 0);
    QTNode *n = nodes[i];
    //Pt a = n->getPosition();
    Pt a = n->getButton()->getCentroid();//getPosition();
    double oldx = a.x;
    double oldy = a.y;
    double b_width = n->getButton()->getWidth();
    double b_height = n->getButton()->getHeight();
    double x = new_positions[i].first /= weight[i];
    double y = new_positions[i].second /= weight[i];

    x = std::min(x2-b_width/2.0,std::max(b_width/2.0,x));
    y = std::min(y1-b_height/2.0,std::max(b_height/2.0,y));
    n->setPosition(x,y);
    max_movement = std::max(max_movement,distance(x,oldx,y,oldy));
    }

    if (max_movement > 0.5) return true;
    */
    return false;

}

void Graph::Randomize() {

    srand ( time(NULL) );

    /*
       for (unsigned int i = 0; i < nodes.size(); i++) {
       if(nodes[i] == NULL)
       continue;

       QTNode *n = nodes[i];

       double x = x2*(((double)(rand() % 100))/100.0);
       double y = y1*(((double)(rand() % 100))/100.0);
       n->setPosition(x,y);
       }
       */
}

void Graph::InitSprings()
{

    springs.clear();


    /*
       for(unsigned int i = 0; i < nodes.size(); ++i)
       {
       nodes[i]->setInitialPt(nodes[i]->getCentroid().x,
       nodes[i]->getCentroid().y);

       springs.push_back(Spring(nodes[i],nodes[i]->getInitialPt(), 420, .5, springindex++, 2));

       for(unsigned int j = 0; j < nodes.size(); ++j)
       {
       if(i != j)
       {
       if(IsConnected(nodes[i], nodes[j]))
       {
       springs.push_back(Spring(nodes[i], nodes[j], 420, .5, springindex++));
       }
       }
       }
       }
       */

}

static bool SortByLastTouched(const QTElement* a, 
        const QTElement* b) {
    return (a->getLastTouched() < b->getLastTouched());
}

void Graph::Draw(glm::mat4 model_view, glm::mat4 projection)
{

    if(viewmode == 1){
        Adjust();
    }



    for(unsigned int i = 0; i < all_edges_for_sorting.size(); ++i) {
        all_edges_for_sorting[i]->Draw();
    }

    m_path_manager.Draw(model_view, projection);

    for(unsigned int i = 0; i < all_nodes_for_sorting.size(); ++i) {
        all_nodes_for_sorting[i]->Draw();
    }
    

    // Call superclass function
    m_button_manager.Draw(model_view, projection);



    /*
       for(unsigned int i = 0; i < layers.size(); ++i) {
       layers[i]->DrawLittleNodes();
       }

       for(unsigned int i = 0; i < layers.size(); ++i) {
       layers[i]->DrawBigNodes();
       }
       */


    /*  std::cout << "nodes num " << nodes.size() << std::endl;
        for(unsigned int i = 0; i < nodes.size(); ++i)
        {
        if(1) //!hidenodes)
        {

    //std::cout << "Draw crap" << std::endl;
    if(nodes[i] != NULL && nodes[i]->IsVisible())// && nodes[i]->NumConnections() >= degreethreshold)
    {
    if(viewmode == 1)
    Adjust();

    nodes[i]->Draw();
    }
    }
    }
    */
    //qtree->DrawTree();
}

void Graph::ScaleElements(double sf)
{
    for(unsigned int i = 0; i < all_nodes_for_sorting.size(); i++){
        //std::vector<QTNode*> *nodes = layers[i]->getNodes();
        //for(unsigned int j = 0; j < nodes->size(); ++j)
        //{
        (all_nodes_for_sorting)[i]->SetScaleFactor(sf);
    }
    //}

    for(unsigned int i = 0; i < all_edges_for_sorting.size(); ++i)
    {
        (all_edges_for_sorting)[i]->SetScaleFactor(sf);
    }
}

/*
   void Graph::SetPolyLine(int val)
   {

   for(unsigned int i = 0; i < edgelist.size(); ++i)
   {
   if(edgelist[i]->GetLineMode() != 2)
   edgelist[i]->SetLineMode(val);
   }

   }
   */

QTNode* Graph::GetNodePointer(int ID, int layerID)
{
    assert (layerID >= 0 && layerID < (int)layers.size());
    return layers[layerID]->GetNodePointer(ID);
}

bool Graph::IsConnected(QTNode *a, QTNode *b)
{

    QTNode *first = a->getID() < b->getID() ? a:b;
    QTNode *second = a->getID() < b->getID() ? b:a;

    if(edges.find(std::make_pair(first,second)) != edges.end())
        return true;
    return false;

}

QTNode* otherend(QTNode *a, QTEdge *edge)
{

    QTEdge *aedge = edge;
    QTNode *aneighbor = NULL;
    unsigned int baseID = a->getID();

    aneighbor = aedge->GetStart()->getID() == baseID ?
        aedge->GetEnd():aedge->GetStart();

    return aneighbor;

}



int Graph::GetLayerID(const std::string &name) const
{
    for(unsigned int i = 0; i < layers.size(); ++i)
    {
        if(layers[i]->GetName() == name)
            return i;
    }
    std::cout << "BAD LAYER NAME " << name << std::endl;
    return -1;
}

Layer* Graph::GetLayerByID(int id) const
{
    return layers[id];
}

std::ostream& operator<<(std::ostream &ostr, const Graph &g){

    for(unsigned int i = 0; i < g.layers.size(); ++i)
    {
        ostr << *g.layers[i];
    }

    return ostr;
}

void Graph::writeToDataFile(char* /*filename*/){

    /*
       std::ofstream ostr(filename);
       std::vector<std::string> types;
       unsigned int num = 0;

       for(unsigned int i = 0; i < layers.size(); i++){

       std::vector<QTNode*> *nodes = layers[i]->getNodes();

       for(unsigned int j = 0; j < nodes->size(); j++){

       Metadata *data = (*nodes)[j]->GetData();

    //if(data->getVal("type")=="Transshipment Point"){
    if(1){

    ostr<<"Printing info for node "<<(*nodes)[j]->getID()<<'\n';

    ostr<<"\tType: "<<data->getType()<<'\n';

    if(std::find(types.begin(),types.end(),data->getType())==types.end())
    types.push_back(data->getType());

    for(unsigned int k = 0; k < data->numFields(); k++){
    ostr<<"\t"<<data->getField(k)<<": "<<data->getVal(k)<<'\n';
    }

    unsigned int num_incoming = 0;
    unsigned int num_outgoing = 0;
    for(unsigned int k = 0; k < (*nodes)[j]->NumConnections();k++){
    if((*nodes)[j]->GetConnection(k)->GetStart()==(*nodes)[j]){
    num_outgoing++;
    //num_incoming++;
    }
    else{
    num_incoming++;
    //num_outgoing++;
    }
    }
    ostr<<"\tNum edges: "<<(*nodes)[j]->NumConnections()<<'\n';
    ostr<<"\tNum incoming edges: "<<num_incoming<<'\n';
    ostr<<"\tNum outgoing edges: "<<num_outgoing<<'\n';

    num++;
    }
    //
    }
    }
    ostr<<"Number of supply points: "<<num<<'\n';
    ostr<<"Number of types: "<<types.size()<<":\n";
    for(unsigned int i = 0; i < types.size(); i++)
    ostr<<types[i]<<" ";
    ostr<<"\n";

*/
}

void Graph::setupInitialState(){
    /*
       resetAllNodes();
    //find the supply points and spread their supplies
    for(unsigned int i = 0; i < layers.size(); i++){

    std::vector<QTNode*> *nodes = layers[i]->getNodes();

    for(unsigned int j = 0; j < nodes->size(); j++){

    QTNode *curNode = (*nodes)[j];

    if(curNode->GetData()->getVal(std::string("type"))=="Supply Point"){

    resetVisited();

    std::string type = curNode->GetData()->getType();
    if(type==std::string("power")){
    curNode->updateStatus(PowerPath,true);
    resetVisited();
    curNode->updateStatus(Power,true);
    }
    else if(type==std::string("water")){
    curNode->updateStatus(WaterPath,true);
    resetVisited();
    curNode->updateStatus(Water,true);
    }
    else if(type==std::string("waste")){
    curNode->updateStatus(WastePath,true);
    resetVisited();
    curNode->updateStatus(Waste,true);
    }
    else if(type==std::string("comm")){
    curNode->updateStatus(CommPath,true);
    resetVisited();
    curNode->updateStatus(Comm,true);
    }
    curNode->updateButtonText();
    }
    }
    }*/
}

void Graph::updateGraph(){
    // 	std::cout<<"Updating graph...\n";
    // 	resetAllStatus();
    // 	for(unsigned int i = 0; i < layers.size(); i++){
    // 	//for(unsigned int i = 0; i < 1; i++){
    //
    // 		std::vector<QTNode*> *nodes = layers[i]->getNodes();
    //
    // 		std::cout<<"Number of nodes in layer "<<i<<": "<<nodes->size()<<'\n';
    // 		for(unsigned int j = 0; j < nodes->size(); j++){
    //
    // 			QTNode *curNode = (*nodes)[j];
    //
    // 			Metadata *data = curNode->GetData();
    // 			//std::cout<<"Node type: "<<curNode->GetData()->getVal(std::string("type"))<<'\n';
    // 			//for(unsigned int k = 0; k < data->numFields(); k++){
    // 				//std::cout<<'\t'<<data->getField(k)<<": "<<data->getVal(k)<<'\n';
    // 			//}
    //
    // 			if(curNode->GetData()->getVal(std::string("type"))==std::string("Supply Point")){
    //
    // 				resetVisited();
    //
    // 				std::string type = curNode->GetData()->getType();
    // 				if(type==std::string("power")){
    // 					curNode->updateStatus(Power,curNode->GetData()->getVal(std::string("damage"))==std::string("0"));
    // 					//curNode->updateStatus(Power,false);//curNode->GetData()->getVal(std::string("damage"))==std::string("0"));
    // 				}
    // 				else if(type==std::string("water")){
    // 					curNode->updateStatus(Water,curNode->GetData()->getVal(std::string("damage"))==std::string("0"));
    // 				}
    // 				else if(type==std::string("waste")){
    // 					curNode->updateStatus(Waste,curNode->GetData()->getVal(std::string("damage"))==std::string("0"));
    // 				}
    // 				else if(type==std::string("comm")){
    // 					curNode->updateStatus(Comm,curNode->GetData()->getVal(std::string("damage"))==std::string("0"));
    // 				}
    // 				curNode->updateButtonText();
    // 			}
    // 		}
    // 	}
}

void Graph::resetAllNodes(){
    // 	for(unsigned int i = 0; i < layers.size(); i++){
    // 		std::vector<QTNode*> *nodes = layers[i]->getNodes();
    // 		for(unsigned int j = 0; j < nodes->size(); j++){
    // 			(*nodes)[j]->reset();
    // 		}
    // 		edgeshashtype *edges = layers[i]->GetEdgeHash();
    // 		for(edgeshashtype::iterator itr = edges->begin(); itr != edges->end(); itr++){
    // 			itr->first.second->GetData()->addData(std::string("status"),std::string("0"));
    // 		}
    // 	}
}

void Graph::generateRandomData(){
    // 	srand(time(NULL));
    // 	char buffer[5];
    // 	for(unsigned int i = 0; i < layers.size(); i++){
    // 		std::vector<QTNode*> *nodes = layers[i]->getNodes();
    // 		for(int j = 0; j < nodes->size(); j++){
    // 			Metadata *data = (*nodes)[j]->GetData();
    // 			//sprintf(buffer,"%d",1 << rand()%4);
    //
    // 			//data->addData(std::string("status"),std::string(buffer));
    // 			//(*nodes)[j]->updateStatus(3,(unsigned char)-1);
    // 			(*nodes)[j]->updateStatus(Comm,true);
    //
    // 			sprintf(buffer,"%d",rand()%2);
    // 			data->addData(std::string("damage"),std::string(buffer));
    //
    // 			(*nodes)[j]->updateButtonText();
    // 		}
    //
    // 		/*for(int i = 0; i < edgelist.size(); i++){
    // 			Metadata *data = edgelist[i]->GetData();
    // 			sprintf(buffer,"%d",rand()%2);
    // 			data->addData(std::string("status"),std::string(buffer));
    // 			sprintf(buffer,"%d",rand()%2);
    // 			data->addData(std::string("damage"),std::string(buffer));
    // 		}*/
    //
    // 	}
}

void Graph::resetVisited(){
    for(unsigned int i = 0; i < layers.size(); i++){
        std::vector<QTNode*> *nodes = layers[i]->getNodes();
        for(unsigned int j = 0; j < nodes->size(); j++){
            (*nodes)[j]->visited = false;
        }
    }
}



bool Graph::HasLinks(const std::string &layername) const {

    Layer* layer = GetLayerByID(GetLayerID(layername));
    assert (layer != NULL);
    //std::cout << "NUM LINKS " << layer->GetEdgeHash()->size() << std::endl;

    if (layer->GetEdgeHash()->size() == 0) {
        //std:: cout << "no links for " << layername << std::endl;
        return false;
    }
    return true;

}

bool Graph::HasCrossLinks(const std::string &layername) const {
    Layer* layer = GetLayerByID(GetLayerID(layername));
    assert (layer != NULL);
    for (edgeshashtype::iterator itr = layer->GetEdgeHash()->begin();
            itr != layer->GetEdgeHash()->end(); itr++) {
        QTEdge *e = itr->second;
        if (e->GetStart()->getType() != e->GetEnd()->getType()) return true;

    }
    return false;
}

void Graph::resetAllStatus(){
    for(unsigned int i = 0; i < layers.size(); i++){
        std::vector<QTNode*> *nodes = layers[i]->getNodes();
        for(unsigned int j = 0; j < nodes->size(); j++){
            (*nodes)[j]->GetData()->addData(std::string("status"),std::string("0"));
        }
        edgeshashtype *edges = layers[i]->GetEdgeHash();
        for(edgeshashtype::iterator itr = edges->begin(); itr != edges->end(); itr++){
            itr->first.second->GetData()->addData(std::string("status"),std::string("0"));
        }
    }
}

