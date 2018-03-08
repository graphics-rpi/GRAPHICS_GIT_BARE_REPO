#ifndef MESH_HPP_INCLUDED_
#define MESH_HPP_INCLUDED_

#include <GL/gl.h>
#include <GL/glu.h>
#include <vector>
#include <Vector3.h>
#include <util.h>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <string>
#include <cstring>
#include <cassert>
#include "RRTextureMapMap.hpp"

#include <fstream>
#include <string>

using namespace std;

class Mesh {
public:
  Mesh(){}
  
  /*Mesh(const char *filename){
    //FILE *fp = fopen(filename, "rt");
    //if (NULL == fp){
      FATAL_ERROR("unable to open %s\n", filename);
    //}
    //Load(fp);
    //fclose(fp);
  }*/

  /*Mesh(){
    //Load(fp);
  }*/

  ~Mesh(){
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
    for (unsigned i=0; i<lines.size(); i++){
      delete lines[i];
    }
  }

private:
  struct Edge;
  struct Vertex;
  struct Texcoord;
  struct Line;
public:

  void Render(v3d center, std::vector<int> textures, 
	      int volume_texture, bool volume_texture_enable){

    glEnable(GL_DEPTH_TEST);

    const double max_dist = 20.;
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT,GL_FILL);
    //printf("printing texture map map \n");
    //texture_map_map.print();
    for (unsigned i=0; i<faces.size(); i++)
    {

      if (faces[i]->material->isVisible()){

     	  int texture_idx=0;
	      if (volume_texture_enable)
        {
          if (faces[i]->material->isHackedLightSource)
          {
             glDisable(GL_TEXTURE_3D);
	           glDisable(GL_TEXTURE_2D);
          } 
          else {
                glEnable(GL_TEXTURE_3D);
                glBindTexture(GL_TEXTURE_3D, volume_texture);
        }

	    } 
      else {
	      texture_idx = getTextureID(faces[i]->material->name.c_str());

	      if (texture_idx>=0 && texture_idx<textures.size() && 
	          textures.at(texture_idx)!=0)  
        { 
       
	          glEnable(GL_TEXTURE_2D);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	          glBindTexture(GL_TEXTURE_2D, textures.at(texture_idx));
	      } else {
    
          //sleep(1);
	        glDisable(GL_TEXTURE_2D);
	        glDisable(GL_TEXTURE_3D);
	      }
	    } 
              	
        glBegin(GL_TRIANGLES);
        for (int j=0;j<3; j++){
          v3d proj_ray = faces[i]->vertices[j]->point - center;
          double distance_factor = proj_ray.dot(proj_ray)/(max_dist*max_dist);
          proj_ray.normalize();

          double angle_factor = 1./std::max(1e-2, faces[i]->normals[j].dot(proj_ray));
          double factor = angle_factor * distance_factor * faces[i]->weights[j]*projector_brightness;
          if (faces[i]->material->isHackedLightSource){
            factor = 1.;
          }
	  
	        /*if (volume_texture_enable){
	          if (0){
	            double s = 1;
	            glTexCoord3d((faces[i]->vertices[j]->point.x()+s)/(2.*s),
			         (faces[i]->vertices[j]->point.z()+s)/(2.*s),
			         (2.1*s-faces[i]->vertices[j]->point.y()+0)/(2.*s));
	          } else {
	            double s = 1.4;
	            glTexCoord3d((faces[i]->vertices[j]->point.z()+s)/(2.*s),
			         (2.1*s-faces[i]->vertices[j]->point.y())/(2.*s),
			         (2*faces[i]->vertices[j]->point.x()+s)/(2.*s));
	          }
	          glColor3d(factor, factor, factor);
	        } else {*/
	          if (texture_idx>=0&& texture_idx<textures.size() &&
		            textures.at(texture_idx)!=0)
            {
	            glTexCoord2d(faces[i]->texcoords[j]->point.y(),
	            	   1-faces[i]->texcoords[j]->point.x());
	            glColor3d(factor, factor, factor);
	          }//endif 
            else 
            {	            
	            glColor3d(0,0,0);
	          }//end else
	        //}

          glVertex3d(faces[i]->vertices[j]->point.x(),
                     faces[i]->vertices[j]->point.y(),
                     faces[i]->vertices[j]->point.z());

        }
        glEnd();
      }
    }

	  //glDisable(GL_TEXTURE_2D);
    glDisable( GL_LIGHTING );
    glEnable (GL_LINE_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glLineWidth (2.0);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glColor3f(1,1,1);


    int size=lines.size();
    static bool first=true;
    first=true;

    
    
    for(int i=0; i<size; i++)
    { 
        
      assert(i<lines.size());
      Line* line=lines[i];  
      Material* material=line->material;
      int texture_idx = getTextureID(material->name.c_str());
      glColor3d(material->Kd.r(),material->Kd.g(),material->Kd.b());


      if (!first||(texture_idx>=0 && texture_idx<textures.size() && 
	          textures.at(texture_idx)!=0) )
      {
        
        if(first)
        {
          glEnable(GL_TEXTURE_2D);
          glBindTexture(GL_TEXTURE_2D, textures.at(texture_idx));
      
          glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        }

        v3d point1=vertices[line->v1-1]->point;
        v3d point2=vertices[line->v2-1]->point;
        glBegin(GL_LINES);
            glTexCoord2d(1,1);
            glVertex3d(point1.x(),
                       point1.y(),
                       point1.z());
            glTexCoord2d(0,0);

            glVertex3d(point2.x(),
                       point2.y(),
                       point2.z());
         glEnd();
      }
      else
        assert(0&&"Problem with line texutre\n");
      first=false;
    }
    //glDisable(GL_TEXTURE_2D);
    glColor3d(0,0,0);
    //texture_map_map.print();

  }

  void Load(SocketReader & sr){
    for (unsigned i=0; i<vertices.size(); i++){
      delete vertices[i];
    }
    for (unsigned i=0; i<edges.size(); i++){
      delete edges[i];
    }
    for (unsigned i=0; i<faces.size(); i++){
      delete faces[i];
    }
    for (unsigned i=0; i<lines.size(); i++){
      delete lines[i];
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
    lines.clear();
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
      } else if (1 == sscanf(textline, "mtllib %1024s", mtlfilename)){
        //ReadMtlFile(mtlfilename);
      } else if (3 == sscanf(textline,
                             "v %lf %lf %lf", &p.x(), &p.y(), &p.z())){
        
        Vertex *vertex = new Vertex;
        vertex->point = p;
        vertices.push_back(vertex);
      } else if (3 == sscanf(textline,
                             "vn %lf %lf %lf", &p.x(), &p.y(), &p.z())){
        normals.push_back(p);
      } else if (2 == sscanf(textline,
                             "vt %lf %lf", &p.x(), &p.y())){
        
        p.z() = 0.;
        //p.y()=1-p.y();
        Texcoord *texcoord = new Texcoord;
        texcoord->point = p;
        texcoords.push_back(texcoord);
      } else if (1 == sscanf(textline, "usemtl %1024s ", mtlname)){
       
        //if(strcmp(mtlname,"colored_line\n"))
          current_material = LookupMaterial(mtlname);
        //else
        //  ;
       
        
      } 
        else if (2 == sscanf(textline,
                             "l %d %d", &v1, &v2)){
        
        Line *line = new Line;
        line->v1 = v1;
        line->v2 = v2;
        line->material=current_material;
        lines.push_back(line);
      }else if (/*3 == sscanf(textline, "f %d %d %d", &v1, &v2, &v3) || 
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


void Load(const char* filename){
    for (unsigned i=0; i<vertices.size(); i++){
      delete vertices[i];
    }
    for (unsigned i=0; i<edges.size(); i++){
      delete edges[i];
    }
    for (unsigned i=0; i<faces.size(); i++){
      delete faces[i];
    }
    for (unsigned i=0; i<lines.size(); i++){
      delete lines[i];
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
    lines.clear();
    normals.clear();
    texcoords.clear();
    edge_map.clear();
    /*
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      printf("unable to open %s\n", filename);
      assert(0);
    }
    */

    // new stuff
    std::ifstream sepplesfile(filename);

    while(!sepplesfile.eof()){
      //char textline[1024];
      //sr.Jgets(textline, 1024);
      //fread(textline, sizeof(char), 1024, fp);
      std::string line;
      getline(sepplesfile, line);
      const char* textline = line.c_str();
	
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
      } else if (1 == sscanf(textline, "mtllib %1024s", mtlfilename)){
        //ReadMtlFile(mtlfilename);
      } else if (3 == sscanf(textline,
                             "v %lf %lf %lf", &p.x(), &p.y(), &p.z())){
        
        Vertex *vertex = new Vertex;
        vertex->point = p;
        vertices.push_back(vertex);
      } else if (3 == sscanf(textline,
                             "vn %lf %lf %lf", &p.x(), &p.y(), &p.z())){
        normals.push_back(p);
      } else if (2 == sscanf(textline,
                             "vt %lf %lf", &p.x(), &p.y())){
        
        p.z() = 0.;
        //p.y()=1-p.y();
        Texcoord *texcoord = new Texcoord;
        texcoord->point = p;
        texcoords.push_back(texcoord);
      } else if (1 == sscanf(textline, "usemtl %1024s ", mtlname)){
       
        //if(strcmp(mtlname,"colored_line\n"))
          current_material = LookupMaterial(mtlname);
        //else
        //  ;
       
        
      } 
        else if (2 == sscanf(textline,
                             "l %d %d", &v1, &v2)){
        
        Line *line = new Line;
        line->v1 = v1;
        line->v2 = v2;
        line->material=current_material;
        lines.push_back(line);
      }else if (/*3 == sscanf(textline, "f %d %d %d", &v1, &v2, &v3) || 
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

    //fclose(fp);

  }

  void setBlendingID(int blending_id){
    this->blending_id = blending_id;
  }
  
  inline void setBlendingBrightness(double b)
  {
    this->projector_brightness = b;
  }

  void loadTextureMapMap(SocketReader & sr){
    texture_map_map.load(sr);
  }

  void LoadPuzzleFile(SocketReader & sr, string* sides, bool* changed){
    texture_map_map.load_puzzle(sr,sides,changed);
  }


 //void LoadMultidisplay(SocketReader & sr, string* sides, bool* changed){
   // texture_map_map.load_multidisplay(sr,sides,changed);
  //}

  void loadBleFile(SocketReader & sr){
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
      } else {

	//fprintf(fp, "%s", textline);
	
	std::string line(textline);
	std::istringstream ss(line);
	std::string token;
	ss >> token;
  static bool isAFace=false;
	if(token == "f") { // begin a new face
    isAFace=true;
	  ++idx;
	  vert = 0;
	} else if(token == "") 
  {
	  continue;
  }
  else if(token == "l")
  {
    isAFace=false;
	} else { // weights
//if(!(idx<faces.size()))
//    printf("faces size %d %d\n", faces.size(), idx);
    if(isAFace)
    {
	    assert(idx<faces.size());
	    assert(vert<faces.at(idx)->weights.size());
	    double& weight= faces.at(idx)->weights.at(vert);
	    if(blending_id == 0) {
	      weight = atof(token.c_str());
	    } else {
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

  }

void loadBleFile(const char* filename){
    //float w[6];
    int idx = -1;
    int vert = 0;
    /*
    char debug_file_name[256];
    sprintf(debug_file_name,"/research/rendering_test_with_sockets/received_%i.ble",blending_id);
    FILE* fp;
    assert(fp=fopen(debug_file_name,"w"));
    */

    /*
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      printf("unable to open %s\n", filename);
      assert(0);
    }
    */

    // new stuff
    std::ifstream sepplesfile(filename);

    while(!sepplesfile.eof()){
      //char textline[1024];
      //sr.Jgets(textline, 1024);
      //fread(textline, sizeof(char), 1024, fp);
      std::string line;
      getline(sepplesfile, line);
      const char* textline = line.c_str();

      if (!strcmp(textline, "END BLE_FILE\n")){  
        break;
      } else {

	//fprintf(fp, "%s", textline);
	
	std::string line(textline);
	std::istringstream ss(line);
	std::string token;
	ss >> token;
  static bool isAFace=false;
	if(token == "f") { // begin a new face
    isAFace=true;
	  ++idx;
	  vert = 0;
	} else if(token == "") 
  {
	  continue;
  }
  else if(token == "l")
  {
    isAFace=false;
	} else { // weights
//if(!(idx<faces.size()))
//    printf("faces size %d %d\n", faces.size(), idx);
    if(isAFace)
    {
	    assert(idx<faces.size());
	    assert(vert<faces.at(idx)->weights.size());
	    double& weight= faces.at(idx)->weights.at(vert);
	    if(blending_id == 0) {
	      weight = atof(token.c_str());
	    } else {
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

    //fclose(fp);
    
  }

  void disableBlending(){
    for (std::vector<Face*>::iterator it=faces.begin();
	 it!=faces.end(); ++it){
      std::vector<double>& weights = (*it)->weights;
      for(std::vector<double>::iterator it2=weights.begin();
	  it2!=weights.end(); ++it2)
	*it2 = 1.;
    }
  }

  void addTexture(char* tex_name, char* file_name)
  {
    //      texture_filenames.push_back(file_name);
    //    material_map[tex_name] = texture_filenames.size()-1;
    //   printf("adding texture %s filename %s size %i \n", tex_name, file_name, texture_filenames.size());
 //     int index=0;
 //     if( material_map[tex_name] == NULL ){
 //        material_map[tex_name] = file_name;
 //      }


    texture_map_map.addTexture( tex_name, file_name );
  }

  // Input a socket and get a texture from it
  void LoadMultidisplay(SocketReader & sr, string* sides, bool* texture_changed){
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
      else if (material_name = strtok(textline, " "))
      {
//cout << "material_name = strtok(textline) " << endl;
        if (texture_file = strtok(NULL, " \t\n"))
        {
          //if( find(texture_filenames.begin(), texture_filenames.end(), texture_file ) == texture_filenames.end() )
          {
            //texture_filenames.push_back(texture_file);
            //printf("Inside load_multidisplay adding %s %s \n", material_name, texture_file);
           // material_map[material_name] = texture_filenames.size()-1;
            //val=getTextureID(material_name);
            addTexture( material_name, texture_file );
            //printf( "%s added\n", texture_file );
	        } // if
        } // if
      } // else if
    } // while
  } // load_multidisplay

  // new file version
  void LoadMultidisplay(const char* filename, string* sides, bool* texture_changed){
    //cout << "LOADING MULTIDISPLAY!" << endl;
    int val;
    /*
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      printf("unable to open %s\n", filename);
      assert(0);
    }
    */
    // new stuff
    std::ifstream sepplesfile(filename);
    while(!sepplesfile.eof()){
      //char textline[1024];
      char *material_name;
      char *texture_file;
      //sr.Jgets(textline, 1024);
      //fread(textline, sizeof(char), 1024, fp);
      std::string line;
      getline(sepplesfile, line);
      char* textline = (char*)line.c_str();
      
      if (!strcmp(textline, "END MULTIDISPLAY\n")){
        break;
      } 
      else if (material_name = strtok(textline, " "))
      {
//cout << "material_name = strtok(textline) " << endl;
        if (texture_file = strtok(NULL, " \t\n"))
        {
          //if( find(texture_filenames.begin(), texture_filenames.end(), texture_file ) == texture_filenames.end() )
          {
            //texture_filenames.push_back(texture_file);
            //printf("Inside load_multidisplay adding %s %s \n", material_name, texture_file);
           // material_map[material_name] = texture_filenames.size()-1;
            //val=getTextureID(material_name);
            addTexture( material_name, texture_file );
            //printf( "%s added\n", texture_file );
	        } // if
        } // if
      } // else if
    } // while
    //fclose(fp);
  } // load_multidisplay



  void loadMtlFile(SocketReader & sr, bool volumetric_hack=false){
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
        } else {
          current_material->isHackedLightSource = false;
        }
            
        materials.push_back(current_material);
        material_map[mtlname] = current_material;
      } else if (3 == sscanf(textline, "Ka %lf %lf %lf", &r, &g, &b)){
        current_material->Ka = v3d(r, g, b);
      } else if (3 == sscanf(textline, "Kd %lf %lf %lf", &r, &g, &b)){
        current_material->Kd = v3d(r, g, b);
      } else if (3 == sscanf(textline, "Pd %lf %lf %lf", &r, &g, &b)){
        current_material->Pd = v3d(r, g, b);
      } else if (3 == sscanf(textline, "Ks %lf %lf %lf", &r, &g, &b)){
        current_material->Ks = v3d(r, g, b);
      } else if (3 == sscanf(textline, "Ke %lf %lf %lf", &r, &g, &b)){
        current_material->Ke = v3d(r, g, b);
      } else if (1 == sscanf(textline, "map_Ka  %s", texfile)){
          if(texture_map_map.getTextureID(mtlname)<0){
	        //printf("************************ adding texture %s %s\r\n", mtlname, texfile);
        
	          texture_map_map.addTexture(mtlname, texfile);               
          }
      } else if (1 == sscanf(textline, "d %d", &d)){
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



  void loadMtlFileFromFile(const char *filename, bool volumetric_hack=false){
    /*
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      printf("unable to open %s\n because errno: %d\n", filename, errno);
      assert(0);
    }
    */

    // new stuff
    std::ifstream sepplesfile(filename);


    for (unsigned i=0; i<materials.size(); i++){
      delete materials[i];
    }
    
    materials.clear();
    material_map.clear();
    //char textline[1024];
    char mtlname[1024];
    char texfile[1024];
    Material *current_material;
    double r, g, b;
    int d;


    while(!sepplesfile.eof()){
      std::string line;
      getline(sepplesfile, line);
      const char* textline = line.c_str();
      //fread(textline, sizeof(char), 1024, fp);
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
        } else {
          current_material->isHackedLightSource = false;
        }
            
        materials.push_back(current_material);
        material_map[mtlname] = current_material;
      } else if (3 == sscanf(textline, "Ka %lf %lf %lf", &r, &g, &b)){
        current_material->Ka = v3d(r, g, b);
      } else if (3 == sscanf(textline, "Kd %lf %lf %lf", &r, &g, &b)){
        current_material->Kd = v3d(r, g, b);
      } else if (3 == sscanf(textline, "Pd %lf %lf %lf", &r, &g, &b)){
        current_material->Pd = v3d(r, g, b);
      } else if (3 == sscanf(textline, "Ks %lf %lf %lf", &r, &g, &b)){
        current_material->Ks = v3d(r, g, b);
      } else if (3 == sscanf(textline, "Ke %lf %lf %lf", &r, &g, &b)){
        current_material->Ke = v3d(r, g, b);
      } else if (1 == sscanf(textline, "map_Ka  %s", texfile)){
          if(texture_map_map.getTextureID(mtlname)<0){
      	    //printf("************************ adding texture %s %s\r\n", mtlname, texfile);
                texture_map_map.addTexture(mtlname, texfile);               
          }
      } else if (1 == sscanf(textline, "d %d", &d)){
        current_material->d = d;
      }
    
   }

    //fclose(fp);

  }

  RRTextureMapMap& getRRTextureMapMap()
  {
    return texture_map_map;
  }

private:
  Mesh(const Mesh &mesh){}
  Mesh& operator=(const Mesh &mesh){}

  struct Material {
    std::string name;
    v3d Ka;
    v3d Kd;
    v3d Pd;
    v3d Ks;
    v3d Ke;
    int d;
    bool isHackedLightSource;
    bool isVisible() {
      return true;
      if (std::string::npos == name.find("DIFFUSE") &&
          std::string::npos == name.find("FILLIN") &&
          std::string::npos == name.find("GLASS") &&
          //          std::string::npos == name.find("EXTRA") &&
          std::string::npos == name.find("SKYLIGHT")){
        return true;
      } else {
        return false;
      }
    }
  };

  struct Face;
  struct Edge;
  struct Vertex {
    v3d point;
    v3d normal;
    std::vector<Face *> faces;
    std::vector<Edge *> edges;
    Vertex() {}
  };

  struct Line
  {
    int v1,v2;
    Material *material;
  };

  struct Texcoord {
    v3d point;
  };

  struct Face;
  struct Edge {
    std::vector<Vertex*> vertices;
    std::vector<Face*> faces;
    

  private:
    int signum(double x){
      const double eps = 1e-4;
      if (x > eps) return 1;
      if (x < -eps) return -1;
      return 0;
    }
  };

  struct Face {
    std::vector<Vertex*> vertices;
    std::vector<Texcoord*> texcoords;
    std::vector<Edge*> edges;
    std::vector<double> weights; // blending weights
    std::vector<v3d> normals;
    Material *material;
    //v3d normal;
  };

  struct EdgeIndex {
    int v1, v2;
    EdgeIndex(int v1, int v2): v1(v1), v2(v2) {}
    bool operator==(const EdgeIndex &a) const {
      return a.v1 == v1 && a.v2 == v2;
    }
  };
  
  struct EdgeIndexHasher {
    int operator() (const EdgeIndex& e) const{
      std::hash<int> int_hasher;
      return int_hasher(e.v1) ^ int_hasher(e.v2);
    }    
  };
  
  int blending_id;
  double projector_brightness;
  std::vector<Vertex*> vertices;
  std::vector<Edge*> edges;
  std::vector<Face*> faces;
  std::vector<Line*> lines;
  std::vector<Material*> materials;
  std::vector<Texcoord*> texcoords;
  std::vector<v3d> normals;
  std::unordered_map<EdgeIndex, Edge*, EdgeIndexHasher> edge_map;
  std::unordered_map<std::string, Material*> material_map;
  RRTextureMapMap texture_map_map;

  int getTextureID(const char *name){
    return texture_map_map.getTextureID(name);
  }
  


  Material *LookupMaterial(const char *name){
    Material *material;
    if(material_map.find(name)!=material_map.end()){
      material = material_map[name];
       return(material);
    }
    else{
     return 0;
     // printf("looking up %s and not finding\n", name);
      //std::unordered_map<std::string, Material*>::const_iterator it = material_map.begin();
     // printf("material map size %i\n",material_map.size());
     // for( ; it != material_map.end(); it++)
     //   printf("material: %s \n", it->first.c_str());
    }
    //if(strcmp(name,"EXTRA_wall_top"))
   //   assert(0);
    //  assert(material);   
  }

  Edge *LookupEdge(int v1, int v2){
    if (v1 > v2){
      int temp = v1;
      v1 = v2;
      v2 = temp;
    }
    Edge *edge = edge_map[EdgeIndex(v1, v2)];
    if (NULL == edge){
      edges.push_back(new Edge);
      edge = edges.back(); 
      edge_map[EdgeIndex(v1, v2)] = edge;
      edge->vertices.push_back(vertices[v1]);
      edge->vertices.push_back(vertices[v2]);
    }
    
    return edge;
  }
      
};
#endif // #ifndef MESH_HPP_INCLUDED_
