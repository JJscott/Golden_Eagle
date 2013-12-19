#ifndef SCENEROOT_HPP
#define SCENEROOT_HPP

class SceneNode;

#include "SceneNode.hpp"

class SceneRoot : public SceneNode {
	public:
		const bool isRootNode() const {
			return true;
		}
};

#endif