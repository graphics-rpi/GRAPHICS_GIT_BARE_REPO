#version 330 core
layout(location=0) in vec3 vertexPosition_modelspace;
layout(location=1) in vec2 vertexUV;

out vec2 UV;

uniform mat4 MVP;

varying vec3 pos;

const float min_val = -2.0f;
const float max_val = 2.0f;

vec3 normalize_range( vec3 input_vec ){
	float x;
	float y;
	float z;

	x = (input_vec.x - min_val) / (max_val - min_val);
	y = (input_vec.y - min_val) / (max_val - min_val);
	z = (input_vec.z - min_val) / (max_val - min_val);

	return vec3(x, y, z);
}

void main(){

  gl_Position = MVP * vec4(vertexPosition_modelspace, 1);
  pos = normalize_range(vertexPosition_modelspace);
  UV = vertexUV;
  
}

