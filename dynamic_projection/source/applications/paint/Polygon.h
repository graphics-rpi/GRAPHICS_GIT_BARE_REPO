#ifndef Polygon_h_
#define Polygon_h_

//Tyler Sammann commenting out CGAL library stuff
#if 0
#include <CGAL/basic.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Partition_traits_2.h>
#include <CGAL/partition_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/random_polygon_2.h>
#endif

#include <list>
#include <iostream>

// Graphics Library Includes
#include "../paint/gl_includes.h"

#include "../../calibration/planar_interpolation_calibration/planar_calibration.h"
#include "../../../../remesher/src/vectors.h"

//needed external globals
extern bool g_debug;
extern unsigned int g_frame_count;

#if 0
//typedefs for CGAL
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Partition_traits_2<K>                         Traits;
typedef Traits::Point_2                                     Point_2;
typedef Traits::Polygon_2                                   Polygon_2;
typedef Polygon_2::Vertex_iterator                          Vertex_iterator;
#endif

//a class for non-convex, simple polygons, i.e. polygons with no self
//intersections (no figure-8's)
class NonConvexPolygon{
	private:
		Polygon_2 polygon;
		std::list<Polygon_2> polygon_partition;
		Traits partition_traits;
		Vec3f color;
		unsigned int counter;

	//constructors
	public:
		NonConvexPolygon():polygon(),color(),counter(0){}
		NonConvexPolygon(const std::list<Pt>&, Vec3f color);
		NonConvexPolygon(const NonConvexPolygon &);

	//get functions
	public:
		Vec3f get_color(){return color;}
		std::list<Pt> get_point_list();

	//set functions
	public:
		void set_color(Vec3f);

	//public functions
	public:
		void add_point(Pt);
		void make_test();
		void draw_polygon();
		bool is_inside(Pt);

	private:
		void partition_polygon();
		std::list<Pt> process_points(const std::list<Pt> &, double);

	//static public functions
	public:
		static void draw_polygon(const std::list<Pt>&);
		static void make_convex(const std::list<Pt>&);
};
#endif
