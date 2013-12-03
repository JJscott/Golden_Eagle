#include "Vector3.hpp"

class Frame {
private:
public:
	const Frame *parent;
	virtual Vector3 getPosition() =0;
};