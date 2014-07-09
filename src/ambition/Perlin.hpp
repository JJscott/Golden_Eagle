
/*
* Perlin engine
* Infulences from code off web, but nothing that was under license
*
* @author Joshua Scott
*/

#pragma once

#include "Initial3D.hpp"

namespace ambition {

	class Perlin {
	public:
		Perlin();
		Perlin(long);
		~Perlin();

		double getNoise(double, double, double, int) const;
		double getNoise(double, double, double) const;

	private:
		static const size_t grad_table_size = 256;

		double smooth(double) const;
		double lattice(int, int, int, double, double, double) const;
		int getIndex(int, int, int) const;
		int permutate(int) const;
		double lerp(double, double, double) const;

		double m_gradients[grad_table_size * 3];
	};

}