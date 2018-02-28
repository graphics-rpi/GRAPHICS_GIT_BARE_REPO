#ifndef _REMESHER_H_
#define _REMESHER_H_

#include <iostream>
#include <vector>
#include <string>
#include "mesh.h"

const Mesh* remesher_main (const std::vector<std::string> &command_line_args, bool stealmesh);

#endif
