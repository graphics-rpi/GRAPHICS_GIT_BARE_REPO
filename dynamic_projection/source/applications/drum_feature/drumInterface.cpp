#include "drumInterface.h"
#include <math.h>

#define PI 3.14159265

void drumInterface::assignPentVerts(){
	//radius
	int r = height/2;
	//center
	int cx = center.x;
	int cy = center.y;
	
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
		labels.push_back(myLabel(labelPts[i], Vec3f(1.0,1.0,1.0), vertNames[i], r/2.5, r/2.5, true));
	}
	
}

bool drumInterface::isLegalMove(Pt m){
	float totalAngle = 0;
	Pt v1, v2;
	float v1length, v2length;
	float dotV1V2;
	
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
}

void drumInterface::updateParameterVec(){

	float length;
	float floatvol; //float version for calculations
	int vol;
	Pt v;
	float r = height / 2;
	
	//draw inner lines from verts toward button (also update volumes)
	for(unsigned int i=0; i<verts.size(); i++){
		v.x = verts[i].x - button->getCentroid().x;
		v.y = verts[i].y - button->getCentroid().y;
		length = sqrt(v.x*v.x + v.y*v.y);

		//update the volumes vector
		floatvol = (length/(1.1*r));
		if(floatvol < 1.0){
			floatvol = (1.0 - floatvol);  //inverse
			floatvol = (((float)volRange.y-(float)volRange.x) * floatvol);
			vol = (int)(floatvol + (float)volRange.x);
			volumes[i] = vol;
		}else{
			volumes[i] = (int)volRange.x;
		}
	}
}

void drumInterface::drawBorder(){

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
		for(int i=0; i<verts.size(); i++){
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
	
	//draw inner lines from verts toward button (also update volumes)
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
		//update the volumes vector
		//floatvol = (length/(1.1*r));
		//if(floatvol < 1.0){
		//	floatvol = (1.0 - floatvol);  //inverse
		//	floatvol = (((float)volRange.y-(float)volRange.x) * floatvol);
		//	vol = (int)(floatvol + (float)volRange.x);
		//	volumes[i] = vol;
		//}else{
		//	volumes[i] = (int)volRange.x;
		//}
	}
	
	//draw dots at corners to cover ugly spots
	glPointSize(3.0);
	glBegin(GL_POINTS);
		for(int i=0; i<verts.size(); i++){
			glColor3f(colors[i].x(),colors[i].y(),colors[i].z());
			glVertex2f(verts[i].x, verts[i].y);
		}
	glEnd();
	
	//draw labels for each vertex
	for(int i=0; i<labels.size(); i++){
		labels[i].render(fdh);
	}
	
}
