/*
 * GraphButton.cpp
 *
 *  Created on: Jul 18, 2011
 *      Author: phipps
 */

#include "GraphButton.h"

GraphButton::GraphButton(){};

GraphButton::GraphButton(const Pt &p, double w, double h, const Vec3f &c, const std::string &t):Button(p, w, h, c, t){

}

using namespace std;
GraphButton::GraphButton(const Pt &p, double w, double h, const std::string &f,
		double u0_, double v0_, double u1_, double v1_):Button(p, w, h, f, u0_, v0_, u1_, v1_) {

}

GraphButton::~GraphButton() {
	// TODO Auto-generated destructor stub
}
