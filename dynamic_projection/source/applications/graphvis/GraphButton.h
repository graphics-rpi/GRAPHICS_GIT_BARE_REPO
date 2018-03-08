/*
 * GraphButton.h
 *
 *  Created on: Jul 18, 2011
 *      Author: phipps
 */

#ifndef GRAPHBUTTON_H_
#define GRAPHBUTTON_H_

class GraphButton {
public:
	GraphButton();
	GraphButton(const Pt &p, double w, double h, const Vec3f &c, const std::string &t="");
	GraphButton(const Pt &p, double w, double h, const std::string &f,
			double u0=0, double v0=0, double u1=1, double v1=1);


	virtual ~GraphButton();
};

#endif /* GRAPHBUTTON_H_ */
