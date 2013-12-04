#ifndef SCENENODE_HPP
#define SCENENODE_HPP

#include <vector>
#include <cstring>

class SceneNode { 
	SceneNode* parent;
	const char* name = NULL;
	std::vector<SceneNode*> children;

public:
	SceneNode(SceneNode* o_parent = NULL, const char* s_name = NULL);
	virtual ~SceneNode();	
	void update();

	SceneNode* getParentNode() const;
	void setParentNode(SceneNode*);

	void addChildNode(SceneNode*);
	void removeChildNode(SceneNode*);

	const char* getNodeName() const;
	const size_t countChildNodes(const bool isRecursiveCount = false) const;

	virtual const bool isRootNode() const = 0;

	SceneNode* getChildNodeByName(const char*);
};

#endif 