#ifndef FRAME_HPP
#define FRAME_HPP

#include "ambition/SceneNode.hpp"
#include "ambition/Initial3D.hpp"

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