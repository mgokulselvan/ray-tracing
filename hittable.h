#ifndef HITTABLE_H
#define HITTABLE_H

#include "rtweekend.h"
#include "aabb.h"

class material;//just declaring so it doesnt give error, its definition will be here later, if we use include, there will be circular importing 

class hit_record {//for each hit , i.e intersection that a ray makes with the object in the world, we store its position , surface normal vector, if its front of the object or the back of it, and also at what "t" value did it hit the object
	public:
		point3 p;
		vec3 normal;
		double t;
		bool front_face;
		double u;//surface coordinates of the ray-object(where the ray hit ON THE OBJECT (point3 p is just overvall position in the 3d vector space)) hit point
		double v;//surface coordinates of the ray-object hit point, used for texture mapping
		shared_ptr<material> mat;

		//we are setting so that the surface normal always go away from the surface,so here we are tracking if its front face or not and then making so that surface normal is pointing away from the surface
		void set_face_normal(const ray& r, const vec3& outward_normal){//if the ray is hitting the front of the surface or back of the surface(front and back with respect to the camera)
			//sets the hit record normal vector, outward_normal is assumed to have unit length
			front_face = dot(r.direction(),outward_normal)<0;
			normal = front_face ? outward_normal : -outward_normal;
		}
};

class hittable {//this is just an interface, meant to have a bunch of classes that inherit this property, thas it
	public:
		virtual ~hittable() = default; // destructor, telling c++ to do it itself
		
		virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0; // every class that inherits this class must overwrite this function
	   //meant to tell if the ray given hits the physical geometric object within the given interval of t
		virtual aabb bounding_box() const = 0;
};

class translate : public hittable {
	public:

		translate(shared_ptr<hittable> object, const vec3& offset) : object(object), offset(offset) {
			bbox = object->bounding_box() + offset;//update the bounding box so that ray going there is checked for if they hit the translated object, or else it might miss hitting the places wherever this object gets offsetted to
		}

		bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
			//Move ray backwards by offset instead of moving the actual object forward
			ray offset_r(r.origin() - offset, r.direction(), r.time());

			//check if the offsetted ray can intersect the origin object, and if it does, store data in rec
			if(!object->hit(offset_r, ray_t, rec))
				return false;

			//now move the intersection point forwards by offset
			rec.p += offset;

			return true;
		}

		aabb bounding_box() const override { return bbox; }

	private:
		shared_ptr<hittable> object;
		vec3 offset;
		aabb bbox;
};

#endif
