#ifndef MESH_H_
#define MESH_H_

#include <string>

#include "canvas.h"
#include "../../common/glm/glm/glm.hpp"

class ArgParser;

class Mesh{
	public:
		// Constructor
		Mesh(ArgParser* args){ args_ = args; }

		// Destructor
		~Mesh();

		// Drawing functions
		void Initialize();
		void LoadTexture( const std::string &filename );
		void Draw();
		void Cleanup();

		static const GLfloat g_vertex_buffer_data_[];
		static const GLfloat g_uv_buffer_data_[];

		// Accessors
		glm::mat4 model() const { return model_; } 
		void set_model( const glm::mat4 & model ){ model_ = model; }
		glm::vec3 Min() const;
		glm::vec3 Max() const;
		void Normalize( glm::vec4 & input ) const;

	private:
		GLuint loadBMP_custom(const char * imagepath);

		GLuint mesh_vao_;
		GLuint mesh_verticies_vbo_;
		GLuint mesh_uv_vbo_;
		GLuint texture_;

		ArgParser* args_;
		glm::mat4 model_;
};

#endif
