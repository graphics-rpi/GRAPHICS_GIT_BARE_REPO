#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>

#include "nosfile.h"
#include "vectors.h"

//#define DEFAULT_VERSION 1
//#define DEFAULT_TYPE 0

NOSFile::NOSFile(const char *filename) {
  _vertices = NULL;
  _triangles = NULL;
  _vertex_size = 0;
  _triangle_size = 0;
  _num_vertices = 0;
  _num_triangles = 0;
  Load(filename);
}

NOSFile::~NOSFile() {
  delete [] _vertices;
  delete [] _triangles;
  _vertices = NULL;
  _triangles = NULL;
  _vertex_size = 0;
  _triangle_size = 0;
  _num_vertices = 0;
  _num_triangles = 0;
  //_version = DEFAULT_VERSION;
  //_type = DEFAULT_TYPE;
}

NOSFile::NOSFile(int vertex_size, int triangle_size,
		      int num_vertices, int num_triangles) {
  _vertices = NULL;
  _triangles = NULL;
  _vertex_size = 0;
  _triangle_size = 0;
  _num_vertices = 0;
  _num_triangles = 0;
  NewSize(vertex_size, triangle_size,
	  num_vertices, num_triangles);

}


void NOSFile::NewSize(int vertex_size, int triangle_size,
		      int num_vertices, int num_triangles) {
  
  if ((_num_vertices!=num_vertices) || (_vertex_size!=vertex_size)) {
    _vertex_size = vertex_size;
    _num_vertices = num_vertices;
    delete [] _vertices;
    _vertices = new float[_num_vertices * _vertex_size];
  }
  
  if ((_num_triangles!=num_triangles) || (_triangle_size!=triangle_size)) {
    _triangle_size = triangle_size;
    _num_triangles = num_triangles;
    delete [] _triangles;
    _triangles = new int[_num_triangles * _triangle_size];
  }
}

void NOSFile::LoadInfo(std::ifstream &iFile) {
  
  int tmp;
  Read_w_Endian(iFile, tmp);  //assert (tmp == 1);
  Read_w_Endian(iFile, tmp);  //assert (tmp == 0);
  
  float bounds[6];
  Read_w_Endian(iFile, bounds, 6);
  
  assert (!iFile.eof());
  
  int n;
  do {
    Read_w_Endian(iFile, n);
    if (n > 0) {
      char *comment = new char[n+1];
      iFile.read(comment, n);
      comment[n] = '\0';
      //cout << "NOS comment: " << comment << std::endl;
      //_comments.PlaceInList(new JLString(comment));
      delete [] comment;
    }
  } while (n > 0);
  
  
  if (iFile.eof()) {
    std::cerr << "Error in NOSFile::Load(), unexpected EOF" << std::endl;
    return;
  }
  
  
  int num_vertices, vertex_size, num_triangles, triangle_size;
  Read_w_Endian(iFile, num_vertices);
  Read_w_Endian(iFile, vertex_size);
  Read_w_Endian(iFile, num_triangles);
  
  Read_w_Endian(iFile, triangle_size);
  
  NewSize(vertex_size, triangle_size, num_vertices, num_triangles);
}


void NOSFile::Load(const char *filename) {
  std::ifstream iFile(filename, std::ios::in);
  if (!iFile) printf ("whoops, can't read this nos file %s\n",filename);
  assert(iFile);
  
  char header[4];
  iFile.read(header, 4);
  assert(!strncmp(header, "NOS ", 4));
  
  LoadInfo(iFile);
  
  Read_w_Endian(iFile, _vertices, _num_vertices * _vertex_size);
  Read_w_Endian(iFile, _triangles, _num_triangles * _triangle_size);
  
  iFile.close();
  
}

void NOSFile::Save(const char *filename) const {

  std::ofstream oFile(filename, std::ios::out);
  assert(oFile);  
  
  oFile.write("NOS ", 4);
  
  Write_w_Endian(oFile, 1);
  Write_w_Endian(oFile, 0);
  
  float bounds[6];
  bounds[0] = 0;
  bounds[1] = 0;
  bounds[2] = 0;
  bounds[3] = 0;
  bounds[4] = 0;
  bounds[5] = 0;
  Write_w_Endian(oFile, bounds, 6);
  
  //  for (int n=0; n<_comments.Size(); n++) {
  //Write_w_Endian(oFile, _comments[n].Length());
  //oFile.write(_comments[n].GetString(), _comments[n].Length());
  //}
  Write_w_Endian(oFile, 0);
  
  
  Write_w_Endian(oFile, _num_vertices);
  Write_w_Endian(oFile, _vertex_size);
  Write_w_Endian(oFile, _num_triangles);
  Write_w_Endian(oFile, _triangle_size);
  
  
  Write_w_Endian(oFile, _vertices, _num_vertices * _vertex_size);
  Write_w_Endian(oFile, _triangles, _num_triangles * _triangle_size);
  
  
  oFile.close();
}

float* NOSFile::GetVertex(int i) {
  assert (i >= 0 && i < _num_vertices);
  return _vertices + i*_vertex_size;
}

int* NOSFile::GetTriangle(int i) {
  assert (i >= 0 && i < _num_triangles);
  return _triangles + i*_triangle_size;
}


void NOSFile::VertexInterp(const float *v1, const float *v2, const float *v3,
						   float s, float t,
						   float *dst) const {

	float u = 1.0f - s - t;

	for (int i=3; i<_vertex_size; i++)
		dst[i] = u * v1[i] + s * v2[i] + t * v3[i];
}


void Write_w_Endian(std::ostream &oFile, int i) {

	int e=1;
	int flip_endian = (((char*)&e)[0]);

	char *buff = (char*)(&i);

	if (flip_endian) {
		char b0 = buff[0];
		char b1 = buff[1];
		buff[0] = buff[3];
		buff[1] = buff[2];
		buff[2] = b1;
		buff[3] = b0;
	}

	oFile.write(buff, sizeof(int));
}

void Write_w_Endian(std::ostream &oFile, float f) {

	int e=1;
	int flip_endian = (((char*)&e)[0]);

	char *buff = (char*)(&f);

	if (flip_endian) {
		char b0 = buff[0];
		char b1 = buff[1];
		buff[0] = buff[3];
		buff[1] = buff[2];
		buff[2] = b1;
		buff[3] = b0;
	}

	oFile.write(buff, sizeof(float));
}


void Write_w_Endian(std::ostream &oFile, const int *i, int n) {

	int e=1;
	int flip_endian = (((char*)&e)[0]);

	char *buff = (char*)(i);

	if (flip_endian) {
		for (int j=0; j<n; j++) {
			char b0 = buff[j*sizeof(int)+0];
			char b1 = buff[j*sizeof(int)+1];
			buff[j*sizeof(int)+0] = buff[j*sizeof(int)+3];
			buff[j*sizeof(int)+1] = buff[j*sizeof(int)+2];
			buff[j*sizeof(int)+2] = b1;
			buff[j*sizeof(int)+3] = b0;
		}
	}

	oFile.write(buff, sizeof(int)*n);

	if (flip_endian) {
		for (int j=0; j<n; j++) {
			char b0 = buff[j*sizeof(int)+0];
			char b1 = buff[j*sizeof(int)+1];
			buff[j*sizeof(int)+0] = buff[j*sizeof(int)+3];
			buff[j*sizeof(int)+1] = buff[j*sizeof(int)+2];
			buff[j*sizeof(int)+2] = b1;
			buff[j*sizeof(int)+3] = b0;
		}
	}
}

void Write_w_Endian(std::ostream &oFile, const float *f, int n) {

	int e=1;
	int flip_endian = (((char*)&e)[0]);

	char *buff = (char*)(f);

	if (flip_endian) {
		for (int j=0; j<n; j++) {
			char b0 = buff[j*sizeof(float)+0];
			char b1 = buff[j*sizeof(float)+1];
			buff[j*sizeof(float)+0] = buff[j*sizeof(float)+3];
			buff[j*sizeof(float)+1] = buff[j*sizeof(float)+2];
			buff[j*sizeof(float)+2] = b1;
			buff[j*sizeof(float)+3] = b0;
		}
	}

	oFile.write(buff, sizeof(float)*n);

	if (flip_endian) {
		for (int j=0; j<n; j++) {
			char b0 = buff[j*sizeof(int)+0];
			char b1 = buff[j*sizeof(int)+1];
			buff[j*sizeof(int)+0] = buff[j*sizeof(int)+3];
			buff[j*sizeof(int)+1] = buff[j*sizeof(int)+2];
			buff[j*sizeof(int)+2] = b1;
			buff[j*sizeof(int)+3] = b0;
		}
	}
}


void Read_w_Endian(std::istream &iFile, int &i) {

	int e=1;
	int flip_endian = (((char*)&e)[0]);

	char *buff = (char*)(&i);

	iFile.read(buff, sizeof(int));

	if (flip_endian) {
		char b0 = buff[0];
		char b1 = buff[1];
		buff[0] = buff[3];
		buff[1] = buff[2];
		buff[2] = b1;
		buff[3] = b0;
	}
}

void Read_w_Endian(std::istream &iFile, float &f) {

	int e=1;
	int flip_endian = (((char*)&e)[0]);

	char *buff = (char*)(&f);

	iFile.read(buff, sizeof(float));

	if (flip_endian) {
		char b0 = buff[0];
		char b1 = buff[1];
		buff[0] = buff[3];
		buff[1] = buff[2];
		buff[2] = b1;
		buff[3] = b0;
	}
}

void Read_w_Endian(std::istream &iFile, int *i, int n) {

	int e=1;
	int flip_endian = (((char*)&e)[0]);

	char *buff = (char*)(i);

	iFile.read(buff, sizeof(int)*n);

	if (flip_endian) {
		for (int j=0; j<n; j++) {
			char b0 = buff[j*sizeof(int)+0];
			char b1 = buff[j*sizeof(int)+1];
			buff[j*sizeof(int)+0] = buff[j*sizeof(int)+3];
			buff[j*sizeof(int)+1] = buff[j*sizeof(int)+2];
			buff[j*sizeof(int)+2] = b1;
			buff[j*sizeof(int)+3] = b0;
		}
	}
}

void Read_w_Endian(std::istream &iFile, float *f, int n) {

	int e=1;
	int flip_endian = (((char*)&e)[0]);

	char *buff = (char*)(f);

	iFile.read(buff, sizeof(float)*n);

	if (flip_endian) {
		for (int j=0; j<n; j++) {
			char b0 = buff[j*sizeof(float)+0];
			char b1 = buff[j*sizeof(float)+1];
			buff[j*sizeof(float)+0] = buff[j*sizeof(float)+3];
			buff[j*sizeof(float)+1] = buff[j*sizeof(float)+2];
			buff[j*sizeof(float)+2] = b1;
			buff[j*sizeof(float)+3] = b0;
		}
	}
}

