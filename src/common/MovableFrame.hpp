#ifndef MOVABLEFRAME_HPP
#define MOVABLEFRAME_HPP

#include "common/Initial3D.hpp"
#include "common/Frame.hpp"

class MovableFrame : public Frame {
private:
	vec3 *Velocity;
};

#endif