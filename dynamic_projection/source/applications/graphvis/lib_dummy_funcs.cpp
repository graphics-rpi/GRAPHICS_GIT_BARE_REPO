#include "../paint/gl_includes.h"

#include <string>
#include <iostream>
#include <cstdio>

int HandleGLError(std::string foo) {
	GLenum error;
	int i = 0;
	while ((error = glGetError()) != GL_NO_ERROR) {
		printf ("GL ERROR(#%d == 0x%x):  %s\n", i, error, gluErrorString(error));
		std::cout << foo << std::endl;
		if (error != GL_INVALID_OPERATION) i++;
	}
	if (i == 0) return 1;
	return 0;
}
