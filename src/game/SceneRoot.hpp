#ifndef SCENEROOT_HPP
#define SCENEROOT_HPP

class SceneNode;

#include "SceneNode.hpp"

class SceneRoot : SceneNode {
	public:
		const bool isRootNode() const {
			return true;
		}
} rootScene;

#endif