//Axis Aligned Bounding Box
//a 3d box that bounds a group of isolated hittable objects , used to optimise the way the ray tracer knows whether it hits an object, by using AABBs we can avoid computing for a lot of the rays.

#ifndef AABB_H
#define AABB_H

#include "interval.h"
#include "vec3.h"

class aabb {
	public:
		interval x, y, z;

		aabb()  {} //default AABB is empty, since we havent included any hittable objects, so its just ,empty

		aabb(const interval& x, const interval& y, const interval& z) : x(x), y(y), z(z) {}

		aabb(const point3& a, const point3& b) {
			//two points a and b are the two extremes of the bounding box, since its not interval, we dont need to have another check for checking if the intervals are in reverse manner like one interval being [3,8] and another being [5, 0]

			x = (a[0] <= b[0]) ? interval(a[0], b[0]) : interval(b[0], a[0]);
			y = (a[1] <= b[1]) ? interval(a[1], b[1]) : interval(b[1], a[1]);
			z = (a[2] <= b[2]) ? interval(a[2], b[2]) : interval(b[2], a[2]);
		}

		aabb( const aabb& box0, const aabb& box1){//using two bounding boxes, to construct a new bigger bounding box, bounding both the bounding boxes
			x = interval(box0.x, box1.x);
			y = interval(box0.y, box1.y);
			z = interval(box0.z, box1.z);
		}

		const interval& axis_interval(int n) const {
			if(n == 1) return y;
			if(n == 2) return z;
			return x; 
		}

		//to check if the ray hits this bounding box
		//we do this 1 axis at a time , and at the end, we check if the time at which each axis's ray colided overlaps or not, if it doesnt overlap at all, it means that it never hit the bounding box
		bool hit(const ray& r, interval ray_t) const {
			const point3& ray_orig = r.origin();
			const vec3& ray_dir = r.direction();

			for(int axis = 0; axis < 3; axis++) {//loop for getting t0 and t1, the two points at which the ray hits the two planes of the bounding box (perpendicular to that axis, that is why we have two points, because bounding box has two planes perpendicular to each axis)
				const interval& ax = axis_interval(axis);
				const double adinv = 1.0 / ray_dir[axis];//some thing , required , for the formula for solving the LINEAR EQUATION

				auto t0 = (ax.min - ray_orig[axis]) * adinv;
				auto t1 = (ax.max - ray_orig[axis]) * adinv;

				if(t0<t1){
					if(t0 > ray_t.min) ray_t.min = t0;
					if(t1 < ray_t.max) ray_t.max = t1;
				} else {
					if(t1 > ray_t.min) ray_t.min = t1;
					if(t0 < ray_t.max) ray_t.max = t0;
				}

				if(ray_t.max <= ray_t.min)
					return false;
			}
			return true;
		}
};


#endif

