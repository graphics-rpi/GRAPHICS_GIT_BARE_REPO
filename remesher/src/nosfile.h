#ifndef _JL_NOSFile_H_
#define _JL_NOSFile_H_

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <fstream>

// ===================================================================
// ===================================================================

class NOSFile {

public: 
  NOSFile() { assert(0); }
  NOSFile(const char *filename);
  NOSFile(const NOSFile &P) { assert(0); exit(0); }
  NOSFile& CopyFrom(const NOSFile&) { assert(0); exit(0); }
  NOSFile& operator=(const NOSFile &P) { assert(0); exit(0); }
  virtual ~NOSFile();
  
  NOSFile(int vertex_size, int triangle_size,
	  int num_vertices, int num_triangles);
  void NewSize(int vertex_size, int triangle_size,
	       int num_vertices, int num_triangles);
  
  void Load(const char *filename);
  void Save(const char *filename) const;
  
  void LoadInfo(std::ifstream &iFile);

  //  int Version() const { return _version; };
  //int Type() const { return _type; };
  
  int NumVertices() const { return _num_vertices; }
  int NumTriangles() const { return _num_triangles; }
  
  float* GetVertex(int i);
  int* GetTriangle(int i);
  
  int VertexSize() const { return _vertex_size; }
  int TriangleSize() const { return _triangle_size; }
  // void IncreaseVertexSize(int floats);
  //void IncreaseTriangleSize(int ints);
  //void DecreaseTriangleSize(int ints);
    
  //void CalcTriangleNormal(int tindex, float n[3]) const;
  //float CalcTriangleVertexAngle(int tindex, int v) const;
  
  //void CalcVertexNormals(float *n, int stride=3) const;
  
  
  void VertexInterp(const float *v1, const float *v2, const float *v3,
		    float s, float t,
		    float *dst) const;
protected:
  float	*_vertices;
  int *_triangles;
  int _vertex_size;		// number of floats in vertex
  int _triangle_size;		// number of ints in triangle
  int _num_vertices;
  int _num_triangles;
};


void Write_w_Endian(std::ostream &oFile, int i);
void Write_w_Endian(std::ostream &oFile, float f);

void Write_w_Endian(std::ostream &oFile, const int *i, int n);
void Write_w_Endian(std::ostream &oFile, const float *f, int n);


void Read_w_Endian(std::istream &iFile, int &i);
void Read_w_Endian(std::istream &iFile, float &f);

void Read_w_Endian(std::istream &iFile, int *i, int n);
void Read_w_Endian(std::istream &iFile, float *f, int n);



#endif
