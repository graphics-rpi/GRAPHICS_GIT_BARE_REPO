#include <cassert>

#include "argparser.h"
#include "mesh.h"
#include "meshio.h"
#include "meshmanager.h"

extern ArgParser *ARGS;

// =========================================================================
// =========================================================================


void MeshIO::LoadLSVMTL(Mesh *mesh, const std::string &filename, ArgParser *args) {
  assert(filename.size() >= 8);
  assert (filename.substr(filename.size()-7,filename.size()) == ".lsvmtl");
  assert (mesh->getMaterials().size() == 0);
  args->save_as_lsvmtl = true;
  (*ARGS->output) << "loading .lsvmtl material file: " << filename << std::endl;

  // open the file
  std::ifstream istr(filename.c_str());
  if (!istr) {
    (*ARGS->output) << "ERROR!  Couldn't open this file: " << filename << std::endl;
    exit(0);
  }  

  std::string token;
  double r,g,b;
  Vec3f v;

  MeshMaterial *mat = NULL;

  while(1) {
    // look ahead for the next char
    char c = istr.peek();
    if (c == EOF) break;
    if (isspace(c) != 0) {
      // if it's whitespace, skip it
      c = istr.get();
      continue;
    }
    if (c == '#') { 
      // if it specifies the file units line, 
      // if it's a comment, skip the rest of the line
      char tmp[1000];
      istr.getline(tmp,1000); 
      //(*ARGS->output) << "ignoring comment: " << tmp << std::endl;
      continue; 
    }
    // read in a whole token
    if (!(istr >> token)) break;
    if (token == "newmtl") { 
      assert (mat == NULL);
      mat = new MeshMaterial();
      istr >> token;
      mat->setName(token);
    } else if (token == "endmtl") { 
      assert (mat != NULL);
      mesh->addMaterial(*mat);
      delete mat;
      mat = NULL;
    } else if (token == "lambertian") {
      assert (mat != NULL);
      mat->setLambertian();
    } else if (token == "glass") {
      assert (mat != NULL);
      mat->setGlass();
    } else if (token == "prismatic") {
      assert (mat != NULL);
      mat->setPrismatic();
    } else if (token == "tabular") {
      assert (mat != NULL);
      mat->setTabular();
    } else if (token == "invisible") {
      assert (mat != NULL);
      mat->setInvisible();
    } else if (token == "extra") {
      assert (mat != NULL);
      mat->setExtra();
    } else if (token == "fillin") {
      assert (mat != NULL);
      mat->setFillin();
    } else if (token == "exterior") {
      assert (mat != NULL);
      mat->setExterior();
    } else if (token == "emitter") {
      assert (mat != NULL);
      mat->setEmitter();
    } else if (token == "diffuse_emitter") {
      assert (mat != NULL);
      mat->setEmitter();
    } else if (token == "sensor") {
      assert (mat != NULL);
      mat->setSensor();
    } else if (token == "DT") {
      assert (mat != NULL);
      istr >> r >> g >> b;
      mat->setDiffuseTransmittance(Vec3f(r,g,b));
    } else if (token == "ST") {
      assert (mat != NULL);
      istr >> r >> g >> b;
      mat->setSpecularTransmittance(Vec3f(r,g,b));
    } else if (token == "DR") {
      assert (mat != NULL);
      istr >> r >> g >> b;
      mat->setDiffuseReflectance(Vec3f(r,g,b));
    } else if (token == "SR") {
      assert (mat != NULL);
      istr >> r >> g >> b;
      (*ARGS->output) << mat->getName() << std::endl;
      mat->setSpecularReflectance(Vec3f(r,g,b));
    } else if (token == "Pd") {
      assert (mat != NULL);
      istr >> r >> g >> b;
      mat->setPhysicalDiffuseReflectance(Vec3f(r,g,b));
    } else if (token == "Ke") {
      assert (mat != NULL);
      istr >> r >> g >> b;
      mat->setEmittance(Vec3f(r,g,b));
    } else if (token == "Data") {
      assert (mat != NULL);
      istr >> token;
      mat->setTabularDirectoryName(token);
    } else if (token == "PrismaticAngles") {
      assert (mat != NULL);
      istr >> r >> g;
      mat->setPrismaticAngles(r,g);
    } else if (token == "PrismaticNormal") {
      assert (mat != NULL);
      istr >> r >> g >> b;
      mat->setPrismaticNormal(Vec3f(r,g,b));
    } else if (token == "PrismaticUp") {
      assert (mat != NULL);
      istr >> r >> g >> b;
      mat->setPrismaticUp(Vec3f(r,g,b));

    } else if (token == "map_Ka") {
      assert (mat != NULL);
      istr >> token;
      mat->setTextureFilename(token);

    } else {
      (*ARGS->output) << " don't understand2 " << token << std::endl;
    }
  }
  assert (mat == NULL);
}

void MeshIO::LoadMTL(Mesh *mesh, const std::string &filename, ArgParser *args) {
  assert(filename.size() >= 5);
  assert (filename.substr(filename.size()-4,filename.size()) == ".mtl");
  assert (mesh->getMaterials().size() == 0);
  // look for an .lsvmtl file instead
  std::string filename2 = filename.substr(0,filename.size()-3)+"lsvmtl";
  std::ifstream test_stream(filename2.c_str());
  if (test_stream) {
    LoadLSVMTL(mesh,filename2,args);
    return;
  }
  //  args->save_as_lsvmtl = false;
  (*ARGS->output) << "loading .mtl material file: " << filename << std::endl;

  // open the file
  std::ifstream istr(filename.c_str());
  if (!istr) {
    (*ARGS->output) << "ERROR!  Couldn't open this file: " << filename << std::endl;
    exit(0);
  }

  std::string token;
  double r,g,b;
  MeshMaterial *mat = NULL;

  while (1) {
    // look ahead for the next char
    char c = istr.peek();
    if (c == EOF) break;
    if (isspace(c) != 0) {
      // if it's whitespace, skip it
      c = istr.get();
      continue;
    }
    if (c == '#') { 
      // if it's a comment, skip the rest of the line
      char tmp[1000];
      istr.getline(tmp,1000); 
      //(*ARGS->output) << "ignoring comment: " << tmp << std::endl;
      continue; 
    }

    istr >> token;

    if (token == "newmtl") { 
      if (mat != NULL) {
	mesh->addMaterial(*mat);
	delete mat;
	mat = NULL;
      }
      mat = new MeshMaterial();
      istr >> token;
      mat->setName(token);
      if (token.find("GLASS") < std::string::npos) { mat->setGlass(); }
      if (token.find("FILLIN") < std::string::npos) { mat->setFillin(); }
      if (token.find("INVISIBLE") < std::string::npos) { mat->setInvisible(); mat->setSensor(); }
      if (token.find("EXTRA") < std::string::npos) { mat->setExtra(); }
      if (token.find("FILLIN") < std::string::npos) { mat->setFillin(); }
      if (token.find("SENSOR") < std::string::npos) { mat->setSensor(); }
      if (token.find("EMITTER") < std::string::npos) { mat->setEmitter(); }

      if (token.find("sensor") < std::string::npos) { mat->setSensor(); }

      if (token.find("sensor_glare") < std::string::npos) { mat->setInvisible(); }
      if (token.find("sensor_illuminance") < std::string::npos) { mat->setInvisible(); }

    } else if (token == "Ka") {
      istr >> r >> g >> b;
      // ignore ambient!
    } else if (token == "Kd") {
      istr >> r >> g >> b;
      mat->setDiffuseReflectance(Vec3f(r,g,b));
    } else if (token == "Pd") { 
      istr >> r >> g >> b;
      mat->setPhysicalDiffuseReflectance(Vec3f(r,g,b));
    } else if (token == "Ks") {
      istr >> r >> g >> b;
      // ignore specular!
    } else if (token == "Ke") {
      istr >> r >> g >> b;
      mat->setEmittance(Vec3f(r,g,b));
    } else if (token == "d") {
      // "dissolve", 1 == opaque, 0 == transparent
      istr >> r;
      // ignore opacity

      if (r < 0.999) {
        mat->setGlass();
        mat->setSpecularTransmittance(Vec3f(1-r,1-r,1-r));
      }

    } else if (token == "Ts") {
      istr >> r >> g >> b;
      // ignore 
    } else if (token == "Tf") {
      istr >> r >> g >> b;
      if (r > 0 || g > 0 || b > 0) {
        mat->setGlass();
        mat->setSpecularTransmittance(Vec3f(r,g,b));
      }
      // ignore 
    } else if (token == "Ns") {
      istr >> r;
      // ignore 

    } else if (token == "map_Ka") {
      assert (mat != NULL);
      istr >> token;
      mat->setTextureFilename(token);


    } else {
      (*ARGS->output) << " don't understand1 " << token << std::endl;
    }
  }
    
  assert (mat != NULL);
  if (mat != NULL) {
    mesh->addMaterial(*mat);
    delete mat;
    mat = NULL;
  }
}


void MeshIO::LoadColorsFile(MeshManager *meshes) {

  Vec3f wall_material_luan = Vec3f(0.9,0.9,0.9);
  Vec3f wall_material_canvas = Vec3f(0.9,0.9,0.9);
  Vec3f wall_material_curved = Vec3f(0.9,0.9,0.9);
  Vec3f wall_material_left_l_shaped = Vec3f(0.9,0.9,0.9);
  Vec3f wall_material_right_l_shaped = Vec3f(0.9,0.9,0.9);
  Vec3f wall_material_big_l_shaped = Vec3f(0.9,0.9,0.9);

  meshes->getWalls()->north_angle = 0;
  if (meshes->args->colors_file == "") {
    //(*ARGS->output) << "WARNING: NO COLORS FILE" << std::endl;
  } else {
    std::ifstream istr(meshes->args->colors_file.c_str());
    if (!istr) {
      printf ("ERROR! CANNOT OPEN '%s'\n",meshes->args->colors_file.c_str());
      return;
    }
    std::string token;
    float a,b,c;
    while (istr >> token) {
      if (token == "floor_material") {
	istr >> a >> b >> c;
	meshes->getWalls()->floor_material = Vec3f(a,b,c);
      } else if (token == "ceiling_material") {
	istr >> a >> b >> c;
	meshes->getWalls()->ceiling_material = Vec3f(a,b,c);
      } else if (token == "wall_material_luan") {
	istr >> a >> b >> c;
	wall_material_luan = Vec3f(a,b,c);
      } else if (token == "wall_material_canvas") {
	istr >> a >> b >> c;
	wall_material_canvas = Vec3f(a,b,c);
      } else if (token == "wall_material_left_l_shaped") {
	istr >> a >> b >> c;
	wall_material_left_l_shaped = Vec3f(a,b,c);
      } else if (token == "wall_material_right_l_shaped") {
	istr >> a >> b >> c;
	wall_material_right_l_shaped = Vec3f(a,b,c);
      } else if (token == "wall_material_big_l_shaped") {
	istr >> a >> b >> c;
	wall_material_big_l_shaped = Vec3f(a,b,c);
      } else if (token == "wall_material_curved") {
	istr >> a >> b >> c;
	wall_material_curved = Vec3f(a,b,c);
      } else if (token == "luan_window") {
	istr >> meshes->getWalls()->luan_window;
      } else if (token == "canvas_window") {
	istr >> meshes->getWalls()->canvas_window;
      } else if (token == "left_l_shaped_window") {
	istr >> meshes->getWalls()->left_l_shaped_window;
      } else if (token == "right_l_shaped_window") {
	istr >> meshes->getWalls()->right_l_shaped_window;
      } else if (token == "big_l_shaped_window") {
	istr >> meshes->getWalls()->big_l_shaped_window;
      } else if (token == "curved_window") {
	istr >> meshes->getWalls()->curved_window;
      } else {
	(*ARGS->output) << " UH OH UNKNOWN COLORS FILE TOKEN " << token << std::endl;
      }
    }
  }
  meshes->getWalls()->wall_materials.clear();
  meshes->getWalls()->wall_materials.push_back(wall_material_luan);
  meshes->getWalls()->wall_materials.push_back(wall_material_canvas);
  meshes->getWalls()->wall_materials.push_back(wall_material_curved);
  meshes->getWalls()->wall_materials.push_back(wall_material_left_l_shaped);
  meshes->getWalls()->wall_materials.push_back(wall_material_right_l_shaped);
  meshes->getWalls()->wall_materials.push_back(wall_material_big_l_shaped);
}

// =======================================================================================
// =======================================================================================
