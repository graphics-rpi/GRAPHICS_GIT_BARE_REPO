#ifndef MESH_HPP_INCLUDED_
#define MESH_HPP_INCLUDED_

#include <GL/gl.h>
#include <vector>
#include <Vector3.h>
#include <util.h>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <cassert>


class Mesh {
public:
  Mesh(){
    default_material = new Material;
    default_material->color = v3d(1., 1., 1.);
    default_material->visible = true;
    default_material->name = std::string("default");
    default_material->type = std::string("default");
    materials.push_back(default_material);
  }
  
  Mesh(const char *filename){
    Load(filename);
  }

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
  }

private:
  struct Edge;
  struct Vertex;

public:
  Edge *getNextSilhouette(Edge *e, Vertex *v, const v3d &view){
    for (unsigned i=0; i<v->edges.size(); i++){
      if (v->edges[i]->isSilhouette(view) && v->edges[i] != e){
        return v->edges[i];
      }
    }
    return NULL;
  }

  void LoadBleFile(const char *filename, int idx){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      FATAL_ERROR("Unable to open %s\n", filename);
    }

    int line_no = 0;
    for (unsigned i=0; i<faces.size(); i++){
      char textline[1024];
      fgets(textline, 1024, fp);
      line_no++;
      if (textline[0] != 'f'){
        fprintf(stderr, "Error parsing ble file: line %d\n", line_no);
        exit(-1);
      }
      for (unsigned j=0; j<faces[i]->vertices.size(); j++){
        double w[4];
        fscanf(fp, "%lf %lf %lf %lf\n", w, w+1, w+2, w+3);
        line_no++;
        faces[i]->weights.push_back(w[idx]);
      }
    }
    
    fclose(fp);
  }

  void BlendingFins(const v3d &view){
    for (unsigned i=0; i<edges.size(); i++){
      if (edges[i]->isSilhouette(view)){
        Edge *e1  = getNextSilhouette(edges[i], edges[i]->vertices[0], view);
        Edge *e2  = getNextSilhouette(edges[i], edges[i]->vertices[1], view);
        if (e1 && e2){
          
        }
      }
    }
  }

  void Render(){
    glBegin(GL_TRIANGLES);
    for (unsigned i=0; i<faces.size(); i++){
      if (faces[i]->material->visible){
	glColor3d(faces[i]->material->color.r(), 
		  faces[i]->material->color.g(),
		  faces[i]->material->color.b());
	for (int j=0;j<3; j++){
	  glVertex3d(faces[i]->vertices[j]->point.x(),
		     faces[i]->vertices[j]->point.y(),
		     faces[i]->vertices[j]->point.z());
	}
      }
    }
    glEnd();
  }

  void RenderWeighted(){
    glBegin(GL_TRIANGLES);
    for (unsigned i=0; i<faces.size(); i++){
      if (faces[i]->material->visible){
	for (int j=0;j<3; j++){
          glColor3d(faces[i]->material->color.r() * faces[i]->weights[j], 
                    faces[i]->material->color.g() * faces[i]->weights[j],
                    faces[i]->material->color.b() * faces[i]->weights[j]);
	  glVertex3d(faces[i]->vertices[j]->point.x(),
		     faces[i]->vertices[j]->point.y(),
		     faces[i]->vertices[j]->point.z());
	}
      }
    }
    glEnd();
  }


  void RenderEdges(){
    glBegin(GL_LINES);
    for (unsigned i=0; i<edges.size(); i++){
      //      if (edges[i]->isSilhouette(v3d(0.,1.,0.))){
        glVertex3d(edges[i]->vertices[0]->point.x(),
                   edges[i]->vertices[0]->point.y(),
                   edges[i]->vertices[0]->point.z());
        glVertex3d(edges[i]->vertices[1]->point.x(),
                   edges[i]->vertices[1]->point.y(),
                   edges[i]->vertices[1]->point.z());
        //      }
    }
    glEnd();
  }

  struct Material {
    v3d color;
    bool visible;
    std::string name;
    std::string type;
    std::vector<std::string> flags;
    v3d Dr;
    v3d Sr;
    v3d Dt;
    v3d St;
    v3d Pd;
  };

  void ColorMaterials(void (*colorer)(Material &m)){
    fprintf(stderr, "%d\n", materials.size());
    return;
    for (unsigned i=0; i<materials.size(); i++){
      colorer(*materials[i]);
    }
  }

  void Load(const char *filename){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      FATAL_ERROR("unable to open %s\n", filename);
    }
    while(!feof(fp)){
      char textline[1024];
      fgets(textline, 1024, fp);
      char mtlfilename[1024];
      char mtlname[1024];
      Material *current_material;
      v3d p;
      int v1, v2, v3;
      if (1 == sscanf(textline, "mtllib %1024s", mtlfilename)){
        ReadMtlFile(mtlfilename);
      } else if (3 == sscanf(textline,
                             "v %lf %lf %lf", &p.x(), &p.y(), &p.z())){
        Vertex *vertex = new Vertex;
        vertex->point = p;
        vertices.push_back(vertex);
      } else if (1 == sscanf(textline, "usemtl %1024s ", mtlname)){
        current_material = LookupMaterial(mtlname);
      } else if (3 == sscanf(textline, "f %d %d %d", &v1, &v2, &v3)){
        v1--;v2--;v3--;
        faces.push_back(new Face);
        Face *face = faces.back();
        face->material = current_material;
        face->vertices.push_back(vertices[v1]);
        face->vertices.push_back(vertices[v2]);
        face->vertices.push_back(vertices[v3]);
        vertices[v1]->faces.push_back(face);
        vertices[v2]->faces.push_back(face);
        vertices[v3]->faces.push_back(face);
        Edge *e1 = LookupEdge(v1, v2);
        Edge *e2 = LookupEdge(v2, v3);
        Edge *e3 = LookupEdge(v3, v1);
        face->edges.push_back(e1);
        face->edges.push_back(e2);
        face->edges.push_back(e3);
        vertices[v1]->edges.push_back(e1);
        vertices[v1]->edges.push_back(e3);
        vertices[v2]->edges.push_back(e1);
        vertices[v2]->edges.push_back(e2);
        vertices[v3]->edges.push_back(e2);
        vertices[v3]->edges.push_back(e3);
        e1->faces.push_back(face);
        e2->faces.push_back(face);
        e3->faces.push_back(face);
      }
    }
    fclose(fp);

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

    // calculate face normals
    for (unsigned i=0; i<faces.size(); i++){
      v3d p1 = faces[i]->vertices[0]->point - faces[i]->vertices[1]->point; 
      v3d p2 = faces[i]->vertices[2]->point - faces[i]->vertices[1]->point;
      v3d n = p1.cross(p2);
      n.normalize();
      faces[i]->normal = n;
    }

  }

  // add a new vertex; returns the vertex index
  unsigned addVertex(v3d p){
    Vertex *vertex = new Vertex;
    vertex->point = p;
    vertices.push_back(vertex);
    return vertices.size() - 1;
  }

  unsigned addTriangle(unsigned v1, unsigned v2, unsigned v3){
    Face *face = new Face;
    faces.push_back(face);

    Edge *e1 = LookupEdge(v1, v2);
    Edge *e2 = LookupEdge(v2, v3);
    Edge *e3 = LookupEdge(v3, v1);

    face->material = default_material;

    face->vertices.push_back(vertices[v1]);
    face->vertices.push_back(vertices[v2]);
    face->vertices.push_back(vertices[v3]);

    face->edges.push_back(e1);
    face->edges.push_back(e2);
    face->edges.push_back(e3);

    vertices[v1]->faces.push_back(face);
    vertices[v2]->faces.push_back(face);
    vertices[v3]->faces.push_back(face);

    e1->faces.push_back(face);
    e2->faces.push_back(face);
    e3->faces.push_back(face);

    v3d p1 = face->vertices[0]->point - face->vertices[1]->point; 
    v3d p2 = face->vertices[2]->point - face->vertices[1]->point;
    v3d n = p1.cross(p2);
    n.normalize();
    face->normal = n;
  }

private:
  Mesh(const Mesh &mesh){}
  Mesh& operator=(const Mesh &mesh){}
  Material *default_material;

  struct Face;
  struct Edge;
  struct Vertex {
    v3d point;
    v3d normal;
    std::vector<Face *> faces;
    std::vector<Edge *> edges;
  };

  struct Face;
  struct Edge {
    std::vector<Vertex*> vertices;
    std::vector<Face*> faces;
    
    bool isSilhouette(const v3d &view){
      if (faces.size() < 2){
	if (faces[0]->material->visible){
	  return true;
	} else {
	  return false;
	}
      } else {
	//        if (signum(view.dot(faces[0]->normal)) == 
        //    signum(view.dot(faces[1]->normal))){
	if (!faces[0]->material->visible || !faces[1]->material->visible) {
	  return false;
	}
        if (faces[0]->normal.dot(faces[1]->normal) > 0.99){
          return false;
        } else {
          return true;
        }
      }
    }

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
    std::vector<Edge*> edges;
    Material *material;
    v3d normal;
    std::vector<double> weights; // weight at each vertex
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
  
  std::vector<Vertex*> vertices;
  std::vector<Edge*> edges;
  std::vector<Face*> faces;
  std::vector<Material*> materials;
  std::unordered_map<EdgeIndex, Edge*, EdgeIndexHasher> edge_map;
  std::unordered_map<std::string, Material*> material_map;

  Material *LookupMaterial(const char *name){
    Material *material = material_map[name];
    assert(material);
    return(material);
  }

  void ReadMtlFile(const char *filename){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      FATAL_ERROR("unable to open %s\n", filename);      
    }
    char textline[1024];
    char mtlname[1024];
    Material *current_material;
    double r, g, b;
    int d;
    while (!feof(fp)){
      fgets(textline, 1024, fp);
      if (1 == sscanf(textline, "newmtl %1024s ", mtlname)){
        current_material = new Material;
        current_material->name = mtlname;
        current_material->color = v3d(0.5, 0.5, 0.5);
        materials.push_back(current_material);
        material_map[mtlname] = current_material;
      } else if (3 == sscanf(textline, "DR %lf %lf %lf", &r, &g, &b)){
        current_material->Dr = v3d(r, g, b);
      } else if (3 == sscanf(textline, "SR %lf %lf %lf", &r, &g, &b)){
        current_material->Sr = v3d(r, g, b);
      } else if (3 == sscanf(textline, "DT %lf %lf %lf", &r, &g, &b)){
        current_material->Dt = v3d(r, g, b);
      } else if (3 == sscanf(textline, "ST %lf %lf %lf", &r, &g, &b)){
        current_material->St = v3d(r, g, b);
      } else if (3 == sscanf(textline, "Pd %lf %lf %lf", &r, &g, &b)){
        current_material->Pd = v3d(r, g, b);
      }
    }
    fclose(fp);
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
      vertices[v1]->edges.push_back(edge);
      vertices[v2]->edges.push_back(edge);
    }
    
    return edge;
  }
      
};
#endif // #ifndef MESH_HPP_INCLUDED_
