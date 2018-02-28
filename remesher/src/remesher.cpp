#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <map>

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


#include "glCanvas.h"
#include "mesh.h"
#include "meshmanager.h"
#include "element.h"

#include "meshio.h"
#include "remesh.h"
#include "directory_locking.h"

#include "argparser.h"
extern ArgParser *ARGS;

void DoOnQuit(MeshManager *meshes);
void DoRemesh(MeshManager &meshes);
void MakeArrangementsImages(MeshManager *meshes);
void MakeCorrespondenceMeshes(ArgParser *args,MeshManager *meshes);

ArgParser *ARGS;

// ====================================================================
// ====================================================================

bool IncrementFileName(std::string &tmp2) { //filename) {
  std::string tmp = tmp2;
  if (tmp.substr(tmp.size()-5,5) == ".wall") {
    std::string number = tmp.substr(tmp.size()-11,6);
    //cout << "TEST" << number << std::endl;
    int num = atoi(number.c_str());
    //cout << "NUM " << num << std::endl;
    num++;
    std::stringstream wall_filename;
    wall_filename << tmp.substr(0,tmp.size()-11) << std::setw(6) << std::setfill('0') << num << ".wall";
    tmp  = wall_filename.str();
  } else {
    assert (tmp.substr(tmp.size()-4,4) == ".led");
    std::string number = tmp.substr(tmp.size()-10,6);
    //cout << "TEST" << number << std::endl;
    int num = atoi(number.c_str());
    //cout << "NUM " << num << std::endl;
    num++;
    std::stringstream wall_filename;
    wall_filename << tmp.substr(0,tmp.size()-10) << std::setw(6) << std::setfill('0') << num << ".led";
    tmp  = wall_filename.str();
  }
  std::ifstream test(tmp.c_str());
  if (test) {
    tmp2 = tmp;
    return true;
  } else {
    return false;
  }
}

// the original main function!
const Mesh* remesher_main(const std::vector<std::string> &command_line_args, bool stealmesh) {

  std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" << std::endl;
  std::cout << "REMESHER core begin!" << std::endl;

  //main(int argc, char *argv[]) {

  // parse the command line arguments & input file
  MeshManager meshes(command_line_args);

  //  (*ARGS->output)<<"before ARGS:"<<meshes.args->walls_create_arrangement<<std::endl;
  ARGS=meshes.args;

  (*ARGS->output) << "REMESHER verbose=" << ARGS->verbose << " stealmesh=" << stealmesh << "\r" << std::endl;


  /*
  if (ARGS->make_correspondences_file) {
    MakeCorrespondencesFile(ARGS);
    exit(0);
  }
  */
  
  DirLock *directory = NULL;
  if (ARGS->locked_directory != "") {
    directory = new DirLock(ARGS->locked_directory.c_str());
  }

  if (meshes.args->offline_viewer ||
      meshes.args->make_arrangement_images ||
      !meshes.args->offline) {
    //assert (meshes.args->run_continuously == false);
    
    (*ARGS->output) << "RESETTING ELEMENT_ID!" << std::endl;
    Element::currentID = 1;

    int argc_fake = 1;
    char **argv_fake = new char*[1];
    //argv_fake[0] = new char[command_line_args[0].size()+1];
    // strcpy(argv_fake[0]
    argv_fake[0] = (char*)command_line_args[0].c_str();
	(*ARGS->output)<<"before MeshIO::Load:"<<meshes.args->walls_create_arrangement<<std::endl;

    MeshIO::Load(&meshes, meshes.args->load_file); 


    //PrintAllElements(meshes.getMesh());


    //glutInit(&argc_fake,(char**)argv_fake);
    glutInit(&argc_fake,argv_fake);

    //(*ARGS->output) << "after fake" << std::endl;
    GLCanvas::initialize(&meshes);
    if (meshes.args->make_arrangement_images) {
      MakeArrangementsImages(&meshes);
    }
    // WAS COMMENTED OUT???
    DoRemesh(meshes);    
    if (!meshes.args->offline) {
      //glutFullScreen();
      glutMainLoop();
    }
  } else {
    assert (meshes.args->offline);
    clock_t start_time = clock();
    int frames = 0;
    do {      

      (*ARGS->output) << "RESETTING ELEMENT_ID!" << std::endl;
      Element::currentID = 1;

      MeshIO::Load(&meshes, meshes.args->load_file); 

      if (meshes.args->level_of_detail_and_correspondences) {
	(*ARGS->output) << "NEED TO DO LEVEL OF DETAIL" << std::endl;
	if (directory != NULL) { directory->Lock(); }
	MakeCorrespondenceMeshes(ARGS,&meshes);
	if (directory != NULL) { directory->Unlock(); }

      } else {
	// WAS COMMENTED OUT
	DoRemesh(meshes); 

	if (directory != NULL) { directory->Lock(); }
	MeshIO::Save(&meshes, meshes.getMesh(), meshes.args->save_file); 
	if (directory != NULL) { directory->Unlock(); }

	if (stealmesh) {
	  // CHECK LATER: MIGHT NOT BE DELETING EVERYTHING CORRECTLY & FULLY
	  (*ARGS->output)<<"in stealmesh "<< meshes.getMesh()->numVertices() << std::endl;
	  

	  std::cout << "REMESHER core finished!" << std::endl;
	  std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" << std::endl;

	  return meshes.stealMesh();
	}
      }

      (*ARGS->output) << "about to clear" << std::endl;
      meshes.Clear();
      (*ARGS->output) << "here" << std::endl;
      frames++;
      clock_t end_time = clock();
      double total_time = (end_time - start_time) / double(CLOCKS_PER_SEC);
      double fps = double(frames) / total_time;
      (*ARGS->output) << "FPS = " << fps << std::endl;
      if (meshes.args->stop_after > 0 && frames >= meshes.args->stop_after) break;
      if (meshes.args->increment_filename ==true) {
	bool incr_success = IncrementFileName(meshes.args->load_file);
	if (!incr_success) break;
      }      
    } while (meshes.args->run_continuously == true);
  }

  if (directory != NULL) { delete directory; }

  DoOnQuit(&meshes);

  std::cout << "REMESHER core finished!" << std::endl;
  std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" << std::endl;
  return NULL;
}

// ====================================================================
// ====================================================================

void MakeArrangementsImages(MeshManager *meshes) {
  if (!meshes->args->make_arrangement_images) return;

  int len = meshes->args->camera_image_file.size();
  std::string b = meshes->args->camera_image_file;
  assert (b.substr(len-4,4) == ".ppm");
  std::string base = b.substr(0,len-4);

  meshes->args->render_triangle_edges = 0;
  meshes->args->render_cluster_boundaries = 0;
  meshes->args->render_non_manifold_edges = 0;
  meshes->args->render_crease_edges = 0;
  meshes->args->render_zero_area_triangles = 0;
  meshes->args->render_bad_normal_triangles = 0;
  meshes->args->render_visibility_planes = 0;
  meshes->args->render_bad_neighbor_triangles = 0;
  meshes->args->ground_plane = 1;

  if (meshes->args->make_all_arrangement_images) {
    // 3D walls
    meshes->args->render_wall_chains = 0;
    meshes->args->render_mode = 0; 
    meshes->args->render_walls = 1;
    meshes->args->camera_image_file = base + "_3D_walls.ppm";
    meshes->args->ground_plane = 1;
    meshes->args->rerender_scene_geometry = 1;
    GLCanvas::display();
  }

  if (!meshes->args->army) {
    // 2D walls
    meshes->args->render_wall_chains = 0;
    meshes->args->render_mode = 7; 
    meshes->args->render_walls = 0;
    meshes->args->camera_image_file = base + "_2D_walls.ppm";
    meshes->args->rerender_scene_geometry = 1;
    meshes->args->ground_plane = 0;
    meshes->args->rerender_scene_geometry = 1;
    GLCanvas::display();
  }

  if (meshes->args->make_all_arrangement_images) {
    // wall chains
    meshes->args->render_wall_chains = 1;
    meshes->args->render_mode = 0; 
    meshes->args->render_walls = 0;
    meshes->args->ground_plane = 1;
    meshes->args->camera_image_file = base + "_wall_chains.ppm";
    meshes->args->rerender_scene_geometry = 1;
    GLCanvas::display();
    
    // enclosed
    meshes->args->render_wall_chains = 0;
    meshes->args->render_mode = 2; 
    meshes->args->render_walls = 0;
    meshes->args->ground_plane = 1;
    meshes->args->point_sampled_enclosure = 0;
    meshes->args->camera_image_file = base + "_enclosed.ppm";
    meshes->args->rerender_scene_geometry = 1;
    GLCanvas::display();
    
    // enclosed
    meshes->args->render_wall_chains = 0;
    meshes->args->render_mode = 2; 
    meshes->args->render_walls = 0;
    meshes->args->ground_plane = 1;
    meshes->args->point_sampled_enclosure = 1;
    meshes->args->camera_image_file = base + "_enclosed_points.ppm";
    meshes->args->rerender_scene_geometry = 1;
    GLCanvas::display();
    
    // arrangement
    meshes->args->render_wall_chains = 0;
    meshes->args->render_mode = 3; 
    meshes->args->render_walls = 0;
    meshes->args->ground_plane = 1;
    meshes->args->camera_image_file = base + "_arrangement.ppm";
    meshes->args->rerender_scene_geometry = 1;
    GLCanvas::display();
    
    // fingerprint
    meshes->args->render_wall_chains = 0;
    meshes->args->render_mode = 4; 
    meshes->args->render_walls = 0;
    meshes->args->camera_image_file = base + "_fingerprint.ppm";
    meshes->args->rerender_scene_geometry = 1;
    GLCanvas::display();
  }

  // floor plan
  meshes->args->render_wall_chains = 0;
  meshes->args->render_mode = 6; 
  meshes->args->render_walls = 0;
  meshes->args->camera_image_file = base + "_floorplan.ppm";
  meshes->args->rerender_scene_geometry = 1;
  GLCanvas::display();

  meshes->args->make_all_arrangement_images = false;
  meshes->args->make_arrangement_images = false;
}

// ====================================================================
// ====================================================================

void DoRemesh(MeshManager &meshes) {

  ReMesh::Triangulate(&meshes);

  ReMesh::CutThroughPlanes(&meshes);

  if (meshes.args->do_remesh) {
    // DO REMESH, DO CHANGE TRIANGULATION
    
    (*ARGS->output) << "BEFORE REMESH: " << meshes.getMesh()->numElements() << " triangles" << std::endl;
    int q = 0;
    while(1) {
      (*ARGS->output) << "HERE" << q << std::endl;
      q++;
      fflush(stdout);
      while (ReMesh::EliminateBadTriangles(&meshes)) 
	{}
      for (int i = 0; i < 15; i++) {
	ReMesh::Evaluate(&meshes); 
	ReMesh::SplitEdges(&meshes);
	ReMesh::FlipEdges(&meshes); 	
	ReMesh::MoveVertices(&meshes);
	ReMesh::CollapseEdges(&meshes);
	(*ARGS->output) << " .";
	fflush(stdout);
      }
      while (ReMesh::EliminateBadTriangles(&meshes)) 
	{}	
      // taken out for EMPAC?
      ReMesh::CheatFakeMaterials(&meshes);
      for (int i = 0; i < 15; i++) {
	ReMesh::Evaluate(&meshes); 
	ReMesh::SplitEdges(&meshes);
	ReMesh::FlipEdges(&meshes); 
	ReMesh::MoveVertices(&meshes);
	ReMesh::CollapseEdges(&meshes);
	(*ARGS->output) << " .";
	fflush(stdout);
      }
      (*ARGS->output) << "AFTER REMESH: " << meshes.getMesh()->numElements() << " triangles" << std::endl;
      
      double worst_angle = ReMesh::WorstAngleOnProjectionTriangle(&meshes);
      (*ARGS->output) << "worst angle: " << worst_angle << " (" << worst_angle*180/M_PI << " degrees)  ";
      (*ARGS->output) << "threshold:   " << meshes.args->worst_angle_threshold << " (" << meshes.args->worst_angle_threshold*180/M_PI << " degrees)"<< std::endl;
      if (q > 20) break;
      if (worst_angle > meshes.args->worst_angle_threshold) break;
      else {
	int foo = 	meshes.args->desired_tri_count;
	meshes.args->desired_tri_count /=100;
	(*ARGS->output) << "DECIMATING: ";
	fflush(stdout);
	while (ReMesh::EliminateBadTriangles(&meshes)) 
	  {}
	for (int i = 0; i < 15; i++) {
	  ReMesh::Evaluate(&meshes); 
	  ReMesh::SplitEdges(&meshes);
	  ReMesh::FlipEdges(&meshes); 
	  ReMesh::MoveVertices(&meshes);
	  ReMesh::CollapseEdges(&meshes);
	  (*ARGS->output) << " .";
	  fflush(stdout);
	}
	(*ARGS->output) << "AFTER DECIMATE: " << meshes.getMesh()->numElements() << " triangles" << std::endl;
	meshes.args->desired_tri_count =foo*1.05;
      }
    }
    while (ReMesh::EliminateBadTriangles(&meshes)) 
      {}
  }

  
  // create patches & zones (used by lsvo)
  if (meshes.args->initialize_patches) {
    meshes.getMesh()->AssignPatchesAndZones(&meshes);
  }

  if (!meshes.args->offline) {
    GLCanvas::updateCounts();
  }
}


void DoOnQuit(MeshManager *meshes) {
  (*ARGS->output) << "do on quit" << std::endl;
  meshes->DeleteMemory(); //delete meshes;
  for (elementshashtype::const_iterator foo = Element::all_elements.begin();
       foo != Element::all_elements.end();
       foo++) {
    Element *e = foo->second;
    delete e;
  }
  Element::all_elements.clear();
  //Element::elements->DeleteAllElements();
  //delete Element::elements;
  //Element::elements = NULL;

  if (Element::acceleration_grid != NULL) { Element::DestroyAccelerationGrid(); }
  GLCanvas::clear();
}

// ====================================================================
// ====================================================================



void SaveCorrespondencesFile(std::vector<std::pair<std::vector<Vec3f>,const MeshMaterial*> > *TRIS_LOW,
			     std::vector<std::pair<std::vector<Vec3f>,const MeshMaterial*> > *TRIS_HIGH,
			     std::string filename) {
  assert (TRIS_LOW != NULL);
  assert (TRIS_HIGH != NULL);
  assert (TRIS_LOW->size() > 0);
  assert (TRIS_HIGH->size() > 0);
  assert (TRIS_LOW->size() <= TRIS_HIGH->size());

  std::vector<std::vector<int> > correspondences(TRIS_LOW->size());
    
  for (unsigned int i = 0; i < TRIS_HIGH->size(); i++) {
    //    bool found_best = false;
    int best = -1;
    double best_dot = -1;
    double best_d_diff = -1;
    double best_distance = -1;
    std::vector<Vec3f> &eb = (*TRIS_HIGH)[i].first;
    const MeshMaterial* mat_b = (*TRIS_HIGH)[i].second;
    assert (eb.size() == 3);
    Vec3f normal_b;  computeNormal(eb[0],eb[1],eb[2],normal_b);
    Vec3f centroid_b = (eb[0]+eb[1]+eb[2])*(1.0/3.0);
    for (unsigned int j = 0; j < TRIS_LOW->size(); j++) {
      std::vector<Vec3f> &ea = (*TRIS_LOW)[j].first;
      const MeshMaterial* mat_a = (*TRIS_LOW)[j].second;
      assert (ea.size() == 3);
      Vec3f normal_a; computeNormal(ea[0],ea[1],ea[2],normal_a);
      Vec3f centroid_a = (ea[0]+ea[1]+ea[2])*(1.0/3.0);
      if (mat_a != mat_b) continue;
      double dot = normal_a.Dot3(normal_b);
      if (dot < 0 || best_dot > dot+0.001) continue;
      double d_diff = fabs(centroid_b.Dot3(normal_b) - centroid_a.Dot3(normal_a));
      if (best_d_diff > -0.5 && best_d_diff < d_diff - 0.001) continue;
      double distance_to_triangle = DistanceBetweenPointAndTriangle(centroid_b,ea[0],ea[1],ea[2]); //t1,t2,t3);
      if (best_distance > -0.5 && best_distance < distance_to_triangle) continue;
      best = j;
      best_dot = dot;
      best_d_diff = d_diff;
      best_distance = distance_to_triangle;
    }
    assert (best >= 0 && best < int(TRIS_LOW->size())); //num_tris_a);
    correspondences[best].push_back(i);
  }

  std::vector<int> backup_correspondences(TRIS_LOW->size(),-1);

  for (unsigned int i = 0; i < TRIS_LOW->size(); i++) {
    if (correspondences[i].size() == 0) {
      int best = -1;
      double best_dot = -1;
      double best_d_diff = -1;
      double best_distance = -1;
      std::vector<Vec3f> &eb = (*TRIS_LOW)[i].first;
      const MeshMaterial* mat_b = (*TRIS_LOW)[i].second;
      assert (eb.size() == 3);
      Vec3f normal_b;  computeNormal(eb[0],eb[1],eb[2],normal_b);
      Vec3f centroid_b = (eb[0]+eb[1]+eb[2])*(1.0/3.0);
      for (unsigned int j = 0; j < TRIS_HIGH->size(); j++) {
	std::vector<Vec3f> &ea = (*TRIS_HIGH)[j].first;
	const MeshMaterial* mat_a = (*TRIS_HIGH)[j].second;
	assert (ea.size() == 3);
	Vec3f normal_a; computeNormal(ea[0],ea[1],ea[2],normal_a);
	Vec3f centroid_a = (ea[0]+ea[1]+ea[2])*(1.0/3.0);
	double dot = normal_a.Dot3(normal_b);
	if (dot < 0 || best_dot > dot+0.001) continue;
	if (mat_a != mat_b) continue;
	double d_diff = fabs(centroid_b.Dot3(normal_b) - centroid_a.Dot3(normal_a));
	if (best_d_diff > -0.5 && best_d_diff < d_diff - 0.001) continue;
	double distance_to_triangle = DistanceBetweenPointAndTriangle(centroid_b,ea[0],ea[1],ea[2]); //t1,t2,t3);
	if (best_distance > -0.5 && best_distance < distance_to_triangle) continue;
	best = j;
	best_dot = dot;
	best_d_diff = d_diff;
	best_distance = distance_to_triangle;
      }
      assert (best >= 0 && best < int(TRIS_HIGH->size()));
      backup_correspondences[i] = best;
    }
  }
  
  std::ofstream ostr(filename.c_str());

  for (unsigned int j = 0; j < TRIS_LOW->size(); j++) {
    std::vector<int> &links = correspondences[j];
    int num_links = links.size();
    ostr << j << " " << num_links << " : ";
    if (num_links == 0) {
      assert (backup_correspondences[j] >= 0);
      ostr << backup_correspondences[j];
    } else {
      assert (backup_correspondences[j] == -1);
      for (int i = 0; i < num_links; i++) {
	ostr << " " << links[i];
      }
    }
    ostr << std::endl;
  }

}



void MakeCorrespondenceMeshes(ArgParser *args, MeshManager *meshes) {

  std::vector<int> res = args->level_of_detail_desired_tri_count;
  assert (res.size() > 0);
  res.push_back(args->desired_tri_count);

  for (unsigned int i = 1; i < res.size(); i++) {
    assert (res[i-1] < res[i]);
  }

  std::vector<std::pair<std::vector<Vec3f>,const MeshMaterial*> > *TRIS_LOW = NULL;
  std::vector<std::pair<std::vector<Vec3f>,const MeshMaterial*> > *TRIS_HIGH = NULL;


  // ------------------------------------------------
  for (int i = int(res.size())-1; i >= 0; i--) {

    // REMESH
    args->desired_tri_count = res[i]; //4.0;
    DoRemesh(*meshes); 

    // SAVE IT
    std::string orig_save = args->save_file;
    assert (orig_save.substr(orig_save.size()-4,4) == std::string(".obj"));
    std::stringstream ss;
    int num_elements = meshes->getMesh()->numElements();
    //ss << orig_save.substr(0,orig_save.size()-4) << "_" << res[i] << ".obj";
    ss << orig_save.substr(0,orig_save.size()-4) << "_" << num_elements << ".obj";
    std::string tmp_out = ss.str();
    (*ARGS->output) << " going to save to " << tmp_out << std::endl;
    std::vector<std::pair<std::vector<Vec3f>,const MeshMaterial*> > *TRIS_LOW = new std::vector<std::pair<std::vector<Vec3f>,const MeshMaterial*> >();
    MeshIO::SaveOBJ(meshes, meshes->getMesh(), tmp_out, TRIS_LOW);
    assert (TRIS_LOW->size() == meshes->getMesh()->numElements());


    // CORRESPOND IT
    if (i != int(res.size())-1) {
      int num_elements2 = TRIS_HIGH->size();
      //(*ARGS->output) << " MAKE CORRESPONDENCE FILE " << res[i] << " -> " << res[i+1] << std::endl;
      (*ARGS->output) << " MAKE CORRESPONDENCE FILE " << num_elements << " -> " << num_elements2 << std::endl;
      std::stringstream ss;
      ss << orig_save.substr(0,orig_save.size()-4) << "_correspondence_" << num_elements << "_to_" << num_elements2 << ".txt";
      std::string tmp_out = ss.str();
      (*ARGS->output) << "correspondences file is " << tmp_out << std::endl;    
      
      
      SaveCorrespondencesFile(TRIS_LOW,TRIS_HIGH,tmp_out);
    }
    
    if (TRIS_HIGH != NULL) delete TRIS_HIGH;
    TRIS_HIGH = TRIS_LOW;
    TRIS_LOW = NULL;
  }
  // ------------------------------------------------

  assert (TRIS_LOW == NULL);
  assert (TRIS_HIGH != NULL);
  delete TRIS_HIGH;
}





//if (directory != NULL) { directory->Unlock(); }

/*
    Mesh a;
  Mesh b;
  
  std::vector<Element*> optional_load_vector_a;
  std::vector<Element*> optional_load_vector_b;

  MeshIO::LoadOBJ(&a,args->correspondences_a_file,args,&optional_load_vector_a);
  MeshIO::LoadOBJ(&b,args->correspondences_b_file,args,&optional_load_vector_b);

  int num_tris_a = a.numTriangles();
  int num_tris_b = b.numTriangles();

  (*ARGS->output) << std::endl << std::endl << "find correspondences from " <<
    num_tris_a << " to " << num_tris_b << std::endl;

  assert (num_tris_a < num_tris_b);


  */
//}

