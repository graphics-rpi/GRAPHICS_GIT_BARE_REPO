/*
 * Layer.cpp
 *
 *  Created on: Jun 21, 2011
 *      Author: phipps
 */

#include "Layer.h"

std::ostream &operator<<(std::ostream &ostr, const Layer l)
{
	ostr << "\nLayer: " << l.ID << " " << l.name << '\n';
	ostr << "------------------------" << std::endl;

	for(unsigned int i = 0; i < l.nodes.size(); ++i)
	{
		ostr << *l.nodes[i] << '\n';
	}

	if(l.nodes.size() == 0)
		ostr << "No nodes in here" << '\n';

	if(l.edgeindex == 0)
		ostr << "No edges in here" << '\n';
	else ostr << "There are: " << l.edgeindex << "edges\n";

	ostr << "------------------------" << '\n';

	return ostr;
}

#include "../../../../../synenv/visualizations/MapView/layer_selector/mapcolors.h"

Layer::Layer(int _ID, std::string _name) //, const Vec3f &color) //float r, float g, float b)
{
	name = _name;
	ID = _ID;
	//mr = r;
	//mg = g;
	//mb = b;
	drawNodes = true;
	drawEdges = true;
	drawCrossEdges = true;
	//layer_color = color;

	// BARB HACK
	//if (global_mapcolors.size() == 0) {
	//  setglobalmapcolors();
	//}
	//std::cout << "NEW LAYER " << name; // << std::endl;
	//layer_color = GLOBAL_getmapcolor(name);
	//std::cout << " " << layer_color; // << std::endl;

	//if (name == "power") {
	// color = Vec3f
	//}



	//mr = r;
	//mg = g;
	//mb = b;
}

Layer::Layer(const Layer &e)
{
	ID = e.ID;
	name = e.name;
	nodes = e.nodes;
	edgeindex = 0;
	mr = e.mr;
	mg = e.mg;
	mb = e.mb;

	drawNodes = e.drawNodes;
	drawEdges = e.drawEdges;
	drawCrossEdges = e.drawCrossEdges;
	//layer_color = e.layer_color;
	//mr = e.mr;
	//mg = e.mg;
	//mb = e.mb;
}


QTEdge* Layer::GetConnection(QTNode *a, QTNode *b){
	edgeshashtype::iterator result = edges.find(std::make_pair(a,b));
	if( result == edges.end() ){
		return NULL;
	}
	return result->second;
}

void Layer::AddConnection(QTNode* a, QTNode* b, QTEdge* edge){
	QTNode* first = a->getID() < b->getID() ? a:b;
	QTNode* second = a->getID() < b->getID() ? b:a;

	edges[std::make_pair(first, second)] = edge;
	first->AddConnection(edge);
}


/*
bool Layer::RemoveNode(QTNode *a)
{

	for(std::vector<QTNode*>::iterator itr = nodes.begin();
			itr != nodes.end(); ++itr)
	{
		if((*itr)->getID() == a->getID())
		{
			(*itr)->RemoveLayer(ID);
			nodes.erase(itr);
			return true;
		}
	}

	return false;
}
*/

QTNode* Layer::GetNodePointer(unsigned int ID)
{
	for(unsigned int i = 0; i < nodes.size(); ++i)
	{
		if(nodes[i]->getID() == ID)
			return nodes[i];
	}

	return NULL;
}

/*
void Layer::HideNodes()
{

	for(unsigned int i = 0; i < nodes.size(); ++i)
	{
		if(nodes[i] == NULL)
			continue;

		nodes[i]->SetVisible(false);
	}
	
	drawNode = false;
}

void Layer::ShowNodes()
{
	
	for(unsigned int i = 0; i < nodes.size(); ++i)
	{
		if(nodes[i] == NULL)
			continue;

		nodes[i]->SetVisible(true);
	}
	
	drawNode = true;
}

void Layer::HideEdges()
{

	for(unsigned int i = 0; i < nodes.size(); ++i)
	{
		if(nodes[i] == NULL)
			continue;

		for(unsigned int k = 0; k < nodes[i]->NumConnections(); ++k)
		{
			nodes[i]->GetConnection(k)->SetVisible(false);
		}
	}
	
}

void Layer::ShowEdges()
{
	
	for(unsigned int i = 0; i < nodes.size(); ++i)
	{
		if(nodes[i] == NULL)
			continue;

		for(unsigned int k = 0; k < nodes[i]->NumConnections(); ++k)
		{
			nodes[i]->GetConnection(k)->SetVisible(true);
		}
	}
	
}
*/


void Layer::DrawEdges() {
  if (!drawNodes) return;
  if (!drawEdges) return;
  //int count = 0;
  for (edgeshashtype::iterator itr = edges.begin();
       itr != edges.end(); itr++) {

    //    const std::string &type = itr->second->GetData()->getType();
    
    //int loc = type.find('_');

    //std::cout << "loc " << loc << " type = '" << type << "'" << std::endl;
    //if (loc == std::string::npos) {
    //std::cout << "SKIPPING DRAW EDGES" << std::endl;
    //continue;
    //}
    //assert (loc > 0 && loc < (int)type.size());
    //std::cout << "GOOD DRAW EDGES" << std::endl;

    //std::string first = type.substr(0,loc);
    //std::string second = type.substr(loc+1,type.size()-loc);
    
    //    assert (first.find('_') == std::string::npos);
    //assert (second.find('_') == std::string::npos);
    //assert (first.size() > 0);
    //assert (second.size() > 0);
    
    //    assert (first == name);

    QTEdge *e = itr->second;
    if (e->isVisible()) {
      itr->second->Draw();
    }

    //if (e->GetStart()->getType() == e->GetEnd()->getType() ||
    //  drawCrossEdges) {
    //itr->second->Draw();
    //}
  }
}


/*
void Layer::DrawLittleNodes() {
  if (!drawNodes) return;
  for(unsigned int i = 0; i < nodes.size(); ++i) {
    nodes[i]->DrawIfLittle();
  }
}

void Layer::DrawBigNodes() {
  if (!drawNodes) return;
  for(unsigned int i = 0; i < nodes.size(); ++i) {
    nodes[i]->DrawIfBig();
  }
}  
*/
