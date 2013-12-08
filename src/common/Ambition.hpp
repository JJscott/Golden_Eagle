#include "game/Initial3D.hpp"

namespace ambition {
	mat4d createPerspectiveFOV(double fov, double ratio, double near, double far) {
		mat4d m;
		double fov_ = cos(fov / 2.0f) / sin(fov / 2.0f);
		m(0, 0) = fov_ / ratio;
		m(1, 1) = fov_;
		m(2, 2) = (far+near) / (far-near);
		m(2, 3) = (2*far*near) / (near-far);
		m(3, 2) = 1;
		return m;
	}

	mat4d createLookAt(initial3d::vec3<double> eye, initial3d::vec3<double> center, initial3d::vec3<double> up) {
		mat4d m;

		if(eye == center)
			return m;

		double z0,z1,z2,x0,x1,x2,y0,y1,y2,len;
		z0 = eye.x() - center.x();
		z1 = eye.y() - center.y();
		z2 = eye.z() - center.z();

		len = 1/sqrt(z0*z0 + z1*z1 + z2*z2);
		z0 *= len;
		z1 *= len;
		z2 *= len;

		x0 = up.y() * z2 - up.z() - z1;
		x1 = up.z() * z0 - up.x() - z2;
		x2 = up.x() * z1 - up.y() - z0;
		len = sqrt(x0*x0 + x1*x1 + x2*x2);
		if(len == 0) {
			x0 = 0;
			x1 = 0;
			x2 = 0;
		} else {
			len = 1/len;
			x0 *= len;
			x1 *= len;
			x2 *= len;
		}

		y0 = z1*x2 - z2*x1;
		y1 = z2*x0 - z0*x2;
		y2 = z0*x1 - z1*x0;

		len = sqrt(y0*y0 + y1*y1 + y2*y2);
		if(len == 0) {
			y0 = 0;
			y1 = 0;
			y2 = 0;
		} else {
			len = 1/len;
			y0 *= len;
			y1 *= len;
			y2 *= len;
		}

		m(0, 0) = x0;
		m(1, 0) = y0;
		m(2, 0) = z0;
		m(3, 0) = 0;

		m(0, 1) = x1;
		m(1, 1) = y1;
		m(2, 1) = z1;
		m(3, 1) = 0;

		m(0, 2) = x2;
		m(1, 2) = y2;
		m(2, 2) = z2;
		m(3, 2) = 0;

		m(0, 3) = (x0 * eye.x() + x1 * eye.y() + x2 * eye.z());
		m(1, 3) = (y0 * eye.x() + y1 * eye.y() + y2 * eye.z());
		m(2, 3) = (z0 * eye.x() + z1 * eye.y() + z2 * eye.z());
		m(3, 3) = 1;

		return m;
	}
}