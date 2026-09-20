#ifndef QUAD_H
#define QUAD_H

#include "hittable.h"

class quad : public hittable{

	public:
		quad(const point3& Q, const vec3& u, const vec3& v, shared_ptr<material> mat) : Q(Q), u(u), v(v), mat(mat){
			auto n = cross(u, v);
			normal = unit_vector(n);
			D = dot(normal, Q);
			w = n/dot(n,n);
			set_bounding_box();
		}

		virtual void set_bounding_box() {
			//compute bounding box of all four vertices
			auto bbox_diagonal1 = aabb(Q, Q+u+v);
			auto bbox_diagonal2 = aabb(Q+u, Q+v);
			bbox = aabb(bbox_diagonal1, bbox_diagonal2);

		}

		aabb bounding_box() const override { return bbox; }

		bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
			auto denom = dot(normal, r.direction());

			if(std::fabs(denom)<1e-8)//ray doesnt hit the plane if its parallel to it
				return false;

			//no hit if the hit point is outside the interval
			auto t = (D - dot(normal, r.origin())) / denom;
			if(!ray_t.contains(t))
				return false;

			//determine if the ray hit the actual quadrilateral or just the plane containing it
			//finding the point of ray at the time of intersection with the plane
			auto intersection = r.at(t);
			vec3 planar_hitpt_vector = intersection - Q;
			//if P is the point of intersection of the ray at "t" on the plane containing the quad
			//P = Q + alpha*v + beta*u
			//calculating the alpha and beta here, which is like coordinates of the point on the quad, alpha and beta range from [0,1] , when both are 0,0 its bottom left of quad, 1,1 being top right of the quad
			auto alpha = dot(w, cross(planar_hitpt_vector, v));
			auto beta = dot(w, cross(u, planar_hitpt_vector));

			if(!is_interior(alpha, beta, rec))
				return false;

			rec.t = t;
			rec.p = intersection;
			rec.mat = mat;
			rec.set_face_normal(r, normal);

			return true;
		}

		virtual bool is_interior(double a, double b, hit_record& rec) const {
			interval unit_interval = interval(0, 1);

			if(!unit_interval.contains(a) || !unit_interval.contains(b))//if alpha / beta is not a unit vector, it means its outside the bounds of the quad
				return false;

			rec.u = a;//u is the x axis coordinate in the quad
			rec.v = b;//v is the y axis coordinate in the quad
			return true;
		}

	private:
		point3 Q;//point of the lower left vertex of the quadrilateral
		vec3 u, v; // u is vector (along with length) representing breath of the quadrilateral and v is the vector (along with length) representing length of the quadrilateral 
		shared_ptr<material> mat;
		aabb bbox;
		vec3 normal;
		double D;
		vec3 w;//used for clalculating hit()

};

#endif
