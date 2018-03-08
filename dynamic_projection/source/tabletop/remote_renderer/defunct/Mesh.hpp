#ifndef MESH_HPP_INCLUDED_
#define MESH_HPP_INCLUDED_

#include <GL/gl.h>
#include <vector>
#include <Vector3.h>
#include <util.h>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <string>
#include <cstring>
#include <cassert>
#include "TextureMapMap.hpp"
#include "SocketReader.h"

using namespace std;

class Mesh {
public:
  Mesh(){}
  
  ~Mesh();

private:
  struct Edge;
  struct Vertex;
  struct Texcoord;

public:

  void render(v3d center, std::vector<int> textures);
 
  void loadFromOBJ(SocketReader & sr);

  void setBlendingID(int blending_id){
    this->blending_id = blending_id;
  }
  
  inline void setBlendingBrightness(double b){
    this->projector_brightness = b;
  }
  
  void loadTextureMapMap(SocketReader & sr){
    texture_map_map.load(sr);
  }
  
  //void LoadMultidisplay(SocketReader & sr, string* sides, bool* changed){
  // texture_map_map.load_multidisplay(sr,sides,changed);
  //}
  void loadBleFile(SocketReader & sr);

  void disableBlending();

  void addTexture(char* tex_name, char* file_name){
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
  void loadMultidisplay(SocketReader & sr);
  
  void loadMtlFile(SocketReader & sr, bool volumetric_hack=false);

private:
  Mesh(const Mesh &mesh){} // FIX
  Mesh& operator=(const Mesh &mesh){} // FIX

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
  std::vector<Material*> materials;
  std::vector<Texcoord*> texcoords;
  std::vector<v3d> normals;
  std::unordered_map<EdgeIndex, Edge*, EdgeIndexHasher> edge_map;
  std::unordered_map<std::string, Material*> material_map;
  TextureMapMap texture_map_map;

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
