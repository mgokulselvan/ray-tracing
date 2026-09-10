#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"

class hit_record {
	public:
		point3 p;
		vec3 normal;
		double t;
		bool front_face;

		//we are setting so that the surface normal always go away from the surface,so here we are tracking if its front face or not and then making so that surface normal is pointing away from the surface
		void set_face_normal(const ray& r, const vec3& outward_normal){
			//sets the hit record normal vector, outward_normal is assumed to have unit length
			front_face = dot(r.direction(),outward_normal)<0;
			normal = front_face ? outward_normal : -outward_normal;
		}
};

class hittable {
	public:
		virtual ~hittable() = default; // destructor, telling c++ to do it itself
		
		virtual bool hit(const ray& r, double ray_tmin, double ray_tmax, hit_record& rec) const = 0; // every class that inherits this class must overwrite this function
};

#endif
