#ifndef FRAME_HPP
#define FRAME_HPP

#include "ambition/SceneNode.hpp"
#include "ambition/Initial3D.hpp"

class vec3;

class Frame : public SceneNode {
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