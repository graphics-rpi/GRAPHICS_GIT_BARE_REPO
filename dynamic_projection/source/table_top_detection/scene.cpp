#include <cstdlib>
#include "scene.h"

void Scene::write(const char *filename){

    // approximate rotation to align pixel angles with x-z world
    // coordinate angles
    double r1 = 0.;
    double c1 = 0.;
    double r2 = 1000.;
    double c2 = 1000.;
    double height = 0.;
    v3d p1 = camera.PointFromPixel(r1, c1, height);
    v3d p2 = camera.PointFromPixel(r2, c2, height);
    double pixel_angle = atan2(r2-r1, c2-c1);
    if (pixel_angle < 0.) pixel_angle += 2*M_PI;
    //    printf("pixel_angle = %f\n", pixel_angle);
    double world_angle = atan2(p1.z()-p2.z(), p2.x()-p1.x());
    if (world_angle < 0.) world_angle += 2*M_PI;
    // printf("world_angle = %f\n", world_angle);
    double angle_adjust = -(world_angle - pixel_angle);
    if (angle_adjust < -M_PI) angle_adjust += 2*M_PI;
    if (angle_adjust >  M_PI) angle_adjust -= 2*M_PI;
    // printf("angle_adjust = %f\n", angle_adjust);

    // adjust the north direction
    north += M_PI;
    north += angle_adjust;
    if (north < -M_PI) north += 2*M_PI;
    if (north >  M_PI) north -= 2*M_PI;

    //#warning hack: align coordinate system with camera frame (correct solution)

    //angle_adjust = 0.;
    angle_adjust -= M_PI/2.;

    //printf("north = %f\n", north);
    
    FILE *fp = fopen(filename, "wt");
    if (NULL == fp){
      fprintf(stderr, "unable to open %s\n", filename);
      exit(-1);
    }

    // print table information
    if (args->find_army_terrain || args->find_architectural_design) {
      v3d center = table->getCenterWorld();
      fprintf(fp, "table %f %f %f %f\n",
	      center.x(), center.y(), center.z(), table->getRadius());
    }  


    // print north arrow, & materials for architecture
    if (args->find_architectural_design) {
      fprintf(fp, "north %+6.3f\n", north);
      fprintf(fp, "floor_material   %5.3f %5.3f %5.3f\n", 
              floor_color.r()/255.,
              floor_color.g()/255.,
              floor_color.b()/255.);
      fprintf(fp, "ceiling_material %5.3f %5.3f %5.3f\n", 
              ceiling_color.r()/255.,
              ceiling_color.g()/255.,
              ceiling_color.b()/255.);
 
    fprintf (fp, "wall_material %f %f %f\n", default_wall_color.r()/255.0, default_wall_color.g()/255.0, default_wall_color.b()/255.0);



     //fprintf(fp, "num_wall_materials %d\n", (int)wall_materials.size());
      /*
      for (unsigned i=0; i<wall_materials.size(); i++){
        fprintf(fp, "wall_material %5.3f %5.3f %5.3f\n", 
                wall_materials[i].r()/255.,
                wall_materials[i].g()/255.,
                wall_materials[i].b()/255.);
      }
      */
    }


    
    int num_arrows = 0;

    // print all object information
    for (unsigned int i = 0; i < objects.size(); i++) {
      if (dynamic_cast<Arrow*>(objects[i]) != NULL) num_arrows++;

      objects[i]->project(camera);
      objects[i]->write(fp);
    }

    std::cout << "FOUND " << objects.size() << " objects" << std::endl;

    if (num_arrows > 1) {
      std::cerr << "ERROR! Found " << num_arrows << " north arrows" << std::endl;
    }

    if (num_arrows == 0) {
      std::cerr << "ERROR! Found no north arrow" << std::endl;
    }
    
    fclose(fp);
}

void Scene::draw(Image<sRGB> &image){
  for (unsigned int i = 0; i < objects.size(); i++) {
    objects[i]->draw(image);
  }
}




void Scene::assign_room_colors() {
  // FLOOR & CEILING
  //ceiling_color = sRGB(255, 255, 255);
  ceiling_color = sRGB(255*0.95,255*0.95,255*0.95);
  //floor_color = sRGB(255, 255, 255);
  floor_color = sRGB(255*0.533, 255*0.431, 255*0.361);
  default_wall_color = sRGB(255, 255, 255);
  for (unsigned int i = 0; i < objects.size(); i++) {
    if (objects[i]->isColorToken()) {
      ColorToken *t = (ColorToken*)objects[i];
      if (t->type == SPECIFIC_WALL_TOKEN_IDX) {
      } else if (t->type == GLOBAL_WALL_TOKEN_IDX) {
	default_wall_color = t->getColor();
      } else if (t->type == FLOOR_TOKEN_IDX || 
		 t->type == FLOOR_TOKEN_IDX2) {
	floor_color = t->getColor();
      } else if (t->type == CEILING_TOKEN_IDX) {
	ceiling_color = t->getColor();
      } else {
	(*args->output) << "UNKNOWN TOKEN TYPE " << t->type << std::endl;
	//assert (0); exit(0);
      }
    }
  }

  // DEFAULT WALL COLOR
  for (unsigned int i = 0; i < objects.size(); i++) {
    if (objects[i]->isWall()) {
      ((Wall*)objects[i])->setColor(default_wall_color);
    }
  }

  // set SPECIFIC colors
  for (unsigned int i = 0; i < objects.size(); i++) {
    if (!objects[i]->isColorToken()) continue;
    ColorToken *t = (ColorToken*)objects[i];
    Wall *best = NULL;
    double best_distance = -1;
    if (t->type == SPECIFIC_WALL_TOKEN_IDX) {
      // loop through the walls
      for (unsigned int j = 0; j < objects.size(); j++) {
	if (!objects[j]->isWall()) continue;
	Wall *tmp = (Wall*)objects[j];	
	double dist = tmp->point_distance(t->getCenter());
	//std::cout << "DIST " << dist << std::endl;
	if (best == NULL || dist < best_distance) {
	  best = tmp;
	  best_distance = dist;
	}
      }
      if (best == NULL) { (*args->output) << "WARNING TRYING TO ASSIGN BEST " << std::endl; }
      else {
	(*args->output) << "SETTING WALL COLOR " << t->getColor() << std::endl;
	best->setColor(t->getColor());
      }
    }
  }
}

