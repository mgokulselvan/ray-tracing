#ifndef CONSTANT_MEDIUM_H
#define CONSTANT_MEDIUM_H

#include "hittable.h"
#include "material.h"
#include "texture.h"


class constant_medium : public hittable {
	public:
		constant_medium(shared_ptr<hittable> boundary, double density, shared_ptr<texture> tex) :
			boundary(boundary), neg_inv_density(-1/density), phase_function(make_shared<isotropic>(tex)){}

		constant_medium(shared_ptr<hittable> boundary, double density, const color& albedo) : boundary(boundary), neg_inv_density(-1/density) , phase_function(make_shared<isotropic>(albedo)) {}

		bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
			hit_record rec1, rec2;

			if(!boundary->hit(r, interval::universe, rec1))//we get the first hit point by just sending it to the hit function of the object which represents our boundary
				return false;

			if(!boundary->hit(r, interval(rec1.t+0.0001, infinity), rec2))//we get the second hit point , by taking the interval as starting from just in front of the 1st hit point
				return false;


			if(rec1.t < ray_t.min) rec1.t = ray_t.min;//making sure the hit points are within the permissable intervals
			if(rec2.t > ray_t.max) rec2.t = ray_t.max;

			if(rec1.t >= rec2.t)//if first hit point is somehow greater or equal to second one, it is just wrong
				return false;

			if(rec1.t < 0)
				rec1.t = 0;

			auto ray_length = r.direction().length();
			auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;//how much distance is between the two hit points i.e. how long the ray is within the medium 
			auto hit_distance = neg_inv_density * std::log(random_double());//distance by how much the ray travells in the constant medium before scattering

			if(hit_distance > distance_inside_boundary)
				return false;

			rec.t = rec1.t + hit_distance / ray_length;//rec is the record of the current scattering, not the boundary or anything, but the actual one thats happening in the world
			rec.p = r.at(rec.t);//storing the position

			rec.normal = vec3(1, 0, 0); //taking an arbitrary normal because in constant_medium like fog, the normal isnt set, and is some arbitrary direction
			rec.front_face = true;//arbitrary
			rec.mat = phase_function;//its an isotropic material

			return true;
		}

		aabb bounding_box() const override { return boundary -> bounding_box(); }

	private:
		shared_ptr<hittable> boundary;
		double neg_inv_density;
		shared_ptr<material> phase_function;
};


#endif
