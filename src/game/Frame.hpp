#ifndef FRAME_HPP
#define FRAME_HPP

#include "SceneNode.hpp"
#include "Initial3D.hpp"

class vec3;

class Frame : virtual public SceneNode {
private:
public:
	const Frame *parent;
	virtual vec3 getPosition() =0;
};

#endif