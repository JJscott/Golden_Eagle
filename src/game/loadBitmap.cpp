#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include "loadBitmap.hpp"

GLuint loadBMP(const char* imagePath) {
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int width, height;
	unsigned int imageSize;

	unsigned char* data;

	FILE* file = fopen(imagePath, "rb");
	if(!file) {
		fprintf(stderr, "Could not open image: \"%s\"\n", imagePath);
		return 0;
	}

	if(fread(header, 1, 54, file) != 54) {
		fprintf(stderr, "Bitmap file not valid\n");
		return false;
	}

	if(header[0] != 'B' || header[1] != 'M') {
		fprintf(stderr, "Bitmap file header invalid\n");
		return 0;
	}	

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	if(imageSize == 0) imageSize = width * height * 3;
	if(dataPos == 0) dataPos = 54;

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);

	// bind to openGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

