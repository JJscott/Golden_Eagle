#ifndef SCENE_HPP
#define SCENE_HPP

#include "common/sg/Entity.hpp"
#include "common/sg/SceneRoot.hpp"

class Scene {
	SceneRoot root;
	public:
		void addEntity(Entity *e);
};

#endif