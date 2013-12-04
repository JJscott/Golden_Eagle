#ifndef SCENENODE_HPP
#define SCENENODE_HPP

#include "Entity.hpp"
#include <vector>

class SceneNode { 
private:
	std::vector<SceneNode*> entities;
	SceneNode* parent;
public:

};

#endif 