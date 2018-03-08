#include "Polygon.h"

std::list<Pt> NonConvexPolygon::process_points(const std::list<Pt> &trail,double tolerance){
	std::list<Pt> processed = trail;
	for(std::list<Pt>::iterator itr = processed.begin(); itr!=processed.end();itr++){
		std::list<Pt>::iterator forward = itr;
		forward++;
		for(;forward!=processed.end() && DistanceBetweenTwoPoints(*itr,*forward)<tolerance;forward=processed.erase(forward));
	}
	processed.push_back(trail.back());
	return processed;
}

NonConvexPolygon::NonConvexPolygon(const std::list<Pt> &points, Vec3f _color){
	counter = 0;
	std::list<Pt> result = NonConvexPolygon::process_points(points,100.0);
	polygon.clear();
	for(std::list<Pt>::const_iterator itr = result.begin(); itr != result.end(); itr++){
		polygon.push_back(Point_2(itr->x,itr->y));
	}
	if(polygon.is_simple() && !polygon.is_counterclockwise_oriented())
		polygon.reverse_orientation();
	color = _color;
}

NonConvexPolygon::NonConvexPolygon(const NonConvexPolygon & other){
	polygon = other.polygon;
	polygon_partition = other.polygon_partition;
	color = other.color;
	counter = 0;
}

void NonConvexPolygon::add_point(Pt p){
	polygon.push_back(Point_2(p.x,p.y));
}

void NonConvexPolygon::make_test(){
	polygon.clear();
	polygon.push_back(Point_2(391, 374));
	polygon.push_back(Point_2(240, 431));
	polygon.push_back(Point_2(252, 340));
	polygon.push_back(Point_2(374, 320));
	polygon.push_back(Point_2(289, 214));
	polygon.push_back(Point_2(134, 390));
	polygon.push_back(Point_2( 68, 186));
	polygon.push_back(Point_2(154, 259));
	polygon.push_back(Point_2(161, 107));
	polygon.push_back(Point_2(435, 108));
	polygon.push_back(Point_2(208, 148));
	polygon.push_back(Point_2(295, 160));
	polygon.push_back(Point_2(421, 212));
	polygon.push_back(Point_2(441, 303));
}

void NonConvexPolygon::draw_polygon(){
	if(g_debug)
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL|GL_POINT);

	glPointSize(10.0);
	
	if(!polygon.is_simple()){
		std::cout<<"WARNING: Polygon is not simple\n";

		if(counter==40){
			counter=0;
		}
		else if(counter<20){
			glColor3f(1,0,0);
			counter++;
		}
		else{
			glColor3f(1,1,1);
			counter++;
		}

		if(g_debug){
			glBegin(GL_LINE_STRIP);
			for(Vertex_iterator vertex_itr = polygon.vertices_begin();
					vertex_itr != polygon.vertices_end(); vertex_itr++){
				glVertex2f(vertex_itr->x(),vertex_itr->y());
			}
			glEnd();
		}
		return;
	}

	glColor3f(color.x(),color.y(),color.z());

	//two cases: polygon is convex or polygon is concave
	if(polygon.is_convex()){

		glBegin(GL_POLYGON);
		for(Vertex_iterator vertex_itr = polygon.vertices_begin();
				vertex_itr != polygon.vertices_end(); vertex_itr++){
			glVertex2f(vertex_itr->x(),vertex_itr->y());
		}
		glEnd();
		return;
	}

	if(polygon_partition.empty())
		partition_polygon();

	for(std::list<Polygon_2>::iterator poly_itr = polygon_partition.begin();
			poly_itr != polygon_partition.end(); poly_itr++){
		glPointSize(10.0);
		glBegin(GL_POLYGON);
		for(Vertex_iterator vertex_itr = poly_itr->vertices_begin();
				vertex_itr != poly_itr->vertices_end(); vertex_itr++)
			glVertex2f(vertex_itr->x(),vertex_itr->y());
		glEnd();
		/*glBegin(GL_POINTS);
		for(Vertex_iterator vertex_itr = poly_itr->vertices_begin();
				vertex_itr != poly_itr->vertices_end(); vertex_itr++)
			glVertex2f(vertex_itr->x(),vertex_itr->y());
		glEnd();*/
	}
}

void NonConvexPolygon::partition_polygon(){
	if(!polygon.is_simple())
		return;
	polygon_partition.clear();
	CGAL::greene_approx_convex_partition_2(polygon.vertices_begin(),polygon.vertices_end(),
                                         std::back_inserter(polygon_partition), partition_traits);
}

bool NonConvexPolygon::is_inside(Pt test_point){
	switch(CGAL::bounded_side_2(polygon.vertices_begin(),polygon.vertices_end(),
				Point_2(test_point.x,test_point.y),partition_traits)){
		case CGAL::ON_BOUNDED_SIDE :
		case CGAL::ON_BOUNDARY :
			return true;
			break;
		case CGAL::ON_UNBOUNDED_SIDE :
			return false;
	}
	return false;
}

void NonConvexPolygon::set_color(Vec3f new_color){
	color = new_color;
}

void NonConvexPolygon::draw_polygon(const std::list<Pt> &points){
	NonConvexPolygon p;
	for(std::list<Pt>::const_iterator itr = points.begin(); itr != points.end(); itr++){
		p.add_point(*itr);
	}
	p.draw_polygon();
	return;
}
