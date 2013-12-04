
#include "SceneNode.hpp"

SceneNode::SceneNode(SceneNode* o_parent, const char* s_name)
	: name(s_name) {
	parent = o_parent;
}

SceneNode::~SceneNode() {
	parent = NULL;
	children.clear();
}

void SceneNode::update() {
	if(!children.empty()) {
		for(size_t i = 0; i < children.size(); i++) {
			if(children[i] != NULL)
				children[i]->update();
		}
	}
}

void SceneNode::setParentNode(SceneNode* newParent) {
	if(NULL != parent)
		parent->removeChildNode(this);
	parent = newParent;
}

void SceneNode::addChildNode(SceneNode* childNode) {
	if(childNode != NULL) {
		childNode->setParentNode(this);
		children.push_back(childNode);
	}
}

void SceneNode::removeChildNode(SceneNode* childNode) {
	if(childNode != NULL && !children.empty()) {
		for(size_t i = 0; i < children.size(); i++) {
			if(children[i] == childNode) {
				children.erase(children.begin() + i);
				break;
			}
		}
	}
}

const char* SceneNode::getNodeName() const {
	return name;
}

const size_t SceneNode::countChildNodes(const bool recursiveCount) const {
	if(!recursiveCount)
		return children.size();
	else
	{
		size_t size = children.size();
		for(size_t i = 0; i < children.size(); i++)
			size += children[i]->countChildNodes(true);
		return size;
	}
}

SceneNode* SceneNode::getChildNodeByName(const char* searchName) {
	SceneNode* ret = NULL;
	if(!children.empty()) {
		for(size_t i = 0; i < children.size(); i++) {
			if(strcmp(children[i]->getNodeName(), searchName) == 0) {
				ret = children[i];
				break;
			}
		}
	}
	return ret;
}
