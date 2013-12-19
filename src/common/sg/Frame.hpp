#ifndef FRAME_HPP
#define FRAME_HPP

#include "common/sg/SceneNode.hpp"
#include "common/Initial3D.hpp"

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