#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include "walls.h"
#include "wall.h"
#include "wall_chain.h"
#include "mesh.h"

#include <cmath>

#include "triangle.h"
#include "quad.h"
#include "polygon.h"

#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "wall_fingerprint.h"

//Visual studio uses slightly different namespacing
#ifndef _WIN32
  using std::min;
  using std::max;
#endif

ArgParser *my_ugly_args_hack;
#include "argparser.h"
extern ArgParser *ARGS;

int BasicWall::wall_counter = 0;

/*
double Walls::GREEN_HEIGHT;
double Walls::BLUE_HEIGHT;
double Walls::RED_HEIGHT;
double Walls::WINDOW_BOTTOM;
double Walls::WINDOW_MIDDLE;
double Walls::WINDOW_TOP;
double Walls::WHEEL_HEIGHT;
*/

std::string Walls::luan_window = "none";
std::string Walls::canvas_window = "none";
std::string Walls::left_l_shaped_window = "neither";
std::string Walls::right_l_shaped_window = "neither";
std::string Walls::big_l_shaped_window = "neither";
std::string Walls::curved_window = "none";

// ========================================================================================
// ========================================================================================
// ========================================================================================
// ========================================================================================


Walls::Walls(ArgParser *_args) { 
    
  furniture_material_bed = Vec3f(0.6,0.8,1.0);
  furniture_material_desk = Vec3f(0.8,0.6,0.2);
  furniture_material_wardrobe = Vec3f(0.6,0.4,0.2);


  args = _args; empac = false; 
  scene_radius = -1;
  scene_center = Vec3f(0,0,0);
  my_ugly_args_hack = args;
  north_angle = 0;



  
  latitude  = 0;
  longitude = 0;
  

  /*  
  GREEN_HEIGHT = (5 * INCH_IN_METERS);
  BLUE_HEIGHT = (8 * INCH_IN_METERS);
  RED_HEIGHT = (10 * INCH_IN_METERS);
  
  WINDOW_BOTTOM= (3 * INCH_IN_METERS);
  WINDOW_MIDDLE = (6.5 * INCH_IN_METERS);
  WINDOW_TOP = (0.5 * INCH_IN_METERS);
  */
}

Walls::~Walls() {
  //  assert(0);
}

BasicWall* Walls::getWallWithName(const std::string& name) {
  for (unsigned int i = 0; i < walls.size(); i++) {
    if (walls[i]->getName() == name)
      return walls[i];
  }
  return NULL;
}

int Walls::numWindows() const {
  unsigned int answer = skylights.size();
  for (unsigned int i = 0; i < walls.size(); i++) {
    answer += walls[i]->getWindows().size();
  }
  return answer;
}

bool Walls::allShort() const {
  for (unsigned int i = 0; i < walls.size(); i++) {
    if (walls[i]->getMaxHeight() > 1.01 * GREEN_HEIGHT)
      return false;
  }
  return true;
}

bool Walls::allTall() const {
  for (unsigned int i = 0; i < walls.size(); i++) {
    if (walls[i]->getMaxHeight() < 0.99 * RED_HEIGHT)
      return false;
  }
  return true;
}

// ========================================================================================

void Walls::paintWallChains() const {
  static std::vector<Vec3f> rand_colors;
  if (rand_colors.size() == 0) {
    rand_colors.push_back(Vec3f(1,0,0)); 
    rand_colors.push_back(Vec3f(0,1,0)); 
    rand_colors.push_back(Vec3f(0,0,1)); 
    rand_colors.push_back(Vec3f(1,1,0)); 
    rand_colors.push_back(Vec3f(1,0,1)); 
    rand_colors.push_back(Vec3f(0,1,1)); 
    rand_colors.push_back(Vec3f(1,0.5,0)); 
    rand_colors.push_back(Vec3f(0.5,1,0)); 
    rand_colors.push_back(Vec3f(1,0,0.5)); 
    rand_colors.push_back(Vec3f(0.5,0,1)); 
    rand_colors.push_back(Vec3f(0,1,0.5)); 
    rand_colors.push_back(Vec3f(0,0.5,1)); 
    rand_colors.push_back(Vec3f(0.5,0.5,0)); 
    rand_colors.push_back(Vec3f(0.5,0.5,0)); 
    rand_colors.push_back(Vec3f(0.5,0,0.5)); 
  }

  glDisable(GL_LIGHTING);
  glBegin(GL_QUADS);
  unsigned int num_chains = wall_chains.size();

  if (rand_colors.size() < num_chains) {
    rand_colors.clear();
    for (unsigned int i = 0; i < num_chains; i++) {
      rand_colors.push_back(args->RandomColor());
    }
  }
  for (unsigned int i = 0; i < num_chains; i++) {
    glColor3f(rand_colors[i].r(),rand_colors[i].g(),rand_colors[i].b());
    const WallChain &chain = wall_chains[i];
    unsigned int num_quads = chain.quads.size();
    for (unsigned int j = 0; j < num_quads; j++) {
      if (j!=0) {
	// verify quads are connected in a chain
	assert (DistanceBetweenTwoPoints(chain.quads[j][0],chain.quads[j-1][1])<0.0000001);
	assert (DistanceBetweenTwoPoints(chain.quads[j][3],chain.quads[j-1][2])<0.0000001);
      }
      VecVertex(chain.quads[j][0]+Vec3f(0,0.01,0));
      VecVertex(chain.quads[j][3]+Vec3f(0,0.01,0));
      VecVertex(chain.quads[j][2]+Vec3f(0,0.01,0));
      VecVertex(chain.quads[j][1]+Vec3f(0,0.01,0));
    }
  }
  glEnd();
#if 0
  glColor3f(1,1,0);
  glLineWidth(2.0);
  glBegin(GL_LINES);
  for (unsigned int i = 0; i < num_chains; i++) {
    const WallChain &chain = wall_chains[i];
    unsigned int num_quads = chain.quads.size();
    for (unsigned int j = 0; j < num_quads; j++) {
      VecVertex(chain.quads[j][0]+Vec3f(0,0.02,0));
      VecVertex(chain.quads[j][1]+Vec3f(0,0.02,0));
    }
  }
  glEnd();
#endif
  glEnable(GL_LIGHTING);
}

// ===========================================================================================

extern std::vector<std::pair<std::pair<Vec3f,Vec3f>,bool> > GLOBAL_PUZZLE_CONNECTIONS;

void Walls::paintWalls() const {
  // draw walls
  for (unsigned int i = 0; i < walls.size(); i++) {
    walls[i]->paint();
  }
  // draw furniture
  for (unsigned int i = 0; i < furniture.size(); i++) {
    furniture[i]->paint();
  }
  // draw skylights
  for (unsigned int i = 0; i < skylights.size(); i++) {
    skylights[i]->paint();
  }
  // draw leds
  glDisable(GL_LIGHTING);
  glPointSize(10);
  glBegin(GL_POINTS);
  for (unsigned int i = 0; i < LEDs.size(); i++) {
    if (LEDs[i].second == 0) 
      glColor3f(1,0,0);
    else if (LEDs[i].second == 1) 
      glColor3f(1,1,0);
    else if (LEDs[i].second == 2) 
      glColor3f(0,1,0);
    else 
      glColor3f(0,0,1);
	
    glVertex3f(LEDs[i].first.x(),LEDs[i].first.y(),LEDs[i].first.z());
  }
  glEnd();

  glLineWidth(10);
  for (unsigned int i = 0; i < GLOBAL_PUZZLE_CONNECTIONS.size(); i++) {
    glBegin(GL_LINES);
    Vec3f A = GLOBAL_PUZZLE_CONNECTIONS[i].first.first;
    Vec3f B = GLOBAL_PUZZLE_CONNECTIONS[i].first.second;
    //A.Print("A");
    A+=Vec3f(0,8*12*INCH_IN_METERS,0);
    B+=Vec3f(0,8*12*INCH_IN_METERS,0);
    //A.Print("A");
    bool tmp = GLOBAL_PUZZLE_CONNECTIONS[i].second;
    if (tmp) glColor3f(0,1,0); else glColor3f(1,0,0);
    glVertex3f(A.x(),A.y(),A.z());
    glVertex3f(B.x(),B.y(),B.z());
    glEnd();
  }
  

}

// ===========================================================================================

void Walls::paintFloor() const {
  if (scene_radius < 0) return;
  // draw floor
  glColor3f(0.8,0.8,0.8);
  glNormal3f(0,1,0);
  glBegin(GL_POLYGON);
  assert (scene_radius > 0);
  for (int i = 0; i < CIRCLE_RES; i++) {
    double angle = i*2*M_PI/CIRCLE_RES;
    Vec3f pt = scene_center + Vec3f(scene_radius*sin(angle),-0.01,scene_radius*cos(angle));
    glVertex3f(pt.x(),pt.y(),pt.z());
  }
  glEnd();
}

void RotateThenTranslatePoint(double &x, double &z, double rotate_angle, double trans_x, double trans_z);

void Walls::RenderNorthArrow() const {

  if (scene_radius < 0) return;

  glDisable(GL_LIGHTING);
  glColor3f(1.0,0,0);
  glNormal3f(0,1,0);

  double a = 0.99 * scene_radius;
  double b = 0;
  double c = 0.85 * scene_radius;
  double d = -0.03 * scene_radius;
  double e = 0.85 * scene_radius;
  double f = 0.03 * scene_radius;

  RotateThenTranslatePoint(a,b,my_ugly_args_hack->walls_rotate_angle+north_angle-0.5*M_PI,0,0);
  RotateThenTranslatePoint(c,d,my_ugly_args_hack->walls_rotate_angle+north_angle-0.5*M_PI,0,0);
  RotateThenTranslatePoint(e,f,my_ugly_args_hack->walls_rotate_angle+north_angle-0.5*M_PI,0,0);

  glBegin(GL_TRIANGLES);
  glNormal3f(0,1,0);
  glVertex3f(a,0.02,b);
  glVertex3f(c,0.02,d);
  glVertex3f(e,0.02,f);
  glEnd();
  
  a = 0.88 * scene_radius;
  b = 0.05 * scene_radius;
  c = 0.95 * scene_radius;
  d = 0.05 * scene_radius;
  e = 0.88 * scene_radius;
  f = 0.08 * scene_radius;
  double g = 0.95 * scene_radius;
  double h = 0.08 * scene_radius;

  RotateThenTranslatePoint(a,b,my_ugly_args_hack->walls_rotate_angle+north_angle-0.5*M_PI,0,0);
  RotateThenTranslatePoint(c,d,my_ugly_args_hack->walls_rotate_angle+north_angle-0.5*M_PI,0,0);
  RotateThenTranslatePoint(e,f,my_ugly_args_hack->walls_rotate_angle+north_angle-0.5*M_PI,0,0);
  RotateThenTranslatePoint(g,h,my_ugly_args_hack->walls_rotate_angle+north_angle-0.5*M_PI,0,0);


  glLineWidth(2);
  glBegin(GL_LINES);
  glVertex3f(a,0.02,b);
  glVertex3f(c,0.02,d);
  glVertex3f(c,0.02,d);
  glVertex3f(e,0.02,f);
  glVertex3f(e,0.02,f);
  glVertex3f(g,0.02,h);
  glEnd();

  glEnable(GL_LIGHTING);
}


// ===========================================================================================
// ===========================================================================================
// WALL ORIENTATION
// ===========================================================================================
// ===========================================================================================

double signed_angle(Vec3f v1, Vec3f v2) {
  double angle = acos(v1.Dot3(v2));
  assert (angle >= 0 && angle <= M_PI);
  Vec3f cross;
  Vec3f::Cross3(cross,v1,v2);
  cross.Normalize();
  Vec3f tmp = Vec3f(0,1,0);
  if (tmp.Dot3(cross) > 0)
    angle *= -1;
  return angle;
}


void Walls::ShuffleWalls() {
  /*
  // "invert" inside out skylights
  for (unsigned int i = 0; i < skylights.size(); i++) {
    skylights[i].invert();
  }
  // "invert" inside out walls
  for (unsigned int i = 0; i < walls.size(); i++) {
    BasicWall *w = walls[i];
    w->invert();
    for (unsigned int j = 0; j < w->getWindows().size(); j++) {
      w->getWindows()[j].invert();
    }
  }
  */
#if 0
  // make 0->1 (and 2->3) be the long edge
  for (unsigned int i = 0; i < walls.size(); i++) {
    //    assert(0); // take this out!
    BasicWall &w = walls[i];
    if (w.numConvexQuads() != 1) continue;
    const ConvexQuad &q = w.getConvexQuad(0);
    /*
    Vec3f v0 = q[0]w->front_left()-w->front_right(); //(*w)[0]-(*w)[1];
    Vec3f v1 = w->front_right()-w->back_right(); //(*w)[0]-(*w)[1];
    Vec3f v2 = w->back_right()-w->back_left(); //(*w)[0]-(*w)[1];
    Vec3f v3 = w->back_left()-w->front_left(); //(*w)[0]-(*w)[1];
    */
    //Vec3f v1 = (*w)[1]-(*w)[2];
    //Vec3f v2 = (*w)[2]-(*w)[3];
    //Vec3f v3 = (*w)[3]-(*w)[0];



    double should_be_long_1 = (q[0]-q[1]).Length();
    double should_be_long_2 = (q[2]-q[3]).Length();
    double should_be_short_1 = (q[1]-q[2]).Length();
    double should_be_short_2 = (q[0]-q[3]).Length();


    //    if (ARGS->verbose_mode) {
    (*ARGS->output) << "AFTER " << should_be_long_1 << " " 
	 << should_be_long_2 << " " 
	 << should_be_short_1 << " " 
	 << should_be_short_2 << " " << std::endl;
    //}
      continue;
    }
	exit(0);

  }
#endif
  /*
  // "flip" so main face (0->1), faces in
  for (unsigned int i = 0; i < walls.size(); i++) {
    Wall *w = walls[i];
    Vec3f centroid = w->getCentroid();
    Vec3f normal;
    computeNormal((*w)[0],(*w)[1],(*w)[4],normal);
    centroid.Normalize();
    double dot = centroid.Dot3(normal);
    if (dot < 0) { 
      w->flip();
    }
  }
  */
}

// ===========================================================================================
// ===========================================================================================





// ===========================================================================================
// ===========================================================================================
// ARRANGEMENT STUFF
// ===========================================================================================
// ===========================================================================================

#define DISTANCE_TOLERANCE 10e-07
//0.00000000001

bool OnLine(const Vec3f &pt, const Vec3f &endpt1, const Vec3f &endpt2) {
  double dist1 = DistanceBetweenTwoPoints(pt,endpt1);
  double dist2 = DistanceBetweenTwoPoints(pt,endpt2);
  if (dist1 < DISTANCE_TOLERANCE || dist2 < DISTANCE_TOLERANCE) {
    //(*ARGS->output) << "PT IS AN ENDPOINT!" << std::endl;
    return true;
  }
  double line_length = DistanceBetweenTwoPoints(endpt1,endpt2);
  if (line_length < DISTANCE_TOLERANCE) {
    //if (ARGS->verbose_mode) {
    (*ARGS->output) << "line length " << line_length << std::endl;
    (*ARGS->output) << "CRAP LINE LENGTH" << std::endl;
    //}
    return true;
    //return false;  // HACK
  }
  assert (line_length > DISTANCE_TOLERANCE);

  double distance = PerpendicularDistanceToLine(pt,endpt1,endpt2);
  if (distance <= DISTANCE_TOLERANCE) {
    //(*ARGS->output) << "PT IS ON LINE!" << std::endl;
    return true;
  }

  //double dist3 = DistanceBetweenTwoPoints(endpt1,endpt2);
  //(*ARGS->output) << "distances " << dist1 << " " << dist2 << " " << dist3 << std::endl;
  
  //(*ARGS->output) << "distance is " << distance << std::endl;
  return false;
}


bool OnSegment(const Vec3f &pt, const Vec3f &endpt1, const Vec3f &endpt2) {
  if (!OnLine(pt,endpt1,endpt2)) {
    //  if (ARGS->verbose_mode) {
    (*ARGS->output) << "WARNING: not online" << std::endl;
    //  }
  }

  double dist1 = DistanceBetweenTwoPoints(pt,endpt1);
  double dist2 = DistanceBetweenTwoPoints(pt,endpt2);
  if (dist1 < DISTANCE_TOLERANCE || dist2 < DISTANCE_TOLERANCE) {
    // pt is an endpoint
    return true;
  }

  Vec3f v1 = endpt1 - pt;
  Vec3f v2 = endpt2 - pt;
  v1.Normalize();
  v2.Normalize();
  /*
  v1.Print("v1");
  v2.Print("v2");
  */

  double dot = v1.Dot3(v2);
  if (dot < -0.9) return true;
  if (dot > 0.9) return false;

  //  if (ARGS->verbose_mode) {
  (*ARGS->output) << "CRAP" << std::endl;
  //	  }
  exit(0);
}










int DigVertFromVPHash(vphashtype &vphash, int a, int b, Vec3f v, Mesh *arrangement) {
  int answer;
  vphashtype::iterator foo = vphash.find(std::make_pair(b,a));
  if (foo == vphash.end()) {
    answer = arrangement->addVertex(v,a,b,0,0); 
    vphash[std::make_pair(a,b)] = answer;
  } else {
    answer = foo->second; 
  } 
  return answer;
}


void CutPolygon_2SegmentsCut(std::vector<int> &verts_on_line, std::vector<std::pair<std::pair<int, int>,Vec3f> > &segments_cut, 
			     Poly *input, Poly*& out1, Poly*& out2, Poly*& out3, Poly*& out4,
			     const Vec3f &a, const Vec3f &b, 
			     Mesh *arrangement, vphashtype &vphash, int wall_id, int wall_edge) {

  assert (input->checkVerts());

  int num_verts = input->numVertices();
  //  (*ARGS->output) << "2 segments cut" << std::endl;
  assert (verts_on_line.size() == 0);
  assert (segments_cut.size() == 2);
  Vec3f c = segments_cut[0].second;
  Vec3f d = segments_cut[1].second;
  bool c_onsegment = OnSegment(c,a,b);
  bool d_onsegment = OnSegment(d,a,b);
  bool a_onsegment = OnSegment(a,c,d);
  bool b_onsegment = OnSegment(b,c,d);

  if (my_ugly_args_hack->CUT_COMPLETELY) {
    c_onsegment = true;
    d_onsegment = true;
  }

  if (!c_onsegment && !d_onsegment) {
    // NEITHER POLYGON EDGE INTERSECTS THE CUTTING SEGMENT
    if (a_onsegment && b_onsegment) {
      // CUTTING SEGMENT CONTAINED WITHIN POLYGON
      out1 = new Poly(arrangement);
      out2 = new Poly(arrangement);
      int vert_a,vert_b;
      //assert (input->PointInside(a));
      // assert (input->PointInside(b));
      if (DistanceBetweenTwoPoints(c,a) < DistanceBetweenTwoPoints(c,b)) {
	vert_a = arrangement->addVertex(a,-1,-1,0,0);
	vert_b = arrangement->addVertex(b,-1,-1,0,0);
      } else {
 	assert(DistanceBetweenTwoPoints(d,a) < DistanceBetweenTwoPoints(d,b));
	vert_a = arrangement->addVertex(b,-1,-1,0,0);
	vert_b = arrangement->addVertex(a,-1,-1,0,0);
      }
      Poly *which_poly = out1;
      for (int i = 0; i < num_verts; i++) {
	which_poly->addVert((*input)[i]);
	if (i == segments_cut[0].first.first) {
	  which_poly->addVert(vert_a);
	  assert (which_poly == out1);
	  which_poly = out2;
	  which_poly->addVert(vert_a);
	}
	if (i == segments_cut[1].first.first) {
	  which_poly->addVert(vert_b);
	  assert (which_poly == out2);
	  which_poly = out1;
	  which_poly->addVert(vert_b);
	}
      }
      int tmp;
      out3 = new Poly(arrangement);
      out3->addVert(vert_a);
      tmp = segments_cut[0].first.first;
      out3->addVert((*input)[tmp]);
      tmp = segments_cut[0].first.second;
      out3->addVert((*input)[tmp]);
      assert (out1->checkVerts());
      assert (out3->checkVerts());
      Poly::StealVertices(out3,out1,vert_a);
      assert (out2->checkVerts());
      assert (out3->checkVerts());
      Poly::StealVertices(out3,out2,vert_a);
      out4 = new Poly(arrangement);
      out4->addVert(vert_b);
      tmp = segments_cut[1].first.first;
      out4->addVert((*input)[tmp]);
      tmp = segments_cut[1].first.second;
      out4->addVert((*input)[tmp]);
      assert (out1->checkVerts());
      assert (out4->checkVerts());
      Poly::StealVertices(out4,out1,vert_b);
      assert (out2->checkVerts());
      assert (out4->checkVerts());
      Poly::StealVertices(out4,out2,vert_b);
    } else {
      assert (!a_onsegment && !b_onsegment);
      // CUTTING SEGMENT DOES NOT OVERLAP THE POLYGON, DO NOTHING
      //assert (!input->PointInside(a));
      //assert (!input->PointInside(b));
    }
    return;
  } else if ((!c_onsegment && d_onsegment) || (c_onsegment && !d_onsegment)) {
    // ONE ENDPOINT OF THE CUTTING SEGMENT IS INSIDE THE POLYGON
    int vert_a, vert_b;
    int dug_vert;
    if (!c_onsegment) {
      //(*ARGS->output) << "c not on segment" << std::endl;
      dug_vert = vert_b = DigVertFromVPHash(vphash,(*input)[segments_cut[1].first.first],(*input)[segments_cut[1].first.second],d,arrangement);
      if (DistanceBetweenTwoPoints(c,a) < DistanceBetweenTwoPoints(c,b)) {
	//assert (input->PointInside(a));
	vert_a = arrangement->addVertex(a,-1,-1,0,0);
	assert (a_onsegment);
	assert (!b_onsegment);
      } else { 
	//assert (!input->PointInside(a));
	//assert (input->PointInside(b));
	vert_a = arrangement->addVertex(b,-1,-1,0,0);
	assert (b_onsegment);
	assert (!a_onsegment);
      }
      int tmp;
      out3 = new Poly(arrangement);
      out3->addVert(vert_a);
      tmp = segments_cut[0].first.first;
      out3->addVert((*input)[tmp]);
      tmp = segments_cut[0].first.second;
      out3->addVert((*input)[tmp]);
    } else {
      assert (!d_onsegment);
      //(*ARGS->output) << "one on segment d not" << std::endl;
      dug_vert = vert_a = DigVertFromVPHash(vphash,(*input)[segments_cut[0].first.first],(*input)[segments_cut[0].first.second],c,arrangement);
      if (DistanceBetweenTwoPoints(d,a) < DistanceBetweenTwoPoints(d,b)) {
	//assert (input->PointInside(a));
	//assert (!input->PointInside(b));
	assert (a_onsegment);
	bool test = b_onsegment;
	if (test) {
	  (*ARGS->output) << "TESTING B SEGMENT FAILED" << std::endl;
	} else {
	  assert (!b_onsegment);
	}
	vert_b = arrangement->addVertex(a,-1,-1,0,0);
      } else { 
	//assert (input->PointInside(b));
	//assert (!input->PointInside(a));
	assert (!a_onsegment);
	assert (b_onsegment);
	vert_b = arrangement->addVertex(b,-1,-1,0,0);
      }
      int tmp;
      out4 = new Poly(arrangement);
      out4->addVert(vert_b);
      tmp = segments_cut[1].first.first;
      out4->addVert((*input)[tmp]);
      tmp = segments_cut[1].first.second;
      out4->addVert((*input)[tmp]);
    }

    //(*ARGS->output) << "VERTS " << vert_a << " " << vert_b << std::endl;
    //a.Print("a");
    //b.Print("b");
    //(*ARGS->output) << "verts " << vert_a << " " << vert_b << std::endl;
    //arrangement->getVertex(vert_a)->get().Print("vert_a");
    //arrangement->getVertex(vert_b)->get().Print("vert_b");
    double dist = DistanceBetweenTwoPoints(arrangement->getVertex(vert_a)->get(),arrangement->getVertex(vert_b)->get());
    //(*ARGS->output) << "DIST " << dist << std::endl;    

    if (dist < 0.00001) { //DistanceBetweenTwoPoints(arrangement->getVertex(vert_a)->get(),arrangement->getVertex(vert_b)->get()) < 0.0000001) {
      //(*ARGS->output) << "massive ugliness" << std::endl;
      // MASSIVE UGLINESS!!!!!!!!!!!
      delete out3; out3=NULL;
      delete out4; out4=NULL;
      //input->Print("input");
      out1 = new Poly(arrangement);
      out2 = new Poly(arrangement);
      /*
      (*ARGS->output) << segments_cut[0].first.first << std::endl;
      (*ARGS->output) << segments_cut[0].first.second << std::endl;
      (*ARGS->output) << segments_cut[1].first.first << std::endl;
      (*ARGS->output) << segments_cut[1].first.second << std::endl;
      */
      out1->addVert(dug_vert);
      out2->addVert(dug_vert);
      for (int i = 0; i < num_verts; i++) {
	int j;
	if (c_onsegment) { j = (segments_cut[0].first.second+i)%num_verts; }
	else { assert (d_onsegment); j = (segments_cut[1].first.second+i)%num_verts; }
	if (i <= 1) out1->addVert((*input)[j]);
	if (i >= 1) out2->addVert((*input)[j]);
      }
      //out1->Print("out1");
      //out2->Print("out2");
      assert (input->checkVerts());
      assert (out1->checkVerts());
      assert (out2->checkVerts());
      Poly::StealVertices(out1,out2,dug_vert);
      return;
    }

    assert (DistanceBetweenTwoPoints(arrangement->getVertex(vert_a)->get(),arrangement->getVertex(vert_b)->get()) > 0.00001);

    out1 = new Poly(arrangement);
    out2 = new Poly(arrangement);
    Poly *which_poly = out1;
    for (int i = 0; i < num_verts; i++) {
      which_poly->addVert((*input)[i]);
      if (i == segments_cut[0].first.first) {
	which_poly->addVert(vert_a);
	assert (which_poly == out1);
	which_poly = out2;
	which_poly->addVert(vert_a);
      }
      if (i == segments_cut[1].first.first) {
	which_poly->addVert(vert_b);
	assert (which_poly == out2);
	which_poly = out1;
	which_poly->addVert(vert_b);
      }
    }

    if (out3 != NULL) {
      assert (out4 == NULL);
      assert (out1->checkVerts());
      assert (out3->checkVerts());
      Poly::StealVertices(out3,out1,vert_a);
      assert (out2->checkVerts());
      assert (out3->checkVerts());
      Poly::StealVertices(out3,out2,vert_a);
    } else {
      assert (out4 != NULL);
      if (!out1->checkVerts()) {
	input->Print("input");
	out1->Print("out1");
	out2->Print("out2");
	out4->Print("out4");
      }
      assert (out1->checkVerts());
      assert (out4->checkVerts());
      Poly::StealVertices(out4,out1,vert_b);
      assert (out2->checkVerts());
      assert (out4->checkVerts());
      Poly::StealVertices(out4,out2,vert_b);
    }
  } else {
    assert (c_onsegment && d_onsegment);
    // TWO EDGES OF THE POLYGON INTERSECT THE CUTTING SEGMENT
    out1 = new Poly(arrangement);
    out2 = new Poly(arrangement);
    int vert_a = DigVertFromVPHash(vphash,(*input)[segments_cut[0].first.first],(*input)[segments_cut[0].first.second],c,arrangement);
    int vert_b = DigVertFromVPHash(vphash,(*input)[segments_cut[1].first.first],(*input)[segments_cut[1].first.second],d,arrangement);
    Poly *which_poly = out1;
    for (int i = 0; i < num_verts; i++) {
      which_poly->addVert((*input)[i]);
      if (i == segments_cut[0].first.first) {
	which_poly->addVert(vert_a);
	assert (which_poly == out1);
	which_poly = out2;
	which_poly->addVert(vert_a);
      }
      if (i == segments_cut[1].first.first) {
	which_poly->addVert(vert_b);
	assert (which_poly == out2);
	which_poly = out1;
	which_poly->addVert(vert_b);
      }
    }
  }
}


void CutPolygon_1SegmentCut(std::vector<int> &verts_on_line, std::vector<std::pair<std::pair<int, int>,Vec3f> > &segments_cut, 
			    Poly *input, Poly*& out1, Poly*& out2, Poly*& out3, const Vec3f &a, const Vec3f &b, 
			    Mesh *arrangement, vphashtype &vphash, int wall_id, int wall_edge) {

  assert (input->checkVerts());

  //(*ARGS->output) << "1 segment cut" << std::endl;
  assert (verts_on_line.size() == 1);
  assert (segments_cut.size() == 1);
  int num_verts = input->numVertices();
  Vec3f c = segments_cut[0].second;
  Vec3f d = arrangement->getVertex((*input)[verts_on_line[0]])->get();
  Vec3f vab = a-b; vab.Normalize();
  Vec3f vcd = c-d; vcd.Normalize();
  bool c_onsegment = OnSegment(c,a,b);
  bool d_onsegment = OnSegment(d,a,b);
  bool a_onsegment = OnSegment(a,c,d);
  bool b_onsegment = OnSegment(b,c,d);
  if (my_ugly_args_hack->CUT_COMPLETELY) {
    c_onsegment = true;
    d_onsegment = true;
  }

  if (c_onsegment) {
    // CUT THROUGH POLYGON 
    assert (verts_on_line[0] != segments_cut[0].first.first && verts_on_line[0] != segments_cut[0].first.second);
    out1 = new Poly(arrangement);
    out2 = new Poly(arrangement);
    int vert_a = DigVertFromVPHash(vphash,(*input)[segments_cut[0].first.first],(*input)[segments_cut[0].first.second],c,arrangement);
    Poly *which_poly = out1;
    for (int j = 0; j < num_verts; j++) {
      int i = (verts_on_line[0]+j) % num_verts;
      which_poly->addVert((*input)[i]);
      if (i == segments_cut[0].first.first) {
	which_poly->addVert(vert_a);
      assert (which_poly == out1);
      which_poly = out2;
      which_poly->addVert(vert_a);
      }
    }
    out2->addVert((*input)[verts_on_line[0]]);
  } else if (!d_onsegment && !a_onsegment && !b_onsegment) {
    // NO OVERLAP, DO NOTHING
  } else {
    // ONLY PARTIAL OVERLAP, MAKE 3 POLYGONS
    assert (verts_on_line[0] != segments_cut[0].first.first && verts_on_line[0] != segments_cut[0].first.second);
    int vert_a;
    if (DistanceBetweenTwoPoints(c,a) <
	DistanceBetweenTwoPoints(c,b)) {
      vert_a = arrangement->addVertex(a,-1,-1,0,0);
      if (DistanceBetweenTwoPoints(a,d) < DISTANCE_TOLERANCE) {
	return;
      }      
    } else {
      vert_a = arrangement->addVertex(b,-1,-1,0,0);
      if (DistanceBetweenTwoPoints(b,d) < DISTANCE_TOLERANCE) {
	return;
      }      
    }
    out1 = new Poly(arrangement);
    out2 = new Poly(arrangement);
    Poly *which_poly = out1;
    for (int j = 0; j < num_verts; j++) {
      int i = (verts_on_line[0]+j) % num_verts;
      which_poly->addVert((*input)[i]);
      if (i == segments_cut[0].first.first) {
	which_poly->addVert(vert_a);
	assert (which_poly == out1);
	which_poly = out2;
	which_poly->addVert(vert_a);
      }
    }
    out2->addVert((*input)[verts_on_line[0]]);
    int tmp;
    out3 = new Poly(arrangement);
    out3->addVert(vert_a);
    tmp = segments_cut[0].first.first;
    out3->addVert((*input)[tmp]);
    tmp = segments_cut[0].first.second;
    out3->addVert((*input)[tmp]);
    assert (out1->checkVerts());
    assert (out3->checkVerts());
    Poly::StealVertices(out3,out1,vert_a);
    assert (out2->checkVerts());
    assert (out3->checkVerts());
    Poly::StealVertices(out3,out2,vert_a);
  }
}



void CutPolygon_0SegmentsCut(std::vector<int> &verts_on_line, 
			    Poly *input, Poly*& out1, Poly*& out2, const Vec3f &a, const Vec3f &b, 
			    Mesh *arrangement, vphashtype &vphash, int wall_id, int wall_edge) {

  assert (input->checkVerts());

  assert (verts_on_line.size() == 2);
  //  (*ARGS->output) << "2 verts on line, no segments cut" << std::endl;

  int num_verts = input->numVertices();
  Vec3f c = arrangement->getVertex((*input)[verts_on_line[0]])->get();
  Vec3f d = arrangement->getVertex((*input)[verts_on_line[1]])->get();
  bool c_onsegment = OnSegment(c,a,b);
  bool d_onsegment = OnSegment(d,a,b);
  bool a_onsegment = OnSegment(a,c,d);
  bool b_onsegment = OnSegment(b,c,d);
  if (my_ugly_args_hack->CUT_COMPLETELY) {
    c_onsegment = true;
    d_onsegment = true;
  }

  if (!c_onsegment && !d_onsegment && !a_onsegment && !b_onsegment) {
    (*ARGS->output) << "0 segments cut, no overlap, do nothing" << std::endl;
  } else {
    assert (num_verts >= 4);
    assert ((verts_on_line[0]+1)%num_verts != verts_on_line[1]);
    assert ((verts_on_line[1]+1)%num_verts != verts_on_line[0]);


    // CUT THROUGH POLYGON 
    out1 = new Poly(arrangement);
    out2 = new Poly(arrangement);

    Poly *which_poly = out1;
    for (int j = 0; j < num_verts; j++) {
      int i = (verts_on_line[0]+j) % num_verts;
      which_poly->addVert((*input)[i]);
      if (i == verts_on_line[1]) {
	assert (which_poly == out1);
	which_poly = out2;
	which_poly->addVert((*input)[i]);
      }
    }
    int i = verts_on_line[0];
    which_poly->addVert((*input)[i]);

    /*
    input->Print("input");
    out1->Print("out1");
    out2->Print("out2");
    */
  }
}


void CutPolygon(Poly *input, Poly*& out1, Poly*& out2, Poly*& out3, Poly*& out4, const Vec3f &a, const Vec3f &b, 
		Mesh *arrangement, vphashtype &vphash, int wall_id, int wall_edge) {
  std::vector<int> verts_on_line;
  std::vector<std::pair<std::pair<int, int>,Vec3f> > segments_cut;

  // loop through the cell edges
  int num_verts = input->numVertices();
  for (int i = 0; i < num_verts; i++) {
    // check to see if this vertex is on the cutting line
    Vec3f c = arrangement->getVertex((*input)[i])->get();
    Vec3f d = arrangement->getVertex((*input)[(i+1)%num_verts])->get();
    if (OnLine(c,a,b)) { verts_on_line.push_back(i); continue; }
    if (OnLine(d,a,b)) { continue; }
    Vec3f intersection;
    /*
    a.Print("a");
    b.Print("b");
    c.Print("c");
    d.Print("d");
    */
    Vec3f v1 = b-a;
    Vec3f v2 = d-c;
    //    v1.Print("v1");
    //v2.Print("v2");
    double length = v2.Length();
    v1.Normalize();
    v2.Normalize();
    assert (fabs(v1.Length()-1) < DISTANCE_TOLERANCE);
    assert (fabs(v2.Length()-1) < DISTANCE_TOLERANCE);
    bool success = Intersect(a,v1,c,v2,intersection);
    if (!success) { continue; }      
    // if intersection is between the cell endpoints...
    Vec3f diff_c = c-intersection;
    Vec3f diff_d = d-intersection;
    double length_c = diff_c.Length();
    double length_d = diff_d.Length();
    assert (length_c > DISTANCE_TOLERANCE);
    assert (length_d > DISTANCE_TOLERANCE);
    if (length + 0.00000001 > length_c + length_d) {
      segments_cut.push_back(std::make_pair(std::make_pair(i,(i+1)%num_verts),intersection));
    }
  }

  if (verts_on_line.size() == 0 && segments_cut.size() == 0) {
    return;
  } else if (verts_on_line.size() == 0 && segments_cut.size() == 2) {
    CutPolygon_2SegmentsCut(verts_on_line,segments_cut,input,out1,out2,out3,out4,a,b,arrangement,vphash,wall_id,wall_edge);
  } else if (verts_on_line.size() == 1 && segments_cut.size() == 0) {
    // only one vert, do nothing
  } else if (verts_on_line.size() == 1 && segments_cut.size() == 1) {
    CutPolygon_1SegmentCut(verts_on_line,segments_cut,input,out1,out2,out3,a,b,arrangement,vphash,wall_id,wall_edge);
  } else if (verts_on_line.size() == 2 && segments_cut.size() == 0) {
    if (((verts_on_line[0]+1)%num_verts == verts_on_line[1]) || ((verts_on_line[1]+1)%num_verts == verts_on_line[0])) {
      // edge is on line, do nothing
    } else {
      CutPolygon_0SegmentsCut(verts_on_line,input,out1,out2,a,b,arrangement,vphash,wall_id,wall_edge);
    }
  } else {
    (*ARGS->output) << "VERTSONLINE SIZE " << verts_on_line.size() << std::endl;
    (*ARGS->output) << "SEGMENTSCUT SIZE " << segments_cut.size() << std::endl;
    //exit(0);
  }
}


void SplitLongEdgesInArrangement(Mesh *arrangement) {
  return;
#if 0
  // to efficiently find shared edge vertices
  vphashtype vphash;
  std::vector<Poly*> to_be_deleted;
  std::vector<Poly*> to_be_added;

  for (elementshashtype::const_iterator foo = arrangement->getElements().begin();
       foo != arrangement->getElements().end();
       foo++) {
    Poly *input = (Poly*)foo->second;
    Poly *out1 = NULL;
    Poly *out2 = NULL;
    Poly *out3 = NULL;
    Poly *out4 = NULL;
    CutPolygon(input,out1,out2,out3,out4,a,b,arrangement,vphash,wall_id,wall_edge);
    if (out1 != NULL) {
      assert (input != NULL && out2 != NULL);
      //if (!out1->checkVerts()) { (*ARGS->output) << "OUT1 CHECK VERTS BAD" << std::endl; assert(0); continue; }
      if (!out1->checkVerts()) { (*ARGS->output) << "OUT1 CHECK VERTS BAD" << std::endl; /* HACK */ continue; }
      if (!out2->checkVerts()) { (*ARGS->output) << "OUT2 CHECK VERTS BAD" << std::endl; assert(0); continue; }
      to_be_deleted.push_back(input);
      //assert(out1->checkVerts());
      //assert(out2->checkVerts());
      to_be_added.push_back(out1);
      to_be_added.push_back(out2);      
      if (out3 != NULL) {
	assert(out3->checkVerts());
	to_be_added.push_back(out3);
      }
      if (out4 != NULL) {
	assert(out4->checkVerts());
	to_be_added.push_back(out4);
      }
    } else {
      assert (out2 == NULL && out3 == NULL && out4 == NULL);
    }
  }
  for (unsigned int i = 0; i < to_be_deleted.size(); i++) {
    arrangement->removeElement(to_be_deleted[i]);
  }
  for (unsigned int i = 0; i < to_be_added.size(); i++) {
    arrangement->addElement(to_be_added[i]);
  }
  vphash.clear();
#endif
}

void CutArrangement(Mesh *arrangement, const Vec3f &a, const Vec3f &b, int wall_id, int wall_edge) {
  // to efficiently find shared edge vertices
  vphashtype vphash;
  std::vector<Poly*> to_be_deleted;
  std::vector<Poly*> to_be_added;

  //(*ARGS->output) << "CUT ARRANGMENT " << wall_id << " " << wall_edge;
  //(*ARGS->output) << "    " << arrangement->getElements().size() << " cells" << std::endl;

  //  if (arrangement->getElements().size() > 33) return;

  for (elementshashtype::const_iterator foo = arrangement->getElements().begin();
       foo != arrangement->getElements().end();
       foo++) {
    Poly *input = (Poly*)foo->second;
    Poly *out1 = NULL;
    Poly *out2 = NULL;
    Poly *out3 = NULL;
    Poly *out4 = NULL;
    CutPolygon(input,out1,out2,out3,out4,a,b,arrangement,vphash,wall_id,wall_edge);
    if (out1 != NULL) {
      assert (input != NULL && out2 != NULL);
      //if (!out1->checkVerts()) { (*ARGS->output) << "OUT1 CHECK VERTS BAD" << std::endl; assert(0); continue; }
      if (!out1->checkVerts()) { (*ARGS->output) << "OUT1 CHECK VERTS BAD" << std::endl; /* HACK */ continue; }
      if (!out2->checkVerts()) { (*ARGS->output) << "OUT2 CHECK VERTS BAD" << std::endl; assert(0); continue; }
      to_be_deleted.push_back(input);
      //assert(out1->checkVerts());
      //assert(out2->checkVerts());
      to_be_added.push_back(out1);
      to_be_added.push_back(out2);      
      if (out3 != NULL) {
	assert(out3->checkVerts());
	to_be_added.push_back(out3);
      }
      if (out4 != NULL) {
	assert(out4->checkVerts());
	to_be_added.push_back(out4);
      }
    } else {
      assert (out2 == NULL && out3 == NULL && out4 == NULL);
    }
  }
  for (unsigned int i = 0; i < to_be_deleted.size(); i++) {
    arrangement->removeElement(to_be_deleted[i]);
  }
  for (unsigned int i = 0; i < to_be_added.size(); i++) {
    arrangement->addElement(to_be_added[i]);
  }
  vphash.clear();
}

bool LongestSegmentFirst(const std::pair<Vec3f,Vec3f> &a, const std::pair<Vec3f,Vec3f> &b) {
  double da = DistanceBetweenTwoPoints(a.first,a.second);
  double db = DistanceBetweenTwoPoints(b.first,b.second);

  //if (da > 0.99) return false;
  //if (db > 0.99) return true;
  if (da > db) return true;
  return false;
}


void Walls::CreateArrangement(Mesh *arrangement) {
  assert (arrangement->numVertices() == 0);
  assert (arrangement->getElements().size() == 0);

  ConnectWallsIntoChains();
  //PrepareForFingerprints();

  std::vector<std::pair<Vec3f,Vec3f> > segments;

  // create the base of arrangement cells
  assert (scene_radius > 0);
  for (int i = 0; i < CIRCLE_RES; i++) {
    double angle = i*2*M_PI/CIRCLE_RES;
    Vec3f pt = scene_center + Vec3f(scene_radius*sin(angle),0,scene_radius*cos(angle));
    arrangement->addVertex(pt,-1,-1,0,0); 
  }
  Poly *p = new Poly(arrangement);  
  for (int i = 0; i < CIRCLE_RES; i++) {
    p->addVert(i);
  }
  arrangement->addElement(p);
  assert (p->checkVerts());

  double before_sum = p->Area();


  // cut the arrangement for the skylights
  for (unsigned int i = 0; i < skylights.size(); i++) {
    Skylight* s = skylights[i];
    for (int j = 0; j < 4; j++) {
      segments.push_back(std::make_pair((*s)[j],(*s)[(j+1)%4]));
      //segments.push_back(std::make_pair(s.interior_corner(j),s.interior_corner((j+1)%4)));      
    }
  }




  // cut the arrangement for the furniture
  for (unsigned int i = 0; i < furniture.size(); i++) {
    Furniture *f = furniture[i];
    for (int j = 0; j < 4; j++) {
      segments.push_back(std::make_pair((*f)[j],(*f)[(j+1)%4]));
      //segments.push_back(std::make_pair(s.interior_corner(j),s.interior_corner((j+1)%4)));      
    }
  }



  unsigned int num_chains = wall_chains.size();
  for (unsigned int i = 0; i < num_chains; i++) {
    const WallChain &chain = wall_chains[i];
    unsigned int num_quads = chain.quads.size();
    const ConvexQuad &first = chain.quads[0];
    const ConvexQuad &last = chain.quads[num_quads-1];
    Vec3f beginning = first[1]-first[0]; beginning.Normalize();
    Vec3f end = last[1]-last[0]; end.Normalize();
    double dot = beginning.Dot3(end);
    if (my_ugly_args_hack->num_curve_segments > 1 && num_quads == 3 && dot > 0.99) {
      segments.push_back(std::make_pair(first[0],last[1]));
      segments.push_back(std::make_pair(first[3],last[2]));
      segments.push_back(std::make_pair(first[1],first[2]));
      segments.push_back(std::make_pair(last[0],last[3]));
    } else {
      for (unsigned int j = 0; j < num_quads; j++) {
	const ConvexQuad &q = chain.quads[j];
	segments.push_back(std::make_pair(q[0],q[1]));
	segments.push_back(std::make_pair(q[2],q[3]));
	// only one of following is needed...
	segments.push_back(std::make_pair(q[0],q[3]));
	//segments.push_back(std::make_pair(q[1],q[2]));
      }
    }
  }

  // cut on windows
  for (unsigned int i = 0; i < walls.size(); i++) {
    BasicWall *w = walls[i];
    int num_windows = w->getWindows().size();
    if (num_windows == 0) continue;
    int num_quads = w->numConvexQuads();

    for (unsigned int k = 0; k < w->getWindows().size(); k++) {
      const Window &wi = w->getWindows()[k];
      int which_quad = wi.whichQuad();
      assert (which_quad >=0 && which_quad < num_quads); 
      const ConvexQuad &q = w->getConvexQuad(which_quad);

      assert (OnSegment(wi[0],q[0],q[1]));
      assert (OnSegment(wi[1],q[0],q[1]));
      assert (OnSegment(wi[2],q[2],q[3]));
      assert (OnSegment(wi[3],q[2],q[3]));
      segments.push_back(std::make_pair(wi[0],wi[3]));
      segments.push_back(std::make_pair(wi[1],wi[2]));
    }
  }

  sort(segments.begin(),segments.end(),LongestSegmentFirst);
  for (unsigned int i = 0; i < segments.size(); i++) {
    CutArrangement(arrangement,segments[i].first,segments[i].second,-1,-1);
  }

  //  if (ARGS->verbose_mode) {
  (*ARGS->output) << "arrangement has ";
  (*ARGS->output) << arrangement->numVertices() << " verts ";
  (*ARGS->output) << arrangement->getElements().size() << " cells ... "; // << std::endl;
  /// }
  SplitLongEdgesInArrangement(arrangement);

  //  if (ARGS->verbose_mode) {
  (*ARGS->output) << "arrangement has ";
  (*ARGS->output) << arrangement->numVertices() << " verts ";
  (*ARGS->output) << arrangement->getElements().size() << " cells ... "; // << std::endl;
  //}

  double sum = 0;
  //  if (ARGS->verbose_mode) {
  (*ARGS->output) << std::endl;
  //}
  for (elementshashtype::const_iterator foo = arrangement->getElements().begin();
       foo != arrangement->getElements().end();
       foo++) {
    double s = foo->second->Area();
    //(*ARGS->output) << " my area " << s << std::endl;
    sum += s;
  }

  //  if (ARGS->verbose_mode) {
  (*ARGS->output) << "arrangement area sum before: " << before_sum << "  after: " << sum << std::endl;
  //}
  //(*ARGS->output) << "PUT BACK ARRANGEMENT CHECK SUM" << std::endl;
  assert (fabs(sum-before_sum) < EPSILON); 

  LabelArrangementCells(arrangement);

  /*
  if (args->output_corners_file != "") {
    ofstream output(args->output_corners_file.c_str());
    assert (output != NULL);
    for (elementshashtype::const_iterator foo = arrangement->getElements().begin();
	 foo != arrangement->getElements().end();
	 foo++) {
      Poly *p = (Poly*)foo->second;
      if (p->getCellType() == ACT_INTERIOR) {
	assert (p->numVertices() == 4);
	int j = -1;
	for (int i = 0; i < 4; i++) {
	  std::vector<Element*> neighbors = p->getNeighbors(i);
	  assert (neighbors.size() == 1);
	  Poly *p2 = (Poly*)neighbors[0];
	  assert (p2->getCellType() == ACT_WALL);
	  BasicWall *wall_here = p2->getWall();
	  assert (wall_here != NULL);
	  std::string mat = wall_material_name(wall_here->getMaterialIndex(),false); 
	  Vec3f color = wall_materials[wall_here->getMaterialIndex()];
	  (*ARGS->output) << mat << std::endl;
	  color.Print();
	  if (color.x() > 0.99 && color.g() < 0.01 && color.b() < 0.01) {
	    assert (j == -1);
	    j = i;
	  }
	}
	assert (j >= 0);
	for (int i = 0; i < 4; i++) {
	  Vec3f v = p->getMesh()->getVertex((*p)[(j+i)%4])->get();
	  output << v.x() << " " << v.y() << " " << v.z() << std::endl;
	}
	for (int i = 0; i < 4; i++) {
	  Vec3f v = p->getMesh()->getVertex((*p)[(j+i)%4])->get();
	  output << v.x() << " " << RED_HEIGHT << " " << v.z() << std::endl;
	}
      }
    }
  }
  */

  //  if (ARGS->verbose_mode) {
  (*ARGS->output) << "arrangement has ";
  (*ARGS->output) << arrangement->numVertices() << " verts ";
  (*ARGS->output) << arrangement->getElements().size() << " cells ... "; // << std::endl;
  //}
  fflush(stdout);
}

void Walls::CreateRoofOutline(Mesh *arrangement) {
  //  if (ARGS->verbose_mode) {
  (*ARGS->output) << "CREATE ROOF OUTLINE" << std::endl;
  //}
}

bool PointInside(Vec3f pt, Vec3f a, Vec3f b, Vec3f c, Vec3f d) {

  // project to the y=0 plane first
  pt.zero_y();
  a.zero_y();
  b.zero_y();
  c.zero_y();
  d.zero_y();

  assert (fabs(pt.y()) < 0.0001);
  assert (fabs(a.y()) < 0.0001);
  assert (fabs(b.y()) < 0.0001);
  assert (fabs(c.y()) < 0.0001);
  assert (fabs(d.y()) < 0.0001);

  double angle_a = AngleBetween((pt-a),(pt-b));
  double angle_b = AngleBetween((pt-b),(pt-c));
  double angle_c = AngleBetween((pt-c),(pt-d));
  double angle_d = AngleBetween((pt-d),(pt-a));

  double sum = angle_a + angle_b + angle_c + angle_d;

  if (fabs(sum - 2*M_PI) < 0.0001)
    return true;
  else {
    //(*ARGS->output) << "sum: " << sum << std::endl;
    return false;
  }
}


bool ConvexQuad::PointInside(const Vec3f &pt) const {
  Vec3f a = (*this)[0];
  Vec3f b = (*this)[1];
  Vec3f c = (*this)[2];
  Vec3f d = (*this)[3];
  return ::PointInside(pt,a,b,c,d);
}

bool BasicWall::PointInside(const Vec3f &pt) const {
  for (unsigned int i = 0; i < convex_quads.size(); i++) {
    if (convex_quads[i].PointInside(pt)) return true;
  }
  return false;
}

/*
void Walls::LabelArrangementCellsWallsOnly(Mesh *arrangement) {
  Poly_labels.Clear();
  for (elementshashtype::const_iterator foo = arrangement->getElements().begin();
       foo != arrangement->getElements().end();
       foo++) {
    Poly *p = (Poly*)foo->second;
    Vec3f centroid = p->getCentroid();
    // LABEL WALLS ONLY
    for (int i = 0; i < numWalls(); i++) {
      const BasicWall &w = walls[i];
      if (w.PointInside(centroid)) {
	p->setCellType(ACT_WALL,i);  // might be overwritten...
      }
    }
  }
}
*/

void Walls::LabelArrangementCells(Mesh *arrangement) {
  polygon_labels.Clear();
  for (elementshashtype::const_iterator foo = arrangement->getElements().begin();
       foo != arrangement->getElements().end();
       foo++) {
    Poly *p = (Poly*)foo->second;
    Vec3f centroid = p->getCentroid();
    // LABEL WALLS & WINDOWS
    for (int i = 0; i < numWalls(); i++) {
      const BasicWall *w = walls[i];
      if (w->PointInside(centroid)) {
	p->setCellType(ACT_WALL,i);  // might be overwritten...
	int num_windows = w->getWindows().size();
	for (int k = 0; k < num_windows; k++) {
	  const Window &wi = w->getWindows()[k];
	  if (wi.PointInside(centroid)) {
	    if (wi.getType() == "cyan")
	      p->setCellType(ACT_WINDOW_CYAN,i,k);
	    else if (wi.getType() == "magenta")
	      p->setCellType(ACT_WINDOW_MAGENTA,i,k);
	    else {
	      assert (wi.getType() == "yellow");
	      p->setCellType(ACT_WINDOW_YELLOW,i,k);
	    }
	  }
	}
      }
    }

    // LABEL SKYLIGHTS
    for (unsigned int i = 0; i < skylights.size(); i++) {
      Skylight* s = skylights[i];
      Vec3f centroid = p->getCentroid();
      if (s->PointInside(centroid) && p->getCellType() == ACT_OTHER) {
	p->setCellType(ACT_OTHER_SKYLIGHT);
      }
    }

    
    // LABEL FURNITURE
    for (unsigned int i = 0; i < furniture.size(); i++) {
      Furniture *f = furniture[i];
      Vec3f centroid = p->getCentroid();
      if (f->PointInside(centroid)) {
        if (p->getCellType() == ACT_OTHER) {
          p->setCellType(ACT_OTHER_FURNITURE,i,f->getFurnitureType());
        } else if (p->getCellType() == ACT_OTHER_SKYLIGHT) {
          p->setCellType(ACT_OTHER_FURNITURE_SKYLIGHT,i,f->getFurnitureType());

        } else if (p->getCellType() == ACT_OTHER_FURNITURE) {

          p->setCellType(ACT_OTHER_FURNITURE,i,std::max(f->getFurnitureType(),p->getFurnitureType()));

        } else if (p->getCellType() == ACT_OTHER_FURNITURE_SKYLIGHT) {

          p->setCellType(ACT_OTHER_FURNITURE_SKYLIGHT,i,std::max(f->getFurnitureType(),p->getFurnitureType()));

        }
      }
    }

    // COMPUTE ENCLOSED
    double enclosed = ComputePercentEnclosed(p);
    p->setPercentEnclosed(enclosed);
    std::vector<WALL_FINGERPRINT> print = FingerprintPoly(p);
    p->print_X = print;
    polygon_labels.InsertPolygon(p);
  }

  // --------------------------------------------------------------------
  assert (polygon_labels.numPrints() > 0);

  double tuned_enclosure_threshold;
  ComputeSampledEnclosure(args,arrangement,tuned_enclosure_threshold);

  polygon_labels.ConnectedGroupAnalysis();
  if (args->SPLIT_BASED_ON_ENCLOSURE_HISTOGRAM) {
    while (polygon_labels.SplitMixedEnclosureGroups(tuned_enclosure_threshold)) {}
  }

  polygon_labels.TabulateEnclosure(args,tuned_enclosure_threshold);
  //polygon_labels.Print();
  polygon_labels.LabelAdditionalInteriors();
  polygon_labels.LabelInferredWalls();
  polygon_labels.SearchForInteriorInferredWalls();
  polygon_labels.LabelTinyUnusedWalls();
  polygon_labels.PrintStats();
    
  // -------------------------------------------------

  // this should go away eventually...
  for (elementshashtype::const_iterator foo = arrangement->getElements().begin(); foo != arrangement->getElements().end(); foo++) {
    Poly *p2 = (Poly*)foo->second;
    std::vector<WALL_FINGERPRINT> print1 = FingerprintPoly(p2);

    if (p2->getCellType() == ACT_OTHER || p2->getCellType() == ACT_OTHER_SKYLIGHT ||
        p2->getCellType() == ACT_OTHER_FURNITURE || p2->getCellType() == ACT_OTHER_FURNITURE_SKYLIGHT) { 
      if (polygon_labels.IsInterior2(print1,p2)) { 
        if (p2->getCellType() == ACT_OTHER)
	  p2->setCellType(ACT_INTERIOR); 
	else if (p2->getCellType() == ACT_OTHER_SKYLIGHT)
	  p2->setCellType(ACT_INTERIOR_SKYLIGHT);
        else if (p2->getCellType() == ACT_OTHER_FURNITURE)
	  p2->setCellType(ACT_INTERIOR_FURNITURE,p2->getWallID(),p2->getFurnitureType()); 
	else if (p2->getCellType() == ACT_OTHER_FURNITURE_SKYLIGHT)
	  p2->setCellType(ACT_INTERIOR_FURNITURE_SKYLIGHT,p2->getWallID(),p2->getFurnitureType());
	else {
	  assert (0);
	}
      }
    }
  }
}

#define HISTOGRAM_SAMPLES 20

void Walls::ComputeSampledEnclosure(ArgParser *args, Mesh *arrangement, double &tuned_enclosure_threshold) {

  double max_radius = 0;
  for (int i = 0; i < numWalls(); i++) {
    BasicWall *w = walls[i];
    for (int i = 0; i < w->numGoodVerts(); i++) {
      Vec3f v = w->getGoodVert(i);
      double dist = v.Length();
      max_radius = max(max_radius,dist);
    }
  }

  //  if (ARGS->verbose_mode) {
    (*ARGS->output) << "MAX RADIUS = " << max_radius << std::endl;
    (*ARGS->output) << "expected area of interior = " << M_PI*max_radius*max_radius << std::endl;
    (*ARGS->output) << "table area                = " << M_PI*scene_radius*scene_radius << std::endl;
    //}
  double fraction =0.5*M_PI*max_radius*max_radius/double(M_PI*scene_radius*scene_radius);
  //if (ARGS->verbose_mode) {  
    (*ARGS->output) << "fraction                  = " << fraction << std::endl;
    //}
  std::vector<int> enclosure_histogram(HISTOGRAM_SAMPLES,0);

  double radius = getSceneRadius();
  //double incr = radius / 25; // HACK
  double incr = radius / 40; // HACK
  Poly *last = NULL;
  int good=0;
  int bad=0;
  for (double x = -radius; x <= radius; x+=incr) {
    for (double z = -radius; z <= radius; z+=incr) {
      Vec3f v(x,0,z);
      if (v.Length() >= 0.99*radius) continue;
      double enclosure = ComputePercentEnclosed(Vec3f(x,0,z));
      if (last == NULL ||(last != NULL && !last->PointInsideOrOnBorder(v))) {
	for (elementshashtype::const_iterator foo = arrangement->getElements().begin(); 
	     foo != arrangement->getElements().end(); foo++) {  
	  //(*ARGS->output) << "testing element " << std::endl;
	  last = (Poly*)foo->second;
	  if (last->PointInsideOrOnBorder(v)) { 
	    //(*ARGS->output) << "inside " << last << std::endl;
	    break; }
	  last=NULL;
	}
      }
      if (last==NULL) {  bad++; enclosure = -1; }
      else { 
	last->enclosure_samples.push_back(std::make_pair(v,enclosure));
	//(*ARGS->output) << last->enclosure_samples.size() << std::endl;
	if (enclosure < -0.5) { /*std:: cout << "fix_later " << enclosure << std::endl;*/ bad++; continue; }
	good++;
	assert (enclosure >= 0 && enclosure <= 1.0);
	int bucket = (int)((enclosure*HISTOGRAM_SAMPLES));
	if (bucket == HISTOGRAM_SAMPLES) bucket = HISTOGRAM_SAMPLES-1;
	assert (bucket >= 0 && bucket < HISTOGRAM_SAMPLES);
	enclosure_histogram[bucket]++;
      }  
    }
  }

  //============================================
  // PRINT OUT A NICE FORMATTING OF THE DATA

  for (int i = 0; i < HISTOGRAM_SAMPLES; i++) {
    (*ARGS->output) << " " << std::setw(4) << (int)i/(double)HISTOGRAM_SAMPLES*100;
  }
  (*ARGS->output) << std::endl;

  for (int i = 0; i < HISTOGRAM_SAMPLES; i++) {
    (*ARGS->output) << " " << std::setw(4) << enclosure_histogram[i];
  }
  (*ARGS->output) << std::endl;

  tuned_enclosure_threshold = -1.0;

  int sum = 0;
  for (int i = 0; i < HISTOGRAM_SAMPLES; i++) {
    sum += enclosure_histogram[i];
    double tmp = 100.0 - (sum * 100.0/good);
    if (tuned_enclosure_threshold < -0.1 && tmp < 100*fraction) tuned_enclosure_threshold = i/double(HISTOGRAM_SAMPLES);
    (*ARGS->output) << " " << std::setw(4) << (int)tmp;
  }
  (*ARGS->output) << std::endl;

  tuned_enclosure_threshold = min2(0.5,tuned_enclosure_threshold);

  //  assert (tuned_enclosure_threshold > 0.1 && tuned_enclosure_threshold < 0.99);


  // if (ARGS->verbose_mode) {
  (*ARGS->output) << "ANSWER_THRESHOLD = " << tuned_enclosure_threshold << std::endl;

  (*ARGS->output) << "bad" << bad << "   good" << good << std::endl;
  (*ARGS->output) << "Created " << bad + good  << " samples of enclosure." << std::endl;
  //}
  //(*ARGS->output) << "Created " << sampled_enclosure.size() << " samples of enclosure." << std::endl;
}

bool Walls::InCenterOfSkylight(const Vec3f &v) const {
  for (unsigned int i = 0; i < skylights.size(); i++) {
    Skylight *s = skylights[i];
    if (s->PointInside(v)) {
      if (PointInside(v, s->interior_corner(0),s->interior_corner(1),s->interior_corner(2),s->interior_corner(3)))
	return true;
      return false;
    }
  }
  assert(0);
  return false;
}

bool Walls::InSkylight(const Vec3f &v) const {
  for (unsigned int i = 0; i < skylights.size(); i++) {
    Skylight *s = skylights[i];
    if (s->PointInside(v)) {
      return true;
    }
  }
  return false;
}

bool Walls::SegmentIntersectsWall(unsigned int i, Vec3f beginning, Vec3f end) const {
  assert (i < walls.size());
  return walls[i]->SegmentIntersectsWall(beginning,end);
}
bool Walls::SegmentIntersectsWallTop(unsigned int i, Vec3f beginning, Vec3f end) const {
  assert (i < walls.size());
  return walls[i]->SegmentIntersectsWallTop(beginning,end);
}

#if 1
bool BasicWall::SegmentIntersectsWall(Vec3f beginning, Vec3f end) const {
  //  Vec3f h = Vec3f(0,getHeight2(),0);
  //Vec3f b = Vec3f(0,getBottomEdge(),0);
  Vec3f b = Vec3f(0,0,0);
  Vec3f h[4] = { Vec3f(0,getHeight2(0),0),
		 Vec3f(0,getHeight2(1),0),
		 Vec3f(0,getHeight2(2),0),
		 Vec3f(0,getHeight2(3),0) };
  unsigned int num_quads = convex_quads.size();
  for (unsigned int i = 0; i < num_quads; i++) {
    const ConvexQuad& q = convex_quads[i];
    
    if (Quad::IntersectsSegment(q[1]+h[1],q[0]+h[0],q[3]+h[3],q[2]+h[2],Vec3f(0,1,0),beginning,end) || // top
	Quad::IntersectsSegment(q[1]+b,q[0]+b,q[0]+h[0],q[1]+h[1],q.getNormal(0),beginning,end) || // front
	Quad::IntersectsSegment(q[3]+b,q[2]+b,q[2]+h[2],q[3]+h[3],q.getNormal(2),beginning,end)) {
      return true;
    }
    
    if (i == 0) { // one side
      if (Quad::IntersectsSegment(q[0]+b,q[3]+b,q[3]+h[3],q[0]+h[0],q.getNormal(3),beginning,end))
	return true;
    }
    
    if (i == num_quads-1) { // other side
      if (Quad::IntersectsSegment(q[2]+b,q[1]+b,q[1]+h[1],q[2]+h[2],q.getNormal(1),beginning,end))
	return true;
    } 
  }
  return false;
}
#else
bool BasicWall::SegmentIntersectsWall(Vec3f beginning, Vec3f end) const {
  //  Vec3f h = Vec3f(0,getHeight2()*1.05,0);
  //Vec3f b = Vec3f(0,getBottomEdge(),0);
  Vec3f h[4] = { Vec3f(0,getHeight2(0),0),
		 Vec3f(0,getHeight2(1),0),
		 Vec3f(0,getHeight2(2),0),
		 Vec3f(0,getHeight2(3),0) };
  Vec3f b = Vec3f(0,0,0);
  unsigned int num_quads = convex_quads.size();
  for (unsigned int i = 0; i < num_quads; i++) {
    const ConvexQuad& q = convex_quads[i];
    
    if (//Quad::IntersectsSegment(q[1]+h,q[0]+h,q[3]+h,q[2]+h,Vec3f(0,1,0),beginning,end) || // top
	Quad::IntersectsSegment(q[1]+b,q[0]+b,q[0]+h[0],q[1]+h[1],q.getNormal(0),beginning,end)) //|| // front
	//Quad::IntersectsSegment(q[3]+b,q[2]+b,q[2]+h,q[3]+h,q.getNormal(2),beginning,end)) 
      {  // back side should be longest...
      return true;
    }
    
    if (i == 0) { // one side
      if (Quad::IntersectsSegment(q[0]+b,q[3]+b,q[3]+h[3],q[0]+h[0],q.getNormal(3),beginning,end))
	return true;
    }
    
    if (i == num_quads-1) { // other side
      if (Quad::IntersectsSegment(q[2]+b,q[1]+b,q[1]+h[1],q[2]+h[2],q.getNormal(1),beginning,end))
	return true;
    } 
  }
  return false;
}
#endif

bool BasicWall::SegmentIntersectsWallTop(Vec3f beginning, Vec3f end) const {
  //  Vec3f h = Vec3f(0,getHeight2(),0);
  Vec3f h[4] = { Vec3f(0,getHeight2(0),0),
		 Vec3f(0,getHeight2(1),0),
		 Vec3f(0,getHeight2(2),0),
		 Vec3f(0,getHeight2(3),0) };
  Vec3f b = Vec3f(0,getBottomEdge(),0);
  unsigned int num_quads = convex_quads.size();
  for (unsigned int i = 0; i < num_quads; i++) {
    const ConvexQuad& q = convex_quads[i];
    if (Quad::IntersectsSegment(q[1]+h[1],q[0]+h[0],q[3]+h[3],q[2]+h[2],Vec3f(0,1,0),beginning,end)) { 
      return true;
    } 
  }
  return false;
}

