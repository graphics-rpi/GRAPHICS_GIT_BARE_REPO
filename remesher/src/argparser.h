#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <fstream>
#include <cassert>
#include <glui.h>
#include "mtrand.h"
#include "OpenGLProjector.h"
#include "vectors.h"
#include "utils.h"


class ArgParser {

public:
  ArgParser() { DefaultValues(); }

  ArgParser(const std::vector<std::string> &command_line_args) {
    DefaultValues();
    for (unsigned int i = 1; i < command_line_args.size(); i++) {

      // filenames
      if (command_line_args[i] == "-input" ||
	  command_line_args[i] == "-i") {
	i++; assert (i < command_line_args.size()); 
	load_file = GLUI_String(command_line_args[i]);
      } else if (command_line_args[i] == "-flip_y_up") {
	load_flip_y_up = true;
      } else if (command_line_args[i] == "-no_remesh") {
	do_remesh = false;
      } else if (command_line_args[i] == "-test_color_calibration") {
	test_color_calibration = true;
      } else if (command_line_args[i] == "-puzzle_mode") {
	puzzle_mode = true;
	i++; assert (i < command_line_args.size()); 
	puzzle_output_file = GLUI_String(command_line_args[i]);
	do_remesh = false;
	walls_create_arrangement = false;

      } else if (command_line_args[i] == "-graph_visualization_mode") {
	graph_visualization_mode = true;

      } else if (command_line_args[i] == "-single_projector_blending") {
	single_projector_blending = true;

      } else if (command_line_args[i] == "-shrink_projection_surfaces") {
	i++; assert (i < command_line_args.size()); 
	shrink_projection_surfaces = atof(command_line_args[i].c_str());  // in meters
      } else if (command_line_args[i] == "-army_tweening") {
	triangle_textures_and_normals = true;
	do_remesh = true;
	gap_under_walls = false; //true;
	walls_create_arrangement = false;
	save_as_lsvmtl = false;
	army = true;
	TWEAK_WALLS = false;
      } else if (command_line_args[i] == "-army") {
	army = true;
	make_arrangement_images = true;
	camera_image_file = "army.ppm";
	TWEAK_WALLS = false;
      } else if (command_line_args[i] == "-tweening") {
	triangle_textures_and_normals = true;
	do_remesh = false; //..true; //false;
	gap_under_walls = false; //true;
	walls_create_arrangement = false;
	save_as_lsvmtl = false;
	//	output_floor_polys = true;
      } else if (command_line_args[i] == "-colors_file") {
	i++; assert (i < command_line_args.size()); 
	colors_file = std::string(command_line_args[i]);
      } else if (command_line_args[i] == "-fillin_ceiling_only") {
	fillin_ceiling_only = true;
      } else if (command_line_args[i] == "-output" ||
		 command_line_args[i] == "-o") {
	i++; assert (i < command_line_args.size()); 
	save_file = GLUI_String(command_line_args[i]);
	//      } else if (command_line_args[i] == "-quads") ||
	// command_line_args[i] == "-q")) {
	//save_as_quads = 1;
      } else if (command_line_args[i] == "-blending_subdivision") {
	i++; assert (i < command_line_args.size()); 
	blending_subdivision = atof(command_line_args[i].c_str());
      } else if (command_line_args[i] == "-no_blending_hack") {
	no_blending_hack = true;
      } else if (command_line_args[i] == "-blending") {
	i++; assert (i < command_line_args.size()); 
	blending_file = GLUI_String(command_line_args[i]);
      } else if (command_line_args[i] == "-output_corners_file") {
	i++; assert (i < command_line_args.size()); 
	output_corners_file = command_line_args[i];
      } else if (command_line_args[i] == "-5_sided_cornell_emissive_wall") { //output_corners_file")) {
	five_sided_cornell_emissive_wall = true;
      } else if (command_line_args[i] == "-worst_angle") {
	i++; assert (i < command_line_args.size()); 
	worst_angle_threshold = atof(command_line_args[i].c_str());
      } else if (command_line_args[i] == "-extend_walls") {
	extend_walls = true;
      } else if (command_line_args[i] == "-physical_diffuse") {
	i++; assert (i < command_line_args.size()); 
	physical_diffuse = command_line_args[i];
      } else if (command_line_args[i] == "-virtual_scene") {
	i++; assert (i < command_line_args.size()); 
	virtual_scene = command_line_args[i];
      } else if (command_line_args[i] == "-normal_tolerance") {
        i++; assert (i < command_line_args.size()); 
	remesh_normal_tolerance = atof(command_line_args[i].c_str());
      }
      else if (command_line_args[i] == "-target_num_triangles" || 
	       command_line_args[i] == "-triangles" ||
	       command_line_args[i] == "-t") {
        i++; assert (i < command_line_args.size()); 
        desired_tri_count = atoi(command_line_args[i].c_str());
      }
      else if (command_line_args[i] == "-target_num_patches" || 
	       command_line_args[i] == "-patches") {
        i++; assert (i < command_line_args.size()); 
        desired_patch_count = atoi(command_line_args[i].c_str());
	initialize_patches = true;
	render_vis_mode = 1;
	render_EXTRA_elements = 0; 
	ground_plane = 0;
	render_cluster_boundaries = 0;
	render_cull_ceiling = 0;

      }
      else if (command_line_args[i] == "-auto_sensors" ) {
        auto_sensor_placement = true;
      }
      else if (command_line_args[i] == "-noglui") {
        glui = false;
      }
      else if (command_line_args[i] == "-create_surface_cameras") {
        create_surface_cameras = true;
      }
      else if (command_line_args[i] == "-surface_cameras_fixed_size") {
	i++; assert (i < command_line_args.size()); 
        surface_cameras_fixed_size = atoi(command_line_args[i].c_str());
	assert (surface_cameras_fixed_size > 0);
      }
      else if (command_line_args[i] == "-floor_cameras_tiled") {
	i++; assert (i < command_line_args.size()); 
        floor_cameras_tiled = atoi(command_line_args[i].c_str());
	assert (floor_cameras_tiled > 0);
      }
      else if (command_line_args[i] == "-offline") {
        offline = 1;
      }
      else if (command_line_args[i] == "-offline_viewer") {
        offline_viewer = 1;
      }
      /*
      else if (command_line_args[i] == "-enclosed")) {
	i++; assert (i < command_line_args.size()); 
	enclosed_threshhold = atof(command_line_args[i].c_str());
      }
      */

      else if (command_line_args[i] == "-non_zero_interior_area") {
	non_zero_interior_area = true;
      }
      /*
      else if (command_line_args[i] == "-not_closed")) {
	closed_model = 0;
      }
      */

      else if (command_line_args[i] == "-fixed_seed") {
	i++; assert (i < command_line_args.size()); 
	int seed = atoi(command_line_args[i].c_str());
	mtrand = new MTRand((unsigned long)seed); 
      }

      // window
      else if (command_line_args[i] == "-width") {
	i++; 
	assert (i < command_line_args.size()); 
	width2 = atoi(command_line_args[i].c_str());
      } else if (command_line_args[i] == "-height") {
	i++; 
	assert (i < command_line_args.size()); 
	height2 = atoi(command_line_args[i].c_str());
      } else if (command_line_args[i] == "-geometry") {
	i++; 
	assert (i < command_line_args.size()); 
	width2 = atoi(command_line_args[i].c_str());
	i++; 
	assert (i < command_line_args.size()); 
	height2 = atoi(command_line_args[i].c_str());
	i++; 
	assert (i < command_line_args.size()); 
	pos_x = atoi(command_line_args[i].c_str());
	i++; 
	assert (i < command_line_args.size()); 
	pos_y = atoi(command_line_args[i].c_str());
      } else if (command_line_args[i] == "-p") {
	i++;
	int count = atoi(command_line_args[i].c_str());
	for (int j = 0; j < count; j++) {
	  i++; 
	  assert (i < command_line_args.size()); 	  
	  projectors.push_back(OpenGLProjector(command_line_args[i].c_str()));	
	  projector_names.push_back(command_line_args[i]);
	  //width = projector->getWidth();
	  //height = projector->getHeight();
	}
      } else if (command_line_args[i] == "-projector_center_override") {
	assert (projectors.size() > 0);
	i++; 
	assert (i < command_line_args.size()); 	  
	std::string glcamfile = command_line_args[i];
	double x,y,z;
	i++; 
	assert (i < command_line_args.size()); 	  
	x = atof(command_line_args[i].c_str());
	i++; 
	assert (i < command_line_args.size()); 	  
	y = atof(command_line_args[i].c_str());
	i++; 
	assert (i < command_line_args.size()); 	  
	z = atof(command_line_args[i].c_str());
	
	bool found = false;
	for (unsigned int j = 0; j < projectors.size(); j++) {
	  if (projector_names[j] == glcamfile) {
	    found = true;
	    std::cout << "FOUND PROJECTOR TO OVERRIDE " << glcamfile << std::endl;
	    projectors[j].setVec3fCenterReplacement(Vec3f(x,y,z));
	  }
	}
	assert (found == true);


      } else if (command_line_args[i] == "-camera" || command_line_args[i] == "-c") {
	i++;
	assert (i < command_line_args.size()); 
	glcam_camera = new OpenGLProjector(command_line_args[i].c_str());
      } else if (command_line_args[i] == "-no_edges") {
	render_mode = 6;  //
	render_vis_mode = 13; // blending weights
	render_wall_chains = 0;
	render_walls = 0;
	render_triangle_edges = 0;
	render_cluster_boundaries = 0;
	render_non_manifold_edges = 0;
	render_crease_edges = 0;
	render_zero_area_triangles = 0;
	render_bad_normal_triangles = 0;
	render_visibility_planes = 0;
	render_bad_neighbor_triangles = 0;
	ground_plane = 1;

      } else if (command_line_args[i] == "-make_arrangement_images") {
	i++;
	make_arrangement_images = true;
	assert (i < command_line_args.size()); 
	camera_image_file = std::string(command_line_args[i]);
      } else if (command_line_args[i] == "-make_all_arrangement_images") {
	i++;
	make_arrangement_images = true;
	make_all_arrangement_images = true;
	assert (i < command_line_args.size()); 
	camera_image_file = std::string(command_line_args[i]);
      } else if (command_line_args[i] == "-extra_length_multiplier") {
	i++; assert (i < command_line_args.size()); 
	extra_length_multiplier = atof(command_line_args[i].c_str());
      } else if (command_line_args[i] == "-cut_completely") {
	CUT_COMPLETELY = true;
      } else if (command_line_args[i] == "-num_curve_segments") {
	i++; assert (i < command_line_args.size()); 
	num_curve_segments = atoi(command_line_args[i].c_str());
	assert (num_curve_segments >= 1);
      } else if (command_line_args[i] == "-num_column_faces") {
	i++; assert (i < command_line_args.size()); 
	num_column_faces = atoi(command_line_args[i].c_str());
	assert (num_column_faces >= 4);
	if (num_column_faces % 2 == 1) num_column_faces++;
      } else if (command_line_args[i] == "-run_continuously") {	
	run_continuously = true;
      } else if (command_line_args[i] == "-stop_after") {
	i++; assert (i < command_line_args.size()); 
	stop_after = atoi(command_line_args[i].c_str());
      } else if (command_line_args[i] == "-increment_filename") {
	increment_filename = true;
      } else if (command_line_args[i] == "-num_planes_to_cut") {
	i++; assert (i < command_line_args.size()); 
	num_planes_to_cut = atoi(command_line_args[i].c_str());
	assert (num_planes_to_cut >= 0);
      } else if (command_line_args[i] == "-walls_rotate_translate") {
	i++; assert (i < command_line_args.size()); 
	walls_rotate_angle = atof(command_line_args[i].c_str())*M_PI/180.0;  // argument in degrees!
	i++; assert (i < command_line_args.size()); 
	walls_translate_x = atof(command_line_args[i].c_str()); 
	i++; assert (i < command_line_args.size()); 
	walls_translate_z = atof(command_line_args[i].c_str());
      } else if (command_line_args[i] == "-floor_plan_walls_vis") {
	floor_plan_walls_vis = true;
      } else if (command_line_args[i] == "-use_locked_output_directory") {
	i++; assert (i < command_line_args.size()); 
	locked_directory = command_line_args[i];
      } else if (command_line_args[i] == "-all_walls_8_inches") {
	all_walls_8_inches = true;
	/*
      } else if (command_line_args[i] == "-make_correspondences_file")) {
	make_correspondences_file = true;
	i++; assert (i < command_line_args.size()); 
	correspondences_a_file = command_line_args[i];
	i++; assert (i < command_line_args.size()); 
	correspondences_b_file = command_line_args[i];
	i++; assert (i < command_line_args.size()); 
	correspondences_out_file = command_line_args[i];
	*/
      } else if (command_line_args[i] == "-level_of_detail_and_correspondences") {
	level_of_detail_and_correspondences = true;
	i++; assert (i < command_line_args.size()); 
	int num_levels = atoi(command_line_args[i].c_str());
	for (int j = 0; j < num_levels; j++) {
	  i++; assert (i < command_line_args.size()); 
	  level_of_detail_desired_tri_count.push_back(atoi(command_line_args[i].c_str()));	
	}
      } else if (command_line_args[i] == "-verbose") {
	verbose = true;
      } else if (command_line_args[i] == "-quiet") {
	verbose = false;

      } else {
	printf ("whoops error with command line argument %d: '%s'\n",i,command_line_args[i].c_str());
	assert(0);
	exit(0);
      }
    }
    //    std::cout<<"AP after load:"<<walls_create_arrangement<<std::endl;
    //cout << " ewm = " << extra_length_multiplier << endl;
    if (verbose) {
      output = &std::cout;
    } else {
      output = new std::ofstream("/dev/null");
    }
  }
  

  void DefaultValues() {
    //std::cout<<"in default values"<<std::endl;
    triangle_textures_and_normals = false;
    gap_under_walls = false;
    //output_floor_polys = false;
    do_remesh = true;
        test_color_calibration = false;
    puzzle_mode = false;
    graph_visualization_mode = false;
    single_projector_blending = false;
    shrink_projection_surfaces = 0;
    no_blending_hack = false;
    //blending_subdivision = 20;  // a hack..
    //blending_subdivision = 30;  // a hack..
    blending_subdivision = 10;  // a hack..
    camera_image_file = "";
    make_arrangement_images = false;
    make_all_arrangement_images = false;
    physical_diffuse = "default";
    virtual_scene = "default";

    create_surface_cameras = false;
    surface_cameras_fixed_size = -1;
    floor_cameras_tiled = 1;

    which_projector = 0;

    five_sided_cornell_emissive_wall = false;
    fillin_ceiling_only = false;

    output_corners_file = "";
    worst_angle_threshold = -1;
    extend_walls = false;

    remesh_normal_tolerance = 0.99;
    //    remesh_normal_tolerance = 0.98;
    //remesh_normal_tolerance = 0.90;
    cut_normal_tolerance = 1; //0.999;
    desired_tri_count = 1000;
    desired_patch_count = 100;
    preserve_volume = 1;
    equal_edge_and_area = 0;
    initialize_patches = false;
    auto_sensor_placement = true;

    // rendering
    rerender_scene_geometry = 1;
    rerender_select_geometry = 1;

    render_mode = 1; // triangles
    render_vis_mode = 0; // diffuse material
    render_which_projector = 0;

    render_flipped_edges = 0;
    render_short_edges = 0;
    render_concave_angles = 0;
    render_faux_true = 0;
    render_problems_as_clusters = 0;
    render_problems_as_planes = 0;

    render_walls = 0;
    render_wall_chains = 0;
    render_PROJECTION_elements = 1;
    render_FILLIN_elements = 1;
    render_EXTRA_elements = 1;

    render_normals = 0;
    render_cracks = 0;

    render_triangle_edges = 1;
    render_cluster_boundaries = 1;
    render_non_manifold_edges = 1;
    render_crease_edges = 1;

    render_zero_area_triangles = 1;
    render_bad_normal_triangles = 1;
    render_bad_neighbor_triangles = 1;

    render_cull_back_facing = 0;
    render_cull_ceiling = 1;

    render_visibility_planes = 0;

    render_cluster_seeds = 0;
    render_voronoi_edges = 0;
    render_triangle_vertices = 0;
    render_voronoi_centers = 0;

    render_inner_hull = 0;
    render_triangle_hanging_chain = 0;
    render_voronoi_hanging_chain = 0;
    render_triangle_hanging_surface = 0;
    render_voronoi_hanging_surface = 0;

    transparency = 0;
    ground_plane = 1;
    white_background = 1;
    two_sided_surface = 1;
    ground_height = 0.1;
    
    sphere_tess_horiz = 30;
    sphere_tess_vert = 20;

    non_zero_interior_area = false;
    //closed_model = 1;

    // window size
    width2 = 800;
    height2 = 800;
    pos_x = 600;
    pos_y = 0;

    gl_lighting = 1;

    clip_x = 0;
    clip_y = 0;
    clip_z = 1;

    min_clip = -1;
    max_clip = 1;

    clip_enabled = 0;

    glui = true;
    offline = 0;
    offline_viewer = 0;
    //enclosed_threshhold = 0.65;
    glcam_camera = NULL;
    use_glcam_camera = true;
    extra_length_multiplier = 4.0;
    load_flip_triangles = 0;
    load_flip_y_up = 0;
    model_units = "meters"; 

    army = false;
    save_as_lsvmtl = true;  // true = save as .lsvmtl, false = save as .mtl
    CUT_COMPLETELY = false;
    num_curve_segments = 7;
    num_column_faces = 10;
    run_continuously = false;
    stop_after = -1;
    increment_filename = false;
    num_planes_to_cut = -1;

    walls_rotate_angle = 0;
    walls_translate_x = 0;
    walls_translate_z = 0;

    floor_plan_walls_vis = false;
    locked_directory = "";
    all_walls_8_inches = false;

    walls_create_arrangement = true;
    point_sampled_enclosure = 0; //false;
    mtrand = new MTRand(); // random seed!!! (use -fixed_seed otherwise... )

    SPLIT_BASED_ON_ENCLOSURE_HISTOGRAM = true;
    REMESH_BEFORE_ENCLOSURE_SPLIT = true;
    TWEAK_WALLS = true;
    PARALLEL_ANGLE_THRESHHOLD      = (5.0 * M_PI/180.0);   // 5 degrees
    PERPENDICULAR_ANGLE_THRESHHOLD = (5.0 * M_PI/180.0);   // 5 degrees
    WALL_THICKNESS                 = (3/8.0 * INCH_IN_METERS);
    ADJUSTABLE_D_OFFSET            = (0.5 * INCH_IN_METERS * 3);
    //ADJUSTABLE_D_OFFSET            = (1.5 * INCH_IN_METERS * 3);
    
    //make_correspondences_file = false;
    level_of_detail_and_correspondences = false;

    verbose = false; //true;

  }

  // return a random vector with each component from -1 -> 1
  Vec3f RandomVector() {
    double x = (*mtrand)() * 2 - 1;
    double y = (*mtrand)() * 2 - 1;
    double z = (*mtrand)() * 2 - 1;
    return Vec3f(x,y,z);
  }

  Vec3f RandomColor() {
    double x,y,z;
    while (1) {
      x = (*mtrand)(); if (x<0) x=0;
      y = (*mtrand)(); if (y<0) y=0;
      z = (*mtrand)(); if (z<0) z=0;
      if (x > 0.2 || y > 0.2 || z > 0.2) return Vec3f(x,y,z);
    }
  }

  // ==============
  // REPRESENTATION
  // all public! (no accessors)

  // filenames
  GLUI_String load_file;
  std::string colors_file;
  GLUI_String materials_file;
  GLUI_String save_file;
  bool triangle_textures_and_normals;
  bool gap_under_walls;
  bool do_remesh;

  bool test_color_calibration;
  bool puzzle_mode;
  bool graph_visualization_mode;
  bool single_projector_blending;
  double shrink_projection_surfaces;
  std::string puzzle_output_file;
  GLUI_String blending_file;
  double blending_subdivision;
  bool no_blending_hack;
  GLUI_String save_density_file;
  GLUI_String save_voronoi_surface_file;
  GLUI_String save_full_voronoi_file;
  GLUI_String save_compressed_voronoi_file;
  GLUI_String save_gaudi_triangles_file;
  GLUI_String save_gaudi_voronoi_file;
  GLUI_String save_tiles_file;   // fabrication
  int load_flip_triangles;
  int load_flip_y_up;

  std::string model_units; // "inches", "meters", "feet"

  std::string camera_image_file;
  bool make_arrangement_images;
  bool make_all_arrangement_images;

  bool five_sided_cornell_emissive_wall;
  bool fillin_ceiling_only;

  std::string output_corners_file;
  double worst_angle_threshold;
  bool extend_walls;

  // remeshing & painting
  float remesh_normal_tolerance;
  float cut_normal_tolerance;
  int desired_tri_count;
  int desired_patch_count;
  int preserve_volume;
  int equal_edge_and_area;
  bool initialize_patches;
  bool auto_sensor_placement;

  // rendering
  int rerender_scene_geometry;
  int rerender_select_geometry;

  int render_mode;
  int render_vis_mode;
  int render_which_projector;


  int render_flipped_edges;
  int render_short_edges;
  int render_concave_angles;
  int render_faux_true;
  int render_problems_as_clusters;
  int render_problems_as_planes;
  
  int render_walls;
  int render_wall_chains;

  int render_PROJECTION_elements;
  int render_FILLIN_elements;
  int render_EXTRA_elements;

  int render_normals;
  int render_cracks;
  
  int render_triangle_edges;
  int render_voronoi_edges;
  int render_cluster_boundaries;
  int render_non_manifold_edges;
  int render_crease_edges;
  int render_zero_area_triangles;
  int render_bad_normal_triangles;
  int render_bad_neighbor_triangles;

  int render_cull_back_facing;
  int render_cull_ceiling;
  int render_visibility_planes;
  int render_cluster_seeds;
  int render_triangle_vertices;
  int render_voronoi_centers;

  int render_inner_hull;
  int render_triangle_hanging_chain;
  int render_voronoi_hanging_chain;
  int render_triangle_hanging_surface;
  int render_voronoi_hanging_surface;

  int transparency;
  int ground_plane;
  int white_background;
  int two_sided_surface;
  float ground_height;

  bool non_zero_interior_area;

  //int closed_model;

  // window size
  int width2;
  int height2;
  int pos_x;
  int pos_y;

  int gl_lighting;

  float clip_x;
  float clip_y;
  float clip_z;

  float min_clip;
  float max_clip;

  int clip_enabled;

  int sphere_tess_horiz;
  int sphere_tess_vert;

  bool glui;
  int offline;
  int offline_viewer;

  //float enclosed_threshhold;

  OpenGLProjector *glcam_camera;
  bool use_glcam_camera;

  std::vector<OpenGLProjector> projectors;
  double extra_length_multiplier;

  std::string physical_diffuse;
  std::string virtual_scene;

  bool create_surface_cameras;
  int surface_cameras_fixed_size;
  int floor_cameras_tiled;

  int which_projector;
  std::vector<std::string> projector_names;

  //bool output_floor_polys;

  bool army;
  bool save_as_lsvmtl;  // true = save as .lsvmtl, false = save as .mtl

  MTRand *mtrand;

  bool CUT_COMPLETELY;
  int num_curve_segments;
  int num_column_faces;

  bool walls_create_arrangement;
  int point_sampled_enclosure;

  bool run_continuously;
  int stop_after;
  bool increment_filename;
  int num_planes_to_cut;

  double walls_rotate_angle;
  double walls_translate_x;
  double walls_translate_z;

  bool floor_plan_walls_vis;
  std::string locked_directory;
  bool all_walls_8_inches;

  bool SPLIT_BASED_ON_ENCLOSURE_HISTOGRAM;
  bool REMESH_BEFORE_ENCLOSURE_SPLIT;
  bool TWEAK_WALLS;

  double PARALLEL_ANGLE_THRESHHOLD;
  double PERPENDICULAR_ANGLE_THRESHHOLD;
  double WALL_THICKNESS;
  double ADJUSTABLE_D_OFFSET;

  //  bool make_correspondences_file;
  //std::string correspondences_a_file;
  //std::string correspondences_b_file;
  //std::string correspondences_out_file;

  bool level_of_detail_and_correspondences;
  std::vector<int> level_of_detail_desired_tri_count;

  bool verbose;
  std::ostream *output;

// ========================================================


};

#endif
