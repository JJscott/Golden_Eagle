
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <algorithm>

#include "loadOBJ.h"
#include "intial3d.h"

bool loadOBJ(const char * path, std::vector<vec3d> &out_vertices, std::vector<vec3> &out_uvs, std::vector<vec3d> &out_normals, unsigned int &out_n_triangles);