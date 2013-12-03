#ifndef MOVABLEFRAME_HPP
#define MOVABLEFRAME_HPP

#include "Initial3D.hpp"
#include "Frame.hpp"

class MovableFrame : public Frame {
private:
	vec3 *Position;
};

#endif