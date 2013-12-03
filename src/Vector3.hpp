#ifndef __VECTOR3
#define __VECTOR3

class Vector3 {
private:
	double X, Y, Z;
public:
	static const Vector3 Zero;
	Vector3();
	Vector3(double, double, double);	
};

#endif