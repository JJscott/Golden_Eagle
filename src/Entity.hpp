#ifndef __ENTITY
#define __ENTITY

#include "MovableFrame.hpp"

class Entity : public MovableFrame {
public:
	virtual void Draw() = 0;
};

#endif