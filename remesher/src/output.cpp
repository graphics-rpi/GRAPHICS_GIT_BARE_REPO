#include "mesh.h"
#include "vertex.h"
#include "triangle.h"
#include "meshio.h"
#include "meshmanager.h"
#include "argparser.h"

#include <string>
#include <set>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "argparser.h"
extern ArgParser *ARGS;

// =======================================================================
// SAVE
// =======================================================================

void MeshIO::Save(MeshManager *meshes, Mesh *m, const std::string &filename_) {
  const char* filename = filename_.c_str();
  assert (m != NULL);
  assert (filename != NULL);
  if (!strcmp(filename,"")) {
    printf ("ERROR:  empty save file\n");
    return;
  }

  if (strstr(filename,".obj")) {
    SaveOBJ(meshes, m, filename);
  } else {
    printf ("ERROR: unknown file extension '%s'\n",filename);
  }
}

// =======================================================================
// =======================================================================

void MeshIO::SaveOBJ(MeshManager *meshes, Mesh *m, const std::string &filename_,
		     std::vector<std::pair<std::vector<Vec3f>,const MeshMaterial*> > *optional_tri_vec) {

  const char* filename = filename_.c_str();
  std::string blending_filename = std::string(filename);
  std::string geolocation_filename = std::string(filename);
  std::set<std::string> used_materials;

  assert (filename_.substr(filename_.size()-4,filename_.size()) == ".obj");
  std::string mat_filename;
  if (meshes->args->save_as_lsvmtl) {
    mat_filename = filename_.substr(0,filename_.size()-3)+"lsvmtl";
  } else {
    mat_filename = filename_.substr(0,filename_.size()-3)+"mtl";
  }

  int len = blending_filename.length();

  blending_filename[len-3] = 'b';
  blending_filename[len-2] = 'l';
  blending_filename[len-1] = 'e';

  geolocation_filename[len-3] = 'g';
  geolocation_filename[len-2] = 'e';
  geolocation_filename[len-1] = 'o';
  geolocation_filename.push_back('l');
  geolocation_filename.push_back('o');
  geolocation_filename.push_back('c');

  if (meshes->args->locked_directory != "") { 
    blending_filename = meshes->args->locked_directory + "/" + blending_filename; 
    geolocation_filename = meshes->args->locked_directory + "/" + geolocation_filename; 
  }

  (*ARGS->output) << "  writing elements to " << filename;
  fflush(stdout);

  std::string tmp = filename;
  if (meshes->args->locked_directory != "") { 
    tmp = meshes->args->locked_directory + "/" + tmp; }


  std::ofstream *obj_ostr = new std::ofstream(tmp.c_str());
  assert (obj_ostr != NULL);
  //FILE *file = fopen(tmp.c_str(),"w");

  //assert(file != NULL);


  // ------------------------------

  for (unsigned int zid = 0; zid < m->numZones(); zid++) {
    const Zone &z = m->getZone(zid);
    (*obj_ostr) << "zone " << z.getName() << std::endl;
  }


  for (unsigned int pid = 0; pid < m->numPatches(); pid++) {
    //const Patch &p = m->getPatch(pid);
    int zid = m->getAssignedZoneForPatch(pid);
    assert (zid == -1 ||
	    (zid >= 0 && zid < (int)m->numZones()));
    (*obj_ostr) << "patch " << zid << std::endl;
  }
  
  //fprintf(file,"mtllib %s\n", mat_filename.c_str());
  (*obj_ostr) << "mtllib " << mat_filename.c_str() << std::endl;


  std::ofstream geolocation_ofstream;
  geolocation_ofstream.open(geolocation_filename.c_str());
  geolocation_ofstream << meshes->getWalls()->north_angle;
  geolocation_ofstream << std::endl;
  geolocation_ofstream << meshes->getWalls()->longitude;
  geolocation_ofstream << std::endl;
  geolocation_ofstream << meshes->getWalls()->latitude;
  geolocation_ofstream << std::endl;
  geolocation_ofstream.close();

  // open the blending file
  std::ofstream blending_ofstream;
  int num_proj = meshes->args->projector_names.size();
  if (num_proj > 0) {
    (*ARGS->output) << "  writing blending weights to " << blending_filename << " ... ";
    fflush(stdout);    
    blending_ofstream.open(blending_filename.c_str());
    assert(blending_ofstream.good());
  }

  for (unsigned int i = 0; i < m->numVertices(); i++) {
    Vertex *vert = m->getVertex(i);
    Vec3f v = vert->get();
    //fprintf(file,"v %f %f %f\n", v.x(),v.y(),v.z());
    (*obj_ostr) << "v " << v.x() << " " << v.y() << " " << v.z() << std::endl;

    if (meshes->args->triangle_textures_and_normals == true) {
      double s,t;
      vert->getTextureCoordinates(s,t);
      (*obj_ostr) << "vt " << s << " " << t << std::endl; 
      Vec3f normal = vert->getNormal();
      (*obj_ostr) << "vn " << normal.x() << " " << normal.y() << " " << normal.z() << std::endl;
    }
  }

  // collect the used_materials
  for (elementshashtype::const_iterator foo = m->getElements().begin();
       foo != m->getElements().end();
       foo++) { 
    Element *e = foo->second;
    std::string matname = e->getRealMaterialName();
    used_materials.insert(matname);
  }


  // ------------------------------
  // OUTPUT THE TRIANGLES
  for (std::set<std::string>::iterator mat_iter = used_materials.begin();
       mat_iter != used_materials.end(); mat_iter++) {
    
    const std::string &THIS_MAT = *mat_iter;
    //fprintf (file,"usemtl %s\n", THIS_MAT.c_str());
    (*obj_ostr) << "usemtl " << THIS_MAT.c_str() << std::endl;

    for (elementshashtype::const_iterator foo = m->getElements().begin();
	 foo != m->getElements().end();
	 foo++) { 

      Element *e = foo->second;
      const std::string &matname = e->getRealMaterialName();
      if (matname != THIS_MAT) continue;

      if (e->isALine()) {
	int e_0 = (*e)[0];
	int e_1 = (*e)[1];
	//fprintf (file,"l %d %d\n", e_0+1,e_1+1);
	(*obj_ostr) << "l " << e_0+1 << " " << e_1+1 << std::endl;
      } else if (e->isATriangle()) {
	// output each element
	int e_0 = (*e)[0];
	int e_1 = (*e)[1];
	int e_2 = (*e)[2];
	if (optional_tri_vec != NULL) {
	  std::vector<Vec3f> tmp(3);
	  tmp[0] = m->getVertex(e_0)->get();
	  tmp[1] = m->getVertex(e_1)->get();
	  tmp[2] = m->getVertex(e_2)->get();
	  const MeshMaterial *mat = e->getMaterialPtr();
	  optional_tri_vec->push_back(make_pair(tmp,mat));
	}

	if (meshes->args->triangle_textures_and_normals == false) {
	  //fprintf (file,"f %d %d %d\n", e_0+1,e_1+1,e_2+1);
	  (*obj_ostr) << "f " << e_0+1 << " " << e_1+1 << " " << e_2+1 << std::endl;
	} else {
	  //fprintf (file,"f %d/%d/%d %d/%d/%d %d/%d/%d\n", 
	  //		   e_0+1,e_0+1,e_0+1,
	  //	   e_1+1,e_1+1,e_1+1,
	  //	   e_2+1,e_2+1,e_2+1);
	  (*obj_ostr) << "f " << 
	    e_0+1 << "/" << e_0+1 << "/" << e_0+1 << " " <<
	    e_1+1 << "/" << e_1+1 << "/" << e_1+1 << " " <<
	    e_2+1 << "/" << e_2+1 << "/" << e_2+1 << std::endl;
	}
	
	int pid = m->getAssignedPatchForElement(e->getID());
	assert (pid == -1 ||
		(pid >= 0 && pid < (int)m->numPatches()));
	if (pid != -1) {
	  (*obj_ostr) << "fp " << pid << std::endl; 
	}

    } else {
	assert (e->isAQuad());
	assert (meshes->args->triangle_textures_and_normals == false);
	int e_0 = (*e)[0];
	int e_1 = (*e)[1];
	int e_2 = (*e)[2];
	int e_3 = (*e)[3];
	//fprintf (file,"f %d %d %d %d\n", e_0+1,e_1+1,e_2+1,e_3+1);
	(*obj_ostr) << "f " << e_0+1 << " " << e_1+1 << " " << e_2+1 << " " << e_3+1 << std::endl;
      }
      
      if (num_proj > 0) {
	assert (blending_ofstream.good());
	if (e->isALine()) {
	  blending_ofstream << "l" << std::endl;
	} else {
	  blending_ofstream << "f" << std::endl;
	}
	for (int i = 0; i < e->numVertices(); i++) {
	  for (int j = 0; j < num_proj; j++) {
	    double w = e->getBlendWeightWithDistance(i,j);
	    if (w < 0.0001) 
	      blending_ofstream << 0 << " ";
	    else if (w > 0.9999)
	      blending_ofstream << 1 << " ";
	    else {
	      blending_ofstream << std::setprecision(2) << std::fixed << w << " " ;
	    }
	  }
	  blending_ofstream << std::endl;
	}
      }
    }
  }

  obj_ostr->close();
  (*ARGS->output) << "done" << std::endl;



  if (meshes->args->save_as_lsvmtl)
    SaveLSVMTL(meshes,m,mat_filename,used_materials);
  else 
    SaveMTL(meshes,m,mat_filename,used_materials);

  (*ARGS->output) << "done" << std::endl;
}




void MeshIO::SaveLSVMTL(MeshManager *meshes, Mesh *m, const std::string &filename, std::set<std::string> used_materials) {
  (*ARGS->output) << "  writing lsv materials to " << filename << " ... ";
  fflush(stdout);

  std::string tmp = filename;
  if (meshes->args->locked_directory != "") { 
    tmp = meshes->args->locked_directory + "/" + tmp; }

  std::ofstream ostr(tmp.c_str());
  assert(ostr.good());
  const std::map<std::string,MeshMaterial> &materials = m->getMaterials();

  for (std::map<std::string,MeshMaterial>::const_iterator i = materials.begin();
       i != materials.end(); i++) {
    const MeshMaterial &mat = i->second;
    std::string name = mat.getName();
    if (used_materials.find(name) == used_materials.end()) { continue; }
    assert (name != "");
    ostr << "newmtl " << name << std::endl;
    if (mat.isLambertian()) ostr << "lambertian" << std::endl;
    if (mat.isGlass()) ostr << "glass" << std::endl;
    if (mat.isPrismatic()) ostr << "prismatic" << std::endl;
    if (mat.isTabular()) ostr << "tabular" << std::endl;
    if (mat.isInvisible()) ostr << "invisible" << std::endl;
    if (mat.isExtra()) ostr << "extra" << std::endl;
    if (mat.isFillin()) ostr << "fillin" << std::endl;
    if (mat.isExterior()) ostr << "exterior" << std::endl;
    if (mat.isEmitter()) ostr << "emitter" << std::endl;
    if (mat.isEmitter()) ostr << "diffuse_emitter" << std::endl;
    if (mat.isSensor()) ostr << "sensor" << std::endl;
    if (mat.isLambertian()) {
      OUTPUT(ostr,mat.getDiffuseReflectance(),"DR");
    }
    if (mat.isGlass()) {
      OUTPUT(ostr,mat.getDiffuseReflectance(),"DR");
      OUTPUT(ostr,mat.getSpecularReflectance(),"SR");
      OUTPUT(ostr,mat.getDiffuseTransmittance(),"DT");
      OUTPUT(ostr,mat.getSpecularTransmittance(),"ST");
    }
    if (mat.isPrismatic()) {
      OUTPUT(ostr,mat.getSpecularTransmittance(),"ST");
      const Vec2f &angles = mat.getPrismaticAngles();
      ostr << "PrismaticAngles " << angles.x() << " " << angles.y() << std::endl;
      OUTPUT(ostr,mat.getPrismaticUp(),"PrismaticUp");
      OUTPUT(ostr,mat.getPrismaticNormal(),"PrismaticNormal");
    }
    if (mat.isTabular()) {
      (*ARGS->output) << "Data " << mat.getTabularDirectoryName() << std::endl;
    }
    if (mat.isEmitter()) {
      OUTPUT(ostr,mat.getEmittance(),"Ke");
    }
    OUTPUT(ostr,mat.getPhysicalDiffuseReflectance(),"Pd");
    ostr << "endmtl" << std::endl;
    ostr << std::endl;
  }

  (*ARGS->output) << "done" << std::endl;
}

void MeshIO::SaveMTL(MeshManager *meshes, Mesh *m, const std::string &filename, std::set<std::string> used_materials) {
  (*ARGS->output) << "  writing materials to " << filename << " ... ";
  fflush(stdout);

  std::string tmp = filename;
  if (meshes->args->locked_directory != "") { 
    tmp = meshes->args->locked_directory + "/" + tmp; }

  std::ofstream ostr(tmp.c_str());
  assert(ostr.good());
  const std::map<std::string,MeshMaterial> &materials = m->getMaterials();

  for (std::map<std::string,MeshMaterial>::const_iterator i = materials.begin();
       i != materials.end(); i++) {
    const MeshMaterial &mat = i->second;
    std::string name = mat.getName();
    if (used_materials.find(name) == used_materials.end()) { continue; }
    assert (name != "");
    ostr << "newmtl " << name << std::endl;
    if (mat.isLambertian() || mat.isGlass()) {
      Vec3f d = mat.getDiffuseReflectance();
      ostr << "Kd " << d.r() << " " << d.g() << " " << d.b() << std::endl;
    }
    if (mat.isEmitter()) {
      Vec3f e = mat.getEmittance();
      ostr << "Ke " << e.r() << " " << e.g() << " " << e.b() << std::endl;
    }
    if (mat.isGlass()) {
      ostr << "d " << 1-mat.getSpecularTransmittance().r() << std::endl;
    }
    if (mat.getTextureFilename() != "") {
      ostr << "map_Ka " << mat.getTextureFilename() << std::endl;
    }
    ostr << std::endl;
  }

  (*ARGS->output) << "done" << std::endl;
}

// =========================================================================

