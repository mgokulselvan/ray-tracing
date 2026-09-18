#ifndef SPHERE_H
#define SPHERE_H


#include "hittable.h"
#include "vec3.h"


class sphere : public hittable {//this sphere is hittable i.e. ray can intersect it,which is required if we have to render it on our image
	public:
		//Stationary sphere, stays one place for time =0 to 1
		sphere(const point3& static_center, double radius, shared_ptr<material> mat) : center(static_center, vec3(0,0,0)), radius(std::fmax(0,radius)), mat(mat) {
			auto rvec = vec3(radius, radius, radius);
			bbox = aabb(static_center - rvec, static_center + rvec);
			//so basically its bounding box range is 
			//center + go up by radius units, right by radius units and backward (in terms of camera/depth/just +z) by radius units
			//and
			//center + go down by radius units, left by radius units and forward (in terms of camera/depth/just +z) by radius units
			//this forms a 3d box, such that point are at the 2 opposite corners of the sphere
		}
		//moving sphere , different centers at diffeetern time 
		sphere(const point3& center1, const point3& center2, double radius, shared_ptr<material> mat) : center(center1, center2-center1), radius(std::fmax(0,radius)), mat(mat) {
			auto rvec = vec3(radius, radius, radius);
			aabb box1(center.at(0) - rvec, center.at(0) + rvec);//center at time = 0 , and finding the bounding box for the sphere at that center
			aabb box2(center.at(1) - rvec, center.at(1) + rvec);//center at time = 1, and finding the bounding box for the sphere at that center
			//time here is only from time=0 to 1
			//think of it as at time=0 the position of objects is where it would be when the camera shutter opens
			//and at time = 1,the position of the objects is where it would be just before the camera shutter closes
			//that is why t=0 to 1 is enough
			bbox = aabb(box1, box2);


		}

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
			//rec.normal = (rec.p - current_center) / radius; redundant
			vec3 outward_normal = (rec.p - current_center) / radius; // /radius to make it an unit vector, we use radius because some genius figured out that to make unit vector instead of doing square root which take so much compute, we use this radius fella, which apparently gives us same answer, how? , unfortunately im not the genius who came up with this math
			rec.set_face_normal(r,outward_normal);//asking the record to record if this ray is inside the sphere, or outside(if its inside it means its the back surface, if its outside its the face of the object)
			rec.mat = mat;
			get_sphere_uv(outward_normal, rec.u, rec.v);//store in the record, the u,v coordinates , which we need so that we can lookup what color was on that 2D coordinate and using that, simulate light and its "physics" accuratly according to the texture of the object

			return true;
		}

		aabb bounding_box() const override { return bbox; }
		
	private:
		//point3 center;
		ray center;//center is now a ray instead of point, because depending on the t value of this center ray, the sphere can exist at different places
		double radius;
		shared_ptr<material> mat;
		aabb bbox;

		static void get_sphere_uv(const point3& p, double& u, double& v){//function that maps the 3d coordinates of a sphere to (u,v) which are 2D coordinates , so basically , 3d position of point to 2d coordinates that indicates the position of the point on the sphere if it was made into a 2d surface
			//p is the point on the sphere
			// u is the normalized angular position around the sphere
			//    (how far around from the start of the texture the point is)
			// v is the normalized angular position from bottom to top
			//    (how far from the bottom to the top of the texture the point is)

			auto theta = std::acos(-p.y());
			auto phi = std::atan2(-p.z(), p.x()) + pi;

			u = phi / (2*pi);
			v = theta / pi;
		}
};

#endif
