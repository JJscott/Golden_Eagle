#ifndef FRAME_HPP
#define FRAME_HPP

class SceneNode;
class vec3;

#include "common/sg/SceneNode.hpp"
#include "common/Initial3D.hpp"

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