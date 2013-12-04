#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "MovableFrame.hpp"

class Entity : public MovableFrame {
public:
	virtual void draw() = 0;
};

#endif