#ifndef _WIN32
#include <unistd.h>
#endif
#include <ctype.h>
#include <fstream>
#include <iomanip>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "utils.h"
#include "mesh.h"
#include "vertex.h"
#include "triangle.h"
#include "quad.h"
#include "meshio.h"
#include "meshmanager.h"
#include "glCanvas.h"
#include "argparser.h"
#include "remesh.h"
#include "nosfile.h"
#include "wall.h"

#include "nlohmann_json.hpp"

#include <iostream>
#include <map>

#include "argparser.h"
extern ArgParser *ARGS;


void PuzzleMode(MeshManager *meshes);
float LoadNorth(const std::string &full_filename);
void  LoadGeoLocation(const std::string &full_filename, double &n, double &lat, double &lon);

// ---------------------------------------
// ---------------------------------------

double color_diff(const Vec3f &color, const Vec3f &target) {
  return (fabs(color.r()-target.r()) + fabs(color.g()-target.g()) + fabs(color.b()-target.b()));
}

void HACK_MY_COLOR(Vec3f &color) {

#if 1
  //(*ARGS->output) << "NOT HACKING" << std::endl;
  return;
#endif

  (*ARGS->output) << "HACK_MY_COLOR ";
  (*ARGS->output) << color.r() << " " << color.g() << " " << color.b();

  double white_diff  = color_diff(color,Vec3f(1.00,1.00,1.00));
  double yellow_diff = color_diff(color,Vec3f(1.00,1.00,0.64));
  double green_diff  = color_diff(color,Vec3f(0.33,0.49,0.45));
  double red_diff    = color_diff(color,Vec3f(0.61,0.31,0.33));
  double blue_diff   = color_diff(color,Vec3f(0.31,0.38,0.67));
  double orange_diff = color_diff(color,Vec3f(0.74,0.52,0.34));

  //(*ARGS->output) << std::endl << white_diff << " " << yellow_diff << " " << green_diff << " " << red_diff << " " << blue_diff << " " << orange_diff << std::endl;

  double best_diff = 10000;
  if (white_diff  < best_diff) { best_diff = white_diff;  color = Vec3f(0.95,0.95,0.95); }
  if (yellow_diff < best_diff) { best_diff = yellow_diff; color = Vec3f(0.70,0.70,0.10); }
  if (green_diff  < best_diff) { best_diff = green_diff;  color = Vec3f(0.10,0.70,0.10); }
  if (red_diff    < best_diff) { best_diff = red_diff;    color = Vec3f(0.70,0.10,0.10); }
  if (blue_diff   < best_diff) { best_diff = blue_diff;   color = Vec3f(0.10,0.10,0.70); }
  if (orange_diff < best_diff) { best_diff = orange_diff; color = Vec3f(0.70,0.40,0.10); }
  
  (*ARGS->output) << " -> " << color.r() << " " << color.g() << " " << color.b() << std::endl;
}

/*  SAMPLE COLORS FROM WALL FILE
1.000 1.000 1.000   
1.000 1.000 0.643   YELLOW
0.329 0.490 0.451   GREEN
0.612 0.306 0.329   RED
0.310 0.384 0.671   BLUE
0.737 0.522 0.337   ORANGY TAN
*/


// ---------------------------------------
// ---------------------------------------

#define MAX_PARSER_TOKEN_LENGTH 100

int getToken(FILE *file, char token[MAX_PARSER_TOKEN_LENGTH]);

// =========================================================================
// =========================================================================

void MeshIO::Load(MeshManager *meshes, const std::string &filename) {
  Mesh *mesh = meshes->getMesh();
  mesh->RecomputeStats();

  if (filename == "") return;
  const char *fname = filename.c_str();

  assert (mesh->getMaterials().size() == 0);

  assert (mesh != NULL);
  (*ARGS->output) << "LOAD " << fname << " ... ";
  // Call the correct load function. Using input file to determine which

  if (!strcmp(&fname[strlen(fname)-4],".obj")) {

    // Loading Mesh
    LoadOBJ(mesh, fname, meshes->args);

    double north;
    double longitude;
    double latitude;
    
    // Loading North
    LoadGeoLocation(filename.substr(0,filename.size()-3)+"geoloc", north, longitude, latitude); // foo.obj -> foo. -> foo.north
    
    // Setting north angle copy
    mesh->setNorthAngleCopy(north);
    mesh->setCoordinateCopy(longitude,latitude);

    // // Loading Location
    // double lat,lon;
    // LoadCoordinate(filename.substr(0,filename.size()-3)+"coordinate",lat,lon); // loads coordinate from files
    // mesh->setCoordinateCopy(lat,lon);

  } else if (!strcmp(&fname[strlen(fname)-4],".nos")) {
    LoadNOS(mesh, fname, meshes->args);
  } else if (!strcmp(&fname[strlen(fname)-5],".json")) {
    LoadJSON(meshes, fname);
    AfterLoading(meshes);
    mesh->setNorthAngleCopy(meshes->getWalls()->getNorthAngle());
    mesh->setCoordinateCopy(meshes->getWalls()->longitude,meshes->getWalls()->latitude);

  } else if (!strcmp(&fname[strlen(fname)-5],".wall")) {
    LoadWALL(meshes, fname);
    AfterLoading(meshes);
    mesh->setNorthAngleCopy(meshes->getWalls()->getNorthAngle());
    mesh->setCoordinateCopy(meshes->getWalls()->longitude,meshes->getWalls()->latitude);
    
  } else if (!strcmp(&fname[strlen(fname)-5],".army")) {
    LoadARMY(meshes, fname);
    AfterLoading(meshes);
  } else if (!strcmp(&fname[strlen(fname)-4],".led")) {
    LoadLED(meshes, fname);
    if (meshes->args->puzzle_mode) { PuzzleMode(meshes); }
  } else {
    (*ARGS->output) << "WARNING!  unknown file type " << fname << std::endl;
  }

  mesh->RecomputeStats();


  (*ARGS->output) << " done" << std::endl;
}

// =========================================================================
// =========================================================================
// =========================================================================

void MeshIO::LoadNOS(Mesh *mesh, const std::string &filename, ArgParser *args) {

  FILE *file = fopen(filename.c_str(),"r");
  if (file == NULL) {
    printf ("ERROR! CANNOT OPEN '%s'\n",filename.c_str());
    return;
  }
  fclose(file);

  // open the nos file
  NOSFile nosfile = NOSFile(filename.c_str());

  int vertcount = nosfile.NumVertices();
  if (vertcount == 0) {
    printf ("ERROR!  no geometry in file '%s'\n", filename.c_str());
    return;
  }
  //int vertSize = nosfile.VertexSize();
  int tricount = nosfile.NumTriangles();
  //int triSize = nosfile.TriangleSize();
  //  assert (vertSize == 3);
  //assert (triSize == 3);

  printf (" loading %s\n  (%d verts, %d triangles)\n",filename.c_str(),vertcount,tricount);
  for (int v = 0; v < vertcount; v++) {
    float x = nosfile.GetVertex(v)[0];
    float y = nosfile.GetVertex(v)[1];
    float z = nosfile.GetVertex(v)[2];
    //int id = 
    
    if (!args->load_flip_y_up) {
      mesh->addVertex(Vec3f(x,y,z),-1,-1,0,0);      
    } else {
      mesh->addVertex(Vec3f(x,z,-y),-1,-1,0,0);
    }
    //Vertex *vert = mesh->getVertex(id);
    /*
      || vertSize == 4);
      if (vertSize == 4) {
        float density = nosfile.GetVertex(v)[3];
        assert (density >= 0 && density <= 1);
        vert->setDensity(density);
        }
    */
  }

  for (int t = 0; t < tricount; t++) {
    int v1 = nosfile.GetTriangle(t)[0];
    int v2 = nosfile.GetTriangle(t)[1];
    int v3 = nosfile.GetTriangle(t)[2];
    mesh->addElement(new Triangle(v1,v2,v3,mesh,"none"));
  }
}

// =========================================================================

void RotateThenTranslatePoint(double &x, double &z, double rotate_angle, double trans_x, double trans_z) {
  // rotate (x,0,z) clockwise around y axis (0,0,0) by rotate_angle radians
  // convert to polar coordiates
  double radius = sqrt(x*x+z*z);
  double theta = atan(z/x); // i think divide by zero will be ok here
  if (x < 0) theta += M_PI;
  theta += rotate_angle;
  z = radius * sin(theta);
  x = radius * cos(theta);
  x += trans_x;
  z += trans_z;
}


void RotateAngles(double &angle1, double &angle2, double rotate_angle) {
  angle1 += rotate_angle;
  angle2 += rotate_angle;
}

// =============================================================================================
// =============================================================================================
// =============================================================================================

void ScaleAddThickness(float &x1, float &z1, float &x2, float &z2,
                       float &a, float &b, float &c, float &d, float &e, float &f, float &g, float &h) {
  
#define pixelscale 0.001

    x1 *= pixelscale;
    z1 *= pixelscale;
    x2 *= pixelscale;
    z2 *= pixelscale;
    
    //#define xtransform -INCH_IN_METERS*15;
    //#define ztransform -INCH_IN_METERS*8;

#define xtransform -INCH_IN_METERS*15;
#define ztransform -INCH_IN_METERS*10;

#if 1
    x1 += xtransform;
    z1 += ztransform;
    x2 += xtransform;
    z2 += ztransform;
#endif

    Vec3f dir = Vec3f(x2-x1,0,z2-z1);
    //std::cout << "LINE SEGMENT LENGTH = " << dir.Length() << std::endl;
    dir = dir.Normalize();
    //std::cout << "SHOULD BE 1 = " << dir.Length() << std::endl;
    Vec3f perp;  Vec3f::Cross3(perp,dir,Vec3f(0,1,0));
    perp = perp.Normalize();
    //std::cout << "SHOULD BE 1 = " << perp.Length() << std::endl;
    perp = (0.5 * INCH_IN_METERS) * perp;

    //std::cout << "length of perp = " << perp.Length() <<  (0.5 * INCH_IN_METERS) << std::endl;

    a = x1 - 0.5*perp.x();
    b = z1 - 0.5*perp.z();
    c = x1 + 0.5*perp.x();
    d = z1 + 0.5*perp.z();

    e = x2 + 0.5*perp.x();
    f = z2 + 0.5*perp.z();
    g = x2 - 0.5*perp.x();
    h = z2 - 0.5*perp.z();

    
    float width = (Vec3f(a,0,b)-Vec3f(c,0,d)).Length();
    float length  = (Vec3f(c,0,d)-Vec3f(e,0,f)).Length();
    float width2= (Vec3f(e,0,f)-Vec3f(g,0,h)).Length();
    float length2 = (Vec3f(g,0,h)-Vec3f(a,0,b)).Length();
    std::cout << "length=  " << length <<  "  width=  " << width << std::endl;
    std::cout << "length2= " << length2 << "  width2= " << width2 << std::endl;

}


void MeshIO::LoadJSON(MeshManager *meshes, const std::string &filename_) {

  BasicWall::resetWallCounter();

  const char* filename = filename_.c_str();
  int l = filename_.length();
  assert (filename_.substr(l-5,5) == ".json");  

  std::ifstream istr(filename);
  if (!istr) {
    std::cout << "ERROR! CANNOT OPEN " << filename << std::endl;
    return;
  }

  nlohmann::json json_object(istr);

  BasicWall *lastwall;

  auto item_list = json_object.find("items");
  std::cout << "size = " << item_list->size() << std::endl;

  BoundingBox *bb = NULL;
  
  // =====================================
  // LOOP OVER THE ITEMS
  for (auto itr = item_list->begin(); itr != item_list->end(); itr++) {
    auto v = itr->find("type");
    if (v == itr->end() || *v != "linesegment") {
      continue;
    }

    v = itr->find("points");
    assert (v != itr->end());
    assert (v->size() >= 2);
    float x1,z1,x2,z2;
    bool first = true;
    for (auto p_itr = v->begin(); p_itr != v->end(); p_itr++) {
      if (first) {
        x1 = p_itr->find("x")->get<nlohmann::json::number_float_t>();
        z1 = p_itr->find("y")->get<nlohmann::json::number_float_t>();
        first = false;
      } else {
        x2 = p_itr->find("x")->get<nlohmann::json::number_float_t>();
        z2 = p_itr->find("y")->get<nlohmann::json::number_float_t>();
      }
    }
    
    std::cout << "****************************************\n";
    float a,b,c,d,e,f,g,h;
    ScaleAddThickness(x1,z1,x2,z2, a,b,c,d,e,f,g,h);

    std::cout << "wall from ";

    std::cout << " <" << a << "," << b << ">";
    std::cout << " <" << c << "," << d << ">   to   ";
    std::cout << " <" << e << "," << f << ">";
    std::cout << " <" << g << "," << h << ">";
    std::cout << std::endl;


    if (bb == NULL) {
      bb = new BoundingBox(Vec3f(a,0,b),Vec3f(c,0,d));
    }
    bb->Extend(Vec3f(a,0,b));
    bb->Extend(Vec3f(c,0,d));
    bb->Extend(Vec3f(e,0,f));
    bb->Extend(Vec3f(g,0,h));

    float bottom_edge = 0;
    float height = 8 * INCH_IN_METERS;
    unsigned int material_index = 0;
    
    meshes->getWalls()->wall_materials.push_back(Vec3f(0.9,0.9,0.9));
    material_index = meshes->getWalls()->wall_materials.size()-1;

    BasicWall *w = BasicWall::QuadWall(a,b,c,d,e,f,g,h,bottom_edge,height,material_index);
    lastwall = w;
    meshes->getWalls()->addWall(w);

    

    v = itr->find("windows");
    assert (v != itr->end());
    std::cout << "THIS WALL HAS " << v->size() << " WINDOW(S)" << std::endl;
    int win_count = 0;
    for (auto w_itr = v->begin(); w_itr != v->end(); w_itr++) {
      std::cout << "WIN COUNT " << win_count << std::endl;
      win_count++;
      //assert ((*w_itr).size() % 2 == 0);
      float x1,z1,x2,z2;
      //for (auto p_itr = (*w_itr).begin(); p_itr != (*w_itr).end(); p_itr++) {
      x1 = w_itr->find("start")->find("x")->get<nlohmann::json::number_float_t>();
      z1 = w_itr->find("start")->find("y")->get<nlohmann::json::number_float_t>();
      x2 = w_itr->find("end")->find("x")->get<nlohmann::json::number_float_t>();
      z2 = w_itr->find("end")->find("y")->get<nlohmann::json::number_float_t>();

      std::cout << "-------------" << std::endl;

      float a,b,c,d,e,f,g,h;
      ScaleAddThickness(x1,z1,x2,z2, a,b,c,d,e,f,g,h);

      std::cout << "    window from ";

      std::cout << " <" << a << "," << b << ">";
      std::cout << " <" << c << "," << d << ">   to   ";
      std::cout << " <" << e << "," << f << ">";
      std::cout << " <" << g << "," << h << ">";
      std::cout << std::endl;


      std::string type = "cyan";
      
      Window wi(a,b,c,d,e,f,g,h,type,0);
      assert (lastwall != NULL);
      lastwall->addWindow(wi,0);
    }
  }

  std::cout << "-----------------MY BB" << *bb << std::endl;
  
#if 0
  bb->getCenter(meshes->getWalls()->scene_center);
  meshes->getWalls()->scene_radius = 2.0*bb->getRadius();
#else
  meshes->getWalls()->scene_center = Vec3f(0,0,0);
  meshes->getWalls()->scene_radius = 21 * INCH_IN_METERS;
#endif

  std::cout << "CENTER " << meshes->getWalls()->scene_center << std::endl;


  delete bb;
  bb = NULL;


  //istr >> x >> y >> z;


  /*


  std::string s;
  double x,y,z;
  //int num; 
  double a,b,c,d,e,f,g,h,bottom_edge,height;
  std::string type;
  unsigned int material_index;
  BasicWall* lastwall = NULL;

  while(istr >> s) {

    if (s == "north") {
      istr >> meshes->getWalls()->north_angle;
      
    } else if (s == "coordinates") {
        
      istr >> meshes->getWalls()->longitude;
      istr >> meshes->getWalls()->latitude;
        
    } else if (s == "table") {
      istr >> x >> y >> z;
      meshes->getWalls()->scene_center = Vec3f(x,y,z);
      istr >> meshes->getWalls()->scene_radius;
      
    } else if (s == "wall_material") {
      istr >> x >> y >> z;
      meshes->getWalls()->wall_materials.push_back(Vec3f(x,y,z));
      material_index = meshes->getWalls()->wall_materials.size()-1;

    } else if (s == "floor_material") {
      istr >> x >> y >> z;
      Vec3f color(x,y,z);
      HACK_MY_COLOR(color);
      meshes->getWalls()->floor_material = color;

    } else if (s == "ceiling_material") {
      istr >> x >> y >> z;
      Vec3f color(x,y,z);
      HACK_MY_COLOR(color);
      meshes->getWalls()->ceiling_material = color; 

    } else if (s == "wall") {
      if (meshes->getWalls()->empac) 
        istr >> a >> b >> c >> d >> e >> f >> g >> h >> bottom_edge >> height;
      else {
        istr >> a >> b >> c >> d >> e >> f >> g >> h >> height;
        bottom_edge = 0;
      }
      RotateThenTranslatePoint(a,b,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(c,d,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(e,f,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(g,h,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      if (meshes->args->all_walls_8_inches == true) {
        height = 8 * INCH_IN_METERS;
      }
      assert (material_index <= meshes->getWalls()->wall_materials.size());
      BasicWall *w = BasicWall::QuadWall(a,b,c,d,e,f,g,h,bottom_edge,height,material_index);
      lastwall = w;
      meshes->getWalls()->addWall(w);

    } else if (s == "window") {
      istr >> a >> b >> c >> d >> e >> f >> g >> h >> type;
      RotateThenTranslatePoint(a,b,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(c,d,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(e,f,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(g,h,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      Window wi(a,b,c,d,e,f,g,h,type,0);
      assert (lastwall != NULL);
      lastwall->addWindow(wi,0);

    } else if (s == "curved_wall") {
      istr >> a >> b >> c >> d >> e >> f;
      RotateThenTranslatePoint(a,b,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateAngles(e,f,meshes->args->walls_rotate_angle);
      assert (material_index <= meshes->getWalls()->wall_materials.size());
      int junk;
      istr >> height >> junk;
      if (meshes->args->all_walls_8_inches == true) {
	height = 8 * INCH_IN_METERS;
      }
      BasicWall *w = BasicWall::CurvedWall(a,b,c,d,e,f,bottom_edge,height,material_index);
      assert (w != NULL);
      meshes->getWalls()->addWall(w);

    } else if (s == "skylight") {
      istr >> a >> b >> c >> d >> e >> f >> g >> h;
      meshes->getWalls()->addSkylight(new Skylight(a,b,c,d,e,f,g,h));

      float junk;
      istr >> junk;

    } else if (s == "column") {
      istr >> a >> b >> c >> d;
      if (e < 0 || e >= meshes->getWalls()->wall_materials.size()) {
        (*ARGS->output) << "ERROR: COLUMN MATERIAL DOES NOT EXIST: " << e << std::endl;
        assert (meshes->getWalls()->wall_materials.size() >= 1);
        e = 0;
      } else {
        assert (e >= 0 && e < meshes->getWalls()->wall_materials.size());
      }
      BasicWall *w = BasicWall::Column(a,b,c,bottom_edge,d,material_index);
      assert (w != NULL);
      meshes->getWalls()->addWall(w);


    } else if (s == "desk") {
      istr >> a >> b >> c >> d >> e >> f >> g >> h >> height;
      Furniture *f2 = Furniture::Desk(a,b,c,d,e,f,g,h,height,material_index);
      assert (f2 != NULL);
      meshes->getWalls()->addFurniture(f2);
      
    } else if (s == "closet" || s == "wardrobe") {
      istr >> a >> b >> c >> d >> e >> f >> g >> h >> height;
      Furniture *f2 = Furniture::Wardrobe(a,b,c,d,e,f,g,h,height,material_index);
      assert (f2 != NULL);
      meshes->getWalls()->addFurniture(f2);

    } else if (s == "bed") {
      istr >> a >> b >> c >> d >> e >> f >> g >> h >> height;
      Furniture *f2 = Furniture::Bed(a,b,c,d,e,f,g,h,height*0.66,material_index);
      assert (f2 != NULL);
      meshes->getWalls()->addFurniture(f2);

    } else if (s == "person") {
      istr >> a >> b >> c >> d; 
      (*ARGS->output) << "IGNORING PERSON" << std::endl;
      static int person_count = 0;
      Person p;
      p.x = a;
      p.z = b;
      p.angle = c;
      p.height = d;
      std::stringstream ss;
      ss << "person" << person_count;
      p.name = ss.str();
      person_count++;
      meshes->getWalls()->people.push_back(p);

    } else {
      (*ARGS->output) << "unknown '" << s << "'" << std::endl;
      exit(0);
    }
  }
  */

  meshes->getWalls()->ShuffleWalls();

}

// =============================================================================================
// =============================================================================================
// =============================================================================================


void MeshIO::LoadWALL(MeshManager *meshes, const std::string &filename_) {

  BasicWall::resetWallCounter();

  const char* filename = filename_.c_str();
  int l = filename_.length();
  assert (filename_.substr(l-5,5) == ".wall");  
  std::ifstream istr(filename);
  if (!istr) {
    std::cout << "ERROR! CANNOT OPEN " << filename << std::endl;
    return;
  }

  std::string s;
  double x,y,z;
  //int num; 
  double a,b,c,d,e,f,g,h,bottom_edge,height;
  std::string type;
  unsigned int material_index;
  BasicWall* lastwall = NULL;

  while(istr >> s) {

    if (s == "north") {
      istr >> meshes->getWalls()->north_angle;
      
    } else if (s == "coordinates") {
        
      istr >> meshes->getWalls()->longitude;
      istr >> meshes->getWalls()->latitude;
        
    } else if (s == "table") {
      istr >> x >> y >> z;
      meshes->getWalls()->scene_center = Vec3f(x,y,z);
      istr >> meshes->getWalls()->scene_radius;
      
    } else if (s == "wall_material") {
      istr >> x >> y >> z;
      meshes->getWalls()->wall_materials.push_back(Vec3f(x,y,z));
      material_index = meshes->getWalls()->wall_materials.size()-1;

    } else if (s == "floor_material") {
      istr >> x >> y >> z;
      Vec3f color(x,y,z);
      HACK_MY_COLOR(color);
      meshes->getWalls()->floor_material = color;

    } else if (s == "ceiling_material") {
      istr >> x >> y >> z;
      Vec3f color(x,y,z);
      HACK_MY_COLOR(color);
      meshes->getWalls()->ceiling_material = color; 

    } else if (s == "wall") {
      if (meshes->getWalls()->empac) 
        istr >> a >> b >> c >> d >> e >> f >> g >> h >> bottom_edge >> height;
      else {
        istr >> a >> b >> c >> d >> e >> f >> g >> h >> height;
        bottom_edge = 0;
      }
      RotateThenTranslatePoint(a,b,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(c,d,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(e,f,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(g,h,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      if (meshes->args->all_walls_8_inches == true) {
        height = 8 * INCH_IN_METERS;
      }
      assert (material_index <= meshes->getWalls()->wall_materials.size());
      BasicWall *w = BasicWall::QuadWall(a,b,c,d,e,f,g,h,bottom_edge,height,material_index);
      lastwall = w;
      meshes->getWalls()->addWall(w);

    } else if (s == "window") {
      istr >> a >> b >> c >> d >> e >> f >> g >> h >> type;
      RotateThenTranslatePoint(a,b,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(c,d,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(e,f,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(g,h,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      Window wi(a,b,c,d,e,f,g,h,type,0);
      assert (lastwall != NULL);
      lastwall->addWindow(wi,0);

    } else if (s == "curved_wall") {
      istr >> a >> b >> c >> d >> e >> f;
      RotateThenTranslatePoint(a,b,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateAngles(e,f,meshes->args->walls_rotate_angle);
      assert (material_index <= meshes->getWalls()->wall_materials.size());
      int junk;
      istr >> height >> junk;
      if (meshes->args->all_walls_8_inches == true) {
	height = 8 * INCH_IN_METERS;
      }
      BasicWall *w = BasicWall::CurvedWall(a,b,c,d,e,f,bottom_edge,height,material_index);
      assert (w != NULL);
      meshes->getWalls()->addWall(w);

    } else if (s == "skylight") {
      istr >> a >> b >> c >> d >> e >> f >> g >> h;
      meshes->getWalls()->addSkylight(new Skylight(a,b,c,d,e,f,g,h));

      float junk;
      istr >> junk;

    } else if (s == "column") {
      istr >> a >> b >> c >> d;
      if (e < 0 || e >= meshes->getWalls()->wall_materials.size()) {
        (*ARGS->output) << "ERROR: COLUMN MATERIAL DOES NOT EXIST: " << e << std::endl;
        assert (meshes->getWalls()->wall_materials.size() >= 1);
        e = 0;
      } else {
        assert (e >= 0 && e < meshes->getWalls()->wall_materials.size());
      }
      BasicWall *w = BasicWall::Column(a,b,c,bottom_edge,d,material_index);
      assert (w != NULL);
      meshes->getWalls()->addWall(w);


    } else if (s == "desk") {
      istr >> a >> b >> c >> d >> e >> f >> g >> h >> height;
      Furniture *f2 = Furniture::Desk(a,b,c,d,e,f,g,h,height,material_index);
      assert (f2 != NULL);
      meshes->getWalls()->addFurniture(f2);
      
    } else if (s == "closet" || s == "wardrobe") {
      istr >> a >> b >> c >> d >> e >> f >> g >> h >> height;
      Furniture *f2 = Furniture::Wardrobe(a,b,c,d,e,f,g,h,height,material_index);
      assert (f2 != NULL);
      meshes->getWalls()->addFurniture(f2);

    } else if (s == "bed") {
      istr >> a >> b >> c >> d >> e >> f >> g >> h >> height;
      Furniture *f2 = Furniture::Bed(a,b,c,d,e,f,g,h,height*0.66,material_index);
      assert (f2 != NULL);
      meshes->getWalls()->addFurniture(f2);

    } else if (s == "person") {
      istr >> a >> b >> c >> d; 
      (*ARGS->output) << "IGNORING PERSON" << std::endl;
      static int person_count = 0;
      Person p;
      p.x = a;
      p.z = b;
      p.angle = c;
      p.height = d;
      std::stringstream ss;
      ss << "person" << person_count;
      p.name = ss.str();
      person_count++;
      meshes->getWalls()->people.push_back(p);

    } else {
      (*ARGS->output) << "unknown '" << s << "'" << std::endl;
      exit(0);
    }
  }

  meshes->getWalls()->ShuffleWalls();

}

void MeshIO::AfterLoading(MeshManager *meshes) {

  if (meshes->args->walls_create_arrangement == true) {
    meshes->getWalls()->CreateArrangement(meshes->getArrangement()); 
    (*ARGS->output) << "going to create roof" << std::endl;
    meshes->getWalls()->CreateRoofOutline(meshes->getArrangement()); 
    (*ARGS->output) << "LOADED " << meshes->getWalls()->numWalls() << " walls" << std::endl;
    meshes->getWalls()->CreateTriMesh(meshes->getArrangement(), meshes->getMesh());
    (*ARGS->output) << "CREATED " << meshes->getMesh()->numElements() << " elements" << std::endl;
  } else {
    meshes->getWalls()->CreateTriMesh(meshes->getMesh());
    (*ARGS->output) << "A TEXTURED TRIANGLES FILE! " << std::endl;    
  }  

  if (meshes->args->create_surface_cameras) {
    meshes->getWalls()->CreateSurfaceCameras();
    (*ARGS->output) << "CREATED surface cameras" << std::endl;
  }
}


void MeshIO::LoadARMY(MeshManager *meshes, const std::string &filename_) {

  BasicWall::resetWallCounter();
  meshes->getWalls()->wall_materials.push_back(Vec3f(1,0,0));

  const char* filename = filename_.c_str();
  int l = filename_.length();
  assert (filename_.substr(l-5,5) == ".army");  
  std::ifstream istr(filename);
  if (!istr) {
    printf ("ERROR! CANNOT OPEN '%s'\n",filename);
    return;
  }

  std::string s;
  double x,y,z;
  double a,b,c,d,e,f,g,h,height;
  std::string type;
  double bottom_edge = 0;
  int material_index = -1;

  while(istr >> s) {

    if (s == "table") {
      istr >> x >> y >> z;
      meshes->getWalls()->scene_center = Vec3f(x,y,z);
      istr >> meshes->getWalls()->scene_radius;
    } else if (s == "wall") {
      istr >> a >> b >> c >> d >> e >> f >> g >> h >> height;
      RotateThenTranslatePoint(a,b,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(c,d,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(e,f,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      RotateThenTranslatePoint(g,h,meshes->args->walls_rotate_angle,meshes->args->walls_translate_x,meshes->args->walls_translate_z);
      BasicWall *w = BasicWall::QuadWall(a,b,c,d,e,f,g,h,bottom_edge,height,material_index);
      assert (w != NULL);
      meshes->getWalls()->addWall(w);
    } else if (s == "column") {
      istr >> a >> b >> c >> height;
      BasicWall *w = BasicWall::Column(a,b,c,bottom_edge,height,material_index);
      assert (w != NULL);
      meshes->getWalls()->addWall(w);
    } else if (s == "platform") {
      istr >> a >> b >> c >> d >> e >> f >> g >> h >> height;
      BasicWall *w = BasicWall::Platform(a,b,c,d,e,f,g,h,bottom_edge,height,material_index);
      assert (w != NULL);
      meshes->getWalls()->addWall(w);
    } else if (s == "ramp") {
      istr >> a >> b >> c >> d >> e >> f >> g >> h >> height;
      BasicWall *w = BasicWall::Ramp(a,b,c,d,e,f,g,h,bottom_edge,height,material_index);
      assert (w != NULL);
      meshes->getWalls()->addWall(w);
    } else if (s == "soldier") {
      istr >> a >> b >> c >> s;
      (*ARGS->output) << "ignoring soldier (" << s << ")" << std::endl;
    } else {
      (*ARGS->output) << "unknown '" << s << "'" << std::endl;
      exit(0);
    }
  }
}



std::pair<Vec3f,double> COMPUTE_CIRCLE(Vec3f a, Vec3f b, Vec3f c) {
  Vec3f mid_ab = (a+b) * 0.5;
  Vec3f mid_bc = (b+c) * 0.5;
  Vec3f perp = Vec3f(0,1,0);
  Vec3f vab = b-a; vab.Normalize();
  Vec3f vbc = c-b; vbc.Normalize();
  Vec3f dir_ab,dir_bc;
  Vec3f::Cross3(dir_ab,perp,vab); dir_ab.Normalize();
  Vec3f::Cross3(dir_bc,perp,vbc); dir_bc.Normalize();
  Vec3f intersection;
  Intersect(mid_ab,dir_ab,mid_bc,dir_bc,intersection);
  double da = DistanceBetweenTwoPoints(a,intersection);
  double db = DistanceBetweenTwoPoints(b,intersection);
  double dc = DistanceBetweenTwoPoints(c,intersection);
  double radius = (da + db + dc) * 1 / 3.0;
  return std::make_pair(intersection, radius);
}


std::pair<Vec3f,double> COMPUTE_CIRCLE(Vec3f a, Vec3f b, Vec3f c, Vec3f d) {
  std::pair<Vec3f,double> abc = COMPUTE_CIRCLE(a,b,c);
  std::pair<Vec3f,double> abd = COMPUTE_CIRCLE(a,b,d);
  std::pair<Vec3f,double> acd = COMPUTE_CIRCLE(a,c,d);
  std::pair<Vec3f,double> bcd = COMPUTE_CIRCLE(b,c,d);
  Vec3f center = (abc.first + abd.first + acd.first + bcd.first) * 0.25;
  double radius = (abc.second + abd.second + acd.second + bcd.second) * 0.25;
  return std::make_pair(center,radius);
}


double COMPUTE_THETA(Vec3f &na,Vec3f &nb,Vec3f &nc,Vec3f &nd,double radius, Vec3f center) {
  Vec3f va = na - center; va.Normalize();
  Vec3f vb = nb - center; vb.Normalize();
  Vec3f vc = nc - center; vc.Normalize();
  Vec3f vd = nd - center; vd.Normalize();
  Vec3f v = Vec3f(1,0,0);
  double ab = SignedAngleBetweenNormalized(vb,va,Vec3f(0,1,0));
  double bc = SignedAngleBetweenNormalized(vc,vb,Vec3f(0,1,0));
  double cd = SignedAngleBetweenNormalized(vd,vc,Vec3f(0,1,0));
  if (ab < 0) {
    Vec3f tmp = na;
    na = nd;
    nd = tmp;
    tmp = nb;
    nb = nc;
    nc = tmp;
    va = na - center; va.Normalize();
    vb = nb - center; vb.Normalize();
    vc = nc - center; vc.Normalize();
    vd = nd - center; vd.Normalize();
    ab = SignedAngleBetweenNormalized(vb,va,Vec3f(0,1,0));
    bc = SignedAngleBetweenNormalized(vc,vb,Vec3f(0,1,0));
    cd = SignedAngleBetweenNormalized(vd,vc,Vec3f(0,1,0));
    //(*ARGS->output) << "had to swap!" << std::endl;
  }
  //(*ARGS->output) << "angles " << ab << " " << bc << " " << cd << std::endl;
  double a2 = SignedAngleBetweenNormalized(va,v,Vec3f(0,1,0));
  double b2 = SignedAngleBetweenNormalized(vb,v,Vec3f(0,1,0));
  double c2 = SignedAngleBetweenNormalized(vc,v,Vec3f(0,1,0));
  double d2 = SignedAngleBetweenNormalized(vd,v,Vec3f(0,1,0));
  //(*ARGS->output) << "angles " << a2 << " " << b2 << " " << c2 << " " << d2 << std::endl;
  if (a2 - b2 > 0) { b2+=2*M_PI; c2+=2*M_PI; d2+=2*M_PI; }
  if (b2 - c2 > 0) { c2+=2*M_PI; d2+=2*M_PI; }
  if (c2 - d2 > 0) { d2+=2*M_PI; }
  //(*ARGS->output) << "angles " << a2 << " " << b2 << " " << c2 << " " << d2 << std::endl;
  double average = (a2 + b2 + c2 + d2) / 4.0;
  //(*ARGS->output) << "aver " << average << std::endl;
  return average;
}

// =======================================================================================
// =======================================================================================
// =======================================================================================

void SWAP_LUAN_LEDS(Vec3f &na, Vec3f &nb, Vec3f &nc) {
  double distab = DistanceBetweenTwoPoints(na,nb);
  double distbc = DistanceBetweenTwoPoints(nb,nc);
  double distac = DistanceBetweenTwoPoints(na,nc);
  if (distac < distbc) {
    Vec3f tmp = nb;
    Vec3f nb = na;
    na = tmp;
    distab = DistanceBetweenTwoPoints(na,nb);
    distbc = DistanceBetweenTwoPoints(nb,nc);
    distac = DistanceBetweenTwoPoints(na,nc);
    (*ARGS->output) << "SWAP LUAN LED" << std::endl;
  }
  assert (distab < distbc && distab < distac);
  assert (distbc < distac);
  Vec3f shrink = nc - na;
  shrink.Normalize();
  double true_length = 7 * 12 * INCH_IN_METERS;
  double correction = true_length - distac;
  na = na - 0.5*correction*shrink;
  nc = nc + 0.5*correction*shrink;
}


void SWAP_CANVAS_LEDS(Vec3f &na, Vec3f &nb, Vec3f &nc) {
  double distab = DistanceBetweenTwoPoints(na,nb);
  double distbc = DistanceBetweenTwoPoints(nb,nc);
  double distac = DistanceBetweenTwoPoints(na,nc);
  if (distac < distbc) {
    Vec3f tmp = nb;
    Vec3f nb = na;
    na = tmp;
    distab = DistanceBetweenTwoPoints(na,nb);
    distbc = DistanceBetweenTwoPoints(nb,nc);
    distac = DistanceBetweenTwoPoints(na,nc);
    (*ARGS->output) << "SWAP CANVAS LED" << std::endl;
  }
  assert (distab < distbc && distab < distac);
  assert (distbc < distac);
  Vec3f shrink = nc - na;
  shrink.Normalize();
  double true_length = 7 * 12 * INCH_IN_METERS;
  double correction = true_length - distac;
  na = na - 0.5*correction*shrink;
  nc = nc + 0.5*correction*shrink;
}

Vec3f ReadPoint(std::istream &istr) {
  std::string foo;
  double x,y,z;
  istr >> foo;
  assert (foo == "point");
  istr >> x >> y >> z;
  Vec3f answer(x,y,z);
  return answer;
}


BasicWall* MeshIO::MakeLShapedWall(MeshManager *meshes, Vec3f &led_a, Vec3f &led_b, Vec3f &led_c, Vec3f &led_d, std::string &window_option) {

  /*
  //             ^
  //           c   b 
  //         /       \
  //       / q1     q0 \
  //     /               \
  //   d                   a
  //
  // right side a & b
  // left side c & d
  */

  // q0
  Vec3f q0_start  = Vec3f(led_a.x(),0,led_a.z());
  Vec3f q0_end    = Vec3f(led_b.x(),0,led_b.z());
  Vec3f q0_diff   = q0_end-q0_start; q0_diff.Normalize();
  q0_start -= q0_diff * 6 * INCH_IN_METERS;
  q0_end   += q0_diff * 6 * INCH_IN_METERS;
  Vec3f q0_perp;
  Vec3f up = Vec3f(0,1,0);
  Vec3f::Cross3(q0_perp,up,q0_diff);
  Vec3f q0_0 = q0_start - q0_perp * 1.25 * INCH_IN_METERS;
  Vec3f q0_3 = q0_start + q0_perp * 1.25 * INCH_IN_METERS;
  Vec3f q0_1 = q0_end - q0_perp * 1.25 * INCH_IN_METERS;
  Vec3f q0_2 = q0_end + q0_perp * 1.25 * INCH_IN_METERS;
    
  // q1
  Vec3f q1_start  = Vec3f(led_c.x(),0,led_c.z());
  Vec3f q1_end    = Vec3f(led_d.x(),0,led_d.z());
  Vec3f q1_diff   = q1_end-q1_start; q1_diff.Normalize();
  q1_start -= q1_diff * 6 * INCH_IN_METERS;
  q1_end   += q1_diff * 6 * INCH_IN_METERS;
  Vec3f q1_perp;
  Vec3f::Cross3(q1_perp,up,q1_diff);
  Vec3f q1_0 = q1_start - q1_perp * 1.25 * INCH_IN_METERS;
  Vec3f q1_3 = q1_start + q1_perp * 1.25 * INCH_IN_METERS;
  Vec3f q1_1 = q1_end - q1_perp * 1.25 * INCH_IN_METERS;
  Vec3f q1_2 = q1_end + q1_perp * 1.25 * INCH_IN_METERS;

  Vec3f BACK,FRONT;
  Intersect(q0_0,(q0_1-q0_0).Normalize(),q1_0,(q1_1-q1_0).Normalize(),BACK);
  Intersect(q0_3,(q0_2-q0_3).Normalize(),q1_3,(q1_2-q1_3).Normalize(),FRONT);
  
  BasicWall *w = BasicWall::LShapedWall(q0_3.x(),q0_3.z(),
					FRONT.x(),FRONT.z(),
					q1_2.x(),q1_2.z(),
					q1_1.x(),q1_1.z(),
					BACK.x(),BACK.z(),
					q0_0.x(),q0_0.z(),
					0/*Walls::WHEEL_HEIGHT*/,2.5019,4);
  
  w->setColor(Vec3f(1,0,1));
  w->setName("right_l_wall");

  assert (window_option == "neither" || window_option == "left" || window_option == "right" || window_option == "both");

  if (window_option == "left" ||
      window_option == "both") {
    Vec3f a = q1_end -   q1_diff * 30 * INCH_IN_METERS + q1_perp * 1.75 * INCH_IN_METERS;
    Vec3f b = q1_end -   q1_diff * 30 * INCH_IN_METERS - q1_perp * 1.75 * INCH_IN_METERS;
    Vec3f c = q1_start + q1_diff * 30 * INCH_IN_METERS - q1_perp * 1.75 * INCH_IN_METERS;
    Vec3f d = q1_start + q1_diff * 30 * INCH_IN_METERS + q1_perp * 1.75 * INCH_IN_METERS;
    w->addWindow(Window(a.x(),a.z(),b.x(),b.z(),c.x(),c.z(),d.x(),d.z(),"yellow",1),1);
  }

  if (window_option == "right" ||
      window_option == "both") {
    Vec3f a = q0_end -   q0_diff * 30 * INCH_IN_METERS + q0_perp * 1.75 * INCH_IN_METERS;
    Vec3f b = q0_end -   q0_diff * 30 * INCH_IN_METERS - q0_perp * 1.75 * INCH_IN_METERS;
    Vec3f c = q0_start + q0_diff * 30 * INCH_IN_METERS - q0_perp * 1.75 * INCH_IN_METERS;
    Vec3f d = q0_start + q0_diff * 30 * INCH_IN_METERS + q0_perp * 1.75 * INCH_IN_METERS;
    w->addWindow(Window(a.x(),a.z(),b.x(),b.z(),c.x(),c.z(),d.x(),d.z(),"yellow",0),0);
  }

  return w;    
}

// =============================================================================

void MeshIO::LoadLED(MeshManager *meshes, const std::string &filename_) {
  (*ARGS->output) << "LOAD LED FILE " << std::endl;
  LoadColorsFile(meshes);
  const char* filename = filename_.c_str();
  int l = filename_.length();
  assert (filename_.substr(l-4,4) == ".led"); 
  std::ifstream istr;
  for (int i = 0; i < 10; i++) {
    istr.open(filename); 
    if (istr.fail()) {
      printf ("ERROR! CANNOT OPEN '%s'\n",filename);
    } else {
      //      (*ARGS->output) << "OK!" << std::endl;
      break;
    }
  }
  if (istr.fail()) exit(0);
  meshes->getWalls()->empac = true;
  meshes->getWalls()->scene_radius =
    std::max((EMPAC_MINUS_X + EMPAC_POS_X) / 2.0,
	     (EMPAC_MINUS_Z + EMPAC_POS_Z) / 2.0);

  meshes->getWalls()->north_angle = 0;
  assert (meshes->getWalls()->wall_materials.size() == 6);
  meshes->getWalls()->floor_material = Vec3f(0.5,0.5,0.5);
  meshes->getWalls()->ceiling_material = Vec3f(1.0,1.0,1.0);

  while (1) {
    std::string foo;
    istr >> foo;
    
    if (foo == "luan_wall") {
      Vec3f na = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(na,0));
      Vec3f nb = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nb,1));
      Vec3f nc = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nc,2));
      SWAP_LUAN_LEDS(na,nb,nc);

      Vec3f left  = Vec3f(na.x(),0,na.z());
      Vec3f right = Vec3f(nc.x(),0,nc.z());
      Vec3f diff  = right-left; diff.Normalize();
      left -=  diff * 6 * INCH_IN_METERS;
      right +=  diff * 6 * INCH_IN_METERS;
      Vec3f perp;
      Vec3f up = Vec3f(0,1,0);
      Vec3f::Cross3(perp,up,diff);
      Vec3f a = left - perp * 1.875 * INCH_IN_METERS;
      Vec3f b = left + perp * 1.875 * INCH_IN_METERS;
      Vec3f c = right + perp * 1.875 * INCH_IN_METERS;
      Vec3f d = right - perp * 1.875 * INCH_IN_METERS;
      BasicWall *w = BasicWall::QuadWall(a.x(),a.z(),b.x(),b.z(),c.x(),c.z(),d.x(),d.z(),0/*Walls::WHEEL_HEIGHT*/,2.5019,0);
      w->setColor(Vec3f(1,0,0));
      //Vec3f center = (a+c)*0.5+Vec3f(0,1.2827,0)-0.02*perp;
      w->setName("luan_wall");
      if (meshes->getWalls()->luan_window == "none") {
      } else if (meshes->getWalls()->luan_window == "single") {
	a = left + diff * 30 * INCH_IN_METERS - perp * 1.75 * INCH_IN_METERS;
	b = left + diff * 30 * INCH_IN_METERS + perp * 1.75 * INCH_IN_METERS;
	c = right - diff * 30 * INCH_IN_METERS + perp * 1.75 * INCH_IN_METERS;
	d = right - diff * 30 * INCH_IN_METERS - perp * 1.75 * INCH_IN_METERS;
	w->addWindow(Window(a.x(),a.z(),b.x(),b.z(),c.x(),c.z(),d.x(),d.z(),"yellow",0),0);
      } else if (meshes->getWalls()->luan_window == "double") {
	a = left + diff * 18 * INCH_IN_METERS - perp * 1.75 * INCH_IN_METERS;
	b = left + diff * 18 * INCH_IN_METERS + perp * 1.75 * INCH_IN_METERS;
	c = right - diff * 65 * INCH_IN_METERS + perp * 1.75 * INCH_IN_METERS;
	d = right - diff * 65 * INCH_IN_METERS - perp * 1.75 * INCH_IN_METERS;
	w->addWindow(Window(a.x(),a.z(),b.x(),b.z(),c.x(),c.z(),d.x(),d.z(),"cyan",0),0);
	a = left + diff * 65 * INCH_IN_METERS - perp * 1.75 * INCH_IN_METERS;
	b = left + diff * 65 * INCH_IN_METERS + perp * 1.75 * INCH_IN_METERS;
	c = right - diff * 18 * INCH_IN_METERS + perp * 1.75 * INCH_IN_METERS;
	d = right - diff * 18 * INCH_IN_METERS - perp * 1.75 * INCH_IN_METERS;
	w->addWindow(Window(a.x(),a.z(),b.x(),b.z(),c.x(),c.z(),d.x(),d.z(),"cyan",0),0);
      } else {
	(*ARGS->output) << "DONT KNOW luan window option" << meshes->getWalls()->luan_window << std::endl;
      }
      meshes->getWalls()->addWall(w);

    } else if (foo == "canvas_wall") {
      Vec3f na = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(na,0));
      Vec3f nb = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nb,1));
      Vec3f nc = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nc,2));
      SWAP_CANVAS_LEDS(na,nb,nc); 
      Vec3f left  = Vec3f(na.x(),0,na.z());
      Vec3f right = Vec3f(nc.x(),0,nc.z());
      Vec3f diff  = right-left; diff.Normalize();
      left -=  diff * 6 * INCH_IN_METERS;
      right +=  diff * 6 * INCH_IN_METERS;
      Vec3f perp;
      Vec3f up = Vec3f(0,1,0);
      Vec3f::Cross3(perp,up,diff);
      Vec3f a = left - perp * 1.75 * INCH_IN_METERS;
      Vec3f b = left + perp * 1.75 * INCH_IN_METERS;
      Vec3f c = right + perp * 1.75 * INCH_IN_METERS;
      Vec3f d = right - perp * 1.75 * INCH_IN_METERS;
      BasicWall *w = BasicWall::QuadWall(a.x(),a.z(),b.x(),b.z(),c.x(),c.z(),d.x(),d.z(),0/*Walls::WHEEL_HEIGHT*/,2.5019,1);
      w->setColor(Vec3f(0,1,0));
      w->setName("canvas_wall");
      if (meshes->getWalls()->canvas_window == "none") {
      } else if (meshes->getWalls()->canvas_window == "single") {
	a = left + diff * 30 * INCH_IN_METERS - perp * 1.75 * INCH_IN_METERS;
	b = left + diff * 30 * INCH_IN_METERS + perp * 1.75 * INCH_IN_METERS;
	c = right - diff * 30 * INCH_IN_METERS + perp * 1.75 * INCH_IN_METERS;
	d = right - diff * 30 * INCH_IN_METERS - perp * 1.75 * INCH_IN_METERS;
	w->addWindow(Window(a.x(),a.z(),b.x(),b.z(),c.x(),c.z(),d.x(),d.z(),"yellow",0),0);
      } else if (meshes->getWalls()->canvas_window == "double") {
	a = left + diff * 18 * INCH_IN_METERS - perp * 1.75 * INCH_IN_METERS;
	b = left + diff * 18 * INCH_IN_METERS + perp * 1.75 * INCH_IN_METERS;
	c = right - diff * 65 * INCH_IN_METERS + perp * 1.75 * INCH_IN_METERS;
	d = right - diff * 65 * INCH_IN_METERS - perp * 1.75 * INCH_IN_METERS;
	w->addWindow(Window(a.x(),a.z(),b.x(),b.z(),c.x(),c.z(),d.x(),d.z(),"cyan",0),0);
	a = left + diff * 65 * INCH_IN_METERS - perp * 1.75 * INCH_IN_METERS;
	b = left + diff * 65 * INCH_IN_METERS + perp * 1.75 * INCH_IN_METERS;
	c = right - diff * 18 * INCH_IN_METERS + perp * 1.75 * INCH_IN_METERS;
	d = right - diff * 18 * INCH_IN_METERS - perp * 1.75 * INCH_IN_METERS;
	w->addWindow(Window(a.x(),a.z(),b.x(),b.z(),c.x(),c.z(),d.x(),d.z(),"cyan",0),0);
      } else {
	(*ARGS->output) << "DONT KNOW canvas window option" << meshes->getWalls()->canvas_window << std::endl;
      }
      meshes->getWalls()->addWall(w);
    } else if (foo == "right_l_shaped_wall") {
      Vec3f na = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(na,0));
      Vec3f nb = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nb,1));
      Vec3f nc = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nc,2));
      Vec3f nd = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nd,3));
      BasicWall *w = MakeLShapedWall(meshes,na,nb,nc,nd,meshes->getWalls()->right_l_shaped_window);
      w->setName("right_l_wall");
      w->setColor(Vec3f(1,1,0));
      meshes->getWalls()->addWall(w);
    } else if (foo == "left_l_shaped_wall") {
      Vec3f na = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(na,0));
      Vec3f nb = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nb,1));
      Vec3f nc = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nc,2));
      Vec3f nd = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nd,3));
      BasicWall *w = MakeLShapedWall(meshes,na,nb,nc,nd,meshes->getWalls()->left_l_shaped_window);
      w->setName("left_l_wall");
      w->setColor(Vec3f(0,1,1));
      meshes->getWalls()->addWall(w);
    } else if (foo == "big_l_shaped_wall") {
      Vec3f na = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(na,0));
      Vec3f nb = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nb,1));
      Vec3f nc = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nc,2));
      Vec3f nd = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nd,3));
      BasicWall *w = MakeLShapedWall(meshes,na,nb,nc,nd,meshes->getWalls()->big_l_shaped_window);
      w->setName("big_l_wall");
      w->setColor(Vec3f(1,1,0));
      meshes->getWalls()->addWall(w);
    } else if (foo == "curved_wall") {
      Vec3f na = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(na,0));
      Vec3f nb = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nb,1));
      Vec3f nc = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nc,2));
      Vec3f nd = ReadPoint(istr); meshes->getWalls()->LEDs.push_back(std::make_pair(nd,3));
      std::pair<Vec3f,double> circle = COMPUTE_CIRCLE(na,nb,nc,nd);
      double TWO_INCHES = 2 * INCH_IN_METERS;
      Vec3f center = circle.first;
      double radius = circle.second;
      double theta = COMPUTE_THETA(na,nb,nc,nd,radius,center);
      meshes->getWalls()->LEDs.push_back(std::make_pair(na,0));
      meshes->getWalls()->LEDs.push_back(std::make_pair(nb,1));
      meshes->getWalls()->LEDs.push_back(std::make_pair(nc,2));
      meshes->getWalls()->LEDs.push_back(std::make_pair(nd,3));
      double theta_1 = theta - 60 * M_PI / 180.0;
      double theta_2 = theta + 60 * M_PI / 180.0;
      BasicWall *w = BasicWall::CurvedWall(center.x(),center.z(),radius-TWO_INCHES,radius+TWO_INCHES,theta_1,theta_2,0/*Walls::WHEEL_HEIGHT*/,2.5019,2);
      if (w != NULL) {
	w->setName("curved_wall");
	w->setColor(Vec3f(0,0,1));
	meshes->getWalls()->addWall(w);
	//	delete w;
      }
    } else if (foo == "") {
      break;
    } else {
      (*ARGS->output) << "ERROR IN READING LED FILE" << std::endl;
      exit(0);
      break;
    }
  }

  meshes->getWalls()->ShuffleWalls();
}

// =========================================================================
// =========================================================================


void InsertNonConvexPolygon(Mesh *m, std::vector<int> &verts, ArgParser *args, std::string material) {
  assert (verts.size() >= 3);

  //int ncontours = 1;
  int cntr[1]; cntr[0] = verts.size();
  double (*vertices)[2] = new double[verts.size()+1][2];
  int (*triangles)[3] = new int[verts.size()-2][3];
  std::vector<int> vt(verts.size()+1);

  Vec3f normal;
  computeNormal(m->getVertex(verts[0])->get(),
                m->getVertex(verts[1])->get(),
                m->getVertex(verts[2])->get(),
                normal);

  //project triangle onto a plane
  for (unsigned int i = 1; i <= verts.size(); i++) {
    Vertex *v = m->getVertex(verts[i-1]);
    vt[i] = verts[i-1];
    if (fabs(normal.x()) >= fabs(normal.y()) && fabs(normal.x()) >= fabs(normal.z())) {
      if (normal.x() > 0) {
        vertices[i][0] = v->get().y();
        vertices[i][1] = v->get().z();
      } else {
        vertices[i][0] = v->get().z();
        vertices[i][1] = v->get().y();
      }
    } else if (fabs(normal.y()) >= fabs(normal.x()) && fabs(normal.y()) >= fabs(normal.z())) {
      if (normal.y() > 0) {
        vertices[i][0] = v->get().z();
        vertices[i][1] = v->get().x();
      } else {
        vertices[i][0] = v->get().x();
        vertices[i][1] = v->get().z();
      }
    } else {
      assert (fabs(normal.z()) >= fabs(normal.x()) && fabs(normal.z()) >= fabs(normal.y()));
      if (normal.z() > 0) {
        vertices[i][0] = v->get().x();
        vertices[i][1] = v->get().y();
      } else {
        vertices[i][0] = v->get().y();
        vertices[i][1] = v->get().x();
      }
    }
  }

  assert(0);
  //  triangulate_polygon(ncontours,cntr,vertices,triangles);

  for (unsigned int j = 0; j < verts.size()-2; j++) {
    int a = vt[triangles[j][0]];
    int b = vt[triangles[j][1]];
    int c = vt[triangles[j][2]];
    if (!args->load_flip_triangles) m->addElement(new Triangle(a,b,c,m,material));
    else                            m->addElement(new Triangle(a,c,b,m,material));
  }

  delete [] vertices;
  delete [] triangles;
}




void getDirectory(const std::string &filename, std::string &directory, std::string &just_filename)
{
  directory="";
  just_filename = filename;

   std::string returnVal=".";
   int i=filename.size()-1;
   bool found=false;
   while(i>0&&found==false)
   {
     if(filename[i]=='/')
     {
       found=true;
       directory=filename.substr(0,i+1);
       just_filename=filename.substr(i,filename.size()-i);
     }
     i--;
   }
}

// Helper function that looks at <filename>.north
float LoadNorth(const std::string &filename) {
  std::ifstream istr(filename);
  if (istr) {
    std::cout << "READING NORTH FILE: " << filename << std::endl;
    float tmp;
    istr >> tmp;
    std::cout << "  north = " << tmp << std::endl;
    return tmp;
  }
  return 0;
}

// Helper function returns lat and long by ref  
// looks at <filename>.coordinate
void LoadGeoLocation(const std::string &filename, double&n, double &lat , double &lon ) {

  // Open input stream from file
  std::ifstream istr(filename);

  if (istr) {

    std::cout << "READING geolocation FILE: " << filename << std::endl;

    // Set by ref
    istr >> n;
    istr >> lon;
    istr >> lat;

    std::cout << "  n   = " << n   << std::endl;
    std::cout << "  lon = " << lon << std::endl;
    std::cout << "  lat = " << lat << std::endl;

  }

}

void MeshIO::LoadOBJ(Mesh *m, const std::string &full_filename, ArgParser *args) {
  
  assert(full_filename.size() >= 5);
  assert (full_filename.substr(full_filename.size()-4,full_filename.size()) == ".obj");
  std::string dir,just_filename;
  getDirectory(full_filename,dir,just_filename);
  std::ifstream istr(full_filename.c_str());
  if (!istr) {
    printf ("ERROR! CANNOT OPEN '%s'\n",full_filename.c_str());
    return;
  }

  std::ifstream blend_istr;
  if (args->blending_file != "") {
    blend_istr.open(args->blending_file.c_str());
    assert (blend_istr.good()); 
    (*ARGS->output) << "READ BLENDING WEIGHTS from " << args->blending_file << std::endl;
    //LoadBlending(mesh,meshes->args->blending_file,meshes->args);
  }
  //void MeshIO::LoadBlending(Mesh *m, const std::string &filename, ArgParser *args) {
  //std::ifstream istr(filename.c_str());
  //assert (istr != NULL);

  int index = 0;
  int vert_count = 0;
  int vert_index = 1;
  std::string line;
  std::string material = "none";

  int last_element_id = 0; // invalid

  std::vector<std::pair<double,double> > texture_coordinates;

  while (getline(istr,line)) { 
    std::istringstream ss(line);
    std::string token;
    ss >> token;
    if (token=="") continue;

    if (token=="mtllib") {
      ss >> token;
      std::string dir2, just_filename2;
      getDirectory(token,dir2,just_filename2);
      if (dir != dir2 && dir != "") {
	(*ARGS->output) << "WARNING: MATERIAL FILE PATH IS DIFFERENT...  " << dir << " vs " << dir2 << std::endl;
      }

      if (just_filename2.size() >=5 &&
	  just_filename2.substr(just_filename2.size()-4,just_filename2.size()) == ".mtl") {
	LoadMTL(m,dir+"/"+just_filename2,args);
      } else if (just_filename2.size() >=8 &&
		 just_filename2.substr(just_filename2.size()-7,just_filename2.size()) == ".lsvmtl") {
	LoadLSVMTL(m,dir+just_filename2,args);
      } else if (token == "NONE") {
	//Material mat;
	//	material = "NONE";
	//mat.setName("NONE");
	//m->addMaterial(mat);
      } else {
	(*ARGS->output) << "Unknown material file type: " << token << std::endl;
	exit(0);
      }
    } else if (token == "usemtl") {
      ss >> material;
    } else if (token == "g") {
      ss >> token;
      vert_index = 1; //vert_count + 1;
      index++;
    } else if (token == "o") {
      ss >> token;
    } else if (token == "v") {
      vert_count++;
      double x,y,z;
      ss >> x >> y >> z;
    

      int which = m->numVertices();
      double texture_s = 0;
      double texture_t = 0;
      if (which < (int)texture_coordinates.size()) {
        texture_s = texture_coordinates[which].first;
        texture_t = texture_coordinates[which].second;
      }
      
      if (!args->load_flip_y_up) {
        m->addVertex(Vec3f(x,y,z),-1,-1,texture_s,texture_t);
      } else {
        m->addVertex(Vec3f(x,z,-y),-1,-1,texture_s,texture_t);
      }
      
      //(*ARGS->output) << "add v " << m->numVertices() << std::endl;
    } else if (token == "l") {
      std::vector<int> verts;
      while (ss >> token) {
	std::istringstream sstoken(token);
        int a;
        sstoken >> a;
        a -= vert_index;
        assert (a >= 0 && a < (int)m->numVertices());
        verts.push_back(a);
      }
      assert (verts.size() == 2);
      Element *e = new Line(verts[0],verts[1],m,material);
      assert (e != NULL);
      m->addElement(e);

    } else if (token == "zone") {
      std::string zone_name;
      ss >> zone_name;
      //std::cout << "LOADED ZONE " << zone_name << std::endl;

      m->AddZone(zone_name,ARGS->RandomColor());

    } else if (token == "patch") {
      int which_zone;
      ss >> which_zone;
      //std::cout << "LOADED PATCH, belongs to zone " << which_zone << std::endl;
      
      assert (which_zone >= 0 && which_zone < (int)m->numZones());
      
      int pid = m->numPatches();
      m->AddPatch(ARGS->RandomColor());
      m->getPatch(pid).setZone(which_zone);
      m->getZone(which_zone).addPatchToZone(pid);


    } else if (token == "fp") {
      int which_patch;
      ss >> which_patch;
      assert (which_patch >= 0 && which_patch < (int)m->numPatches());
      //std::cout << "LOADED ELEMENT, belongs to patch " << which_patch << std::endl;
      assert (last_element_id != 0);

      m->getPatch(which_patch).addElementToPatch(last_element_id);
      m->element_to_patch[last_element_id] = which_patch;


    } else if (token == "f") {
      std::vector<int> verts;
      while (ss >> token) {
	std::istringstream sstoken(token);
        int a;
        sstoken >> a;
        a -= vert_index;
        assert (a >= 0 && a < (int)m->numVertices());
        verts.push_back(a);
      }
      if (verts.size() < 3) {
        (*ARGS->output) << " not enough vertices " << verts.size() << std::endl;
        for (unsigned int i = 0; i < verts.size(); i++) {
          (*ARGS->output) << verts[i] << std::endl;
        }
        continue;
      }

      assert (verts.size() >= 3);

      Element *e = NULL;
      //(*ARGS->output) << "add t " << verts[0] << " " << verts[1] << " " << verts[2];
      //(*ARGS->output) << "    #verts " << m->numVertices() << "   #tris " << m->numTriangles() << std::endl;

      if (!args->load_flip_triangles) {
	if (verts.size() == 3) {
	  e = new Triangle(verts[0],verts[1],verts[2],m,material); m->addElement(e);
	} else if (verts.size() == 4) {
	  e = new Quad(verts[0],verts[1],verts[2],verts[3],m,material); m->addElement(e);
	} else {
	  (*ARGS->output) << "can't handle > 4 vertices per face!" << std::endl;
	  exit(0);
	}     
      } else {
	if (verts.size() == 3) {
	  e = new Triangle(verts[0],verts[2],verts[1],m,material); m->addElement(e);
	} else if (verts.size() == 4) {
	  e = new Quad(verts[0],verts[3],verts[2],verts[1],m,material); m->addElement(e);
	} else {
	  (*ARGS->output) << "can't handle > 4 vertices per face!" << std::endl;
	  exit(0);
	}
      }

      //(*ARGS->output) << " element added " << std::endl;

      assert (e != NULL);
      last_element_id = e->getID();


      /*
      if (optional_load_vector != NULL) {
	optional_load_vector->push_back(e);
      }
      */

      if (args->blending_file != "") {	
	char x;
	blend_istr >> x;
	assert (x == 'f');
	for (unsigned int i = 0; i < verts.size(); i++) {
	  for (int j = 0; j < 4; j++) {
	    float blend;
	    blend_istr >> blend;
	    e->setBlendWeight(i,j,blend);
	  }
	}
      }
    } else if (token ==  "vt") {

      // assuming if vt is used, it is a 1-1 correspondence with the v's
      // not actually true for .obj format!!!
      double texture_s;
      double texture_t;
      ss >> texture_s >> texture_t;
      int which = texture_coordinates.size();
      texture_coordinates.push_back(std::make_pair(texture_s,texture_t));
      if (which < (int)m->numVertices()) {
        m->getVertex(which)->setTextureCoordinates(texture_s,texture_t);
      }

    } else if (token == "vn") {
    } else if (token[0] == '#') {
      // if it's a comment, skip the rest of the line
      char tmp[1000];
      istr.getline(tmp,1000); 
      //(*ARGS->output) << "ignoring comment: " << tmp << std::endl;
      continue; 
    } else {
      (*ARGS->output) << "LINE: " << line << std::endl;
    }
  }
  (*ARGS->output) << "LOADED " <<
    m->numVertices() <<
    " vertices " <<
    m->numElements() <<
    " elements ... " << std::endl;

  m->CheckPatches();

}



// =========================================================================
// =========================================================================
// ====================================================================

int last_token_set = 0;
char last_token = '\0';

bool isSeparator(char c) {
  if (c == ',' ||
      c == '[' ||
      c == ']' ||
      c == '{' ||
      c == '}') return true;
  return false;
}

int getToken(FILE *file, char token[MAX_PARSER_TOKEN_LENGTH]) {
  // for simplicity, tokens must be separated by whitespace
  assert (file != NULL);

  int i = 0;

  if (last_token_set) {
    token[i] = last_token;
    last_token_set = 0;
    i++;
  }

  while (1) {

    assert (i < MAX_PARSER_TOKEN_LENGTH - 2);
    if (i > 0 && isSeparator(token[i-1])) break;

    char c = fgetc(file);
    if (c == EOF) break;
    if (isspace(c)) {
      if (i == 0) continue;
      break;
    }
    if (isSeparator(c) && i > 0) {
      last_token = c;
      last_token_set = 1;
      break;
    }

    token[i] = c;
    i++;
  }
  token[i] = '\0';

  //printf ("my token %s\n", token);

  return (i > 0);
}

// ====================================================================
// ====================================================================

