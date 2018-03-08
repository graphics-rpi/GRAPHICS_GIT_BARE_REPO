#include "PolyInterface.h"
#include <math.h>

#define PI 3.14159265

void PolyInterface::assignPentVerts(){
	//radius
	int r = height/2;
	//center
	int cx = center.x;
	int cy = center.y;
	verts.clear();
	labels.clear();
	
	//place vertices
	double angleInc = 2*PI/vertNames.size();
	double angle = PI/2;
	for(unsigned int i=0; i<vertNames.size(); i++){
		verts.push_back(Pt((cx+cos(angle)*r),(cy+sin(angle)*r)));
		angle += angleInc;
	}
	
	//assign labels for the vertices
	std::vector<Pt> labelPts;
	int vx, vy;
	for(unsigned int i=0; i<verts.size(); i++){
		vx = verts[i].x - cx;
		vy = verts[i].y - cy;
		labelPts.push_back(Pt((verts[i].x + vx/4),(verts[i].y + vy/4)));
		labels.push_back(FadeLabel(labelPts[i], Vec3f(1.0,1.0,1.0), vertNames[i], 
			r/2.5, r/2.0, true));
	}
	
}

bool PolyInterface::isLegalMove(Pt m){
	float totalAngle = 0;
	Pt v1, v2;
	float v1length, v2length;
	float dotV1V2;

	if(verts.size() > 2){
		for(unsigned int i=0; i<verts.size(); i++){
			v1.x = verts[i].x - m.x;
			v1.y = verts[i].y - m.y;
			
			if((i+1) == verts.size()){
				v2.x = verts[0].x - m.x;
				v2.y = verts[0].y - m.y;
			}else{
				v2.x = verts[i+1].x - m.x;
				v2.y = verts[i+1].y - m.y;
			}
			
			//normalization for v1 and v2
			v1length = sqrt( v1.x*v1.x + v1.y*v1.y );
			v2length = sqrt( v2.x*v2.x + v2.y*v2.y );
			
			//dot product
			dotV1V2 = v1.x*v2.x + v1.y*v2.y;
			
			totalAngle += acos(dotV1V2/(v1length*v2length))*(180/PI);
		}
		
		if(totalAngle < 359){
			return false;
		}else{
			return true;
		}
	//If only two vertices (shape is a line and has no volume)
	}else if(verts.size() == 2){
		v1.x = verts[0].x;
		v1.y = verts[0].y;
		v2.x = verts[1].x;
		v2.y = verts[1].y;

		//distance between the two vertices
		float distance = sqrt(v1.x*v2.x + v1.y*v2.y);
		float virtualDist = distance / 8.0;
		//std::cout << "before" << std::endl;
		//if the line is vertical
		if(abs(v1.x-v2.x) < abs(v1.y-v2.y)){ 	
			//if within horizontal bounds
			if(m.x > (v1.x - virtualDist) &&
				m.x < (v1.x + virtualDist)){
				//if v1 is the lower point
				if(v1.y < v2.y){
					//within vertical bounds
					if(m.y < v2.y && m.y > v1.y){
						return true;
					}
				//if v2 is the lower point
				}else{
					//within vertical bounds
					if(m.y < v1.y && m.y > v2.y){
						return true;
					}
				}
			}
			//if not within boundaries
			return false;

		//if the line is horizontal
		}else{
			return true;
		}
	}
	//shouldn't get here because no polybutton should have
	//less than two verticesf
	return false;
}

void PolyInterface::updateParameterVec(){

	float length;
	float floatvol; //float version for calculations
	int vol;
	Pt v;
	float radius = height / 2;
	
	//draw inner lines from verts toward button (also update paramValues)
	for(unsigned int i=0; i<verts.size(); i++){
		v.x = verts[i].x - button->getCentroid().x;
		v.y = verts[i].y - button->getCentroid().y;
		length = sqrt(v.x*v.x + v.y*v.y);

		//update the paramValues vector
		floatvol = (length/(1.1*radius));
		if(floatvol < 1.0){
			floatvol = (1.0 - floatvol);  //inverse
			floatvol = (((float)paramValueRange.y-(float)paramValueRange.x) * floatvol);
			vol = (int)(floatvol + (float)paramValueRange.x);
			paramValues[i] = vol;
		}else{
			paramValues[i] = (int)paramValueRange.x;
		}
	}
}

void PolyInterface::Resize(double w, double h){
	width = w;
	height = h;
	button->setDimensions(width/11, height/11);

	assignPentVerts();
}

void PolyInterface::Move(Pt &pos){ 
	Pt bPos = button->getCentroid();
	Pt moveVec = Pt(pos.x - center.x, pos.y - center.y);

	center = pos; 
	assignPentVerts();

	bPos = Pt(bPos.x + moveVec.x, bPos.y + moveVec.y);
	//std::cout << "number of verts: " << verts.size() << std::endl;
	button->MoveNoDamping(bPos);

}
void PolyInterface::MoveButton(const Pt &pos){ 
	Pt bPos = button->getCentroid();

	//if only two vertices
	if(verts.size() == 2){
		//std::cout << "only two verts with move" << std::endl;
		//if the line is vertical
		if(abs(verts[0].x-verts[1].x) < abs(verts[0].y-verts[1].y)){ 	
			bPos = Pt(bPos.x, pos.y);
		//if the line is horizontal
		}else{
			bPos = Pt(pos.x, bPos.y);
		}
			
	}else{
		//std::cout << "more than two verts" << std::endl;
		bPos = pos;
	}
	button->MoveNoDamping(bPos);

}

void PolyInterface::drawBorder(){

	//draw background polygon with color
	glColor3f(backgroundColor.x(),backgroundColor.y(),backgroundColor.z());
	//glColor3f(0.15,0.15,0.15);
	glBegin(GL_POLYGON);
	for(unsigned int i=0; i<verts.size(); i++){
			glVertex2f(verts[i].x, verts[i].y);
		}
	glEnd();

	//outer border lines
	glLineWidth(4.0);
	glBegin(GL_LINE_LOOP);
		for(unsigned int i=0; i<verts.size(); i++){
			glColor3f(colors[i].x(),colors[i].y(),colors[i].z());
			glVertex2f(verts[i].x, verts[i].y);
		}
	glEnd();
	
	//draw inner lines connected to button
	float lineWidth;
	float length;
	//float floatvol; //float version for calculations
	//int vol;
	Pt v;
	float r = height / 2;
	
	//draw inner lines from verts toward button (also update paramValues)
	for(unsigned int i=0; i<verts.size(); i++){
		v.x = verts[i].x - button->getCentroid().x;
		v.y = verts[i].y - button->getCentroid().y;
		length = sqrt(v.x*v.x + v.y*v.y);
		lineWidth = ((1.1*r)/length);
		if(lineWidth > 1.0){
			
			if(lineWidth > 3.5){ lineWidth = 3.5; }

			glLineWidth(lineWidth*lineWidth);
			glBegin(GL_LINES);
				glColor3f(colors[i].x(),colors[i].y(),colors[i].z());
				glVertex2f(button->getCentroid().x, button->getCentroid().y);
				glVertex2f(verts[i].x, verts[i].y);
			glEnd();
		}
		updateParameterVec();
		//update the paramValues vector
		//floatvol = (length/(1.1*r));
		//if(floatvol < 1.0){
		//	floatvol = (1.0 - floatvol);  //inverse
		//	floatvol = (((float)paramValueRange.y-(float)paramValueRange.x) * floatvol);
		//	vol = (int)(floatvol + (float)paramValueRange.x);
		//	paramValues[i] = vol;
		//}else{
		//	paramValues[i] = (int)paramValueRange.x;
		//}
	}
	
	//draw dots at corners to cover ugly spots
	glPointSize(3.0);
	glBegin(GL_POINTS);
		for(unsigned int i=0; i<verts.size(); i++){
			glColor3f(colors[i].x(),colors[i].y(),colors[i].z());
			glVertex2f(verts[i].x, verts[i].y);
		}
	glEnd();
	
	//draw labels for each vertex
	for(unsigned int i=0; i<labels.size(); i++){
		labels[i].render(fdh);
	}
	
}
