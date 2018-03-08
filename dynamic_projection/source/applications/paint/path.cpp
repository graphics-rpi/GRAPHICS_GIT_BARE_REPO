
#include "../paint/gl_includes.h"


#include "../../calibration/planar_interpolation_calibration/tracker.h"
#include "path.h"

//#define RESOLUTION 1
#define RESOLUTION 5 
#define TOLERANCE 20 
#define LINE_WIDTH 75 

// New OpenGL 3.2 functionality
void Path::populatePath(std::vector<glm::vec3> & edge_positions, 
        std::vector<glm::vec4> & edge_colors, 
        std::vector<GLfloat> & edge_widths){

    double z = getZOrder();

    Pt first_point = magnified_trail.front();
    Pt last_point = magnified_trail.back(); 

    glm::vec4 glm_color = glm::vec4(color.r(), color.g(), color.b(), 1.0f);

    edge_positions.push_back(glm::vec3(first_point.x, first_point.y, z));
    edge_colors.push_back(glm_color);
    edge_widths.push_back(path_width);
    for( int i = 1; i < magnified_trail.size() - 1; i++ ){
        edge_positions.push_back(glm::vec3(magnified_trail[i].x, magnified_trail[i].y, z));
        edge_colors.push_back(glm_color);
        edge_widths.push_back(path_width);
        edge_positions.push_back(glm::vec3(magnified_trail[i].x, magnified_trail[i].y, z));
        edge_colors.push_back(glm_color);
        edge_widths.push_back(path_width);
    }
    edge_positions.push_back(glm::vec3(last_point.x, last_point.y, z));
    edge_colors.push_back(glm_color);
    edge_widths.push_back(path_width);
}
void Path::populateJoints(std::vector<glm::vec3> & vertex_positions, 
        std::vector<glm::vec4> & vertex_colors, 
        std::vector<GLfloat> & vertex_radii){
    double z = getZOrder();

    Pt first_point = magnified_trail.front();
    Pt last_point = magnified_trail.back(); 

    glm::vec4 glm_color = glm::vec4(color.r(), color.g(), color.b(), 1.0f);

    for( int i = 0; i < magnified_trail.size(); i++ ){
        vertex_positions.push_back(glm::vec3(magnified_trail[i].x, magnified_trail[i].y, z));
        vertex_colors.push_back(glm_color);
        vertex_radii.push_back(path_width);
        vertex_positions.push_back(glm::vec3(magnified_trail[i].x, magnified_trail[i].y, z));
        vertex_colors.push_back(glm_color);
        vertex_radii.push_back(path_width);
    }
}


Pt bspline_pt(const Pt &a, const Pt &b, const Pt &c, const Pt &d, double t) {
  double alpha = (1-t)*(1-t)*(1-t)/6.0;
  double beta = (3*t*t*t - 6*t*t + 4) / 6.0;
  double gamma = (-3*t*t*t + 3*t*t + 3*t + 1) / 6.0;
  double delta = t*t*t/6.0;
  return a*alpha + b*beta + c*gamma + d*delta;  
}

void bspline2(const Pt &a, const Pt &b, const Pt &c, const Pt &d,std::deque<Pt> &points) {
  for (int i = 0; i <= RESOLUTION; i++) {
    double t = i / double(RESOLUTION);    
		points.push_back(bspline_pt(a,b,c,d,t)); 
	}
}

void Path::draw() const {
  glColor3f(color.r(),color.g(),color.b());
  draw_smooth_stroke(trail,path_width);
}

void Path::draw_polygonal_stroke2(const std::list<Pt> &trail) {
  glBegin(GL_LINE_STRIP);
  std::list<Pt>::const_iterator itr = trail.begin();
  assert (trail.size() > 0);
  Pt a = trail.front(); 
  itr++;
  glVertex2f(a.x,a.y);
  while (itr != trail.end()) {
    a = *itr;
    glVertex2f(a.x,a.y);
    itr++;
  }
  glEnd();
}

std::deque<Pt> process_points(std::deque<Pt> trail,double tolerance){
	std::deque<Pt> processed = trail;
	for(std::deque<Pt>::iterator itr = processed.begin(); itr!=processed.end();itr++){
		std::deque<Pt>::iterator forward = itr;
		forward++;
		for(;forward!=processed.end() && DistanceBetweenTwoPoints(*itr,*forward)<tolerance;forward=processed.erase(forward));
	}
	processed.push_back(trail.back());
	return processed;
}

void remove_consecutive_duplicates(std::deque<Pt>& trail){
	for(std::deque<Pt>::iterator itr = trail.begin(); itr != trail.end();itr++){
		std::deque<Pt>::iterator forward = itr;
		forward++;
		for(;forward!=trail.end() && ((itr->x==forward->x) && (itr->y==forward->y));forward=trail.erase(forward));
	}
}

void draw_circle(Pt center, float radius){
	double step = M_PI/20;
	double theta = 0;
	glBegin(GL_POLYGON);
	for(int i = 0; i<=40;i++){
		glVertex2f(center.x+radius*cos(theta),center.y+radius*sin(theta));
		theta+=step;
	}
	glEnd();
}

void Path::generate_bspline(const std::deque<Pt> &trail, std::deque<Pt> &result){
	std::deque<Pt>::const_iterator itr = trail.begin();
	assert (trail.size() > 0);
	Pt a = trail.front();
	Pt b = trail.front();
	Pt c = trail.front();
	Pt d = trail.front();
	while (itr != trail.end()) {
		a = b;
		b = c;
		c = d;
		d = *itr;
		bspline2(a,b,c,d,result);
		itr++;
	}
	bspline2(b,c,d,d,result);
	bspline2(c,d,d,d,result);
	remove_consecutive_duplicates(result);
}

void Path::draw_smooth_stroke2(const std::deque<Pt> &trail,double width) {
	//glBegin(GL_QUADS);
	//glBegin(GL_LINE_STRIP);
	//glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	std::deque<Pt> result;
	std::deque<Pt> processed = process_points(trail,width/2);
	if(processed.size()==1){
		draw_circle(processed.front(),width);
	}
	else{
		//std::list<Pt>::const_iterator itr = processed.begin();
		//assert (processed.size() > 0);
		//Pt a = processed.front();
		//Pt b = processed.front();
		//Pt c = processed.front();
		//Pt d = processed.front();
		//while (itr != processed.end()) {
			//a = b;
			//b = c;
			//c = d;
			//d = *itr;
			//bspline2(a,b,c,d,result);
			//itr++;
		//}
		//bspline2(b,c,d,d,result);
		//bspline2(c,d,d,d,result);
		generate_bspline(processed,result);
		std::list<Pt> upper, lower;
		generate_trail(result, width/2,upper,lower);
		//std::list<Pt> upper_bspline, lower_bspline;
		//generate_bspline(upper,upper_bspline);
		//generate_bspline(lower,lower_bspline);
		//smooth_trail(upper);
		//smooth_trail(lower);
		//draw_trail_line(upper,lower);
		draw_trail_quad(upper,lower);
		//draw_variable_line(result, width/2);
		draw_circle(result.front(),width/2);
		draw_circle(result.back(),width/2);
	}
}

/*void Path::draw_trail_line(const std::list<Pt> &upper, const std::list<Pt> &lower){
	std::list<Pt>::const_iterator itr = trail.begin();
	Pt prev(*itr++);
	Pt curr(*itr++);
	Pt next;
	Vec3f cross1;
	Vec3f cross2;
	Vec3f avg1;
	Vec3f avg2;
	Vec3f::Cross3(cross1,Vec3f(curr.x-prev.x,curr.y-prev.y,0),Vec3f(0,0,1));
	cross1.Normalize();
	avg1=cross1;
	for(;itr!=trail.end();itr++){
		next=*itr;
		Vec3f::Cross3(cross1,Vec3f(curr.x-prev.x,curr.y-prev.y,0),Vec3f(0,0,1));
		Vec3f::Cross3(cross2,Vec3f(next.x-curr.x,next.y-prev.y,0),Vec3f(0,0,1));
		Vec3f::Average(avg2,cross1,cross2);
		avg2.Normalize();

		glColor3f(0.0,1.0,0.0);
		glBegin(GL_LINES);
		glVertex2f(prev.x+radius*avg1.x(),prev.y+radius*avg1.y());
		glVertex2f(prev.x-radius*avg1.x(),prev.y-radius*avg1.y());
		glEnd();

		glColor3f(1.0,0,0);
		glBegin(GL_LINES);
		glVertex2f(prev.x-radius*avg1.x(),prev.y-radius*avg1.y());
		glVertex2f(curr.x-radius*avg2.x(),curr.y-radius*avg2.y());
		glEnd();

		glColor3f(1.0,0.0,1.0);
		glBegin(GL_LINES);
		glVertex2f(prev.x+radius*avg1.x(),prev.y+radius*avg1.y());
		glVertex2f(curr.x+radius*avg2.x(),curr.y+radius*avg2.y());
		glEnd();

		prev=curr;
		curr=next;
		avg1=avg2;
	}

	Vec3f::Cross3(cross1,Vec3f(curr.x-prev.x,curr.y-prev.y,0),Vec3f(0,0,1));
	Vec3f::Cross3(cross2,Vec3f(next.x-curr.x,next.y-prev.y,0),Vec3f(0,0,1));
	cross2.Normalize();
	cross1.Normalize();

	glColor3f(0.0,1.0,0.0);
	glBegin(GL_LINES);
	glVertex2f(prev.x+radius*avg1.x(),prev.y+radius*avg1.y());
	glVertex2f(prev.x-radius*avg1.x(),prev.y-radius*avg1.y());
	glEnd();

	glBegin(GL_LINES);
	glVertex2f(curr.x+radius*cross1.x(),curr.y+radius*cross1.y());
	glVertex2f(curr.x-radius*cross1.x(),curr.y-radius*cross1.y());
	glEnd();

	glColor3f(1.0,0,0);
	glBegin(GL_LINES);
	glVertex2f(prev.x-radius*avg1.x(),prev.y-radius*avg1.y());
	glVertex2f(curr.x-radius*cross1.x(),curr.y-radius*cross1.y());
	glEnd();

	glColor3f(1.0,0.0,1.0);
	glBegin(GL_LINES);
	glVertex2f(prev.x+radius*avg1.x(),prev.y+radius*avg1.y());
	glVertex2f(curr.x+radius*cross1.x(),curr.y+radius*cross1.y());
	glEnd();

	return;
}*/

void Path::draw_trail_line(const std::list<Pt> &upper, const std::list<Pt> &lower){
	std::list<Pt>::const_iterator upper_itr = upper.begin();
	std::list<Pt>::const_iterator lower_itr = lower.begin();
	Pt upper_prev = *upper_itr++;
	Pt lower_prev = *lower_itr++;
	Pt upper_curr;
	Pt lower_curr;
	while(upper_itr != upper.end()){
		upper_curr = *upper_itr++;
		lower_curr = *lower_itr++;
		glBegin(GL_LINES);
		glVertex2f(upper_prev.x,upper_prev.y);
		glVertex2f(lower_prev.x,lower_prev.y);
		glEnd();

		glBegin(GL_LINES);
		glVertex2f(upper_prev.x,upper_prev.y);
		glVertex2f(upper_curr.x,upper_curr.y);
		glEnd();

		glBegin(GL_LINES);
		glVertex2f(lower_prev.x,lower_prev.y);
		glVertex2f(lower_curr.x,lower_curr.y);
		glEnd();

		lower_prev = lower_curr;
		upper_prev = upper_curr;

	}
	glBegin(GL_LINES);
	glVertex2f(upper_curr.x,upper_curr.y);
	glVertex2f(lower_curr.x,lower_curr.y);
	glEnd();
}

void Path::draw_trail_quad(const std::list<Pt> &upper, const std::list<Pt> &lower){
	std::list<Pt>::const_iterator upper_itr = upper.begin();
	std::list<Pt>::const_iterator lower_itr = lower.begin();
	Pt upper_prev = *upper_itr++;
	Pt lower_prev = *lower_itr++;
	Pt upper_curr;
	Pt lower_curr;
	while(upper_itr != upper.end()){
		upper_curr = *upper_itr++;
		lower_curr = *lower_itr++;

		glBegin(GL_QUADS);
		glVertex2f(upper_prev.x,upper_prev.y);
		glVertex2f(lower_prev.x,lower_prev.y);
		glVertex2f(lower_curr.x,lower_curr.y);
		glVertex2f(upper_curr.x,upper_curr.y);
		glEnd();

		lower_prev = lower_curr;
		upper_prev = upper_curr;

	}
}

void Path::generate_trail(const std::deque<Pt> &trail, double radius,std::list<Pt> &upper, std::list<Pt>& lower){
	//glLineWidth(1.0);
	//glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	//glPointSize(10.0);
	//glBegin(GL_POINTS);
	//for(std::list<Pt>::const_iterator itr = trail.begin(); itr!= trail.end();itr++){
		//glVertex2f(itr->x,itr->y);
	//}
	//glEnd();
	int count = 0;
	if(trail.size()==1){
		draw_circle(trail.front(),radius);
	}
	else{
		std::deque<Pt>::const_iterator itr = trail.begin();
		Pt prev(*itr++);
		Pt curr(*itr++);
		Pt next;
		Vec3f cross1;
		Vec3f cross2;
		Vec3f avg1;
		Vec3f avg2;
		Vec3f::Cross3(cross1,Vec3f(curr.x-prev.x,curr.y-prev.y,0),Vec3f(0,0,1));
		cross1.Normalize();
		avg1=cross1;
		//glBegin(GL_QUADS);
		for(;itr!=trail.end();itr++){
			next=*itr;
			Vec3f::Cross3(cross1,Vec3f(curr.x-prev.x,curr.y-prev.y,0),Vec3f(0,0,1));
			Vec3f::Cross3(cross2,Vec3f(next.x-curr.x,next.y-prev.y,0),Vec3f(0,0,1));
			Vec3f::Average(avg2,cross1,cross2);
			avg2.Normalize();

			//glVertex2f(prev.x+radius*avg1.x(),prev.y+radius*avg1.y());
			//glVertex2f(prev.x-radius*avg1.x(),prev.y-radius*avg1.y());
			//glVertex2f(curr.x-radius*avg2.x(),curr.y-radius*avg2.y());
			//glVertex2f(curr.x+radius*avg2.x(),curr.y+radius*avg2.y());
			//if(count==0){
			lower.push_back(Pt(prev.x-radius*avg1.x(),prev.y-radius*avg1.y()));
			upper.push_back(Pt(prev.x+radius*avg1.x(),prev.y+radius*avg1.y()));
			//}

			count++;
			count=count%10;
			prev=curr;
			curr=next;
			avg1=avg2;
		}
		//glEnd();

		Vec3f::Cross3(cross1,Vec3f(curr.x-prev.x,curr.y-prev.y,0),Vec3f(0,0,1));
		Vec3f::Cross3(cross2,Vec3f(next.x-curr.x,next.y-prev.y,0),Vec3f(0,0,1));
		cross2.Normalize();
		cross1.Normalize();

		//glBegin(GL_QUADS);
		//glVertex2f(prev.x+radius*avg1.x(),prev.y+radius*avg1.y());
		//glVertex2f(prev.x-radius*avg1.x(),prev.y-radius*avg1.y());
		//glVertex2f(curr.x-radius*cross1.x(),curr.y-radius*cross1.y());
		//glVertex2f(curr.x+radius*cross1.x(),curr.y+radius*cross1.y());
		//glEnd();
		lower.push_back(Pt(curr.x-radius*avg1.x(),curr.y-radius*avg1.y()));
		upper.push_back(Pt(curr.x+radius*avg1.x(),curr.y+radius*avg1.y()));
	}
	draw_circle(trail.front(),radius);
	draw_circle(trail.back(),radius);
	//glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	return;
}

void Path::smooth_trail(std::list<Pt> &trail){
	std::list<Pt> smoothed;
	std::list<Pt>::iterator itr = trail.begin();
	Pt prev = *itr++;
	Pt curr = *itr++;
	Pt next;
	smoothed.push_back(Pt(0.5*prev.x+0.5*curr.x,0.5*prev.y+0.5*curr.y));
	for(;itr!=trail.end();itr++){
		next = *itr;
		smoothed.push_back(Pt(0.375*prev.x+0.375*next.x+0.25*curr.x,0.375*prev.y+0.375*next.y+0.25*curr.y));
		smoothed.push_back(Pt(0.4*prev.x+0.4*next.x+0.2*curr.x,0.4*prev.y+0.4*next.y+0.2*curr.y));
		//smoothed.push_back(Pt((1/3)*prev.x+(1/3)*next.x+(1/3)*curr.x,(1/3)*prev.y+(1/3)*next.y+(1/3)*curr.y));
		prev=curr;
		curr=next;
	}
	smoothed.push_back(Pt(.5*prev.x+.5*curr.x,.5*prev.y+.5*curr.y));
	trail.clear();
	trail=smoothed;
}
