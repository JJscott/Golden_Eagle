#include "loadOBJ.hpp"

#include <iostream>

bool loadOBJ(const char * path, vector<vec3d > &out_vertices, vector<vec3d > &out_uvs, vector<vec3d > &out_normals, unsigned int &out_n_triangles) {
	vector<unsigned int> vertexIndices, uvIndices, normalIndicies;
	vector<vec3d> temp_vertices;
	vector<vec3d> temp_uvs;
	vector<vec3d> temp_normals;


	FILE* file = fopen(path, "r");
	if(file == NULL) {
		fprintf(stderr, "Unable to open \"%s\"\n", path);
		return false;
	}

	while(1) {
		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if(res == EOF)
			break;
		float x, y, z;

		if(strcmp(lineHeader, "v") == 0) {
			

			fscanf(file, "%f %f %f\n", &x, &y, &z);
			printf("Scanned: %f %f %f\n", x, y, z);
			vec3d vx(x, y, z);
			printf("vxScan: %f %f %f\n", vx.x(), vx.y(), vx.z());
			temp_vertices.push_back(vx);
		} else if(strcmp(lineHeader, "vt") == 0) {
			
			fscanf(file, "%f %f\n", &x, &y);
			vec3<double> uv(x, y, 0);
			temp_uvs.push_back(uv);
		} else if(strcmp(lineHeader, "vn") == 0) {
			
			fscanf(file, "%f %f %f\n", &x, &y, &z);
			vec3<double> normal(x, y, z);
			temp_normals.push_back(normal);
		} else if(strcmp(lineHeader, "f") == 0) {
			int vertexIndex[3], uvIndex[3], normalIndex[3];
			uvIndex[0] = 0;
			uvIndex[1] = 0;
			uvIndex[2] = 0;
			// int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			// int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);

			if(matches != 9) {
				fprintf(stderr, "Invalid file (matches=%d)\n", matches);
				return false;
			}

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);

			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);

			normalIndicies.push_back(normalIndex[0]);
			normalIndicies.push_back(normalIndex[1]);
			normalIndicies.push_back(normalIndex[2]);

			printf("%d/%d/%d %d/%d/%d %d/%d/%d\n", vertexIndex[0], uvIndex[0], normalIndex[0], vertexIndex[1], uvIndex[1], normalIndex[1], vertexIndex[2], uvIndex[2], normalIndex[2]);
			out_n_triangles++;
		}
	}

	for(unsigned int i = 0; i < vertexIndices.size(); i++) {
		int vertexIndex = vertexIndices[i];
		printf("Vertex Index=%d\n", vertexIndex);
		vec3d vertex = temp_vertices[vertexIndex-1];
		std::cout << "Adding: " << vertexIndex << " " << vertex << endl;
		out_vertices.push_back(vertex);
	}

	for(unsigned int i = 0; i < uvIndices.size(); i++) {
		int uvIndex = uvIndices[i];
		vec3<double> uv = temp_uvs[uvIndex-1];
		out_uvs.push_back(uv);
	}
	out_uvs.push_back(vec3d(0, 0, 0));

	// for(int i = 0; i < normalIndicies.size(); i++) {
	//         int normalIndex = normalIndicies[i];
	//         vec3d normal = temp_normals[normalIndex-1];
	//         out_normals.push_back(normal);
	// }

	return true;
}





// // void geometry::ReadOBJ(const char *filename) {

// bool loadOBJ(const char * path, std::vector < vec3d > & out_vertices, std::vector < vec3 > & out_uvs, std::vector < vec3d > & out_normals, unsigned int & out_n_triangles) {

// 	std::vector<unsigned int> vertexIndices, uvIndices, normalIndicies;
// 	std::vector<vec3d> temp_vertices;
// 	std::vector<vec3> temp_uvs;
// 	std::vector<vec3d> temp_normals;
// 	ifstream f_istream(filename);

// 	if (!f_istream.is_open()) {
// 		fprintf(stderr, "Unable to open \"%s\"\n", path);
// 		return false;
// 	} else {
// 		// good() means that failbit, badbit and eofbit are all not set
// 		// failbit => logical error: stream didnt like what you did
// 		// badbit => i/o error: stream broke
// 		// eofbit => stream reached end
// 		// sometimes if eofbit gets set so does failbit
// 		// fail() means failbit OR badbit are set
// 		while(f_istream.good()){
// 			string line;
// 			std::getline(f_istream, line);
// 			// need to use a string input stream
// 			istringstream s_istream(line);
			
// 			string s;
// 			s_istream >> s;
// 			// reading like this means whitespace at the start of the line is fine
// 			// attempting to read from an empty string will set the failbit
// 			if (!s_istream.fail()){
// 				// make sure to read from the line input stream in here
// 				switch (s[0]) {
// 				case '#':
// 					// nothing
// 				break;
// 				case 'v':
// 					if (s.length() > 1) {
// 						vec3d vn;
// 						coordUV vt;
// 						switch (s[1]){
// 						case 'n' : //Vertex Normal
// 							s_istream >> vn.x() >> vn.y >> vn.z;
// 							m_NormalArray.push_back(vn);
// 							// cout << "VN " << vn << endl;
// 						break;
// 						case 't' : //Vertex Texture coord
							
// 							s_istream >> vt.u >> vt.v;
// 							m_UVArray.push_back(vt);
// 							// cout << "VT " << vt << endl;
// 						break;
// 						default: 
// 						break;
// 						}

// 					} else { //Vertex Coord
// 						vec3d v;
// 						s_istream >> v.x() >> v.y >> v.z;
// 						m_VertexArray.push_back(v);
// 						// cout << "V " << v << endl;
// 					}
// 				break;

// 				case 'f': //Polygon Face
// 					vector<poly_vertex> poly_vertex_l;
// 					do {
// 						poly_vertex ver;
// 						s_istream >> ver.v;
// 						if (s_istream.peek() == '/') {
// 							s_istream.ignore(1);
// 							if (s_istream.peek() != '/')
// 								s_istream >> ver.t;
// 							if (s_istream.peek() == '/') {
// 								s_istream.ignore(1);
// 								s_istream >> ver.n;
// 							}
// 						}
// 						poly_vertex_l.push_back(ver);
// 					} while (s_istream.good());
// 					if(poly_vertex_l.size() == 3){
// 						triangle tri;
// 						tri.v1 = poly_vertex_l[0];
// 						tri.v2 = poly_vertex_l[1];
// 						tri.v3 = poly_vertex_l[2];
// 						m_Triangles.push_back(tri);
// 						// cout << "F " << tri << endl;
// 					}

// 				break;
// 				// default:
// 				// 	cerr << "unknown element type: " << s << endl;
// 				}
// 			}
// 		}
// 		if (m_NormalArray.size() <= 1){
// 			cout << "Populating normals!!!" << endl;
// 			PopulateNormals();
// 		}
// 	}

// }