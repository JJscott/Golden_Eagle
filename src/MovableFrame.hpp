#ifndef __MOVABLEFRAME
#define __MOVABLEFRAME

#include "Frame.hpp"
#include "Vector3.hpp"

class MovableFrame : public Frame {
private:
	Vector3 Position;
};

#endif