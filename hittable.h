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

class rotate_y : public hittable {
	public:

		rotate_y(shared_ptr<hittable> object, double angle) : object(object) {//modifying the bounding box so that when ray passes through that space it wouldnt miss the rotated object, cuz we arent actually rotating the object, the object stays still and we are only rotating the ray (both incoming and outgoing) and simulate it as if the object was actually rotated
			auto radians = degrees_to_radians(angle);
			sin_theta = std::sin(radians);
			cos_theta = std::cos(radians);
			bbox = object->bounding_box();

			point3 min(infinity, infinity, infinity);
			point3 max(-infinity, -infinity, -infinity);

			for(int i=0;i<2;i++) {

				for(int j=0;j<2;j++) {

					for(int k=0;k<2;k++) {
						auto x = i*bbox.x.max + (1-i)*bbox.x.min;
						auto y = j*bbox.x.max + (1-j)*bbox.y.min;
						auto z = k*bbox.x.max + (1-k)*bbox.z.min;

						auto newx = cos_theta*x + sin_theta*z;
						auto newz = -sin_theta*x + cos_theta*z;

						vec3 tester(newx, y, newz);

						for(int c=0; c<3; c++){
							min[c] = std::fmin(min[c], tester[c]);
							max[c] = std::fmax(max[c], tester[c]);
						}
					}
				}
			}
			bbox = aabb(min, max);
		}

		bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
			//Transform the ray from world space to object space
			auto origin = point3(
					(cos_theta * r.origin().x()) - (sin_theta * r.origin().z()),
					r.origin().y(),
					(sin_theta * r.origin().x()) + (cos_theta * r.origin().z())
					);

			auto direction = vec3(
					(cos_theta * r.direction().x()) -(sin_theta * r.direction().z()),
					r.direction().y(),
					(sin_theta * r.direction().x()) + (cos_theta * r.direction().z())
					);

			ray rotated_r(origin, direction, r.time());


			//Determine wheter an intersection exists in object space(and more details as in where)
			if(!object->hit(rotated_r, ray_t, rec))
				return false;

			//Transform the intersection from object space back to world space
			rec.p = point3(
					(cos_theta * rec.p.x()) + (sin_theta * rec.p.z()),
					rec.p.y(),
					(-sin_theta * rec.p.x()) + (cos_theta * rec.p.z())
					);

			rec.normal = vec3(
					(cos_theta * rec.normal.x()) + (sin_theta * rec.normal.z()),
					rec.normal.y(),
					(-sin_theta * rec.normal.z()) + (cos_theta * rec.normal.z())
					);

			return true;
		}

		aabb bounding_box() const override { return bbox; }

	private:
		shared_ptr<hittable> object;
		double sin_theta;
		double cos_theta;
		aabb bbox;
};

#endif
