//Ray as  a function : P(t) = A +tb
//P is 3D position along a line in 3D
//A is orign of the ray
//b is ray direction
//for different t values, you get different position

#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray{
	public:
		ray() {}

		ray(const point3& origin, const vec3& direction) : orig(origin), dir(direction) {}

		const point3& origin() const { return orig; }//returns an immutable reference to its member orig, users can use reference, or make mutable copy of it
		const vec3& direction() const { return dir; }

		point3 at(double t) const {
			return orig + t*dir;
		}

	private:
		point3 orig;
		vec3 dir;
		//point3 and vec3 are aliases, written different only for readability
};

#endif
