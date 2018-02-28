#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <fstream>

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
#include "mesh.h"
#include <cmath>

#include "triangle.h"
#include "quad.h"
#include "polygon.h"



extern ArgParser *ARGS;

// PHYSICAL DIFFUSE CONSTANTS
Vec3f PD_floor(0.95,0.95,0.95);
//Vec3f PD_skylight_frame(0.57,0.86,0.89);
//Vec3f PD_skylight_center(0.90,0.90,0.90);
Vec3f PD_skylight_frame(0.95,0.95,0.95);
Vec3f PD_skylight_center(0.95,0.95,0.95);
Vec3f PD_wall(0.90,0.90,0.90);
Vec3f dark_grey(0.25,0.25,0.25);
Vec3f black(0,0,0);



#define HASH_FLOOR           0   // 0.0
#define HASH_BED             1   // 22 inches = 0.55 m
#define HASH_DESK            2   // 29 inches = 0.73 m  (a.k.a. window bottom)
#define HASH_DIVIDER         3   // 60 inches = 1.524 m
#define HASH_WARDROBE        4   //                     (a.k.a. window middle)
#define HASH_WINDOW_TOP      5
#define HASH_CEILING         6



int AddVertexHelper(Mesh *arrangement, Mesh *mesh, vphashtype &vphash, int poly_mesh_id, int index, double y) {
  vphashtype::iterator foo = vphash.find(std::make_pair(poly_mesh_id,index));
  if (foo == vphash.end()) {
    Vec3f v = arrangement->getVertex(poly_mesh_id)->get();
    int answer = mesh->addVertex(Vec3f(v.x(),y,v.z()),-1,-1,0,0);
    vphash[std::make_pair(poly_mesh_id,index)] = answer;
    return answer;
  } else {
    return foo->second;
  }
}



double Walls::ceilingHeight(Vec3f pt) {


  return BLUE_HEIGHT;
  //  return RED_HEIGHT;


  // EMPAC FLAG
  //if (empac) return EMPAC_HEIGHT;

  if (allShort()) return GREEN_HEIGHT;
  if (allTall()) return RED_HEIGHT;

  //#ifdef CEILING_EQUALS_MEDIUM
  ///return BLUE_HEIGHT;
  //#else
  double length = pt.Length();
  static double max = 0;
  if (length > max) max = length;
  //(*ARGS->output) << max << std::endl;
  //double height = BLUE_HEIGHT + (length / 0.343304)*(RED_HEIGHT-BLUE_HEIGHT);
  double height = BLUE_HEIGHT + 0.01; //(length / 0.343304)*(RED_HEIGHT-BLUE_HEIGHT);
  return height;
  //#endif
}

void Walls::CreateTrianglesForCell(Mesh *arrangement, Mesh *mesh, vphashtype &vphash,
                                   Poly *p, int HASH, double height, bool upside_down, std::string material) {
  //(*ARGS->output) << " ctfc " << std::endl;
  std::vector<int> verts;
  int num_verts = p->numVertices();
  for (int i = 0; i < num_verts; i++)
    verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[i],HASH,height));
  if (!upside_down) {
    for (int i = 1; i < num_verts-1; i++) {
      mesh->addElement(new Triangle(verts[0],verts[i],verts[i+1],mesh,material));
    }
  } else {
    for (int j = 1; j < num_verts-1; j++) {
#if 1 // HACK
      if (!allShort() ||
          (allShort() &&
           !isWallOrWindow(p->getCellType()))) {
        mesh->addElement(new Triangle(verts[0],verts[j+1],verts[j],mesh,material));
      }
#endif
    }
  }
}

std::string wall_material_name(int i, bool fillin) {
  std::string s;
  std::stringstream ss(s);
  if (fillin)
    ss << "FILLIN_";
  ss << "wall_" << i;
  std::string name;
  ss >> name;
  return name;
}


void Walls::SetBoxMaterials(Vec3f red, Vec3f white, Vec3f blue) {
  floor_material = white;
  ceiling_material = white;
  for (unsigned int i = 0; i < wall_materials.size(); i++) {
    if (wall_materials[i].g() < 0.01 ) {
      if (wall_materials[i].r() > 0.99 ) {
        assert (wall_materials[i].b() < 0.01);
        wall_materials[i] = red;
      } else {
        assert (wall_materials[i].r() < 0.01);
        assert (wall_materials[i].b() > 0.99);
        wall_materials[i] = blue;
      }
    } else {
      assert (wall_materials[i].r() > 0.99);
      assert (wall_materials[i].g() > 0.99);
      assert (wall_materials[i].b() > 0.99);
      wall_materials[i] = white;
    }
  }
}


bool sort_by_visibility(const std::pair<double,Vec3f> &a,
                        const std::pair<double,Vec3f> &b) {
  return (a.first > b.first);
}



double CalculateVisibility(const Vec3f &pos, const Vec3f &norm, ArgParser *args,
                           const std::vector<BasicWall*> walls) {

  // double answer = 1;

  // create vectors for the projector centers & directions
  unsigned int num_projectors = args->projector_names.size();
  if (num_projectors == 0) return 1.0;
  std::vector<Vec3f> projector_centers;
  std::vector<Vec3f> projector_directions;
  for (unsigned int j = 0; j < num_projectors; j++) {
    projector_centers.push_back(args->projectors[j].getVec3fCenterReplacement());
    projector_directions.push_back(args->projectors[j].getProjDirectionReplacement());
  }

  double radius = 3 * INCH_IN_METERS;


  double answer = 0;
  for (unsigned int j = 0; j < num_projectors; j++) {

    int clear = 0;
    int total = 0;
    for (double x = -radius; x <= radius; x += radius / 5.0) {
      for (double z = -radius; z <= radius; z += radius / 5.0) {

        Vec3f p = pos + Vec3f(x,0,z);
        for (unsigned int i = 0; i < walls.size(); i++) {

          total++;

          if (walls[i]->SegmentIntersectsWall(p,projector_centers[j])) {
          } else {
            clear++;
          }
        }
      }
    }

    double dist = DistanceBetweenTwoPoints(projector_centers[j],pos);

    dist = 1; // / pow (dist, 10); //*= dist * dist;
    /*
      (*ARGS->output) << "dist  " << dist << std::endl;
    */

    if (1) {//dist < 1.6) {

      if (total == clear) {
        answer += (1 / double(dist)) * 1 / double(num_projectors);
      } else {
        answer += (1 / double(dist)) * 0.1 * (1 / double(num_projectors)) * (clear / double(total));
      }
    }
  }
  return answer;

}

void MakeAssignments(std::vector<int> &assignments,
                     const std::vector<Person> &people,
                     const std::vector<std::pair<double,Vec3f> > &view_spots) {

  assert (assignments.size() == people.size());
  assert (assignments.size() <= view_spots.size());

  std::vector<int> all;
  for (unsigned int i = 0; i < assignments.size(); i++) { all.push_back(i); }

  double best = -1;

  do {

    double dist = 0;
    for (unsigned int i = 0; i < assignments.size(); i++) {
      Vec3f a = view_spots[all[i]].second;
      Vec3f b = Vec3f(people[i].x,0,people[i].z);
      dist += DistanceBetweenTwoPoints(a,b);
    }

    if (best == -1 || dist < best) {
      assignments = all;
      best = dist;
    }


  } while ( std::next_permutation(all.begin(),all.end()) );




  //  assignments = all;

}


#define VIEW_VIS 0
//#define VIEW_VIS 0







void Walls::CreateTriMesh(Mesh *mesh) {

  (*ARGS->output) << "VERSION B" << std::endl;

  (*ARGS->output) << "CREATE TRI MESH" << std::endl;

  // CREATE THE FLOOR
  int tiles = args->floor_cameras_tiled;
  assert (tiles >= 1);
  double x_spacing = 2.0*scene_radius / tiles;
  double z_spacing = 2.0*scene_radius / tiles;

#if 0
  double minus_x = -scene_radius;
  //double pos_x   =  scene_radius;
  double minus_z = -scene_radius;
  //double pos_y   =  scene_radius;
#else
  double minus_x = -scene_radius + scene_center.x();
  //double pos_x   =  scene_radius;
  double minus_z = -scene_radius + scene_center.z();
  //double pos_y   =  scene_radius;
#endif

  if (empac == true) {
    (*ARGS->output) << "EMPAC FLOOR!" << std::endl;
    assert (tiles == 1);
    if (args->graph_visualization_mode == false) {
      assert (fabs((EMPAC_POS_X + EMPAC_MINUS_X) - (EMPAC_POS_Z + EMPAC_MINUS_Z)) < 0.001);
      minus_x = -EMPAC_MINUS_X;
      minus_z = -EMPAC_MINUS_Z;
      x_spacing = EMPAC_POS_X + EMPAC_MINUS_X;
      z_spacing = EMPAC_POS_Z + EMPAC_MINUS_Z;
    } else {
      minus_x = -EMPAC_SMALL_MINUS_X;
      minus_z = -EMPAC_SMALL_MINUS_Z;
      x_spacing = EMPAC_SMALL_POS_X + EMPAC_SMALL_MINUS_X;
      z_spacing = EMPAC_SMALL_POS_Z + EMPAC_SMALL_MINUS_Z;
    }
  }

  MeshMaterial m;

  // FLOOR
  std::vector<std::vector<int> > verts2(tiles+1,std::vector<int>(tiles+1));
  // create vertices

  //if (0) { //args->graph_visualization_mode == true) {
  if (args->graph_visualization_mode == true) {
    
    // didn't fix the problem...
    verts2[0][0] = mesh->addVertex(Vec3f( 0.2925498, 0, -1.95938),-1,-1,0,0);
    verts2[0][1] = mesh->addVertex(Vec3f( 0.387447 , 0,  3.78857),-1,-1,0,1);
    verts2[1][0] = mesh->addVertex(Vec3f( 4.50141  , 0, -2.09495),-1,-1,1,0);
    verts2[1][1] = mesh->addVertex(Vec3f( 4.60036  , 0,  3.64312),-1,-1,1,1);

    /*
      verts2[0][0] = mesh->addVertex(Vec3f( 0.0925498, 0, -2.15938),-1,-1,0,0);
      verts2[0][1] = mesh->addVertex(Vec3f( 0.187447 , 0,  3.98857),-1,-1,0,1);
      verts2[1][0] = mesh->addVertex(Vec3f( 4.70141  , 0, -2.29495),-1,-1,1,0);
      verts2[1][1] = mesh->addVertex(Vec3f( 4.80036  , 0,  3.84312),-1,-1,1,1);
    */

  } else {
    for (int i = 0; i <= tiles; i++) {
      for (int j = 0; j <= tiles; j++) {
        //verts2[i][j] = mesh->addVertex(Vec3f(-scene_radius+i*spacing,0,-scene_radius+j*spacing),-1,-1,i,j);

        verts2[i][j] = mesh->addVertex(Vec3f(minus_x+i*x_spacing,0,minus_z+j*z_spacing),-1,-1,i,j);
      }
    }
  }


  // create the triangles
  for (int i = 0; i < tiles; i++) {
    for (int j = 0; j < tiles; j++) {
      std::stringstream texture_filename;
      texture_filename << "floor_" << i << "_" << j << "_texture.ppm";
      std::stringstream material_name;
      material_name << "floor_" << i << "_" << j;
      std::string text_file = texture_filename.str();
      std::string mat_name = material_name.str();
      if (args->army) {
        text_file = "floor.ppm";
        mat_name = "floor";
        assert (i == 0 && j == 0);
      }
      m.setName(mat_name); m.setDiffuseReflectance(Vec3f(0.8,0.8,0.8)); m.setTextureFilename(text_file,i,j); mesh->addMaterial(m);
      mesh->addElement(new Triangle(verts2[i][j],verts2[i][j+1],verts2[i+1][j+1],mesh,mat_name));
      mesh->addElement(new Triangle(verts2[i][j],verts2[i+1][j+1],verts2[i+1][j],mesh,mat_name));
    }
  }

  if (empac == true) {
    // EMPAC BACK WALL
    // create vertices
    int verts[4];
    verts[0] = mesh->addVertex(Vec3f(EMPAC_BACK_WALL_DISTANCE,EMPAC_BACK_WALL_BOTTOM_EDGE,EMPAC_BACK_WALL_LEFT_EDGE) ,-1,-1,0,0);
    verts[1] = mesh->addVertex(Vec3f(EMPAC_BACK_WALL_DISTANCE,EMPAC_BACK_WALL_BOTTOM_EDGE,EMPAC_BACK_WALL_RIGHT_EDGE),-1,-1,0,1);
    verts[2] = mesh->addVertex(Vec3f(EMPAC_BACK_WALL_DISTANCE,EMPAC_BACK_WALL_TOP_EDGE,   EMPAC_BACK_WALL_RIGHT_EDGE),-1,-1,1,1);
    verts[3] = mesh->addVertex(Vec3f(EMPAC_BACK_WALL_DISTANCE,EMPAC_BACK_WALL_TOP_EDGE,   EMPAC_BACK_WALL_LEFT_EDGE) ,-1,-1,1,0);

    // create the triangles
    std::stringstream texture_filename;
    texture_filename << "empac_back_wall_texture.ppm";
    std::stringstream material_name;
    material_name << "empac_back_wall";
    m.setName(material_name.str()); m.setDiffuseReflectance(Vec3f(0.8,0.8,0.8)); m.setTextureFilename(texture_filename.str()); mesh->addMaterial(m);
    mesh->addElement(new Triangle(verts[0],verts[1],verts[2],mesh,material_name.str()));
    mesh->addElement(new Triangle(verts[0],verts[2],verts[3],mesh,material_name.str()));
  }

  if (args->army) {
    m.setName("wall");      m.setDiffuseReflectance(Vec3f(0.2,0.2,0.9));  m.setTextureFilename("wall.ppm");  mesh->addMaterial(m);
    m.setName("platform");  m.setDiffuseReflectance(Vec3f(0.2,0.9,0.2));  m.setTextureFilename("platform.ppm");  mesh->addMaterial(m);
    m.setName("ramp");      m.setDiffuseReflectance(Vec3f(0.9,0.2,0.2));  m.setTextureFilename("ramp.ppm");  mesh->addMaterial(m);
    m.setName("black");     m.setDiffuseReflectance(Vec3f(0.1,0.1,0.1));  m.setTextureFilename("black.ppm");  mesh->addMaterial(m);
  } else {
    m.setName("EXTRA_wall_top"); m.setExtra(); m.setDiffuseReflectance(Vec3f(0.2,0.2,0.2)); mesh->addMaterial(m);
    m.setName("EXTRA_under_wall"); m.setExtra(); m.setDiffuseReflectance(Vec3f(0.2,0.2,0.2)); mesh->addMaterial(m);
  }

  std::vector<int> verts;
  unsigned int num_walls = numWalls();
  std::stringstream material_name;
  for (unsigned int i = 0; i < num_walls; i++) {
    BasicWall *w = walls[i];
    const std::string &name = w->getName();
    const Vec3f &color = w->getColor();

    m.SetDefaults();

    Vec3f bottom = Vec3f(0,w->getBottomEdge(),0);

    if (w->IsCurvedWall()) {
      Vec3f max_height = Vec3f(0,w->getMaxHeight(),0);
      // ----------------------------
      // CURVED WALL
      int numGoodVerts = w->numGoodVerts();
      std::stringstream texture_filename;

      // inside surface
      material_name.str("");
      material_name << name << "_" << 0;
      texture_filename << material_name.str() << "_texture.ppm";
      m.setName(material_name.str());  m.setDiffuseReflectance(color);  m.setTextureFilename(texture_filename.str()); mesh->addMaterial(m);
      // create the vertices
      std::vector<int> bot_verts;
      std::vector<int> top_verts;
      for (int k = 0; k <= numGoodVerts/2-1; k++) {
        Vec3f tmp = w->getGoodVert(k);
        if (k == 0) {
          Vec3f v = w->getGoodVert(k+1)-w->getGoodVert(k); v.Normalize();
          tmp += v*args->shrink_projection_surfaces;
        }
        if (k == numGoodVerts/2-1) {
          Vec3f v = w->getGoodVert(k)-w->getGoodVert(k-1); v.Normalize();
          tmp -= v*args->shrink_projection_surfaces;
        }
        Vec3f height_adjust(0,args->shrink_projection_surfaces,0);
        bot_verts.push_back(mesh->addVertex(tmp+bottom+height_adjust,-1,-1,0,1-(k)/(numGoodVerts/2.0-1)));
        top_verts.push_back(mesh->addVertex(tmp+max_height-height_adjust,-1,-1,1,1-(k)/(numGoodVerts/2.0-1)));
      }
      // create the faces
      for (int k = 0; k < numGoodVerts/2-1; k++) {
        Vertex *vert_a = mesh->getVertex(bot_verts[k+1]);
        Vertex *vert_b = mesh->getVertex(bot_verts[k]);
        Vertex *vert_c = mesh->getVertex(top_verts[k]);
        Vertex *vert_d = mesh->getVertex(top_verts[k+1]);
        Vec3f center = w->getCircleCenter();
        Vec3f a = vert_a->get(); a.zero_y(); a = center-a; a.Normalize();
        Vec3f b = vert_b->get(); b.zero_y(); b = center-b; b.Normalize();
        Vec3f c = vert_c->get(); c.zero_y(); c = center-c; c.Normalize();
        Vec3f d = vert_d->get(); d.zero_y(); d = center-d; d.Normalize();
        vert_a->setSpecialNormal(a);
        vert_b->setSpecialNormal(b);
        vert_c->setSpecialNormal(c);
        vert_d->setSpecialNormal(d);
        mesh->addElement(new Triangle(bot_verts[k+1],bot_verts[k],top_verts[k],mesh,material_name.str()));
        mesh->addElement(new Triangle(bot_verts[k+1],top_verts[k],top_verts[k+1],mesh,material_name.str()));
      }

      // outside surface
      material_name.str("");
      texture_filename.str("");
      material_name << name << "_" << 2;
      texture_filename << material_name.str() << "_texture.ppm";
      m.setName(material_name.str());   m.setDiffuseReflectance(color);  m.setTextureFilename(texture_filename.str()); mesh->addMaterial(m);
      // create the vertices
      top_verts = std::vector<int>();
      bot_verts = std::vector<int>();
      for (int k = numGoodVerts/2; k <= numGoodVerts-1; k++) {
        Vec3f tmp = w->getGoodVert(k);
        if (k == numGoodVerts/2) {
          Vec3f v = w->getGoodVert(k+1)-w->getGoodVert(k); v.Normalize();
          tmp += v*args->shrink_projection_surfaces;
        }
        if (k == numGoodVerts-1) {
          Vec3f v = w->getGoodVert(k)-w->getGoodVert(k-1); v.Normalize();
          tmp -= v*args->shrink_projection_surfaces;
        }
        Vec3f height_adjust(0,args->shrink_projection_surfaces,0);
        bot_verts.push_back(mesh->addVertex(tmp+bottom+height_adjust,-1,-1,0,1-(k-numGoodVerts/2.0  )/(numGoodVerts/2.0-1)));
        top_verts.push_back(mesh->addVertex(tmp+max_height-height_adjust,-1,-1,1,1-(k-numGoodVerts/2.0  )/(numGoodVerts/2.0-1)));
        //bot_verts.push_back(mesh->addVertex(w->getGoodVert(k)+bottom,-1,-1,0,1-(k-numGoodVerts/2.0  )/(numGoodVerts/2.0-1)));
        //top_verts.push_back(mesh->addVertex(w->getGoodVert(k)+height,-1,-1,1,1-(k-numGoodVerts/2.0  )/(numGoodVerts/2.0-1)));
      }
      // create the faces
      for (int k_ = numGoodVerts/2; k_ < numGoodVerts-1; k_++) {
        int k = k_ - numGoodVerts/2;
        Vertex *vert_a = mesh->getVertex(bot_verts[k+1]);
        Vertex *vert_b = mesh->getVertex(bot_verts[k]);
        Vertex *vert_c = mesh->getVertex(top_verts[k]);
        Vertex *vert_d = mesh->getVertex(top_verts[k+1]);
        Vec3f center = w->getCircleCenter();
        Vec3f a = vert_a->get(); a.zero_y(); a = a-center; a.Normalize();
        Vec3f b = vert_b->get(); b.zero_y(); b = b-center; b.Normalize();
        Vec3f c = vert_c->get(); c.zero_y(); c = c-center; c.Normalize();
        Vec3f d = vert_d->get(); d.zero_y(); d = d-center; d.Normalize();
        vert_a->setSpecialNormal(a);
        vert_b->setSpecialNormal(b);
        vert_c->setSpecialNormal(c);
        vert_d->setSpecialNormal(d);
        mesh->addElement(new Triangle(bot_verts[k+1],bot_verts[k],top_verts[k],mesh,material_name.str()));
        mesh->addElement(new Triangle(bot_verts[k+1],top_verts[k],top_verts[k+1],mesh,material_name.str()));
      }

      // short edge walls
      material_name.str("");
      texture_filename.str("");
      material_name << name << "_" << 1;
      texture_filename << material_name.str() << "_texture.ppm";
      m.setName(material_name.str());   m.setDiffuseReflectance(color);  m.setTextureFilename(texture_filename.str()); mesh->addMaterial(m);
      verts.clear();
      verts.push_back(mesh->addVertex(w->getGoodVert(numGoodVerts/2-1)+bottom,-1,-1,0,1));
      verts.push_back(mesh->addVertex(w->getGoodVert(numGoodVerts/2-1)+max_height,-1,-1,1,1));
      verts.push_back(mesh->addVertex(w->getGoodVert(numGoodVerts/2  )+max_height,-1,-1,1,0));
      verts.push_back(mesh->addVertex(w->getGoodVert(numGoodVerts/2  )+bottom,-1,-1,0,0));
      mesh->addElement(new Triangle(verts[0],verts[1],verts[2],mesh,material_name.str()));
      mesh->addElement(new Triangle(verts[0],verts[2],verts[3],mesh,material_name.str()));
      material_name.str("");
      texture_filename.str("");
      material_name << name << "_" << 3;
      texture_filename << material_name.str() << "_texture.ppm";
      m.setName(material_name.str());   m.setDiffuseReflectance(color);  m.setTextureFilename(texture_filename.str()); mesh->addMaterial(m);
      verts.clear();
      verts.push_back(mesh->addVertex(w->getGoodVert(numGoodVerts-1)+bottom,-1,-1,0,1));
      verts.push_back(mesh->addVertex(w->getGoodVert(numGoodVerts-1)+max_height,-1,-1,1,1));
      verts.push_back(mesh->addVertex(w->getGoodVert(0)+max_height,             -1,-1,1,0));
      verts.push_back(mesh->addVertex(w->getGoodVert(0)+bottom,             -1,-1,0,0));
      mesh->addElement(new Triangle(verts[0],verts[1],verts[2],mesh,material_name.str()));
      mesh->addElement(new Triangle(verts[0],verts[2],verts[3],mesh,material_name.str()));

      // top of curved wall
      int numQuads = w->numConvexQuads();
      for (int k = 0; k < numQuads; k++) {
        ConvexQuad q = w->getConvexQuad(k);
        verts.clear();
        verts.push_back(mesh->addVertex(q[0]+max_height,-1,-1,0,0));
        verts.push_back(mesh->addVertex(q[3]+max_height,-1,-1,0,1));
        verts.push_back(mesh->addVertex(q[2]+max_height,-1,-1,1,1));
        verts.push_back(mesh->addVertex(q[1]+max_height,-1,-1,1,0));
        mesh->addElement(new Triangle(verts[0],verts[1],verts[2],mesh,"EXTRA_wall_top"));
        mesh->addElement(new Triangle(verts[0],verts[2],verts[3],mesh,"EXTRA_wall_top"));
      }

    } else {

      std::vector<Vec3f> heights;
      for (int k = 0; k < w->numGoodVerts(); k++) {
        heights.push_back(Vec3f(0,w->getHeight2(k),0));
      }

      // ----------------------------
      // FLAT OR L-SHAPED WALL
      int numGoodVerts = w->numGoodVerts();
      for (int k = 0; k < numGoodVerts; k++) {
        material_name.str("");
        material_name << name << "_" << k;
        std::stringstream texture_filename;
        texture_filename << material_name.str() << "_texture.ppm";
        //(*ARGS->output) << "set texture filename " << texture_filename.str() << std::endl;
        if (!args->army) {
          m.setName(material_name.str());   m.setDiffuseReflectance(color);  m.setTextureFilename(texture_filename.str()); mesh->addMaterial(m);
        }
        verts.clear();

        verts.push_back(mesh->addVertex(w->getGoodVert((k+1)%numGoodVerts)+bottom   ,-1,-1,0,0));
        verts.push_back(mesh->addVertex(w->getGoodVert(k)                 +bottom   ,-1,-1,0,1));
        double my_height = w->getHeight2(k);
        verts.push_back(mesh->addVertex(w->getGoodVert(k)                 +heights[k],-1,-1,my_height/w->getMaxHeight(),1));
        my_height = w->getHeight2((k+1)%numGoodVerts);
        verts.push_back(mesh->addVertex(w->getGoodVert((k+1)%numGoodVerts)+heights[(k+1)%numGoodVerts],-1,-1,my_height/w->getMaxHeight(),0));

        std::string mat_name = material_name.str();
        if (args->army) {
          if (w->IsRamp()) {
            mat_name = "ramp";
          } else if (w->IsPlatform()) {
            mat_name = "platform";
          } else {
            mat_name = "wall";
          }
        }

        if (mat_name == "ramp" && k == 0) {
          mat_name = "black";
        }


        mesh->addElement(new Triangle(verts[0],verts[1],verts[2],mesh,mat_name));
        mesh->addElement(new Triangle(verts[0],verts[2],verts[3],mesh,mat_name));
      }

      int numQuads = w->numConvexQuads();
      for (int k = 0; k < numQuads; k++) {
        ConvexQuad q = w->getConvexQuad(k);
        verts.clear();
        std::string my_mat = "EXTRA_wall_top";
        if (!args->army) {
          verts.push_back(mesh->addVertex(q[0]+heights[0],-1,-1,0,0));
          verts.push_back(mesh->addVertex(q[3]+heights[3],-1,-1,0,1));
          verts.push_back(mesh->addVertex(q[2]+heights[2],-1,-1,1,1));
          verts.push_back(mesh->addVertex(q[1]+heights[1],-1,-1,1,0));
        } else {
          my_mat = "floor";
          Vec3f v; double s,t;
          v = q[0]+heights[0]; s = (v.x()+scene_radius)/(2*scene_radius); t = (v.z()+scene_radius)/(2*scene_radius);
          verts.push_back(mesh->addVertex(v,-1,-1,s,t));
          v = q[3]+heights[3]; s = (v.x()+scene_radius)/(2*scene_radius); t = (v.z()+scene_radius)/(2*scene_radius);
          verts.push_back(mesh->addVertex(v,-1,-1,s,t));
          v = q[2]+heights[2]; s = (v.x()+scene_radius)/(2*scene_radius); t = (v.z()+scene_radius)/(2*scene_radius);
          verts.push_back(mesh->addVertex(v,-1,-1,s,t));
          v = q[1]+heights[1]; s = (v.x()+scene_radius)/(2*scene_radius); t = (v.z()+scene_radius)/(2*scene_radius);
          verts.push_back(mesh->addVertex(v,-1,-1,s,t));
        }


        mesh->addElement(new Triangle(verts[0],verts[1],verts[2],mesh,my_mat));
        mesh->addElement(new Triangle(verts[0],verts[2],verts[3],mesh,my_mat));

      }
    }
  }





  for (int i = 0; i < furniture.size(); i++) {
    Furniture *f = furniture[i];
    double my_height = f->getHeight();
    Vec3f height = Vec3f(0,my_height,0);
    for (int k = 0; k < 4; k++) {

      std::stringstream material_name;
      material_name << "furniture_" << i << "_" << k;
      std::stringstream texture_filename;
      texture_filename << material_name.str() << "_texture.ppm";
      std::string mat_name = material_name.str();

      if (f->getFurnitureType() == FURNITURE_BED) {
        m.setName(material_name.str());   m.setDiffuseReflectance(furniture_material_bed);  m.setTextureFilename(texture_filename.str()); mesh->addMaterial(m);
      } else if (f->getFurnitureType() == FURNITURE_DESK) {
        m.setName(material_name.str());   m.setDiffuseReflectance(furniture_material_desk);  m.setTextureFilename(texture_filename.str()); mesh->addMaterial(m);
      } else {
        assert (f->getFurnitureType() == FURNITURE_WARDROBE);
        m.setName(material_name.str());   m.setDiffuseReflectance(furniture_material_wardrobe);  m.setTextureFilename(texture_filename.str()); mesh->addMaterial(m);
      }

      (*ARGS->output) << "set texture filename " << texture_filename.str() << std::endl;

      verts.clear();
      verts.push_back(mesh->addVertex((*f)[(k+1)%4] ,-1,-1,0,0));
      verts.push_back(mesh->addVertex((*f)[k]       ,-1,-1,0,1));

      verts.push_back(mesh->addVertex((*f)[k      ]+height,-1,-1,1,1));
      verts.push_back(mesh->addVertex((*f)[(k+1)%4]+height,-1,-1,1,0));


    
      mesh->addElement(new Triangle(verts[0],verts[1],verts[2],mesh,mat_name));
      mesh->addElement(new Triangle(verts[0],verts[2],verts[3],mesh,mat_name));
    }
    
    std::stringstream material_name;
    material_name << "furniture_" << i << "_" << 4;
    std::stringstream texture_filename;
    texture_filename << material_name.str() << "_texture.ppm";
    std::string mat_name = material_name.str();

    if (f->getFurnitureType() == FURNITURE_BED) {
      m.setName(material_name.str());   m.setDiffuseReflectance(furniture_material_bed);  m.setTextureFilename(texture_filename.str()); mesh->addMaterial(m);
    } else if (f->getFurnitureType() == FURNITURE_DESK) {
      m.setName(material_name.str());   m.setDiffuseReflectance(furniture_material_desk);  m.setTextureFilename(texture_filename.str()); mesh->addMaterial(m);
    } else {
      assert (f->getFurnitureType() == FURNITURE_WARDROBE);
      m.setName(material_name.str());   m.setDiffuseReflectance(furniture_material_wardrobe);  m.setTextureFilename(texture_filename.str()); mesh->addMaterial(m);
    }

    // top
    verts.clear();
    verts.push_back(mesh->addVertex((*f)[0]+height,-1,-1,0,0));
    verts.push_back(mesh->addVertex((*f)[3]+height,-1,-1,0,1));
    verts.push_back(mesh->addVertex((*f)[2]+height,-1,-1,1,1));
    verts.push_back(mesh->addVertex((*f)[1]+height,-1,-1,1,0));
    
    mesh->addElement(new Triangle(verts[0],verts[1],verts[2],mesh,mat_name));
    mesh->addElement(new Triangle(verts[0],verts[2],verts[3],mesh,mat_name));
  }


  std::string MATNAMES[6] = {"red_line","yellow_line","green_line","cyan_line","blue_line","purple_line" };
#if VIEW_VIS
  Vec3f MATCOLORS[6] = { Vec3f(0,0,0),
                         Vec3f(0,0,0),
                         Vec3f(0,0,0),
                         Vec3f(0,0,0),
                         Vec3f(0,0,0),
                         Vec3f(0,0,0) };
#else
  Vec3f MATCOLORS[6] = { Vec3f(1,0,0),
                         Vec3f(1,1,0),
                         Vec3f(0,1,0),
                         Vec3f(0,1,1),
                         Vec3f(0,0,1),
                         Vec3f(1,0,1) };

#endif
  std::string MATFILES[6] = {"solid/white.ppm","solid/white1.ppm","solid/white2.ppm","solid/white3.ppm","solid/white4.ppm","solid/white5.ppm"};

  for (int i = 0; i < 6; i++) {
    m.setName(MATNAMES[i]);
    m.setDiffuseReflectance(MATCOLORS[i]);
    m.setTextureFilename(MATFILES[i]);
    mesh->addMaterial(m);
  }


  std::vector<std::pair<double,Vec3f> > view_spots;
  const int num_views_a = 16;
  const int num_views_b = 11;

  for (int i = 0; i < num_views_a; i++) {
    double theta = 2*M_PI*i/double(num_views_a);
    //(*ARGS->output) << "theta " << theta << std::endl;
    Vec3f pos(sin(theta),0,cos(theta));
    pos *= 15 * INCH_IN_METERS;
    pos += scene_center;

    double visibility = CalculateVisibility(pos,Vec3f(0,1,0),args,walls);

    (*ARGS->output) << "VIS a " << visibility << std::endl;

    //if (visibility > 0.5)
    view_spots.push_back(std::make_pair(visibility,pos));
  }


  for (int i = 0; i < num_views_b; i++) {
    double theta = 2*M_PI*i/double(num_views_b);
    //(*ARGS->output) << "theta " << theta << std::endl;
    Vec3f pos(sin(theta),0,cos(theta));
    pos *= 10.5 * INCH_IN_METERS;
    pos += scene_center;

    double visibility = CalculateVisibility(pos,Vec3f(0,1,0),args,walls);

    (*ARGS->output) << "VIS b " << visibility << std::endl;

    //if (visibility > 0.5)
    view_spots.push_back(std::make_pair(visibility*2,pos));
  }

#if VIEW_VIS
  const int num_views = num_views_a + num_views_b;
#endif

  std::sort(view_spots.begin(),view_spots.end(),sort_by_visibility);
  std::vector<int> assignments(people.size(),-1);
  MakeAssignments(assignments,people,view_spots);

  std::ofstream view_file("out.view");
  assert (view_file);
  view_file << "table_center " << scene_center.x() << " " << scene_center.y() << " " << scene_center.z() << std::endl;
  view_file << "table_radius " << scene_radius << std::endl;


#if VIEW_VIS
  for (int j = 0; j < num_views; j++) {
    int assignment = j;
    int i = j % people.size();
#else
    for (unsigned int i = 0; i < people.size(); i++) {
      int assignment = assignments[i];
#endif

      std::string name = people[i].name;
      std::string my_mat = "VIEW_" + name;
      MeshMaterial m;
      m.setName(my_mat);
      m.setTextureFilename(my_mat+".ppm");

      if (!mesh->materialExists(my_mat)) {
        mesh->addMaterial(m);
      }
      Vec3f height(0,0.002,0);
      Vec3f person_position(people[i].x,0,people[i].z);

      verts.clear();

      Vec3f view_placement = view_spots[assignment].second;

      Vec3f top_vector = scene_center-view_placement;
      top_vector.Normalize();
      Vec3f right_vector;
      Vec3f::Cross3(right_vector,top_vector,Vec3f(0,1,0));

      top_vector *= 1.6;
      right_vector *= 1.6;

      view_file << "view " << view_placement.x() << " "<< view_placement.y() << " "<< view_placement.z() << std::endl;

#define PERSON_RADIUS (INCH_IN_METERS * 0.5)

      // crops the circular fisheye to a rectangle w/ no undefined pixels
      verts.push_back(mesh->addVertex(view_placement-0.03*top_vector-0.04*right_vector+height,-1,-1,0.2,0.1));
      verts.push_back(mesh->addVertex(view_placement-0.03*top_vector+0.04*right_vector+height,-1,-1,0.2,0.9));
      verts.push_back(mesh->addVertex(view_placement+0.03*top_vector+0.04*right_vector+height,-1,-1,0.8,0.9));
      verts.push_back(mesh->addVertex(view_placement+0.03*top_vector-0.04*right_vector+height,-1,-1,0.8,0.1));
      mesh->addElement(new Triangle(verts[0],verts[1],verts[2],mesh,my_mat));
      mesh->addElement(new Triangle(verts[0],verts[2],verts[3],mesh,my_mat));

      Vec3f no = view_placement - person_position;
      no.Normalize();
      Vec3f start = person_position + PERSON_RADIUS * no;

      //verts.push_back(mesh->addVertex(person_position+height*0.5,-1,-1,0,0));
      verts.push_back(mesh->addVertex(start+height*0.5,-1,-1,0,0));
      verts.push_back(mesh->addVertex(view_placement+height*0.5,-1,-1,0,0));

#if VIEW_VIS
#else
      mesh->addElement(new Line(verts[4],verts[5],mesh,MATNAMES[i]));
#endif

      verts.push_back(mesh->addVertex(view_placement-0.032*top_vector-0.042*right_vector+0.5*height,-1,-1,0.2,0.1));
      verts.push_back(mesh->addVertex(view_placement-0.032*top_vector+0.042*right_vector+0.5*height,-1,-1,0.2,0.9));
      verts.push_back(mesh->addVertex(view_placement+0.032*top_vector+0.042*right_vector+0.5*height,-1,-1,0.8,0.9));
      verts.push_back(mesh->addVertex(view_placement+0.032*top_vector-0.042*right_vector+0.5*height,-1,-1,0.8,0.1));

      mesh->addElement(new Line(verts[6],verts[7],mesh,MATNAMES[i]));
      mesh->addElement(new Line(verts[7],verts[8],mesh,MATNAMES[i]));
      mesh->addElement(new Line(verts[8],verts[9],mesh,MATNAMES[i]));
      mesh->addElement(new Line(verts[9],verts[6],mesh,MATNAMES[i]));

#define SLOTS 30
      //for (int k = 0; k < SLOTS; k++) {
      //  Vec3f p = person_position + PERSON_RADIUS*Vec3f(cos(k*2*M_PI/double(SLOTS)),0,sin(k*2*M_PI/double(SLOTS)));
      //  verts.push_back(mesh->addVertex(p+height*0.5,-1,-1,0,0));
      //}

      //for (int k = 0; k < SLOTS; k++) {
      //  mesh->addElement(new Line(verts[10+k],verts[10+(k+1)%SLOTS],mesh,MATNAMES[i]));
      //}



    }


}


bool IsInteriorType(Poly *p) {
  return (p != NULL && 
          (p->getCellType() == ACT_INTERIOR ||
           p->getCellType() == ACT_INTERIOR_SKYLIGHT ||
           p->getCellType() == ACT_INTERIOR_FURNITURE ||
           p->getCellType() == ACT_INTERIOR_FURNITURE_SKYLIGHT));
}

bool IsInteriorFurniture(Poly *p) {
  return (p != NULL &&
          (p->getCellType() == ACT_INTERIOR_FURNITURE ||
           p->getCellType() == ACT_INTERIOR_FURNITURE_SKYLIGHT));
}


void Walls::CreateTriMesh(Mesh *arrangement, Mesh *mesh) {

  //  assert (0);

  //    return;

  (*ARGS->output) << "VERSION A" << std::endl;
  
  //bool all_short = allShort();
  //bool all_tall = allTall();
  
  if (args->physical_diffuse == "default") {
    // do nothing
  } else if (args->physical_diffuse == "white_paint") {
    PD_floor = PD_wall = 0.940 * Vec3f(1,1,1);
  } else if (args->physical_diffuse == "white_matte_board") {
    PD_floor = PD_wall = 0.900 * Vec3f(1,1,1);
  } else if (args->physical_diffuse == "12_grey") {
    PD_floor = PD_wall = 0.836 * Vec3f(1,1,1);
  } else if (args->physical_diffuse == "25_grey") {
    PD_floor = PD_wall = 0.775 * Vec3f(1,1,1);
  } else if (args->physical_diffuse == "50_grey") {
    PD_floor = PD_wall = 0.710 * Vec3f(1,1,1);
  } else if (args->physical_diffuse == "75_grey") {
    PD_floor = PD_wall = 0.654 * Vec3f(1,1,1);
  } else if (args->physical_diffuse == "doodle") {
    PD_floor = PD_wall = 0.642 * Vec3f(1,1,1);
  } else if (args->physical_diffuse == "100_grey") {
    PD_floor = PD_wall = 0.615 * Vec3f(1,1,1);
  } else if (args->physical_diffuse == "grey") {
    PD_floor = PD_wall = 0.365 * Vec3f(1,1,1);
  } else if (args->physical_diffuse == "medium") {
    PD_floor = PD_wall = 0.169 * Vec3f(1,1,1);
  } else if (args->physical_diffuse == "black") {
    PD_floor = PD_wall = 0.059 * Vec3f(1,1,1);
  } else if (args->physical_diffuse == "polarized") {
    PD_floor = PD_wall = 0.151 * Vec3f(1,1,1);
  } else {
    (*ARGS->output) << "ERROR: unknown physical_diffuse " << args->physical_diffuse << std::endl;
    assert(0);
  }

  if (args->virtual_scene == "default") {
    // do nothing
  } else if (args->virtual_scene == "saturated_cornell_box") {
    SetBoxMaterials(Vec3f(0.95,0.05,0.05),Vec3f(0.8,0.8,0.8),Vec3f(0.05,0.05,0.95));
  } else if (args->virtual_scene == "pastel_cornell_box") {
    SetBoxMaterials(Vec3f(0.8,0.5,0.5),Vec3f(0.836,0.836,0.836),Vec3f(0.5,0.5,0.8));
  } else if (args->virtual_scene == "dark_cornell_box") {
    SetBoxMaterials(Vec3f(0.6,0.2,0.2),Vec3f(0.615,0.615,0.615),Vec3f(0.2,0.2,0.6));
  } else if (args->virtual_scene == "light_box") {
    SetBoxMaterials(Vec3f(0.9,0.9,0.9),Vec3f(0.9,0.9,0.9),Vec3f(0.9,0.9,0.9));
  } else if (args->virtual_scene == "dark_box") {
    SetBoxMaterials(Vec3f(0.4,0.4,0.4),Vec3f(0.4,0.4,0.4),Vec3f(0.4,0.4,0.4));
  } else if (args->virtual_scene == "light_and_dark_box") {
    SetBoxMaterials(Vec3f(0.9,0.9,0.9),Vec3f(0.6,0.6,0.6),Vec3f(0.2,0.2,0.2));
  } else if (args->virtual_scene == "nearly_black_box") {
    SetBoxMaterials(Vec3f(0.05,0.05,0.05),Vec3f(0.6,0.6,0.6),Vec3f(0.2,0.6,0.2));
  } else if (args->virtual_scene == "nearly_black_box_2") {
    SetBoxMaterials(Vec3f(0.05,0.05,0.05),Vec3f(0.4,0.4,0.4),Vec3f(0.05,0.4,0.05));
  } else if (args->virtual_scene == "gamma_test_1") {
    SetBoxMaterials(Vec3f(0.05,0.05,0.05),Vec3f(0.95,0.95,0.95),Vec3f(0.05,0.05,0.05));
  } else if (args->virtual_scene == "gamma_test_2") {
    SetBoxMaterials(Vec3f(0.05,0.05,0.05),Vec3f(0.5,0.5,0.5),Vec3f(0.95,0.95,0.95));
  } else if (args->virtual_scene == "gamma_test_3") {
    SetBoxMaterials(Vec3f(0.95,0.95,0.95),Vec3f(0.05,0.05,0.05),Vec3f(0.5,0.5,0.5));
  } else {
    (*ARGS->output) << "ERROR: unknown virtual_scene " << args->virtual_scene << std::endl;
    assert(0);
  }


  // create materials
  MeshMaterial m;
  m.setName("floor"); m.setDiffuseReflectance(floor_material); m.setPhysicalDiffuseReflectance(PD_floor); mesh->addMaterial(m);
  for (unsigned int i = 0; i < wall_materials.size(); i++) {
    m.setName(wall_material_name(i,false)); m.setDiffuseReflectance(wall_materials[i]); m.setPhysicalDiffuseReflectance(PD_wall); mesh->addMaterial(m);
    m.setName(wall_material_name(i,true)); m.setDiffuseReflectance(wall_materials[i]); m.setFillin(); m.setPhysicalDiffuseReflectance(black); mesh->addMaterial(m);
  }
  m.setName("FILLIN_ceiling"); m.setDiffuseReflectance(ceiling_material); m.setPhysicalDiffuseReflectance(black); m.setFillin(); mesh->addMaterial(m);
  m.setName("EXTRA_floor"); m.setDiffuseReflectance(dark_grey); m.setPhysicalDiffuseReflectance(PD_floor); m.setExtra(); mesh->addMaterial(m);
  m.setName("EXTRA_wall"); m.setDiffuseReflectance(dark_grey); m.setPhysicalDiffuseReflectance(PD_wall); m.setExtra(); mesh->addMaterial(m);
  m.setName("GLASS_1"); m.setDiffuseReflectance(black); m.setPhysicalDiffuseReflectance(PD_wall); m.setGlass(); m.setSpecularTransmittance(Vec3f(0.8,0.8,0.8)); mesh->addMaterial(m);
  m.setName("FILLIN_GLASS_1"); m.setDiffuseReflectance(black); m.setPhysicalDiffuseReflectance(PD_wall); m.setGlass(); m.setFillin(); m.setSpecularTransmittance(Vec3f(0.8,0.8,0.8)); mesh->addMaterial(m);
  m.setName("EMITTER"); m.setDiffuseReflectance(ceiling_material); m.setPhysicalDiffuseReflectance(PD_wall); m.setEmitter(); m.setEmittance(Vec3f(1,1,1)); mesh->addMaterial(m);
  m.setName("FILLIN_EMITTER"); m.setDiffuseReflectance(ceiling_material); m.setPhysicalDiffuseReflectance(PD_wall); m.setFillin(); m.setEmitter(); m.setEmittance(Vec3f(1,1,1)); mesh->addMaterial(m);



  
  m.setName("furniture_bed"); m.setDiffuseReflectance(furniture_material_bed); m.setPhysicalDiffuseReflectance(PD_floor); mesh->addMaterial(m);
  m.setName("furniture_desk"); m.setDiffuseReflectance(furniture_material_desk); m.setPhysicalDiffuseReflectance(PD_floor); mesh->addMaterial(m);
  m.setName("furniture_wardrobe"); m.setDiffuseReflectance(furniture_material_wardrobe); m.setPhysicalDiffuseReflectance(PD_floor); mesh->addMaterial(m);


  // to efficiently find shared edge vertices
  vphashtype vphash;



  for (elementshashtype::const_iterator foo = arrangement->getElements().begin();
       foo != arrangement->getElements().end();
       foo++) {
    Poly *p = (Poly*)foo->second;

    //std::cout << "call AnalyzeForShortInferredWall(this) A" << std::endl;
    bool shortinferred = p->AnalyzeForShortInferredWall(this);
    if (shortinferred) {
      (*ARGS->output) << "HAPPY DAY! " << std::endl;
    }

    // ================================================================================================================
    // HORIZONTAL SURFACES
    // ================================================================================================================
    if (p->getCellType() == ACT_INTERIOR) {
      // FLOOR POLYGON
      CreateTrianglesForCell(arrangement,mesh,vphash,p,HASH_FLOOR,0,false,"floor");
      // CEILING POLYGON
      Vec3f h = arrangement->getVertex((*p)[0])->get();
      CreateTrianglesForCell(arrangement,mesh,vphash,p,HASH_CEILING,ceilingHeight(h),true,"FILLIN_ceiling");
    } else if (p->getCellType() == ACT_INTERIOR_SKYLIGHT) {
      // SKYLIGHT FLOOR POLYGON
      CreateTrianglesForCell(arrangement,mesh,vphash,p,HASH_FLOOR,0,false,"floor");
      // CEILING POLYGON
      Vec3f h = arrangement->getVertex((*p)[0])->get();
      CreateTrianglesForCell(arrangement,mesh,vphash,p,HASH_CEILING,ceilingHeight(h),true,"FILLIN_GLASS_1");
    } else if (IsInteriorFurniture(p)) {
      // TOP OF FURNITURE POLYGON
      Vec3f h = arrangement->getVertex((*p)[0])->get();
      if (p->getFurnitureType() == FURNITURE_BED) {
        CreateTrianglesForCell(arrangement,mesh,vphash,p,HASH_BED,BED_HEIGHT,false,"furniture_bed");
      } else if (p->getFurnitureType() == FURNITURE_DESK) {
        CreateTrianglesForCell(arrangement,mesh,vphash,p,HASH_DESK,DESK_HEIGHT,false,"furniture_desk");
      } else {
        assert (p->getFurnitureType() == FURNITURE_WARDROBE);
        CreateTrianglesForCell(arrangement,mesh,vphash,p,HASH_WARDROBE,WARDROBE_HEIGHT,false,"furniture_wardrobe");
      }
      if (p->getCellType() == ACT_INTERIOR_FURNITURE) {
        // CEILING POLYGON
        CreateTrianglesForCell(arrangement,mesh,vphash,p,HASH_CEILING,ceilingHeight(h),true,"FILLIN_ceiling");
      } else {
        assert (p->getCellType() == ACT_INTERIOR_FURNITURE_SKYLIGHT);
        CreateTrianglesForCell(arrangement,mesh,vphash,p,HASH_CEILING,ceilingHeight(h),true,"FILLIN_GLASS_1");
      }
    } else if (isWallOrWindow(p->getCellType()) || shortinferred) {
      // TOP OF WALL POLYGON
      double height = GREEN_HEIGHT;
      if (!shortinferred) {
        height = walls[p->getWallID()]->getMaxHeight();
      }
      Vec3f h = arrangement->getVertex((*p)[0])->get();
      height = ceilingHeight(h);
      CreateTrianglesForCell(arrangement,mesh,vphash,p,HASH_CEILING,height,false,"EXTRA_wall");
    } else {
      // EXTRA FLOOR POLYGONS
      CreateTrianglesForCell(arrangement,mesh,vphash,p,HASH_FLOOR,0,false,"EXTRA_floor");
    }

    // ================================================================================================================
    // VERTICAL SURFACES
    // ================================================================================================================

    // handle walls
    int num_verts = p->numVertices();
    for (int i = 0; i < num_verts; i++) {
      std::vector<Element*> neighbors = p->getNeighbors(i);
      assert (neighbors.size() <= 1);
      Poly *p2 = NULL;
      if (neighbors.size() == 1)
        p2 = (Poly*)neighbors[0];

      bool projection_surface =
        IsInteriorType(p) &&
        (p2 != NULL && isWallOrWindow(p2->getCellType()));
      bool fillin_surface =
        IsInteriorType(p) &&
        (p2 == NULL || (!isWallOrWindow(p2->getCellType()) && !IsInteriorType(p2)));
      bool extra_surface =
        !IsInteriorType(p) &&
        !isWallOrWindow(p->getCellType()) &&
        (p2 != NULL && isWallOrWindow(p2->getCellType()));

      //bool furniture =  IsInteriorType(p) && !IsInteriorFurniture(p) && IsInteriorFurniture(p2);
      bool furniture =  
        IsInteriorType(p) && 
        IsInteriorFurniture(p2) &&
        (!IsInteriorFurniture(p) || p->getFurnitureType() < p2->getFurnitureType());

      bool shortinferred = false;
      if (fillin_surface) {
        //std::cout << "call AnalyzeForShortInferredWall(this) B" << std::endl;
        shortinferred = p2->AnalyzeForShortInferredWall(this);
      }

      if (projection_surface || fillin_surface || extra_surface || furniture) {

        Vec3f h = arrangement->getVertex((*p)[i])->get();
        Vec3f h2 = arrangement->getVertex((*p)[(i+1)%num_verts])->get();
        std::vector<int> verts;
        
        verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[i              ],HASH_FLOOR     ,0                                )); //0
        verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[(i+1)%num_verts],HASH_FLOOR     ,0                                )); //1
        verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[i              ],HASH_BED       ,BED_HEIGHT                       )); //2
        verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[(i+1)%num_verts],HASH_BED       ,BED_HEIGHT                       )); //3
        verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[i              ],HASH_DESK      ,DESK_HEIGHT                      )); //4
        verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[(i+1)%num_verts],HASH_DESK      ,DESK_HEIGHT                      )); //5
        verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[i              ],HASH_DIVIDER   ,GREEN_HEIGHT                     )); //6
        verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[(i+1)%num_verts],HASH_DIVIDER   ,GREEN_HEIGHT                     )); //7
        verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[i              ],HASH_WARDROBE  ,WARDROBE_HEIGHT                  )); //8
        verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[(i+1)%num_verts],HASH_WARDROBE  ,WARDROBE_HEIGHT                  )); //9
        verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[i              ],HASH_WINDOW_TOP,ceilingHeight(h)-WINDOW_FROM_TOP )); //10
        verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[(i+1)%num_verts],HASH_WINDOW_TOP,ceilingHeight(h2)-WINDOW_FROM_TOP)); //11
        verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[i              ],HASH_CEILING   ,ceilingHeight(h)                 )); //12
        verts.push_back(AddVertexHelper(arrangement,mesh,vphash,(*p)[(i+1)%num_verts],HASH_CEILING   ,ceilingHeight(h2)                )); //13

        double wall_height = -1;

        if (!fillin_surface) {
          assert (p2 != NULL);
          int id = p2->getWallID();
          // ----------------
          // FIXME:  NOT SURE WHY THE WALL ID MIGHT BE INVALID  (but this was happening jan/feb 2016)
          // ----------------
          if (id < walls.size()) {
            assert (id >= 0 && id < walls.size());
            BasicWall* w = walls[id];
            wall_height = w->getMaxHeight();
          }
        }

        std::string mat_1, mat_2, mat_3;
        if (projection_surface) {
          assert (p2 != NULL && !fillin_surface && !extra_surface);
          enum ARRANGEMENT_CELL_TYPE type2 = p2->getCellType();
          assert (isWallOrWindow(type2));
          int wall_id = p2->getWallID();
          assert (wall_id >= 0); 
          mat_1 = mat_2 = mat_3 = wall_material_name(walls[wall_id]->getMaterialIndex(),false);
          if (type2 == ACT_WINDOW_CYAN || type2 == ACT_WINDOW_YELLOW)  { mat_2 = "GLASS_1"; }
          if (type2 == ACT_WINDOW_CYAN || type2 == ACT_WINDOW_MAGENTA) { mat_3 = "GLASS_1"; }
          Vec3f normal;
          computeNormal(mesh->getVertex(verts[0])->get(),
                        mesh->getVertex(verts[1])->get(),
                        mesh->getVertex(verts[2])->get(),
                        normal);
        } 

        else if (fillin_surface) {
          assert (!extra_surface && !projection_surface);
          assert (p != NULL);
          if (p2 == NULL) continue;
          assert (p2 != NULL);
          try {
            WallFromFingerprint(p,p2);
          } catch (...) {
          }
          int i = 0;
          //HACK HACK HACK
          std::string name = wall_material_name(i,true);
          if (args->fillin_ceiling_only) {
            name = wall_material_name(i,false);
          }
          //HACK HACK HACK
          mat_1 = mat_2 = mat_3 = name;
        } 

        else if (extra_surface) {
          assert (!projection_surface && !fillin_surface);
          mat_1 = mat_2 = mat_3 = "EXTRA_wall";
        }


        if (furniture && p2->getFurnitureType() == FURNITURE_BED) {
          mat_1 = mat_2 = mat_3 = "furniture_bed";
        }
        if (furniture && p2->getFurnitureType() == FURNITURE_DESK) {
          mat_1 = mat_2 = mat_3 = "furniture_desk";
        }
        if (furniture && p2->getFurnitureType() == FURNITURE_WARDROBE) {
          mat_1 = mat_2 = mat_3 = "furniture_wardrobe";
        }


        if (!IsInteriorFurniture(p)) {
          mesh->addElement(new Triangle(verts[0],verts[3],verts[1],mesh,mat_1));
          mesh->addElement(new Triangle(verts[0],verts[2],verts[3],mesh,mat_1));
        }

        if (furniture && p2->getFurnitureType() == FURNITURE_BED) { continue; }

        if (!IsInteriorFurniture(p) || p->getFurnitureType() == FURNITURE_BED) {
          mesh->addElement(new Triangle(verts[2],verts[5],verts[3],mesh,mat_1));
          mesh->addElement(new Triangle(verts[2],verts[4],verts[5],mesh,mat_1));
        }

        if (furniture && p2->getFurnitureType() == FURNITURE_DESK) { continue; }

        mesh->addElement(new Triangle(verts[4],verts[7],verts[5],mesh,mat_2));
        mesh->addElement(new Triangle(verts[4],verts[6],verts[7],mesh,mat_2));

        if (wall_height == GREEN_HEIGHT) { continue; }

        mesh->addElement(new Triangle(verts[6],verts[9],verts[7],mesh,mat_3));
        mesh->addElement(new Triangle(verts[6],verts[8],verts[9],mesh,mat_3));

        if (furniture && p2->getFurnitureType() == FURNITURE_WARDROBE) { continue; }
        
        mesh->addElement(new Triangle(verts[8],verts[11],verts[9],mesh,mat_3));
        mesh->addElement(new Triangle(verts[8],verts[10],verts[11],mesh,mat_3));
        
        mesh->addElement(new Triangle(verts[10],verts[13],verts[11],mesh,mat_1));
        mesh->addElement(new Triangle(verts[10],verts[12],verts[13],mesh,mat_1));
        
      }
    }

  }

  vphash.clear();

  std::cout << "DONE" << std::endl;
}








std::vector<WallCamera> GLOBAL_surface_cameras;

// defined in surface_camera.cpp
void SurfaceCamera(const Vec3f &eye, const Vec3f &direction, const Vec3f &up,
                   GLdouble height, GLdouble width,
                   GLint texture_height, GLint texture_width,
                   GLdouble near, GLdouble far,
                   const char *glcam_filename);
 
void LogSurfaceCamera(const Vec3f &eye, const Vec3f &direction, const Vec3f &up,
                      GLdouble height, GLdouble width,
                      GLint texture_height, GLint texture_width,
                      GLdouble nearParam, GLdouble farParam,
                      const char *glcam_filename) {
  WallCamera tmp;
  tmp.eye = eye;
  tmp.up = up;
  tmp.direction = direction;
  Vec3f::Cross3(tmp.horizontal,tmp.up,tmp.direction);
  tmp.height = height;
  tmp.width = width;
  tmp.nearPlane = nearParam;
  tmp.farPlane = farParam;
  GLOBAL_surface_cameras.push_back(tmp);
  SurfaceCamera(eye,direction,up,height,width,texture_height,texture_width,nearParam,farParam,glcam_filename);
}

void Walls::CreateSurfaceCameras() {
  GLOBAL_surface_cameras.clear();
  //(*ARGS->output) << "IN CREATE SURFACE CAMERAS" << std::endl;
  int num_walls = numWalls();
  double epsilon = scene_radius*0.002;
  double pixels_per_meter = 1024;
  for (int i = 0; i < num_walls; i++) {
    BasicWall *w = walls[i];
    const std::string &name = w->getName();
    // ----------------------------
    // CURVED WALL
    if (w->IsCurvedWall()) {
      Vec3f front_start = w->front_start();  // front left
      Vec3f front_end   = w->front_end();    // front right
      Vec3f back_start  = w->back_start();   // back left  (viewed from front)
      Vec3f back_end    = w->back_end();     // back right (viewed from front)
      // front side (surface #0)
      GLdouble height = w->getMaxHeight();
      double bottom = w->getBottomEdge();
      GLdouble width = (front_start-front_end).Length();
      double depth = -1;
      GLdouble arclength = 0;
      for (int j = 0; j < w->numConvexQuads(); j++) {
        const ConvexQuad &q = w->getConvexQuad(j);
        double d = PerpendicularDistanceToLine(q[0],front_start,front_end);
        arclength += DistanceBetweenTwoPoints(q[0],q[1]);
        if (d > depth) { depth = d; }
      }
      assert (arclength >= width);
      Vec3f v_w = front_end-front_start;
      v_w.Normalize();
      Vec3f up = Vec3f(0,1,0);
      Vec3f direction; Vec3f::Cross3(direction,v_w,up);
      assert (direction.Length() > 0.99);
      GLdouble nearPoint = 1.0*epsilon;
      
      GLdouble farPoint = 2000.0*epsilon + depth;

      Vec3f eye = 0.5 * (front_start + front_end) + (0.5*(height-bottom)+bottom)*up - 2.0*epsilon*direction;
      GLint texture_height = (height-bottom) * pixels_per_meter;
      GLint texture_width = arclength * pixels_per_meter;
      if (args->surface_cameras_fixed_size > 0) {
        texture_height=texture_width = args->surface_cameras_fixed_size;
      }
      std::stringstream glcam_filename;
      glcam_filename << "surface_camera_" << name << "_" << 0 << ".glcam";
      //(*ARGS->output) << "FILENAME: " << glcam_filename.str() << std::endl;
      LogSurfaceCamera(eye,direction,up,height-bottom,width,texture_height,texture_width, nearPoint,farPoint,glcam_filename.str().c_str());
      // back side (surface #2)
      width = (back_start-back_end).Length();
      depth = -1;
      arclength = 0;
      for (int j = 0; j < w->numConvexQuads(); j++) {
        const ConvexQuad &q = w->getConvexQuad(j);
        double d = PerpendicularDistanceToLine(q[3],back_start,back_end);
        arclength += DistanceBetweenTwoPoints(q[2],q[3]);
        if (d > depth) { depth = d; }
      }
      texture_width = arclength * pixels_per_meter;
      if (args->surface_cameras_fixed_size > 0) {
        texture_height=texture_width = args->surface_cameras_fixed_size;
      }
      assert (arclength >= width);
      nearPoint = 1.0*epsilon;

      farPoint = 2000.0*epsilon + depth;


      direction = -direction;
      eye = 0.5 * (back_start+back_end) + (0.5*(height-bottom)+bottom)*up - (2.0*epsilon+depth)*direction;
      glcam_filename.str("");
      glcam_filename << "surface_camera_" << name << "_" << 2 << ".glcam";
      //(*ARGS->output) << "FILENAME: " << glcam_filename.str() << std::endl;
      LogSurfaceCamera(eye,direction,up,height-bottom,width,texture_height,texture_width, nearPoint,farPoint,glcam_filename.str().c_str());
    }
    // ----------------------------
    // FLAT WALL
    else {
      int good_verts = w->numGoodVerts();
      GLdouble height = w->getMaxHeight();
      double bottom = w->getBottomEdge();
      for (int j = 0; j < good_verts; j++) {
        if (j == good_verts-1) continue;
        if (j == good_verts/2-1) continue;
        Vec3f A = w->getGoodVert(j);
        Vec3f B = w->getGoodVert((j+1)%good_verts);
        GLdouble width = (B-A).Length();
        GLdouble nearPoint = 1.0*epsilon;

        // FIXME:  WHEN CURVED WALLS ARE REIMPLEMENTED, WILL NEED TO CHANGE THIS!
        // this setting worked for curved walls (but for flat things, this shows data on the other side!)
        // GLdouble farPoint = 2000.0*epsilon;
        // 
        GLdouble farPoint = 5.0*epsilon;

        Vec3f up = Vec3f(0,1,0);
        Vec3f horizontal = B-A; horizontal.Normalize();
        Vec3f direction;  Vec3f::Cross3(direction,horizontal,up);
        assert (fabs(1-direction.Length()) < 0.00001);
        Vec3f eye = 0.5 * (B+A) + (0.5*(height-bottom)+bottom)*up - 2.0*epsilon*direction;
        GLint texture_height = (height-bottom) * pixels_per_meter;
        GLint texture_width = width * pixels_per_meter;
        if (args->surface_cameras_fixed_size > 0) {
          texture_height=texture_width = args->surface_cameras_fixed_size;
        }
        std::stringstream glcam_filename;
        glcam_filename << "surface_camera_" << name << "_" << j << ".glcam";
        //(*ARGS->output) << "FILENAME: " << glcam_filename.str() << std::endl;
        LogSurfaceCamera(eye,direction,up,height-bottom,width,texture_height,texture_width, nearPoint,farPoint,glcam_filename.str().c_str());
      }
    }
  }

  // FURNITURE
  for (int i = 0; i < furniture.size(); i++) {
    Furniture* f = furniture[i];
    GLdouble height = f->getHeight();
    double bottom = 0 ;
    for (int j = 0; j < 4; j++) {
      Vec3f A = (*f)[j];
      Vec3f B = (*f)[(j+1)%4];
      GLdouble width = (B-A).Length();
      GLdouble nearPoint = 1.0*epsilon;

      // FIXME:  WHEN CURVED WALLS ARE REIMPLEMENTED, WILL NEED TO CHANGE THIS!
      // this setting worked for curved walls (but for flat things, this shows data on the other side!)
      // GLdouble farPoint = 2000.0*epsilon;
      // 
      GLdouble farPoint = 5.0*epsilon;

      Vec3f up = Vec3f(0,1,0);
      Vec3f horizontal = B-A; horizontal.Normalize();
      Vec3f direction;  Vec3f::Cross3(direction,horizontal,up);
      assert (fabs(1-direction.Length()) < 0.00001);
      Vec3f eye = 0.5 * (B+A) + (0.5*(height-bottom)+bottom)*up - 2.0*epsilon*direction;
      GLint texture_height = (height-bottom) * pixels_per_meter;
      GLint texture_width = width * pixels_per_meter;
      if (args->surface_cameras_fixed_size > 0) {
        texture_height=texture_width = args->surface_cameras_fixed_size;
      }


      (*ARGS->output) << " UP:         " << up << std::endl;
      (*ARGS->output) << " HORIZONTAL: " << horizontal << std::endl;
      (*ARGS->output) << " DIRECTION:  " << direction << std::endl;

      std::stringstream glcam_filename;
      glcam_filename << "surface_camera_furniture_" << i << "_" << j << ".glcam";
      LogSurfaceCamera(eye,direction,up,height-bottom,width,texture_height,texture_width, nearPoint,farPoint,glcam_filename.str().c_str());
    }


    Vec3f A = (*f)[0]+Vec3f(0,height,0);
    Vec3f B = (*f)[1]+Vec3f(0,height,0);
    Vec3f C = (*f)[2]+Vec3f(0,height,0);
    Vec3f D = (*f)[3]+Vec3f(0,height,0);
    GLdouble width = (B-A).Length();
    GLdouble depth = (B-C).Length();
    GLdouble nearPoint = 1.0*epsilon;

    // FIXME:  WHEN CURVED WALLS ARE REIMPLEMENTED, WILL NEED TO CHANGE THIS!
    // this setting worked for curved walls (but for flat things, this shows data on the other side!)
    // GLdouble farPoint = 2000.0*epsilon;
    // 
    GLdouble farPoint = 5.0*epsilon;

    Vec3f up = B-A/*C-B*/; up.Normalize();
    Vec3f horizontal = B-C/*B-A*/;  horizontal.Normalize();
    Vec3f direction;  Vec3f::Cross3(direction,horizontal,up);

    (*ARGS->output) << "UP:         " << up << std::endl;
    (*ARGS->output) << "HORIZONTAL: " << horizontal << std::endl;
    (*ARGS->output) << "DIRECTION:  " << direction << std::endl;

    assert (fabs(1-direction.Length()) < 0.00001);
    //    Vec3f eye = 0.25 * (A+B+C+D) - 2.0*epsilon*direction;
    Vec3f eye = 0.25 * (A+B+C+D);
    (*ARGS->output) << "EYE:         " << eye << std::endl;
    eye = eye - 2.0*epsilon*direction;
    (*ARGS->output) << "EYE after:         " << eye << std::endl;
    (*ARGS->output) << "epsilon:         " << epsilon << std::endl;

    GLint texture_height = depth * pixels_per_meter;
    GLint texture_width = width * pixels_per_meter;


    (*ARGS->output) << "height:         " << height << std::endl;

    if (args->surface_cameras_fixed_size > 0) {
      texture_height=texture_width = args->surface_cameras_fixed_size;
    }
    std::stringstream glcam_filename;
    glcam_filename << "surface_camera_furniture_" << i << "_" << 4 << ".glcam";
    LogSurfaceCamera(eye,direction,up,width,depth,texture_height,texture_width, nearPoint,farPoint,glcam_filename.str().c_str());

  }
  
  // ----------------------------
  // THE FLOOR
  // the floor plane cameras...

#if 0
  double minus_x = -scene_radius;
  //double pos_x   =  scene_radius;
  double minus_z = -scene_radius;
  //double pos_y   =  scene_radius;
#else
  double minus_x = -scene_radius + scene_center.x();
  //double pos_x   =  scene_radius;
  double minus_z = -scene_radius + scene_center.z();
  //double pos_y   =  scene_radius;
#endif

  if (empac == true) {
    (*ARGS->output) << "EMPAC FLOOR! FOR FLOOR PLANE CAMERAS" << std::endl;
    assert (fabs((EMPAC_POS_X + EMPAC_MINUS_X) - (EMPAC_POS_Z + EMPAC_MINUS_Z)) < 0.001);
    minus_x = -EMPAC_MINUS_X;
    minus_z = -EMPAC_MINUS_Z;
  }

  GLdouble height = 2.0*scene_radius;
  GLdouble width = 2.0*scene_radius;
  GLdouble nearPoint = 1.0*epsilon;

  // FIXME:  WHEN CURVED WALLS ARE REIMPLEMENTED, WILL NEED TO CHANGE THIS!
  // this setting worked for curved walls (but for flat things, this shows data on the other side!)
  // GLdouble farPoint = 2000.0*epsilon;
  // 
  GLdouble farPoint = 5.0*epsilon;

  Vec3f direction = Vec3f(0,-1,0);
  Vec3f up = Vec3f(1,0,0);

  int tiles = args->floor_cameras_tiled;
  assert (tiles >= 1);
  //(*ARGS->output) << "TILED FLOOR " << tiles << "x" << tiles << std::endl;

  double spacing = 2.0*scene_radius / tiles;
  assert (args->graph_visualization_mode == false);

  height = width = spacing;
  GLint texture_height = height * pixels_per_meter;
  GLint texture_width = width * pixels_per_meter;
  if (args->surface_cameras_fixed_size > 0) {
    texture_height=texture_width = args->surface_cameras_fixed_size;
  }

  for (int i = 0; i < tiles; i++) {
    for (int j = 0; j < tiles; j++) {
      //Vec3f eye = Vec3f(-scene_radius,0,-scene_radius) - 2.0*epsilon*direction;
      Vec3f eye = Vec3f(minus_x,0,minus_z) - 2.0*epsilon*direction;
      eye += Vec3f((i+0.5)*spacing,0,(j+0.5)*spacing);
      std::stringstream glcam_filename;
      glcam_filename << "surface_camera_floor_" << i << "_" << j << ".glcam";
      //(*ARGS->output) << "FILENAME: " << glcam_filename.str() << std::endl;
      LogSurfaceCamera(eye,direction,up,height,width,texture_height,texture_width, nearPoint,farPoint,glcam_filename.str().c_str());
    }
  }
}
