#ifndef __MOVABLEFRAME
#define __MOVABLEFRAME

#include "Initial3D.hpp"
#include "Frame.hpp"

class MovableFrame : public Frame {
private:
	vec3 *Position;
};

#endif