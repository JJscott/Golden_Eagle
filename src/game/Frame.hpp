#ifndef FRAME_HPP
#define FRAME_HPP

#include "common/SceneNode.hpp"
#include "common/Initial3D.hpp"

class Frame : public SceneNode {
private:
	initial3d::vec3d position;
public:

	const bool isRootNode() const {
		return true;
	}

	initial3d::vec3d getPosition() {
		return this->position;
	}
};

#endif