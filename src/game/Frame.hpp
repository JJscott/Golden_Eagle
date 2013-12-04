#ifndef FRAME_HPP
#define FRAME_HPP

#include "SceneNode.hpp"
#include "Initial3D.hpp"

class vec3;

class Frame : virtual public SceneNode {
private:
	vec3* position;
public:

	const bool isRootNode() const {
		return true;
	}

	vec3* getPosition() {
		return this->position;
	}
};

#endif