#ifndef SPHERE_H
#define SPHERE_H


#include "hittable.h"
#include "vec3.h"


class sphere : public hittable {//this sphere is hittable i.e. ray can intersect it,which is required if we have to render it on our image
	public:
		//Stationary sphere, stays one place for time =0 to 1
		sphere(const point3& static_center, double radius, shared_ptr<material> mat) : center(static_center, vec3(0,0,0)), radius(std::fmax(0,radius)), mat(mat) {}
		//moving sphere , different centers at diffeetern time 
		sphere(const point3& center1, const point3& center2, double radius, shared_ptr<material> mat) : center(center1, center2-center1), radius(std::fmax(0,radius)), mat(mat) {}

		bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
			//the below code is a math heavy optimisation of sphere equation , and tells if a point given is in the sphere, on the sphere, or no way near it, which helps us render the sphere in the world among the list of objects
			point3 current_center = center.at(r.time());
			vec3 oc = current_center - r.origin();
			auto a = r.direction().length_squared();
			auto h = dot(r.direction(),oc);
			auto c = oc.length_squared() - radius*radius;
			
			auto discriminant = h*h - a*c;
			if (discriminant < 0)
				return false;

			auto sqrtd = std::sqrt(discriminant);

			//Find the nearest root that lies in teh acceptable range ( tmax and tmin)
			auto root = (h-sqrtd)/a;
			if(!ray_t.surrounds(root)){
				root = (h+sqrtd) / a;//if one point doesnt come inside the desired range , what if the other point comes,thats what we are doing over here
				if(!ray_t.surrounds(root))
					return false;
			}

			//if the code ever runs till here ,it means that......
			//its hitting!, its hitting!
			//so we are storing all about it in our records, this is given by the function call(if you remember its in the hittable_list class , and that gives us the record to here, and records the one which is closest , so yea, this is what enables it to do so)
			rec.t = root;
			rec.p = r.at(rec.t);
			//now normal depends on current center, which just means center at that time, now the objects have time as a parameter to find the position of the object in the vector 3d space at that time which is needed to find how ray bounces off/ refracts from surface
			rec.normal = (rec.p - current_center) / radius;
			vec3 outward_normal = (rec.p - current_center) / radius; // /radius to make it an unit vector, we use radius because some genius figured out that to make unit vector instead of doing square root which take so much compute, we use this radius fella, which apparently gives us same answer, how? , unfortunately im not the genius who came up with this math
			rec.set_face_normal(r,outward_normal);//asking the record to record if this ray is inside the sphere, or outside(if its inside it means its the back surface, if its outside its the face of the object)
			rec.mat = mat;

			return true;
		}

	private:
		//point3 center;
		ray center;//center is now a ray instead of point, because depending on the t value of this center ray, the sphere can exist at different places
		double radius;
		shared_ptr<material> mat;
};

#endif
