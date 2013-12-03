#ifndef FRAME_HPP
#define FRAME_HPP

#include "Initial3D.hpp"

class vec3;

class Frame {
private:
public:
	const Frame *parent;
	virtual vec3 getPosition() =0;
};

#endif