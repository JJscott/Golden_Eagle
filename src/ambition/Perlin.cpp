
/*
* Perlin engine
* Infulences from code off web, but nothing that was under license
*
* @author Joshua Scott
*/

#include <algorithm>
#include <cmath> 
#include <cstdlib>
#include <random>

#include "Initial3D.hpp"
#include "Perlin.hpp"

using namespace std;

static int m_perm[] = {
	225,155,210,108,175,199,221,144,203,116, 70,213, 69,158, 33,252,
	5, 82,173,133,222,139,174, 27,  9, 71, 90,246, 75,130, 91,191,
	169,138,  2,151,194,235, 81,  7, 25,113,228,159,205,253,134,142,
	248, 65,224,217, 22,121,229, 63, 89,103, 96,104,156, 17,201,129,
	36,  8,165,110,237,117,231, 56,132,211,152, 20,181,111,239,218,
	170,163, 51,172,157, 47, 80,212,176,250, 87, 49, 99,242,136,189,
	162,115, 44, 43,124, 94,150, 16,141,247, 32, 10,198,223,255, 72,
	53,131, 84, 57,220,197, 58, 50,208, 11,241, 28,  3,192, 62,202,
	18,215,153, 24, 76, 41, 15,179, 39, 46, 55,  6,128,167, 23,188,
	106, 34,187,140,164, 73,112,182,244,195,227, 13, 35, 77,196,185,
	26,200,226,119, 31,123,168,125,249, 68,183,230,177,135,160,180,
	12,  1,243,148,102,166, 38,238,251, 37,240,126, 64, 74,161, 40,
	184,149,171,178,101, 66, 29, 59,146, 61,254,107, 42, 86,154,  4,
	236,232,120, 21,233,209, 45, 98,193,114, 78, 19,206, 14,118,127,
	48, 79,147, 85, 30,207,219, 54, 88,234,190,122, 95, 67,143,109,
	137,214,145, 93, 92,100,245,  0,216,186, 60, 83,105, 97,204, 52
	};

namespace ambition {

	Perlin::Perlin() {
		default_random_engine gen;
		uniform_real_distribution<double> randomd(0.0,1.0);
		//InitGradients
		for (int i = 0; i < int(grad_table_size); i++){
			double z = 1.0 - 2.0 * randomd(gen);
			double r = sqrt(1.0 - z * z);
			double theta = 2 * initial3d::math::pi() * randomd(gen);
			m_gradients[i * 3] = r * cos(theta);
			m_gradients[i * 3 + 1] = r * sin(theta);
			m_gradients[i * 3 + 2] = z;
		}
	}

	Perlin::Perlin(long seed) {
		default_random_engine gen;
		uniform_real_distribution<double> randomd(0.0,1.0);
		//InitGradients
		for (int i = 0; i < int(grad_table_size); i++){
			double z = 1.0 - 2.0 * randomd(gen);
			double r = sqrt(1.0 - z * z);
			double theta = 2 * initial3d::math::pi() * randomd(gen);
			m_gradients[i * 3] = r * cos(theta);
			m_gradients[i * 3 + 1] = r * sin(theta);
			m_gradients[i * 3 + 2] = z;
		}
	}

	Perlin::~Perlin() { }

	//returns [-0.5..0.5) I think
	double Perlin::getNoise(double x, double y, double z, int octaves) const {
		double noise = 0;

		double amp = 0.5f;
		double fq = 8;

		for(int i=0; i<octaves; i++, fq*=2, amp*=0.5){
			noise += amp*getNoise(fq*x, fq*y, fq*z);
		}

		return noise;
	}

	double Perlin::getNoise(double x, double y, double z) const {
		int ix = (int)floor(x);
		double fx0 = x - ix;
		double fx1 = fx0 - 1;
		double wx = smooth(fx0);

		int iy = (int)floor(y);
		double fy0 = y - iy;
		double fy1 = fy0 - 1;
		double wy = smooth(fy0);

		int iz = (int)floor(z);
		double fz0 = z - iz;
		double fz1 = fz0 - 1;
		double wz = smooth(fz0);

		double vx0 = lattice(ix, iy, iz, fx0, fy0, fz0);
		double vx1 = lattice(ix + 1, iy, iz, fx1, fy0, fz0);
		double vy0 = lerp(wx, vx0, vx1);

		vx0 = lattice(ix, iy + 1, iz, fx0, fy1, fz0);
		vx1 = lattice(ix + 1, iy + 1, iz, fx1, fy1, fz0);
		double vy1 = lerp(wx, vx0, vx1);

		double vz0 = lerp(wy, vy0, vy1);

		vx0 = lattice(ix, iy, iz + 1, fx0, fy0, fz1);
		vx1 = lattice(ix + 1, iy, iz + 1, fx1, fy0, fz1);
		vy0 = lerp(wx, vx0, vx1);

		vx0 = lattice(ix, iy + 1, iz + 1, fx0, fy1, fz1);
		vx1 = lattice(ix + 1, iy + 1, iz + 1, fx1, fy1, fz1);
		vy1 = lerp(wx, vx0, vx1);

		double vz1 = lerp(wy, vy0, vy1);
		return lerp(wz, vz0, vz1);
	}

	//Private Methods

	double Perlin::smooth(double x) const {
		return x * x * (3 - 2 * x);
	}

	double Perlin::lattice(int ix, int iy, int iz, double fx, double fy, double fz) const {
		int index = getIndex(ix, iy, iz);
		int g = index * 3;
		return m_gradients[g] * fx + m_gradients[g + 1] * fy + m_gradients[g + 2] * fz;
	}

	int Perlin::getIndex(int ix, int iy, int iz) const {
		return permutate(ix + permutate(iy + permutate(iz)));
	}

	int Perlin::permutate(int x) const {
		int mask = grad_table_size - 1;
		return m_perm[x & mask];
	}

	double Perlin::lerp(double t, double value0, double value1) const {
		return value0 + t * (value1 - value0);
	}

}