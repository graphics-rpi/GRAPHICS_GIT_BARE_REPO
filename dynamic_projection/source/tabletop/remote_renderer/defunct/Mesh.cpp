#include "Mesh.hpp"

Mesh::~Mesh(){
  for (unsigned i=0; i<vertices.size(); i++){
    delete vertices[i];
  }
  for (unsigned i=0; i<edges.size(); i++){
    delete edges[i];
  }
  for (unsigned i=0; i<faces.size(); i++){
    delete faces[i];
  }
  for (unsigned i=0; i<materials.size(); i++){
    delete materials[i];
  }
}

void Mesh::render(v3d center, std::vector<int> textures){
  glEnable(GL_DEPTH_TEST);
  
  const double max_dist = 20.;
  glDisable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  //printf("printing texture map map \n");
  //printf("render 1\n");
  //texture_map_map.print();
  for (unsigned i=0; i<faces.size(); i++){
    //printf("render 2\n");    
    if (faces[i]->material->isVisible()){
      int texture_idx=0;
      /*
      if (volume_texture_enable){
	if (faces[i]->material->isHackedLightSource){
	  glDisable(GL_TEXTURE_3D);
	  glDisable(GL_TEXTURE_2D);
	} 
	else {
	  glEnable(GL_TEXTURE_3D);
	  glBindTexture(GL_TEXTURE_3D, volume_texture);
	}
      }
      else {
      */
      //printf("render 3\n");
      texture_idx = getTextureID(faces[i]->material->name.c_str());
      //printf("texture index %i \n", texture_idx);
      // printf("textures.size() %i \n",textures.size());
      //if(texture_idx>=0)
      //  printf("textures at texture_idx %i \n",textures.at(texture_idx));
      if (texture_idx>=0 && texture_idx<textures.size() && 
	  textures.at(texture_idx)!=0)  { 
	//printf("FOUND TEXTURE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %i \n",texture_idx);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures.at(texture_idx));
      } else {
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_3D);
      }
      //} 
      //printf("render 4\n");              	
      glBegin(GL_TRIANGLES);
      for (int j=0;j<3; j++){
	v3d proj_ray = faces[i]->vertices[j]->point - center;
	double distance_factor = proj_ray.dot(proj_ray)/(max_dist*max_dist);
	proj_ray.normalize();
	//printf("render 5\n");
	double angle_factor = 1./std::max(1e-2, faces[i]->normals[j].dot(proj_ray));
	double factor = angle_factor * distance_factor * faces[i]->weights[j]*projector_brightness;
	if (faces[i]->material->isHackedLightSource){
	  factor = 1.;
	}
	
	/*
	if (volume_texture_enable){
	  if (0){
	    double s = 1;
	    glTexCoord3d((faces[i]->vertices[j]->point.x()+s)/(2.*s),
			 (faces[i]->vertices[j]->point.z()+s)/(2.*s),
			 (2.1*s-faces[i]->vertices[j]->point.y()+0)/(2.*s));
	  } 
	  else {
	    double s = 1.4;
	    glTexCoord3d((faces[i]->vertices[j]->point.z()+s)/(2.*s),
			 (2.1*s-faces[i]->vertices[j]->point.y())/(2.*s),
			 (2*faces[i]->vertices[j]->point.x()+s)/(2.*s));
	  }
	  glColor3d(factor, factor, factor);
	} 
	else {
	  */
	//printf("render 6\n");
	if (texture_idx>=0&& texture_idx<textures.size() &&
	    textures.at(texture_idx)!=0){
	  glTexCoord2d(faces[i]->texcoords[j]->point.y(),
		       1-faces[i]->texcoords[j]->point.x());
	  glColor3d(factor, factor, factor);
	} 
	else {
	    /*
	      glColor3d(factor*faces[i]->material->Kd.r(), 
	      factor*faces[i]->material->Kd.g(),
	      factor*faces[i]->material->Kd.b());
	    */
	  glColor3d(0,0,0);
	}
	//}
	
	glVertex3d(faces[i]->vertices[j]->point.x(),
		   faces[i]->vertices[j]->point.y(),
		   faces[i]->vertices[j]->point.z());
	//printf("triangle at %f %f %f \n", faces[i]->vertices[j]->point.x(),
	//           faces[i]->vertices[j]->point.y(),
	//           faces[i]->vertices[j]->point.z());
      }
      glEnd();
    }
  }
  printf("end of render\n");
  
}

void Mesh::loadFromOBJ(SocketReader & sr){
  for (unsigned i=0; i<vertices.size(); i++){
    delete vertices[i];
  }
  for (unsigned i=0; i<edges.size(); i++){
    delete edges[i];
  }
  for (unsigned i=0; i<faces.size(); i++){
    delete faces[i];
  }
  /*
    for (unsigned i=0; i<materials.size(); i++){
    delete materials[i];
    }*/
#ifdef WRITE_FILE
  /*
    char debug_file_name[256];
    sprintf(debug_file_name,"/research/rendering_test_with_sockets/received_%i.obj",rank);
    FILE* fp;
    assert(fp=fopen(debug_file_name,"w"));
  */
#endif
  vertices.clear();
  edges.clear();
  faces.clear();
  normals.clear();
  texcoords.clear();
  edge_map.clear();
  while(!sr.eof()){
    char textline[1024];
    sr.Jgets(textline, 1024);
#ifdef WRITE_FILE
    //fprintf(fp,"%s",textline);
#endif
    char mtlfilename[1024];
    char mtlname[1024];
    Material *current_material;
    v3d p;
    int v1, v2, v3;
    int t1, t2, t3;
    int n1, n2, n3;
    t1 = 1;
    t2 = 1;
    t3 = 1;
    //fprintf(stdout, "-->%s", textline);
    if (!strcmp(textline, "END OBJ_FILE\n")){      
      break;
    } 
    else if (1 == sscanf(textline, "mtllib %1024s", mtlfilename)){
      //ReadMtlFile(mtlfilename);
    } 
    else if (3 == sscanf(textline,
			 "v %lf %lf %lf", &p.x(), &p.y(), &p.z())){
      
      Vertex *vertex = new Vertex;
      vertex->point = p;
      vertices.push_back(vertex);
    } 
    else if (3 == sscanf(textline,
			 "vn %lf %lf %lf", &p.x(), &p.y(), &p.z())){
      normals.push_back(p);
    } 
    else if (2 == sscanf(textline,
			 "vt %lf %lf", &p.x(), &p.y())){      
      p.z() = 0.;
      //p.y()=1-p.y();
      Texcoord *texcoord = new Texcoord;
      texcoord->point = p;
      texcoords.push_back(texcoord);
    } 
    else if (1 == sscanf(textline, "usemtl %1024s ", mtlname)){      
      current_material = LookupMaterial(mtlname);              
    } 
    else if (/*3 == sscanf(textline, "f %d %d %d", &v1, &v2, &v3) || 
	       6 == sscanf(textline, "f %d/%d %d/%d %d/%d", 
	       &v1, &t1, &v2, &t2, &v3, &t3) || */
	     9 == sscanf(textline, "f %d/%d/%d %d/%d/%d %d/%d/%d", 
			 &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3)) {
      v1--;v2--;v3--;
      t1--;t2--;t3--;
      n1--;n2--;n3--;
      faces.push_back(new Face);
      Face *face = faces.back();
      face->material = current_material;
      face->vertices.reserve(3);
      face->vertices.push_back(vertices[v1]);
      face->vertices.push_back(vertices[v2]);
      face->vertices.push_back(vertices[v3]);
      face->weights.resize(3);
      face->texcoords.reserve(3);
      face->texcoords.push_back(texcoords[t1]);
      face->texcoords.push_back(texcoords[t2]);
      face->texcoords.push_back(texcoords[t3]);
      face->normals.push_back(normals[n1]);
      face->normals.push_back(normals[n2]);
      face->normals.push_back(normals[n3]);
      vertices[v1]->faces.push_back(face);
      vertices[v2]->faces.push_back(face);
      vertices[v3]->faces.push_back(face);
      Edge *e1 = LookupEdge(v1, v2);
      Edge *e2 = LookupEdge(v2, v3);
      Edge *e3 = LookupEdge(v3, v1);
      face->edges.reserve(3);
      face->edges.push_back(e1);
      face->edges.push_back(e2);
      face->edges.push_back(e3);
      vertices[v1]->edges.push_back(e1);
      vertices[v1]->edges.push_back(e3);
      vertices[v2]->edges.push_back(e1);
      vertices[v2]->edges.push_back(e2);
      vertices[v3]->edges.push_back(e2);
      vertices[v3]->edges.push_back(e3);
      e1->vertices.push_back(vertices[v1]);
      e1->vertices.push_back(vertices[v2]);
      e2->vertices.push_back(vertices[v2]);
      e2->vertices.push_back(vertices[v3]);
      e3->vertices.push_back(vertices[v3]);
      e3->vertices.push_back(vertices[v1]);
      e1->faces.push_back(face);
      e2->faces.push_back(face);
      e3->faces.push_back(face);
    }
  }
#ifdef WRITE_FILE
  //fclose(fp);
#endif   
  
  // ensure vertex edge list is unique
  for (unsigned i=0; i<vertices.size(); i++){
    std::sort(vertices[i]->edges.begin(), vertices[i]->edges.end());
    std::vector<Edge*>::iterator new_end =
      std::unique(vertices[i]->edges.begin(), vertices[i]->edges.end());
    vertices[i]->edges.erase(new_end, vertices[i]->edges.end()); 
  }
  
  // ensure edge vertex list is unique
  for (unsigned i=0; i<edges.size(); i++){
    std::sort(edges[i]->vertices.begin(), edges[i]->vertices.end());
    std::vector<Vertex*>::iterator new_end =
      std::unique(edges[i]->vertices.begin(), edges[i]->vertices.end());
    edges[i]->vertices.erase(new_end, edges[i]->vertices.end());
  } 
  
  /*
    
  // calculate face normals
  for (unsigned i=0; i<faces.size(); i++){
  v3d p1 = faces[i]->vertices[0]->point - faces[i]->vertices[1]->point; 
  v3d p2 = faces[i]->vertices[2]->point - faces[i]->vertices[1]->point;
  v3d n = p1.cross(p2);
  n.normalize();
  faces[i]->normals[0] = n;
  faces[i]->normals[1] = n;
  faces[i]->normals[2] = n;
  }
  */  
}



void Mesh::loadBleFile(SocketReader & sr){
  //float w[6];
  int idx = -1;
  int vert = 0;
  /*
    char debug_file_name[256];
    sprintf(debug_file_name,"/research/rendering_test_with_sockets/received_%i.ble",blending_id);
    FILE* fp;
    assert(fp=fopen(debug_file_name,"w"));
  */
  
  while (!sr.eof()){
    char textline[1024];
    sr.Jgets(textline, 1024);
    if (!strcmp(textline, "END BLE_FILE\n")){  
      break;
    } 
    else {
      
      //fprintf(fp, "%s", textline);
      
      std::string line(textline);
      std::istringstream ss(line);
      std::string token;
      ss >> token;
      
      if(token == "f") { // begin a new face
	++idx;
	vert = 0;
      } 
      else if(token == "") {
	continue;
      } 
      else { // weights
	assert(idx<faces.size());
	assert(vert<faces.at(idx)->weights.size());
	double& weight= faces.at(idx)->weights.at(vert);
	if(blending_id == 0) {
	  weight = atof(token.c_str());
	} 
	else {
	  // skip until to the weights
	  for(int i=1; i<=blending_id; ++i) {
	    ss >> weight;
	  }
	}
	// increase to the next vertex
	++vert;
      }
    }
  }
}

void Mesh::disableBlending(){
  for (std::vector<Face*>::iterator it=faces.begin();
       it!=faces.end(); ++it){
    std::vector<double>& weights = (*it)->weights;
    for(std::vector<double>::iterator it2=weights.begin();
	it2!=weights.end(); ++it2)
      *it2 = 1.;
  }
}


// Input a socket and get a texture from it
void Mesh::loadMultidisplay(SocketReader & sr){
  //cout << "LOADING MULTIDISPLAY!" << endl;
  int val;
  while(!sr.eof()){
    char textline[1024];
    char *material_name;
    char *texture_file;
    sr.Jgets(textline, 1024);
    if (!strcmp(textline, "END MULTIDISPLAY\n")){
      break;
    } 
    else if (material_name = strtok(textline, " ")){
      //cout << "material_name = strtok(textline) " << endl;
      if (texture_file = strtok(NULL, " \t\n")){	
	addTexture( material_name, texture_file );       
      } // if
    } // else if
  } // while
} // load_multidisplay


void Mesh::loadMtlFile(SocketReader & sr, bool volumetric_hack){
  //  void ReadMtlFile(const char *filename){
  //    FILE *fp = fopen(filename, "rt");
  // if (NULL == fp){
  //  FATAL_ERROR("unable to open %s\n", filename);      
  // }
  for (unsigned i=0; i<materials.size(); i++){
    delete materials[i];
  }
  
  materials.clear();
  material_map.clear();
  char textline[1024];
  char mtlname[1024];
  char texfile[1024];
  Material *current_material;
  double r, g, b;
  int d;
  while (!sr.eof()){
    sr.Jgets(textline, 1024);
    if (!strcmp(textline, "END MTL_FILE\n")){  
      break;
    } else if (1 == sscanf(textline, "newmtl %1024s ", mtlname)){
      current_material = new Material;
      current_material->name = mtlname;
      if (volumetric_hack && (
			      "left_l_wall_0" == current_material->name ||
			      "left_l_wall_1" == current_material->name ||
			      "left_l_wall_3" == current_material->name ||
			      "left_l_wall_4" == current_material->name ||
			      "right_l_wall_0" == current_material->name ||
			      "right_l_wall_1" == current_material->name ||
			      "right_l_wall_3" == current_material->name ||
			      "right_l_wall_4" == current_material->name)) {
	current_material->isHackedLightSource = true;	
      } 
      else {
	current_material->isHackedLightSource = false;
      }
      
      materials.push_back(current_material);
      material_map[mtlname] = current_material;
    } 
    else if (3 == sscanf(textline, "Ka %lf %lf %lf", &r, &g, &b)){
      current_material->Ka = v3d(r, g, b);
    } 
    else if (3 == sscanf(textline, "Kd %lf %lf %lf", &r, &g, &b)){
      current_material->Kd = v3d(r, g, b);
    } 
    else if (3 == sscanf(textline, "Pd %lf %lf %lf", &r, &g, &b)){
      current_material->Pd = v3d(r, g, b);
    } 
    else if (3 == sscanf(textline, "Ks %lf %lf %lf", &r, &g, &b)){
      current_material->Ks = v3d(r, g, b);
    } 
    else if (3 == sscanf(textline, "Ke %lf %lf %lf", &r, &g, &b)){
      current_material->Ke = v3d(r, g, b);
    } 
    else if (1 == sscanf(textline, "map_Ka  %s", texfile)){
      if(texture_map_map.getTextureID(mtlname)<0){               
	texture_map_map.addTexture(mtlname, texfile);               
      }
    } 
    else if (1 == sscanf(textline, "d %d", &d)){
      current_material->d = d;
    }    
  }
  if(sr.eof()){    
    sr.println();
  }
  else{    
    sr.println();
  }  
}
