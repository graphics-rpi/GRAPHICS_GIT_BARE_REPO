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

#include "polygon.h"
#include "mesh.h"
#include "render.h"
#include "meshmanager.h"
#include "glCanvas.h"
#include "triangle.h"
#include "wall.h"
#include "walls.h"
#include "edge.h"

#include "argparser.h"
extern ArgParser *ARGS;

// defined in wall_fingerprint.cpp
Vec3f GLOBAL_FLOOR_PLAN_COLOR(const std::vector<WALL_FINGERPRINT> &print_X, int which_subgroup);

extern ArgParser *my_ugly_args_hack;

void Poly::addVert(int v) { 
  assert (getMesh() != NULL);
  assert (v >= 0 && v < (int)getMesh()->numVertices());
  verts.push_back(v); 
}


void Poly::StealVertices(Poly*& taker, Poly*& giver, int vert) {
  assert (taker != NULL);
  assert (giver != NULL);

  assert (taker->checkVerts());
  assert (giver->checkVerts());

  if (!taker->checkVerts()) { (*ARGS->output) << "TAKER CHECK VERTS BAD" << std::endl; return; } 
  if (!giver->checkVerts()) { (*ARGS->output) << "GIVER CHECK VERTS BAD" << std::endl; return; } 

  Mesh *m = taker->getMesh();
  assert (m == giver->getMesh());

  //taker->Print("taker");
  //giver->Print("giver");

  unsigned int num_verts_taker = taker->numVertices();
  unsigned int num_verts_giver = giver->numVertices();
  assert (num_verts_taker >= 3);
  assert (num_verts_giver >= 3);
  if (num_verts_giver == 3) return;

  int which_vertex_taker = taker->WhichVertex(vert);
  int which_vertex_giver = giver->WhichVertex(vert);
  
  int taker_minus = (which_vertex_taker+num_verts_taker-1)%num_verts_taker;
  int taker_plus = (which_vertex_taker+1)%num_verts_taker;
  int giver_minus = (which_vertex_giver+num_verts_giver-1)%num_verts_giver;
  int giver_plus = (which_vertex_giver+1)%num_verts_giver;

  //(*ARGS->output) << "taker: " << (*taker)[taker_minus] << " " << (*taker)[which_vertex_taker] << " " << (*taker)[taker_plus] << std::endl;
  //(*ARGS->output) << "giver: " << (*giver)[giver_minus] << " " << (*giver)[which_vertex_giver] << " " << (*giver)[giver_plus] << std::endl;

  assert ((*taker)[which_vertex_taker] == (*giver)[which_vertex_giver]);
  assert ((*taker)[which_vertex_taker] == vert);

  //==============================================================
  if ((*taker)[taker_minus] == (*giver)[giver_plus]) {
    //(*ARGS->output) << "CASE A" << std::endl;
    Vec3f a = m->getVertex((*taker)[taker_plus])->get();
    Vec3f b = m->getVertex(vert)->get();
    Vec3f c = m->getVertex((*giver)[giver_minus])->get();
    int best_i = -1;
    double best_giver_angle = -1;
    for (int i = giver_plus; i != giver_minus; i = (i+1)%num_verts_giver) {
      Vec3f q = m->getVertex((*giver)[i])->get();
      Vec3f taker_cross, giver_cross;
      Vec3f va = a-b; va.Normalize();
      Vec3f vq = q-b; vq.Normalize();
      Vec3f vc = c-b; vc.Normalize();
      Vec3f::Cross3(taker_cross,va,vq);
      Vec3f::Cross3(giver_cross,vq,vc);
      taker_cross.Normalize();
      giver_cross.Normalize();
      if (taker_cross.Dot3(Vec3f(0,1,0)) < 0.9 || taker_cross.Dot3(Vec3f(0,1,0)) < 0.9) { continue; }
      double taker_angle = AngleBetweenNormalized(va,vq);
      double giver_angle = AngleBetweenNormalized(vq,vc);
      if (taker_angle > 160*M_PI/180.0) { continue; }
      if (best_i == -1 || fabs(giver_angle-120*M_PI/180) < fabs(best_giver_angle-120*M_PI/180)) {
	best_i = i;
	best_giver_angle = giver_angle;
      }
      //(*ARGS->output) << "ta: " << taker_angle*180/M_PI << "  ga: " << giver_angle*180/M_PI << std::endl;
    }
    //(*ARGS->output) << "best " << best_i << " " << best_giver_angle*180/M_PI << std::endl;
    if (best_i != -1 && best_i != 0) { 
      // make the new elements
      Poly *new_taker = new Poly(m);

      new_taker->verts.push_back(taker->verts[which_vertex_taker]);
      for (int i = taker_plus; i != taker_minus; i = (i+1)%num_verts_taker) {
      	new_taker->verts.push_back(taker->verts[i]); }
      for (int i = giver_plus; i != best_i; i = (i+1)%num_verts_giver) {
	new_taker->verts.push_back(giver->verts[i]); }
      new_taker->verts.push_back(giver->verts[best_i]);

      Poly *new_giver = new Poly(m);
      new_giver->verts.push_back(giver->verts[giver_minus]);
      new_giver->verts.push_back(giver->verts[which_vertex_giver]);
      for (int i = best_i; i != giver_minus; i = (i+1)%num_verts_giver) {
	new_giver->verts.push_back(giver->verts[i]); }
      assert(new_taker->checkVerts());
      assert(new_giver->checkVerts());


      /*
      new_taker->Print("new_taker");
      new_giver->Print("new_giver");
      (*ARGS->output) << " counts " << std::endl;
      (*ARGS->output) << new_taker->numVertices() << std::endl;
      (*ARGS->output) << new_giver->numVertices() << std::endl;
      (*ARGS->output) << taker->numVertices() << std::endl;
      (*ARGS->output) << giver->numVertices() << std::endl;
      */
      assert (new_taker->numVertices() + new_giver->numVertices() ==
	      taker->numVertices() + giver->numVertices());
      // replace the old versions
      delete taker;
      delete giver;
      taker = new_taker;
      giver = new_giver;
    }
  } 
  //==============================================================
  else {
    assert ((*taker)[taker_plus] == (*giver)[giver_minus]);
    //(*ARGS->output) << "CASE B" << std::endl;
    Vec3f a = m->getVertex((*giver)[giver_plus])->get();
    Vec3f b = m->getVertex(vert)->get();
    Vec3f c = m->getVertex((*taker)[taker_minus])->get();
    int best_i = -1;
    double best_giver_angle = -1;
    for (int i = (giver_plus+1)%num_verts_giver; i != which_vertex_giver; i = (i+1)%num_verts_giver) {
      Vec3f q = m->getVertex((*giver)[i])->get();
      Vec3f taker_cross, giver_cross;
      Vec3f va = a-b; va.Normalize();
      Vec3f vq = q-b; vq.Normalize();
      Vec3f vc = c-b; vc.Normalize();
      Vec3f::Cross3(giver_cross,va,vq);
      Vec3f::Cross3(taker_cross,vq,vc);
      taker_cross.Normalize();
      giver_cross.Normalize();
      if (taker_cross.Dot3(Vec3f(0,1,0)) < 0.9 || taker_cross.Dot3(Vec3f(0,1,0)) < 0.9) { continue; }
      double giver_angle = AngleBetweenNormalized(va,vq); 
      double taker_angle = AngleBetweenNormalized(vq,vc); 
      if (taker_angle > 160*M_PI/180.0) { continue; }
      if (best_i == -1 || fabs(giver_angle-120*M_PI/180) < fabs(best_giver_angle-120*M_PI/180)) {
	best_i = i;
	best_giver_angle = giver_angle;
      }
      //(*ARGS->output) << "ta: " << taker_angle*180/M_PI << "  ga: " << giver_angle*180/M_PI << std::endl;
    }
    //    (*ARGS->output) << "best " << best_i << " " << best_giver_angle*180/M_PI << std::endl;
    if (best_i != -1 && best_i != 0) { 
      // make the new elements
      Poly *new_taker = new Poly(m);

      new_taker->verts.push_back(taker->verts[taker_plus]); 
      for (int i = (taker_plus+1)%num_verts_taker; i != taker_plus; i = (i+1)%num_verts_taker) {
	new_taker->verts.push_back(taker->verts[i]); }
      for (int i = best_i; i != giver_minus; i = (i+1)%num_verts_giver) {
	new_taker->verts.push_back(giver->verts[i]); }

      Poly *new_giver = new Poly(m);
      new_giver->verts.push_back(giver->verts[which_vertex_giver]);
      for (int i = giver_plus; i != best_i; i = (i+1)%num_verts_giver) {
	new_giver->verts.push_back(giver->verts[i]); }
      new_giver->verts.push_back(giver->verts[best_i]);
      /*
      (*ARGS->output) << " counts " << std::endl;
      (*ARGS->output) << new_taker->numVertices() << std::endl;
      (*ARGS->output) << new_giver->numVertices() << std::endl;
      (*ARGS->output) << taker->numVertices() << std::endl;
      (*ARGS->output) << giver->numVertices() << std::endl;
      */
      assert(new_taker->checkVerts());
      assert(new_giver->checkVerts());
      /*
      new_taker->Print("new_taker");
      new_giver->Print("new_giver");
      */
      assert (new_taker->numVertices() + new_giver->numVertices() ==
	      taker->numVertices() + giver->numVertices());

      // replace the old versions
      delete taker;
      delete giver;
      taker = new_taker;
      giver = new_giver;
    }
  }
  //==============================================================
  //(*ARGS->output) << "VERT " << vert << std::endl;
}

#define CV_DISTANCE_TOLERANCE 0.00000001

bool Poly::checkVerts() const { 
  //(*ARGS->output) << "CV" << std::endl;
  Mesh *m = getMesh();
  bool answer = true;
  unsigned int num_verts = numVertices();
  assert (num_verts >= 3);
  for (unsigned int i = 0; i < num_verts; i++) {
    Vec3f a = m->getVertex((*this)[i])->get();
    Vec3f b = m->getVertex((*this)[(i+1)%num_verts])->get();
    Vec3f c = m->getVertex((*this)[(i+2)%num_verts])->get();
    Vec3f vab = a-b;
    double len_ab = vab.Length();
     if (len_ab < CV_DISTANCE_TOLERANCE) {
      //(*ARGS->output) << "verts too close ab" << len_ab << std::endl;
      answer = false;
    }
    Vec3f vbc = b-c;
    double len_bc = vbc.Length();
    if (len_bc < CV_DISTANCE_TOLERANCE) {
      //(*ARGS->output) << "verts too close bc" << len_bc << std::endl;
      answer = false;
    }
    Vec3f cross;
    vab.Normalize();
    vbc.Normalize();
    //vab.Print("vab");
    //vbc.Print("vac");
    if (fabs(vab.Dot3(vbc))> 0.999999) {
      //      (*ARGS->output) << "parallel" << std::endl;
      //vab.Print("vab");
      //vbc.Print("vbc");
      //answer = false;
    }
    Vec3f::Cross3(cross,vab,vbc);
    cross.Normalize();
    //cross.Print("cross");
    if (cross.Dot3(Vec3f(0,1,0)) < 0.99) {
      (*ARGS->output) << "cross wrong way" << std::endl;
      cross.Print("cross");
      answer = false;
    }
  }
  //(*ARGS->output) << "STD::ENDL" << std::endl;
  return answer;
}


#define RAMP_TOP 0.25
#define RAMP_BOTTOM (0.25*RAMP_TOP/2.0)

void Poly::Paint(int flag, Walls *walls) {

  Mesh *m = getMesh();
  assert (m != NULL); // && m->isPolyMesh());
  Vec3f normal;
  ::computeNormal(m->getVertex((*this)[0])->get(),
		  m->getVertex((*this)[1])->get(),
		  m->getVertex((*this)[2])->get(),
		  normal);
  glNormal3f(normal.x(),normal.y(),normal.z());

  if (flag == 0) { // arrangement % enclosed
    if (percent_enclosed == -1)
      glColor3f(1,0,1);
    else {
      assert (percent_enclosed >=0 && percent_enclosed <= 1);
      glColor3f(percent_enclosed,percent_enclosed,percent_enclosed);
    }
  }
  else if (flag == 1) {  // arrangement type
    ARRANGEMENT_CELL_TYPE t = getCellType();
    if (t == ACT_INTERIOR) {
      glColor3f(0.5,0.5,0.5);
    } else if (t == ACT_WALL) {
      glColor3f(0,0,0);
    } else if (t == ACT_WINDOW_MAGENTA) {
      glColor3f(1,0,1);
    } else if (t == ACT_WINDOW_YELLOW) {
      glColor3f(1,1,0);
    } else if (t == ACT_WINDOW_CYAN) {
      glColor3f(0,1,1);
    } else if (t == ACT_INTERIOR_SKYLIGHT) {
      glColor3f(0.9,0.9,1);
    } else if (t == ACT_OTHER_SKYLIGHT) {
      glColor3f(0.5,0.5,1);
    } else if (t == ACT_BOUNDARY) {
      glColor3f(1,0,0);
    } else if (t == ACT_OTHER) {
      glColor3f(0.5,0.9,0.5);

    } else if (t == ACT_INTERIOR_FURNITURE) {
      glColor3f(0.8,0.4,0.0);
    } else if (t == ACT_INTERIOR_FURNITURE_SKYLIGHT) {
      glColor3f(0.8,0.4,0.8);

    } else if (t == ACT_OTHER_FURNITURE) {
      glColor3f(1.0,0.2,0.0);
    } else if (t == ACT_OTHER_FURNITURE_SKYLIGHT) {
      glColor3f(1.0,0.2,0.6);

    } else {
      assert (0);
      glColor3f(0,0,0);
    }
    
  } else if (flag == 2) { // arrangement fingerprints
    Vec3f c = FingerprintColor(my_ugly_args_hack,print_X,which_subgroup);
    glColor3f(c.x(),c.y(),c.z());
  } else if (flag == 3) { // arrangement num_middles
    Vec3f c = NumMiddlesColor(print_X);
    glColor3f(c.x(),c.y(),c.z());
  } else if (flag == 4) { // floor plan

#if 1
    ARRANGEMENT_CELL_TYPE t = getCellType();
    if (t == ACT_WINDOW_MAGENTA || 
	t == ACT_WINDOW_YELLOW || 
	t == ACT_WINDOW_CYAN) {
      return; // don't draw anything for windows, they are drawn later
    }
    bool interior = walls->polygon_labels.IsInterior2(print_X,this);
    bool mixed = walls->polygon_labels.IsMixed(print_X,this);
    bool inferredwall = walls->polygon_labels.IsInferredWall(print_X,this);

    bool shortinferredwall = false;

    bool shortwall = false;
    if (this->isWall()) {
      double height = walls->getWall(this->getWallID())->getMaxHeight();
      if (height == GREEN_HEIGHT) {
	shortwall = true;
      }
    }


    if (inferredwall) {
      //      (*ARGS->output) << "IN PAINT" << std::endl;
      shortinferredwall = AnalyzeForShortInferredWall(walls);
    }
    
    if (t == ACT_WALL) {
      if (my_ugly_args_hack->floor_plan_walls_vis) {
	if (inferredwall) {
	  if (shortwall) {
	    glColor3f(0.4,0.1,0.1);  // PINK GREY
	  } else {
	    glColor3f(0.1,0.1,0.1);  // DARK GREY
	  }
	} else if (interior) {
	  glColor3f(0.0,0.7,0.1);  // DARK GREEN
	} else {
	  glColor3f(0.0,0.1,0.7);  // DARK BLUE
	} 
      } else {
	glColor3f(0.1,0.1,0.1);    // DARK GREY
      }
    } else {
      if (inferredwall) {
	if (my_ugly_args_hack->floor_plan_walls_vis) {	  
	  if (shortinferredwall) {
	    glColor3f(1.0,0.7,0.7);   // PINK
	  } else {
	    glColor3f(0.7,0.1,0.1);   // DARK RED
	  }
	} else {
	  glColor3f(0.1,0.1,0.1);   // DARK GREY
	}
      } else if (interior) {
	if (mixed) {
	  glColor3f(0.9,0.9,0);       // YELLOW
	} else {
	  glColor3f(0.7,0.7,0.7);     // MED GREY
	}
      } else {
	if (mixed) {
	  glColor3f(1,1,0.8);         // PALE YELLOW
	} else {
	  glColor3f(0.97,0.97,0.97);  // PALE PALE GREY
	}
      }
    }
#else
    // print_X
    // which_subgroup

    Vec3f color = GLOBAL_FLOOR_PLAN_COLOR(print_X,which_subgroup);
    glColor3d(color.x(),color.y(),color.z());

#endif


  } else if (flag == 5) { // 2D walls
    ARRANGEMENT_CELL_TYPE t = getCellType();
    if (t == ACT_INTERIOR) {
      glColor3f(0.95,0.95,0.95);
    } else if (t == ACT_WALL) {
      int w = getWallID();
      BasicWall *wall = walls->getWall(w);
      Vec3f color = wall->getColor();
      color = 0.5 * color; // make it a little darker
      glColor3f(color.r(),color.g(),color.b());
    } else if (t == ACT_WINDOW_MAGENTA) {
      glColor3f(0.5,0,0.5);
    } else if (t == ACT_WINDOW_YELLOW) {
      glColor3f(0.5,0.5,0);
    } else if (t == ACT_WINDOW_CYAN) {
      glColor3f(0,0.5,0.5);
    } else if (t == ACT_INTERIOR_SKYLIGHT) {
      glColor3f(0.95,0.95,0.95);
    } else if (t == ACT_OTHER_SKYLIGHT) {
      glColor3f(0.5,0.5,0.95);
    } else if (t == ACT_BOUNDARY) {
      glColor3f(0.95,0.95,0.95);
    } else if (t == ACT_OTHER) {
      glColor3f(0.95,0.95,0.95);
    } else {
      assert (0);
      glColor3f(0,0,0);
    }
  } else if (flag == 6) { // army floor plan
    ARRANGEMENT_CELL_TYPE t = getCellType();
    if (t == ACT_WALL) {
      int w = getWallID();
      BasicWall *wall = walls->getWall(w);
      if (wall->IsRamp()) {
 
	glBegin(GL_TRIANGLE_FAN);
	Vec3f pos;
	int num_verts = this->numVertices();
	double h;
	
	pos = m->getVertex((*this)[0])->get();    
	h = wall->getRampHeight(pos); h = (RAMP_TOP-RAMP_BOTTOM) * h / (2*INCH_IN_METERS) + RAMP_BOTTOM;
	(*ARGS->output) << "H " << h << std::endl;
	glColor3f(h,h,h);  
	glVertex3f(pos.x(),pos.y(),pos.z());
	pos = m->getVertex((*this)[1])->get();  
	h = wall->getRampHeight(pos); h = (RAMP_TOP-RAMP_BOTTOM) * h / (2*INCH_IN_METERS) + RAMP_BOTTOM;
	(*ARGS->output) << "H " << h << std::endl;
	glColor3f(h,h,h);  
	glVertex3f(pos.x(),pos.y(),pos.z());
	for (int i = 2; i < num_verts; i++) {
	  pos = m->getVertex((*this)[i])->get();    
	  h = wall->getRampHeight(pos); h = (RAMP_TOP-RAMP_BOTTOM) * h / (2*INCH_IN_METERS) + RAMP_BOTTOM;
	(*ARGS->output) << "H " << h << std::endl;
	  glColor3f(h,h,h);
	  glVertex3f(pos.x(),pos.y(),pos.z());
	}
	glEnd();
	return;

     } else {
	double h = wall->getHeight2(0);
	if (h > 2.5 * INCH_IN_METERS) {
	  glColor3f(1,1,1);
	} else {
	  // platform
	  glColor3f(RAMP_TOP,RAMP_TOP,RAMP_TOP);
	}
      }
    } else {

      glColor3f(0,0,0);
    }
  } else {
    assert(0);
    exit(0);
  }


  glBegin(GL_TRIANGLE_FAN);
  Vec3f pos;
  int num_verts = this->numVertices();
  pos = m->getVertex((*this)[0])->get();    glVertex3f(pos.x(),pos.y(),pos.z());
  pos = m->getVertex((*this)[1])->get();  glVertex3f(pos.x(),pos.y(),pos.z());
  for (int i = 2; i < num_verts; i++) {
    pos = m->getVertex((*this)[i])->get();    glVertex3f(pos.x(),pos.y(),pos.z());
  }
  glEnd();
}


bool Poly::AnalyzeForShortInferredWall(Walls *walls, bool recurse) {

  (*ARGS->output) << "in Poly::AnalyzeForShortInferredWall " << recurse << std::endl;

  if (recurse==false) {
    
    (*ARGS->output) << "before this->isMarked() " << this->isMarked() << std::endl;
    //assert (!this->isMarked());

    Markable::NextMark();    

    (*ARGS->output) << "after this->isMarked() " << this->isMarked() << std::endl;
    //assert (!this->isMarked());
  }

  assert (!this->isMarked());
  this->Mark();

  ARRANGEMENT_CELL_TYPE t = getCellType();
  if (t == ACT_WALL) return false;
  if (t == ACT_WINDOW_MAGENTA) return false;
  if (t == ACT_WINDOW_YELLOW) return false;
  if (t == ACT_WINDOW_CYAN) return false;
  if (t != ACT_OTHER) return false;
  assert (t != ACT_WALL);
  //  (*ARGS->output) << t << std::endl;
  assert (t == ACT_OTHER);
  if (!walls->polygon_labels.IsInferredWall(print_X,this)) return false;
  
  //(*ARGS->output) << "-------------------" << std::endl;
  int num_vertices = numVertices();
  bool no_other = true;
  bool short_wall = false;

  std::vector<Poly*> recurse_list;

  for (int i = 0; i < num_vertices; i++) {

    Edge *e = get_my_edge(i);
    assert (e != NULL);
    const std::vector<Edge*>& opps = e->getOpposites();
    if (opps.size() == 0) continue;
    assert (opps.size() == 1);
    Edge *opp = opps[0];
    assert (opp != NULL);
    Element *opp_e = opp->getElement();
    assert (opp_e != NULL);
    assert (opp_e->isAPolygon());
    Poly* p = (Poly*)opp_e;
    
    ARRANGEMENT_CELL_TYPE t = p->getCellType();
    if (t == ACT_INTERIOR) {
      //(*ARGS->output) << "INTERIOR!" << " ";


    } else if (t == ACT_WALL) {
      //(*ARGS->output) << "WALL!" << " ";
      if (p->isWall()) {
	double height = walls->getWall(p->getWallID())->getMaxHeight();

	//(*ARGS->output) << "HEIGHT " << height << std::endl;

	if (height == GREEN_HEIGHT) {
	  short_wall = true;
	  //(*ARGS->output) << "SHORT WALL****************" << " ";
	}
      }

    } else if (t == ACT_WINDOW_MAGENTA) {
      //(*ARGS->output) << "WINDOW MAGENTA" << " ";
    } else if (t == ACT_WINDOW_YELLOW) {
      //(*ARGS->output) << "WINDOW YELLOW" << " ";
    } else if (t == ACT_WINDOW_CYAN) {
      //(*ARGS->output) << "WINDOW CYAN" << " ";
    } else if (t == ACT_INTERIOR_SKYLIGHT) {
      //(*ARGS->output) << "INTIOER SKYLIGHT" << " ";
    } else if (t == ACT_OTHER_SKYLIGHT) {
      //(*ARGS->output) << "OTHER STYLIGHT" << " ";
    } else if (t == ACT_BOUNDARY) {
      //(*ARGS->output) << "BOUNDARY" << " ";
    } else if (t == ACT_OTHER) {
      //(*ARGS->output) << "OTHER!" << " ";
      no_other = false;
      if (!p->isMarked()) {
	recurse_list.push_back(p);
      }
      //if (walls->polygon_labels.IsInferredWall(print_X,this)) {
	
      //} else 
      if (NumMiddles(print_X) > 1) {
	return false;

	//	(*ARGS->output) << "NOT AN INFERRED WALL!" << std::endl;
	//return false;
      }
    } else {
      assert (0);
    }
  }

  //return short_wall;
  //  return true;

  if (short_wall) {
    return true;
  }

  //  assert (no_other == false);

  if (recurse_list.size() == 0) {
    return false;
  }
  
  //(*ARGS->output) << "ATTEMPT RECURSE" << std::endl;
  for (unsigned int i = 0; i < recurse_list.size(); i++) {
    if (!recurse_list[i]->isMarked()) {
      bool answer = recurse_list[i]->AnalyzeForShortInferredWall(walls,true);
      if (answer) return true;
    }
  }

  return false;

  //return short_wall;

  if (no_other) {
    //(*ARGS->output) << "************************" << std::endl;
    return true;
  } else {
    (*ARGS->output) << std::endl;
  }
  return false;
}

void Walls::PaintFloorPlanWindows() const {
  //  (*ARGS->output) << "PAINT WINDOWS" << std::endl;
  unsigned int num_walls = walls.size();
  for (unsigned int i = 0; i < num_walls; i++) {
    const BasicWall *w = walls[i];
    unsigned int num_windows = w->getWindows().size();
    for (unsigned int j = 0; j < num_windows; j++) {
      const Window &wi = w->getWindows()[j];
      Vec3f a = 0.65*wi[0] + 0.35*wi[3];
      Vec3f b = 0.65*wi[1] + 0.35*wi[2];
      Vec3f c = 0.35*wi[1] + 0.65*wi[2];
      Vec3f d = 0.35*wi[0] + 0.65*wi[3];

      Vec3f a2 = 0.8*wi[0] + 0.2*wi[3];
      Vec3f b2 = 0.8*wi[1] + 0.2*wi[2];
      Vec3f c2 = 0.2*wi[1] + 0.8*wi[2];
      Vec3f d2 = 0.2*wi[0] + 0.8*wi[3];

      //const Vec3f &centroid = wi.getCentroid();
      //std::vector<WALL_FINGERPRINT> print = FingerprintPoint(centroid);
      Vec3f pt1 = 0.51*wi[0] + 0.51*wi[1] - 0.01*wi[2] - 0.01*wi[3];
      Vec3f pt2 = 0.51*wi[2] + 0.51*wi[3] - 0.01*wi[0] - 0.01*wi[1];
      std::vector<WALL_FINGERPRINT> print1 = FingerprintPoint(pt1);
      std::vector<WALL_FINGERPRINT> print2 = FingerprintPoint(pt2);
      //printPrint(print);
      //printPrint(print1);
      //printPrint(print2);

      assert(HandleGLError("before"));
  
      glBegin(GL_QUADS);

      // window pane centers
      glColor3f(1,1,1);
      glVertex3f(a.x(),a.y(),a.z());
      glVertex3f(d.x(),d.y(),d.z());
      glVertex3f(c.x(),c.y(),c.z());
      glVertex3f(b.x(),b.y(),b.z());

      // window pane edges
      glColor3f(0,0,0);
      glVertex3f(a2.x(),a2.y(),a2.z());
      glVertex3f(a.x(),a.y(),a.z());
      glVertex3f(b.x(),b.y(),b.z());
      glVertex3f(b2.x(),b2.y(),b2.z());

      glVertex3f(c2.x(),c2.y(),c2.z());
      glVertex3f(c.x(),c.y(),c.z());
      glVertex3f(d.x(),d.y(),d.z());
      glVertex3f(d2.x(),d2.y(),d2.z());

      // window sill side a
      if (polygon_labels.IsInterior2(print1,0)) { //NULL)/*.find(print1) != polygon_labels.interiors.end()*/) {
	//      if (polygon_labels.IsInterior(print1,0)/*.find(print1) != polygon_labels.interiors.end()*/) {
	glColor3f(0.7,0.7,0.7);
      } else {
	glColor3f(0.97,0.97,0.97);
      }
      glVertex3f(wi[0].x(),wi[0].y(),wi[0].z());
      glVertex3f(a2.x(),a2.y(),a2.z());
      glVertex3f(b2.x(),b2.y(),b2.z());
      glVertex3f(wi[1].x(),wi[1].y(),wi[1].z());

      // window sill side b
      if (polygon_labels.IsInterior2(print2,0)) { //NULL)/*.find(print2) != polygon_labels.interiors.end()*/) {
	//if (polygon_labels.IsInterior(print2,0)/*.find(print2) != polygon_labels.interiors.end()*/) {
	glColor3f(0.7,0.7,0.7);
      } else {
	glColor3f(0.97,0.97,0.97);
      }
      glVertex3f(d2.x(),d2.y(),d2.z());
      glVertex3f(wi[3].x(),wi[3].y(),wi[3].z());
      glVertex3f(wi[2].x(),wi[2].y(),wi[2].z());
      glVertex3f(c2.x(),c2.y(),c2.z());
      glEnd();

      assert(HandleGLError("after"));

    }
  }
}


Vec3f Poly::getCentroid() {
  Vec3f center(0,0,0);
  Mesh *m = getMesh();
  int num_verts = this->numVertices();
  for (int j = 0; j < num_verts; j++) {
    center += m->getVertex((*this)[j])->get();
  }
  center /= num_verts;
  return center;
}

Vec3f Poly::getOffsetVertex(int i) {
  Mesh *m = getMesh();
  int num_verts = this->numVertices();
  assert (i >= 0 && i < num_verts);

  Vec3f center = getCentroid();

  Vec3f offset = center - m->getVertex((*this)[i])->get();
  
#if 1
  offset *= 0.0;
#else
  offset *= 0.8;
  if (offset.Length() > 0.01) {
    offset.Normalize();
    offset *= 0.01;
  }
#endif
  
  Vec3f answer = m->getVertex((*this)[i])->get() + offset;
  
  return answer;
}

void Poly::PaintEdges() {
  int num_verts = this->numVertices();
  Vec3f pos;
  for (int i = 0; i < num_verts; i++) {
    pos = getOffsetVertex(i);   
    glVertex3f(pos.x(),pos.y(),pos.z());
    pos = getOffsetVertex((i+1)%num_verts);
    glVertex3f(pos.x(),pos.y(),pos.z());
  }
}

void Poly::PaintBoundaryEdges() {
  int num_verts = this->numVertices();
  Vec3f pos;
  for (int i = 0; i < num_verts; i++) {
    if (getNeighbors(i).size() != 0) continue;
    pos = getOffsetVertex(i);   
    glVertex3f(pos.x(),pos.y(),pos.z());
    pos = getOffsetVertex((i+1)%num_verts);
    glVertex3f(pos.x(),pos.y(),pos.z());
  }
}

double Poly::Area() const { 
  Mesh *m = getMesh();
  int num_verts = numVertices();
  double sum = 0;
  for (int i = 1; i < num_verts-1; i++) {
    sum += ::AreaOfTriangle(m->getVertex((*this)[0])->get(),
			    m->getVertex((*this)[i])->get(),
			    m->getVertex((*this)[i+1])->get());
  }
  return sum;
}

void Poly::Print(const char *s) const {
  Mesh *m = getMesh();
  (*ARGS->output) << "POLYGON: " << std::string(s) << std::endl;
  //  Mesh *m = getMesh();
  int num_verts = numVertices();
  for (int i = 0; i < num_verts; i++) {
    (*ARGS->output) << "  " << (*this)[i] << "  ";
    m->getVertex((*this)[i])->get().Print();
    (*ARGS->output) << "     dist " << DistanceBetweenTwoPoints(m->getVertex((*this)[i])->get(),m->getVertex((*this)[(i+1)%num_verts])->get()) << std::endl;
    double angle = AngleBetween(m->getVertex((*this)[i])->get()-m->getVertex((*this)[(i+num_verts-1)%num_verts])->get(),
				m->getVertex((*this)[i])->get()-m->getVertex((*this)[(i+1)%num_verts])->get());
    (*ARGS->output) << "     angle " << angle * 180 / M_PI << std::endl;
  }
  (*ARGS->output) << std::endl;
}

void Poly::PrintEnclosure() const {
  int num = enclosure_samples.size();
  double e = -1;
  double stddev = -1;
  if (num > 0) {
    e = 0;
    for (int i = 0; i < num; i++) {
      e += enclosure_samples[i].second;
    }
    e /= (double)num;
    stddev = 0;
    for (int i = 0; i < num; i++) {
      stddev += square(enclosure_samples[i].second-e);
    }
    stddev = sqrt( stddev / (double) num );
  }
  (*ARGS->output) << "  " << percent_enclosed << "  " << num << " " << e << " " << stddev << std::endl;
}


bool Poly::PointInside(const Vec3f &_pt) const {
  Mesh *m = getMesh();
  
  // project to the y=0 plane first
  Vec3f pt = _pt;
  pt.zero_y();
  std::vector<Vec3f> pts;
  int num_verts = numVertices();
  for (int i = 0; i < num_verts; i++) {
    Vec3f v = m->getVertex((*this)[i])->get();
    v.zero_y();
    pts.push_back(v);
  }


  double sum = 0;
  for (int i = 0; i < num_verts; i++) {
    double angle = AngleBetween((pt-pts[i]),(pt-pts[(i+1)%num_verts])); 
    sum += angle;//M_PI - angle;
    //(*ARGS->output) << "   angle " << angle << "    Sum " << sum << std::endl;
  }

  //(*ARGS->output) << "SUM " << sum << "   "; //std::endl;
  if (fabs(sum - 2*M_PI) < 0.0001) {
    //(*ARGS->output) << "INSIDE" << std::endl;
    return true;
  } else {
    //(*ARGS->output) << "OUTSIDE" << std::endl;
    return false;
  }
}


bool BarycentricCoordinates(const Vec3f &a, const Vec3f &b, const Vec3f &c,
			    const Vec3f &v,
			    double &alpha, double &beta, double &gamma) {
  double det = ((a.x()-c.x())*(b.z()-c.z())) - ((b.x()-c.x())*(a.z()-c.z()));
  //(*ARGS->output) << "det " << det << std::endl;
  if (fabs(det) < 0.00000001) {
    alpha = beta = gamma = 0;
    return false;
  }
  alpha = (((b.z()-c.z())*(v.x()-c.x()))-(b.x()-c.x())*(v.z()-c.z())) / det;
  beta = -(((a.z()-c.z())*(v.x()-c.x()))-(a.x()-c.x())*(v.z()-c.z())) / det;
  gamma = 1 - alpha - beta;
  return true;
}


bool Poly::PointInsideOrOnBorder(const Vec3f &_pt) const {
  Mesh *m = getMesh();
  
  // project to the y=0 plane first
  Vec3f pt = _pt;
  pt.zero_y();
  
  std::vector<Vec3f> pts;
  int num_verts = numVertices();
  for (int i = 0; i < num_verts; i++) {
    Vec3f v = m->getVertex((*this)[i])->get();
    v.zero_y();
    pts.push_back(v);
  }


  double sum = 0;
  for (int i = 0; i < num_verts; i++) {
    double angle = AngleBetween((pt-pts[i]),(pt-pts[(i+1)%num_verts])); 
    sum += angle;
  }

  for (int i = 2; i < num_verts; i++) {
    double alpha, beta, gamma;
    BarycentricCoordinates(pts[0],pts[i-1],pts[i],pt,alpha,beta,gamma);
    if (alpha + beta + gamma < 0.9) continue;
    if (alpha >= 0 && alpha <= 1 &&
	beta  >= 0 && beta  <= 1 &&
	gamma >= 0 && gamma <= 1) { 
      //(*ARGS->output) << "pt " << pt.x() << " " << pt.z() << std::endl;
      //(*ARGS->output) << "alpha beta gamma " << alpha << " " << beta << " " << gamma << std::endl;
    return true; } //(*ARGS->output) <<"    INSIDE" << std::endl; }
  }

  if (fabs(sum - 2*M_PI) < 0.0001) {
    (*ARGS->output) << "INSIDE" << std::endl;
    //(*ARGS->output) << "alpha beta gamma " << alpha << " " << beta << " " << gamma << std::endl;
    //assert(0);
    return true;
  } else {
    //(*ARGS->output) << "OUTSIDE" << std::endl;
    return false;
  }
}
