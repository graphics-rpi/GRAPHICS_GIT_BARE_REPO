// Included files for OpenGL Rendering
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
#include <map>

#include "argparser.h"
#include "mesh.h"
#include "meshmanager.h"
#include "triangle.h"
#include "polygon.h"
#include "vertex.h"
#include "render.h"
#include "edge.h"
#include "glCanvas.h"
#include "walls.h"
#include "remesh.h"

//Visual studio uses slightly different namespacing
#ifndef _WIN32
  using std::min;
  using std::max;
#endif

#define CRACK 0.1f
#define TRANS_M 0.5
int inner_count = 0;

std::map<std::string,Vec3f> GLOBAL_random_color_map;

Vec3f lookupRandomColor(ArgParser *args, std::string s) {
  if (GLOBAL_random_color_map.find(s) == GLOBAL_random_color_map.end()) {
    GLOBAL_random_color_map[s] = args->RandomColor();
  }
  return GLOBAL_random_color_map[s];
}

Vec3f heightColor(double val, double min, double max) {
  double x = (val - min) / (max - min);

  if (x > 0.9) {
    double var = (x-0.9)/0.1;
    // red -> dark red
    return Vec3f(0.25+(1-var)*0.75, 0, 0);

  } else if (x > 0.7) {
    double var = (x-0.7)/0.2;
    // yellow -> orange
    return Vec3f(1, (1-var)*0.5, 0);

  } else if (x > 0.55) {
    double var = (x-0.55)/0.15;
    // yellow -> orange
    return Vec3f(1, (1-var)*0.5+0.5, 0);

  } else if (x > 0.4) {
    double var = (x-0.4)/0.15;
    // green -> yellow
    return Vec3f(var, 1, 0);

  } else if (x > 0.25) {
    double var = (x-0.25)/0.15;
    // cyan -> green
    return Vec3f(0, 1, 1-var);

  } else if (x > 0.1) {
    double var = (x-0.1)/0.15;
    // blue -> cyan
    return Vec3f (0, var, 1);

  } else {
    double var = x/0.1;
    // dark blue -> blue
    return Vec3f (0, 0, 0.25 + var*0.75);
  }
}

Vec3f normalColor(const Vec3f &n) {
  //double angle = fabs(n.Dot3(Vec3f(0,0,1)));
  Vec3f a,b,c,d;
  return Vec3f(0.5+0.5*n.x(),0.5+0.5*n.y(),0.5+0.5*n.z());
  return Vec3f(fabs(n.x()),fabs(n.y()),fabs(n.z()));

  if (n.x() >= 0 && n.y() >= 0) {
    a = Vec3f(1,1,1);
    b = Vec3f(1,0.5,0);
    c = Vec3f(0.5,0,0.5);
    d = Vec3f(1,0,0);
  } else if (n.x() >= 0 && n.y() <= 0) {
    a = Vec3f(1,1,1);
    b = Vec3f(1,0.5,0);
    c = Vec3f(0.5,1,0);
    d = Vec3f(1,1,0);
  } else if (n.x() <= 0 && n.y() <= 0) {
    a = Vec3f(1,1,1);
    b = Vec3f(0,0.5,0.5);
    c = Vec3f(0.5,1,0);
    d = Vec3f(0,1,0);
  } else {
    a = Vec3f(1,1,1);
    b = Vec3f(0,0.5,0.5);
    c = Vec3f(0.5,0,0.5);
    d = Vec3f(0,0,1);
  }
  
  double u = fabs(n.x());
  double v = fabs(n.y());
  Vec3f color = 
    (1-u)*((1-v)*a + v*c) +
    u    *((1-v)*b + v*d);
  return color;
}

Vec3f seamChangeColor(const Vec3f &vec) {
  double dx = vec.x();
  double dy = vec.y();

  Vec3f a,b,c,d;
  if (dx >= 0 && dy >= 0) {
    a = Vec3f(1,1,1);
    b = Vec3f(1,0.5,0);
    c = Vec3f(0.5,0,0.5);
    d = Vec3f(1,0,0);
  } else if (dx >= 0 && dy <= 0) {
    a = Vec3f(1,1,1);
    b = Vec3f(1,0.5,0);
    c = Vec3f(0.5,1,0);
    d = Vec3f(1,1,0);
  } else if (dx <= 0 && dy <= 0) {
    a = Vec3f(1,1,1);
    b = Vec3f(0,0.5,0.5);
    c = Vec3f(0.5,1,0);
    d = Vec3f(0,1,0);
  } else {
    a = Vec3f(1,1,1);
    b = Vec3f(0,0.5,0.5);
    c = Vec3f(0.5,0,0.5);
    d = Vec3f(0,0,1);
  }
  
  double u = fabs(dx);
  double v = fabs(dy);

  if (u > 1) u = 1;
  if (v > 1) v = 1;

  Vec3f color = 
    (1-u)*((1-v)*a + v*c) +
    u    *((1-v)*b + v*d);
  return color;
}

// ==============================================================
// ==============================================================

inline const Vec3f ToColor(double value) {
  if (value > 1) value = 1;
  if (value < -1) value = -1;
  assert (value >= -1 && value <= 1);
  if (value < 0) 
    return Vec3f(1+value,1+value,1);
  return Vec3f(1,1-value,1-value);
}

void Render::renderForSelect(MeshManager *meshes) {

  // special render pass to allow fast selection (picking)

  static GLuint select_display_list = glGenLists(1);
  assert (select_display_list != 0);
  Mesh *mesh = meshes->getMesh();

  if (meshes->args->rerender_select_geometry == 1) {
    meshes->args->rerender_select_geometry = 0;
    // re-compile this display list
    glNewList(select_display_list, GL_COMPILE_AND_EXECUTE);
    glInitNames();
    for (elementshashtype::const_iterator foo = mesh->getElements().begin();
	 foo != mesh->getElements().end();
	 foo++) {
      Element *e = foo->second;
      //Iterator<Element*> *iter = mesh->getElements()->StartIteration();
      //while (Element *e = iter->GetNext()) {
      glPushName(e->getID());
      glBegin(GL_POLYGON); //TRIANGLES);
      for (int i = 0 ; i < e->numVertices(); i++) {
	Vec3f v = mesh->getVertex((*e)[i])->get();
	glVertex3f(v.x(),v.y(),v.z());
      }
      glEnd();
      glPopName();
    }
    //mesh->getElements()->EndIteration(iter);
    glEndList();
  } else {
    glCallList(select_display_list);
  }
}

extern std::vector<QuadThing> GLOBAL_occlusions;

void RenderSilhouetteQuads(MeshManager *meshes) {
  glLineWidth(10);
  glColor3f(0,1,1);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  glColor4f(0,1,0.5,0.1);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_LIGHTING);


  glColor4f(1,0,0,0.1);
  glBegin(GL_QUADS); 
  for (unsigned int i = 0; i < GLOBAL_occlusions.size(); i++) {
    QuadThing q = GLOBAL_occlusions[i];
    glVertex3f(q.pts[0].x(),q.pts[0].y(),q.pts[0].z());
    glVertex3f(q.pts[1].x(),q.pts[1].y(),q.pts[1].z());
    glVertex3f(q.pts[2].x(),q.pts[2].y(),q.pts[2].z());
    glVertex3f(q.pts[3].x(),q.pts[3].y(),q.pts[3].z());
  }
  glEnd();

  glColor4f(0,1,0.5,0.1);
  glBegin(GL_TRIANGLES);
  int num_projectors = meshes->args->projectors.size();
  for (int i = 0; i < num_projectors; i++) {
    //cout << "PROJECTOR!" << endl;
    Vec3f C = meshes->args->projectors[i].getVec3fCenterReplacement();
    Vec3f a = meshes->args->projectors[i].getNearPlaneVec3fReplacement(0);
    Vec3f b = meshes->args->projectors[i].getNearPlaneVec3fReplacement(1);
    Vec3f c = meshes->args->projectors[i].getNearPlaneVec3fReplacement(2);
    Vec3f d = meshes->args->projectors[i].getNearPlaneVec3fReplacement(3);

    glVertex3f(C.x(),C.y(),C.z());
    glVertex3f(a.x(),a.y(),a.z());
    glVertex3f(b.x(),b.y(),b.z());

    glVertex3f(C.x(),C.y(),C.z());
    glVertex3f(b.x(),b.y(),b.z());
    glVertex3f(c.x(),c.y(),c.z());

    glVertex3f(C.x(),C.y(),C.z());
    glVertex3f(c.x(),c.y(),c.z());
    glVertex3f(d.x(),d.y(),d.z());

    glVertex3f(C.x(),C.y(),C.z());
    glVertex3f(d.x(),d.y(),d.z());
    glVertex3f(a.x(),a.y(),a.z());


  }
  glEnd();

  glBegin(GL_QUADS);


#if 0
  std::vector<CutPlane> &planes = meshes->planes;
  for (unsigned int i = 0; i < planes.size(); i++) {
    if (meshes->args->num_planes_to_cut >=0 &&
	int(i) >= meshes->args->num_planes_to_cut) break;
    Vec3f a = planes[i].quad[0];
    Vec3f b = planes[i].quad[1];
    Vec3f c = planes[i].quad[2];
    Vec3f d = planes[i].quad[3];
    glVertex3f(a.x(),a.y(),a.z());
    glVertex3f(b.x(),b.y(),b.z());
    glVertex3f(c.x(),c.y(),c.z());
    glVertex3f(d.x(),d.y(),d.z());
  }
#endif

#if 0
  int num_surface_cameras = GLOBAL_surface_cameras.size();
  for (int i = 0; i < num_surface_cameras; i++) {
    WallCamera &tmp = GLOBAL_surface_cameras[i];
    Vec3f a = tmp.eye-0.5*tmp.horizontal*tmp.width-0.5*tmp.up*tmp.height + tmp.near*tmp.direction;
    Vec3f b = tmp.eye-0.5*tmp.horizontal*tmp.width+0.5*tmp.up*tmp.height + tmp.near*tmp.direction;
    Vec3f c = tmp.eye+0.5*tmp.horizontal*tmp.width+0.5*tmp.up*tmp.height + tmp.near*tmp.direction;
    Vec3f d = tmp.eye+0.5*tmp.horizontal*tmp.width-0.5*tmp.up*tmp.height + tmp.near*tmp.direction;

    Vec3f a2 = tmp.eye-0.5*tmp.horizontal*tmp.width-0.5*tmp.up*tmp.height + tmp.far*tmp.direction;
    Vec3f b2 = tmp.eye-0.5*tmp.horizontal*tmp.width+0.5*tmp.up*tmp.height + tmp.far*tmp.direction;
    Vec3f c2 = tmp.eye+0.5*tmp.horizontal*tmp.width+0.5*tmp.up*tmp.height + tmp.far*tmp.direction;
    Vec3f d2 = tmp.eye+0.5*tmp.horizontal*tmp.width-0.5*tmp.up*tmp.height + tmp.far*tmp.direction;

    glVertex3f(a.x(),a.y(),a.z());
    glVertex3f(b.x(),b.y(),b.z());
    glVertex3f(b2.x(),b2.y(),b2.z());
    glVertex3f(a2.x(),a2.y(),a2.z());

    glVertex3f(b.x(),b.y(),b.z());
    glVertex3f(c.x(),c.y(),c.z());
    glVertex3f(c2.x(),c2.y(),c2.z());
    glVertex3f(b2.x(),b2.y(),b2.z());

    glVertex3f(c.x(),c.y(),c.z());
    glVertex3f(d.x(),d.y(),d.z());
    glVertex3f(d2.x(),d2.y(),d2.z());
    glVertex3f(c2.x(),c2.y(),c2.z());

    glVertex3f(d.x(),d.y(),d.z());
    glVertex3f(a.x(),a.y(),a.z());
    glVertex3f(a2.x(),a2.y(),a2.z());
    glVertex3f(d2.x(),d2.y(),d2.z());
  }
#endif 

  glEnd();
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glDisable(GL_LIGHTING);
}


void RenderSilhouetteEdges(MeshManager *meshes) {
  glLineWidth(2);
  glColor3f(0,1,1);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_LIGHTING);
  glBegin(GL_LINES);
  std::vector<CutPlane> &planes = meshes->planes;


  int num_projectors = meshes->args->projectors.size();
  for (int i = 0; i < num_projectors; i++) {
    glColor3f(0,1,0);

    //cout << "PROJECTOR!" << endl;
    Vec3f C = meshes->args->projectors[i].getVec3fCenterReplacement();
    Vec3f a = meshes->args->projectors[i].getNearPlaneVec3fReplacement(0);
    Vec3f b = meshes->args->projectors[i].getNearPlaneVec3fReplacement(1);
    Vec3f c = meshes->args->projectors[i].getNearPlaneVec3fReplacement(2);
    Vec3f d = meshes->args->projectors[i].getNearPlaneVec3fReplacement(3);

    a += 10*(a-C);
    b += 10*(b-C);
    c += 10*(c-C);
    d += 10*(d-C);

    glVertex3f(C.x(),C.y(),C.z());
    glVertex3f(a.x(),a.y(),a.z());

    glVertex3f(C.x(),C.y(),C.z());
    glVertex3f(b.x(),b.y(),b.z());

    glVertex3f(C.x(),C.y(),C.z());
    glVertex3f(c.x(),c.y(),c.z());

    glVertex3f(C.x(),C.y(),C.z());
    glVertex3f(d.x(),d.y(),d.z());

    Vec3f dir = meshes->args->projectors[i].getProjDirectionReplacement();
    Vec3f D = C + 10*dir;

    glColor3f(1,0,0);
    glVertex3f(C.x(),C.y(),C.z());
    glVertex3f(D.x(),D.y(),D.z());    
    


  }


  for (unsigned int i = 0; i < planes.size(); i++) {
    if (meshes->args->num_planes_to_cut >=0 &&
	int(i) >= meshes->args->num_planes_to_cut) break;
    Vec3f a = planes[i].quad[0];
    Vec3f b = planes[i].quad[1];
    glVertex3f(a.x(),a.y(),a.z());
    glVertex3f(b.x(),b.y(),b.z());
  }

#if 0
  int num_surface_cameras = GLOBAL_surface_cameras.size();
  for (int i = 0; i < num_surface_cameras; i++) {
    WallCamera &tmp = GLOBAL_surface_cameras[i];
    Vec3f a = tmp.eye-0.5*tmp.horizontal*tmp.width-0.5*tmp.up*tmp.height;
    Vec3f b = tmp.eye-0.5*tmp.horizontal*tmp.width+0.5*tmp.up*tmp.height;
    Vec3f c = tmp.eye+0.5*tmp.horizontal*tmp.width+0.5*tmp.up*tmp.height;
    Vec3f d = tmp.eye+0.5*tmp.horizontal*tmp.width-0.5*tmp.up*tmp.height;
    glVertex3f(a.x(),a.y(),a.z());
    glVertex3f(b.x(),b.y(),b.z());
    glVertex3f(b.x(),b.y(),b.z());
    glVertex3f(c.x(),c.y(),c.z());
    glVertex3f(c.x(),c.y(),c.z());
    glVertex3f(d.x(),d.y(),d.z());
    glVertex3f(d.x(),d.y(),d.z());
    glVertex3f(a.x(),a.y(),a.z());
  }
#endif

  glEnd();
  glDisable(GL_LIGHTING);
}



void Render::RenderAll(MeshManager *meshes) {

  //Mesh *mesh = meshes->getMesh();
  //mesh->ComputeBlendDistanceFromOcclusion();
  //mesh->ComputeBlendWeights();

  assert(HandleGLError("renderall"));
  static GLuint display_list = glGenLists(1);
  assert (display_list != 0);
  if (meshes->args->rerender_scene_geometry == 1) {
    // re-compile the display list!
    meshes->args->rerender_scene_geometry = 0;
    glNewList(display_list, GL_COMPILE_AND_EXECUTE);
    if (meshes->args->transparency) {
      assert (0);
      float mcolor[] = { 0.2,0.2,1,1};
      glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, mcolor);
      glEnable(GL_BLEND);
      glBlendFunc(GL_DST_COLOR, GL_ZERO);
      glDisable(GL_DEPTH_TEST);
    }
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1,1);
    
    if (meshes->args->render_cull_back_facing) {
      glCullFace(GL_BACK);
      glEnable(GL_CULL_FACE);
    }

        RenderLines(meshes);

    if (meshes->args->render_mode == 0)      {   /* don't render anything */ }   // no fill triangles
    else if (meshes->args->render_mode == 1) {   RenderTriangles(meshes); }      // triangles
    else if (meshes->args->render_mode == 2) { // arrangement % enclosed 
      if (meshes->args->point_sampled_enclosure == 0) {
	RenderArrangement(meshes,0); 
      } else {
	RenderPointSampledEnclosure(meshes);
      }    
    }  
    else if (meshes->args->render_mode == 3) {   RenderArrangement(meshes,1); }  // arrangement type
    else if (meshes->args->render_mode == 4) {   RenderArrangement(meshes,2); }  // arrangement fingerprints
    else if (meshes->args->render_mode == 5) {   RenderArrangement(meshes,3); }  // arrangement num_middles
    else if (meshes->args->render_mode == 6) {   RenderArrangement(meshes,4);   if (!meshes->args->army) meshes->getWalls()->RenderNorthArrow(); }  // floor plan
    else if (meshes->args->render_mode == 7) {   RenderArrangement(meshes,5);   meshes->getWalls()->RenderNorthArrow(); }  // 2D walls
    else if (meshes->args->render_mode == 8) {  } // Render3DModel(meshes); }  // 3D model
    else { assert(0); }

    if (meshes->args->render_wall_chains) { meshes->getWalls()->paintWallChains(); }

    if (meshes->args->render_walls)       { meshes->getWalls()->paintWalls(); }

    if (meshes->args->ground_plane)  {
      meshes->getWalls()->paintFloor(); 
    }
    
    glDisable(GL_POLYGON_OFFSET_FILL);

    //if (meshes->args->render_flipped_edges)          RenderFlippedEdges(meshes);
    //if (meshes->args->render_short_edges)            RenderShortEdges(meshes);
    //    if (meshes->args->render_concave_angles)         RenderConcaveAngles(meshes);
    if (meshes->args->transparency) {
      glDisable(GL_BLEND);
      glEnable(GL_DEPTH_TEST);
    }
    if (meshes->args->render_triangle_vertices)  RenderTriangleVertices(meshes);
    if (meshes->args->render_triangle_edges)     RenderTriangleEdges(meshes);
    if (meshes->args->render_cluster_boundaries) RenderClusterBoundaries(meshes);

    if (meshes->args->render_zero_area_triangles || 
	meshes->args->render_bad_normal_triangles ||
	meshes->args->render_bad_neighbor_triangles) RenderBadTriangles(meshes);
    if (meshes->args->render_cull_back_facing) {
      glDisable(GL_CULL_FACE);
    }

    if (meshes->args->render_visibility_planes) {
      RenderSilhouetteQuads(meshes);
      RenderSilhouetteEdges(meshes);
    }
    if (meshes->args->render_non_manifold_edges) {
      RenderNonManifoldEdges(meshes);
    }
    if (meshes->args->render_crease_edges) {
      RenderCreaseEdges(meshes);
    }

    glEndList();
  } else { 
    // call the display list!
    glCallList(display_list);
  }



  assert(HandleGLError("renderalll2"));
}

// ==============================================================
// ==============================================================
// ==============================================================

void Render::RenderTriangleEdges(MeshManager *meshes) {

  glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

  Mesh *mesh = meshes->getMesh();
  glDisable(GL_LIGHTING);
  glLineWidth(1);
  glColor4f(0,0,0,1);

  Vec3f pos[3];
  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    Element *e = foo->second;
    const MeshMaterial *m = e->getMaterialPtr();
    if (!meshes->args->render_EXTRA_elements && m->isExtra()) continue;
    if (!meshes->args->render_FILLIN_elements && m->isFillin()) continue;
    if (!meshes->args->render_PROJECTION_elements && m->isProjection()) continue;
    if (meshes->args->render_cull_ceiling && m->getName() == "FILLIN_ceiling") continue;
    glBegin(GL_POLYGON);
    for (int index = 0; index < e->numVertices(); index++) {
      pos[index] = mesh->getVertex((*e)[index])->get();
      glVertex3f(pos[index].x(),pos[index].y(),pos[index].z());
    }
    glEnd();
  }

  if (meshes->args->render_vis_mode == 1) {
    glLineWidth(5);
    glColor4f(1,0,0,1);
    for (unsigned int p = 0; p < mesh->numPatches(); p++) {
      ElementID id = mesh->getPatch(p).getSeed();
      if (id == 0) continue;
      Element *e = Element::GetElement(id);
      if (e == NULL) continue;
      glBegin(GL_POLYGON);
      for (int index = 0; index < e->numVertices(); index++) {
	pos[index] = mesh->getVertex((*e)[index])->get();
	glVertex3f(pos[index].x(),pos[index].y(),pos[index].z());
      }
      glEnd();    
    }
    /*
    glPointSize(10);
    glColor4f(1,0,1,1);
    glBegin(GL_POINTS);
    for (unsigned int p = 0; p < mesh->numPatches(); p++) {
      Vec3f centroid = mesh->getPatch(p).getCentroid();
      glVertex3f(centroid.x(),centroid.y(),centroid.z());
    }
    glEnd();    
    */
  } else if (meshes->args->render_vis_mode == 2) {
    glLineWidth(5);
    glColor4f(1,0,0,1);
    mesh->getBoundingBox()->paint();
  }
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  if (meshes->args->render_vis_mode == 1 ||
      meshes->args->render_vis_mode == 2) {
    meshes->getWalls()->RenderNorthArrow(); 
  }
  glEnable(GL_LIGHTING);
}


Vec3f RenderElementColor(int vis_mode, int which_projector, Mesh *mesh, Element *e, ArgParser *args, int vert) {

  const MeshMaterial *m = e->getMaterialPtr();

  if (vis_mode == 0) {
    // white / magenta
    return Vec3f(1,1,1);
  } else if (vis_mode == 1) {
    // patch color
    PatchID patch = mesh->getAssignedPatchForElement(e->getID());
    if (patch == -1) {
      return Vec3f(0.5,0.5,0.5);
    }
    else {
      return mesh->getPatchColor(patch);
    }
  } else if (vis_mode == 2) {
    // zone color
    ZoneID zone = mesh->getAssignedZoneForElement(e->getID());
    //std::cout << "zone = " << zone << std::endl;
    if (zone == -1) {
      return Vec3f(0.5,0.5,0.5);
    }
    else {
      return mesh->getZoneColor(zone);
    }

  } else if (vis_mode == 3) {  
    // diffuse material color
    if (m->isLambertian() || m->isGlass())
      return m->getDiffuseReflectance();
    return Vec3f(1,0,0);
  } else if (vis_mode == 4) {  
    // physical material color
    return m->getPhysicalDiffuseReflectance();
  } else if (vis_mode == 5) {  
    // randomized material color
    return lookupRandomColor(args,e->getFakeMaterial());
  } else if (vis_mode == 6) {  
    // glass only
    assert (m->isGlass());
    Vec3f trans = m->getSpecularTransmittance() + m->getDiffuseTransmittance();
    //return trans; //Vec3f(0.1+0.8*trans,0.1+0.8*trans,1);
    return Vec3f(0.1+0.8*trans.r(),0.1+0.8*trans.r(),1);
  } else if (vis_mode == 7) {
    // invisible sensors only
    assert (m->isInvisibleSensor());
    return Vec3f(0.3,1,1);
  } else if (vis_mode == 8) {
    // other sensors only
    assert (m->isOpaqueSensor());
    return Vec3f(1, 0.5 ,0.1);    
  } else if (vis_mode == 9) {
    // other sensors only
    assert (m->isExterior());
    return Vec3f(0.5,0.8,0.1);
  } else if (vis_mode == 10) {
    // texture coordinates
    double s,t;
    mesh->getVertex((*e)[vert])->getTextureCoordinates(s,t);
    if (s > 0 || t > 0) 
      std::cout << "HERE " << s << " " << t << std::endl;
    s-=m->getTextureOffsetS();
    t-=m->getTextureOffsetT();
    return Vec3f(1-s,1-t,1);
  } else if (vis_mode == 11) {
    // normal
    const Vec3f &n = mesh->getVertex((*e)[vert])->getNormal();
    return normalColor(n);
  } else if (vis_mode >= 12) {
    // projector blending
    const std::string sidedness = e->getSidedness();
    unsigned int num = args->projector_names.size();
    if (vis_mode == 12) {
      // visibility count
      double count = 0;
      for (unsigned int i = 0; i < num; i++) {
	if (sidedness[i] == 'a') 
	  count+=1;
      }
      count /= double(num);
      return Vec3f(count,count,count);
    } else if (vis_mode == 13) { 
      // sum projector blending weight
      double sum = 0;
      unsigned int num = args->projector_names.size();
      for (unsigned int i = 0; i < num; i++) {
	sum += e->getBlendWeightWithDistance(vert,i);
      }
      return Vec3f(sum,sum,sum);
    } else if (vis_mode == 14) { 
      // individual projector blending weight
      double blend = e->getBlendWeight(vert,which_projector);
      return Vec3f(blend,blend,blend);
    } else if (vis_mode == 15) { 
      // blending distance
      Vertex *v = mesh->getVertex((*e)[vert]);
      double d = 10*min(0.1,v->getBlendDistanceFromOcclusion(which_projector));
      if (d < 0.01) {
	return Vec3f(0,0,1);
      } else {
	double dist = max(0.01,d); //v->getBlendDistanceFromOcclusion(which_projector);
	//double dist = v->getBlendDistanceFromOcclusion(which_projector);
	return Vec3f(dist,dist,dist);
      }


    } else if (vis_mode == 16) { 
      // blend weight with distance
      double blend = e->getBlendWeightWithDistance(vert,which_projector);
      return Vec3f(blend,blend,blend);
      /*
      Vertex *v = mesh->getVertex((*e)[vert]);

      unsigned int num = args->projector_names.size();

      std::vector<double> distances(num);
      std::vector<double> weights(num);
      std::vector<double> distanceweights(num);

      double sum = 0;

      for (unsigned int i = 0; i < num; i++) {
	distances[i] = std::min(0.1,v->getBlendDistanceFromOcclusion(i));
	//if (distances[i] > 0)
	//std::cout << distances[i] << std::endl;
	weights[i] = e->getBlendWeight(vert,i);
	distanceweights[i] = distances[i]*weights[i];
	sum += distanceweights[i];
      }

      double value = distanceweights[which_projector];
      if (sum > 0) {
	value /= sum;
      }

      return Vec3f(value,value,value);
      */
    }
  } else {
    assert (0);
  }
  assert(0); 
  return Vec3f(0,0,0);
}

void Render::RenderLines(MeshManager *meshes) {

  //  glDisable(GL_LIGHTING);
  glLineWidth(10);
  glBegin(GL_LINES);

  Mesh *mesh = meshes->getMesh();

  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    Element *e = foo->second;
    if (!e->isALine()) continue;

    const MeshMaterial *m = e->getMaterialPtr();

    Vec3f color = m->getDiffuseReflectance();

    glColor3f(color.r(),color.g(),color.b());
    ///glColor3f(1,0,0); //color.r(),color.g(),color.b());


    Vec3f pos = mesh->getVertex((*e)[0])->get();
    glVertex3f(pos.x(),pos.y(),pos.z());
    pos = mesh->getVertex((*e)[1])->get();
    glVertex3f(pos.x(),pos.y(),pos.z());
 
  
  }

  glEnd();
  //glEnable(GL_LIGHTING);
}


void Render::RenderTriangles(MeshManager *meshes) {

  Mesh *mesh = meshes->getMesh();
  int vis_mode = meshes->args->render_vis_mode;
  int which_projector = meshes->args->render_which_projector;

  if (!meshes->args->gl_lighting || vis_mode == 1 || vis_mode == 2 || vis_mode >= 10) {
    //} else if (vis_mode >= 9) {
    glDisable(GL_LIGHTING);
  }
  
  // DRAW TRIANGLES
  Vec3f pos;
  Vec3f color;
  Vec3f normal;
  //Bag<Element*> *elements = mesh->getElements();

  if (meshes->args->render_cull_back_facing &&
      (vis_mode == 6 ||
       vis_mode == 7 ||
       vis_mode == 8)) {
    glDisable(GL_CULL_FACE);
  } 

  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    Element *e = foo->second;
    //Iterator<Element*> *iter = elements->StartIteration();
    //while (Element *e = iter->GetNext()) {

    const MeshMaterial *m = e->getMaterialPtr();
    if (vis_mode == 6 && !m->isGlass()) continue;
    if (vis_mode == 7 && !m->isInvisibleSensor()) continue;
    if (vis_mode == 8 && !m->isOpaqueSensor()) continue;
    if (vis_mode == 9 && !m->isExterior()) continue;
    if (meshes->args->render_cull_ceiling && m->getName() == "FILLIN_ceiling") continue;
    if (!meshes->args->render_EXTRA_elements && m->isExtra()) continue;
    if (!meshes->args->render_FILLIN_elements && m->isFillin()) continue;
    if (!meshes->args->render_PROJECTION_elements && m->isProjection()) continue;

    // render the element!
    glBegin(GL_POLYGON);
    for (int index = 0; index < e->numVertices(); index++) {
      pos = mesh->getVertex((*e)[index])->get();
      e->computeNormal(normal); 
      color = RenderElementColor(vis_mode,which_projector,mesh,e,meshes->args,index);
      if (meshes->args->render_normals) {
	normal = mesh->getVertex((*e)[index])->getNormal();
      }
      if (meshes->args->transparency) {
	color = Vec3f(TRANS_M,TRANS_M,TRANS_M) + color*(1-TRANS_M);
      }
      glNormal3f(normal.x(),normal.y(),normal.z());
      glColor3f (color.x(),color.y(),color.z());
      glVertex3f(pos.x(),pos.y(),pos.z());
    }
    glEnd();
  }
  //elements->EndIteration(iter);

  if (meshes->args->render_cull_back_facing &&
      (vis_mode == 6 ||
       vis_mode == 7 ||
       vis_mode == 8)) {
    glEnable(GL_CULL_FACE);
  }
}

void Render::RenderPointSampledEnclosure(MeshManager *meshes) {
  glDisable(GL_LIGHTING);
  glColor3f(1,0,0);
  glPointSize(10);
  glBegin(GL_POINTS);

  for (elementshashtype::const_iterator foo = meshes->getArrangement()->getElements().begin(); 
       foo != meshes->getArrangement()->getElements().end(); foo++) {  
    Poly *p = (Poly*)foo->second;    
    const std::vector<std::pair<Vec3f,double> >& sampled_enclosure = p->enclosure_samples;
    int num_samples = sampled_enclosure.size();
    for (int i = 0; i < num_samples; i++) {
      const Vec3f& v = sampled_enclosure[i].first;
      double e = sampled_enclosure[i].second;
      if (e < -0.1)
	glColor3f(1,0,1);
      else 
	glColor3f(e,e,e);
      glVertex3f(v.x(),0,v.z());
    }
  }
  glEnd();
  glEnable(GL_LIGHTING);
}


void Render::RenderArrangement(MeshManager *meshes, int flag) {

  RenderArrangementBoundaries(meshes);
  glDisable(GL_LIGHTING);
  Mesh *arrangement = meshes->getArrangement();
  for (elementshashtype::const_iterator foo = arrangement->getElements().begin();
       foo != arrangement->getElements().end();
       foo++) {
    Element *p = foo->second;       
    assert (p->isAPolygon()); 
    if (meshes->args->army) {
      ((Poly*)p)->Paint(6,meshes->getWalls());
    } else {
      ((Poly*)p)->Paint(flag,meshes->getWalls());
    }
  }
  
  if (flag == 0) {  // arrangement % enclosed
    return;
  } else if (flag == 1) {  // arrangement type
    glColor3f(1,1,1);
  } else if (flag == 2) { // arrangement fingerprints
    return;
  } else if (flag == 3) {  // arrangement num middles
    return;
  } else if (flag == 4) { // floor plan
    meshes->getWalls()->PaintFloorPlanWindows();
    return;
  } else if (flag == 5) { // 2D walls
    return;
  } else {
    assert (0);
  }

  glLineWidth(1);
  glBegin(GL_LINES);
  for (elementshashtype::const_iterator foo = arrangement->getElements().begin();
       foo != arrangement->getElements().end();
       foo++) {
    Element *p = foo->second;       
    ((Poly*)p)->PaintEdges();
  }
  glEnd();

  glLineWidth(10);
  glColor3f(1,0,0);
  glBegin(GL_LINES);
  for (elementshashtype::const_iterator foo = arrangement->getElements().begin();
       foo != arrangement->getElements().end();
       foo++) {
    Element *p = foo->second;       
    ((Poly*)p)->PaintBoundaryEdges();
  }
  glEnd();
  glEnable(GL_LIGHTING);
}


// ==============================================================
// ==============================================================
// ==============================================================

void Render::RenderTriangleVertices(MeshManager *meshes) {
  Mesh *mesh = meshes->getMesh();
  glDisable(GL_LIGHTING);
  glPointSize(10);   
  glColor4f(0,0,0,1);
  glBegin(GL_POINTS);
  for (unsigned int i = 0; i < mesh->numVertices(); i++) {
    Vec3f v2 = mesh->getVertex(i)->get();   
    glVertex3f(v2.x(),v2.y(),v2.z());
  }
  glEnd();
  glEnable(GL_LIGHTING);  
}

void Render::RenderClusterBoundaries(MeshManager *meshes) {
  Mesh *mesh = meshes->getMesh();
  glDisable(GL_LIGHTING);
  glLineWidth(5);
  glColor3f(1,0,0);
  glBegin(GL_LINES);
  const edgeshashtype& edges = meshes->getMesh()->getEdges();
  for (edgeshashtype::const_iterator foo = edges.begin(); //StartIteration();
       foo != edges.end();
       foo++) {
    assert (foo->second.size() > 0);
    for (std::vector<Edge*>::const_iterator bar = foo->second.begin();
	 bar < foo->second.end();
	 bar++) {
      Edge *e = *bar;
      //Bag<Edge*> *edges = meshes->getMesh()->getEdges();
      //Iterator<Edge*> *iter = edges->StartIteration();
      //while (Edge *e = iter->GetNext()) {
      if (mesh->IsSimpleBoundaryEdge(e)) { //,(*e)[0],(*e)[1])) {
	Vec3f a = mesh->getVertex((*e)[0])->get();
	Vec3f b = mesh->getVertex((*e)[1])->get();
	glVertex3f(a.x(),a.y(),a.z());
	glVertex3f(b.x(),b.y(),b.z());
      }
    }
  }
  //  edges->EndIteration(iter);
  glEnd();
  glEnable(GL_LIGHTING);
}


void Render::RenderNonManifoldEdges(MeshManager *meshes) {
  glDisable(GL_LIGHTING);
  glLineWidth(10);
  glColor4f(1,1,0,1);
  Mesh *mesh = meshes->getMesh();
  //Bag<Edge*> *edges = meshes->getMesh()->getEdges();
  glBegin(GL_LINES);
  const edgeshashtype& edges = meshes->getMesh()->getEdges();
  for (edgeshashtype::const_iterator foo = edges.begin(); //StartIteration();
       foo != edges.end();
       foo++) {
    assert (foo->second.size() > 0);
    for (std::vector<Edge*>::const_iterator bar = foo->second.begin();
	 bar < foo->second.end();
	 bar++) {
      Edge *e = *bar;
      //Iterator<Edge*> *iter = edges->StartIteration();
      //while (Edge *e = iter->GetNext()) {
      if (mesh->IsNonManifoldEdge(e)) { //,(*e)[0],(*e)[1])) {
	Vec3f a = mesh->getVertex((*e)[0])->get();
	Vec3f b = mesh->getVertex((*e)[1])->get();
	glVertex3f(a.x(),a.y(),a.z());
	glVertex3f(b.x(),b.y(),b.z());
      }
    }
  }
  //  edges->EndIteration(iter);
  glEnd();
  glEnable(GL_LIGHTING);
}


void Render::RenderCreaseEdges(MeshManager *meshes) {
  Mesh *mesh = meshes->getMesh();
  glDisable(GL_LIGHTING);
  glLineWidth(5);
  glBegin(GL_LINES);
  glColor4f(1,0,1,1);
  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    Element *e = foo->second;
    //->StartIteration();
    //while (Element *t = iter2->GetNext()) {
    int numverts = e->numVertices();
    for (int i = 0; i < numverts; i++) {
      std::vector<Element*> neighbors = e->getNeighbors(i);
      if (neighbors.size() != 1) continue;
      continue;
      Vec3f a,b;
      a = mesh->getVertex((*e)[i])->get();
      b = mesh->getVertex((*e)[(i+1)%numverts])->get();
      glVertex3f(a.x(),a.y(),a.z());
      glVertex3f(b.x(),b.y(),b.z());
    }
  }
  //  mesh->getElements()->EndIteration(iter2);
  glEnd();
  glEnable(GL_LIGHTING);
}



void Render::RenderArrangementBoundaries(MeshManager *meshes) {
  Mesh *arrangement = meshes->getArrangement();
  glDisable(GL_LIGHTING);
  glLineWidth(5);
  glBegin(GL_LINES);

  for (elementshashtype::const_iterator foo = arrangement->getElements().begin();
       foo != arrangement->getElements().end();
       foo++) {
    Element *p = foo->second;       
    //  Iterator<Element*> *iter2 = arrangement->getElements()->StartIteration();
    //while (Element *p = iter2->GetNext()) {
    int num_verts = p->numVertices();
    for (int i = 0; i < num_verts; i++) {
      
      std::vector<Element*> neighbors = p->getNeighbors(i);
      if (neighbors.size() == 0) {
	continue;
	glColor4f(1,0,0,1);
      } else if (neighbors.size() > 1) {
	glColor4f(1,1,0,1);
      } else {
        continue;
      }

      Vec3f a,b;
      a = arrangement->getVertex((*p)[i])->get();
      b = arrangement->getVertex((*p)[(i+1)%num_verts])->get();

      glVertex3f(a.x(),a.y(),a.z());
      glVertex3f(b.x(),b.y(),b.z());
    }
  }
  //  arrangement->getElements()->EndIteration(iter2);
  glEnd();
  glEnable(GL_LIGHTING);
}



void Render::RenderBadTriangles(MeshManager *meshes) {
  Mesh *mesh = meshes->getMesh();
  glDisable(GL_LIGHTING);
  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    Element *e = foo->second;       
    //Iterator<Element*> *iter = mesh->getElements()->StartIteration();
    //while (Element *e = iter->GetNext()) {
    if (e->NearZeroArea() && meshes->args->render_zero_area_triangles) {
      glLineWidth(10);
      glColor4f(0,1,0,1); // green
    } else if (e->BadNormal() && meshes->args->render_bad_normal_triangles) {
      glLineWidth(6);
      glColor4f(0,1,1,1); //cyan
    } else if (e->HasBadNeighbor() && meshes->args->render_bad_neighbor_triangles) {
      glLineWidth(3);
      glColor4f(0,0,1,1); // blue
    } else {
      continue;
    }
    glBegin(GL_LINES);
    for (int i = 0; i < e->numVertices(); i++) {
      Vec3f a = mesh->getVertex((*e)[i])->get();
      Vec3f b = mesh->getVertex((*e)[(i+1)%3])->get();
      glVertex3f(a.x(),a.y(),a.z());
      glVertex3f(b.x(),b.y(),b.z());
    }
    glEnd();
  }
  //mesh->getElements()->EndIteration(iter);
  glEnable(GL_LIGHTING);
}


void Render::RenderGroundPlane(MeshManager *meshes) {

  BoundingBox *box = meshes->getMesh()->getBoundingBox();
  Vec3f min,max;
  box->Get(min,max);
  double fix_y = min.y() + (max.y()-min.y())*meshes->args->ground_height;
  
  double maxd = max3(max.x()-min.x(),
		     max.y()-min.y(),
		     max.z()-min.z()) / 4.0;

  min -= Vec3f(maxd,maxd,maxd);
  max += Vec3f(maxd,maxd,maxd);

  glColor3f(0.8,0.8,0.8);
  glBegin(GL_TRIANGLES);
  glNormal3f(0,1,0);
  glVertex3f(min.x(),fix_y,min.z());
  glVertex3f(min.x(),fix_y,max.z());
  glVertex3f(max.x(),fix_y,max.z());
  glVertex3f(min.x(),fix_y,min.z());
  glVertex3f(max.x(),fix_y,max.z());
  glVertex3f(max.x(),fix_y,min.z());
  glEnd();

}


