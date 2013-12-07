#ifndef LOADOBJ_HPP
#define LOADOBJ_HPP

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <algorithm>

#include "Initial3D.hpp"

using namespace std;
using namespace initial3d;

bool loadOBJ(const char * path, vector<vec3<double> > &out_vertices, vector<vec3<double> > &out_uvs, vector<vec3<double> > &out_normals, unsigned int &out_n_triangles);

#endif