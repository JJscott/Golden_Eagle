#ifndef MOVABLEFRAME_HPP
#define MOVABLEFRAME_HPP

#include "ambition/Initial3D.hpp"
#include "ambition/Frame.hpp"

class MovableFrame : public Frame {
private:
	vec3 *Velocity;
};

#endif