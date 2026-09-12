#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"

class material {
	public:
		virtual ~material() = default;

		virtual bool scatter(
				const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
				) const {
			return false;
		}
};

class lambertian : public material {//for the diffuse ( matte like) surfaces
	public:
		lambertian(const color& albedo) : albedo(albedo) {}

		bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
			auto scatter_direction = rec.normal + random_unit_vector();//this is just lambertian using spheres and their kind of distribution

			//catch degenerate scatter direction
			if(scatter_direction.near_zero())
				scatter_direction = rec.normal;//if scatter direction is 0 ,it means light goes nowhere, which doesnt make sense, so we just replace it with the normal at the point of intersection

			scattered = ray(rec.p, scatter_direction);
			attenuation = albedo; //attenuation tells how much light's intensity decreases , but how is that equal to albedo?
								  //the ray we shoot goes in reverse manner as to how it acts in real life, so when you think about it, when light comes , it hits on this surface, we multiply it with albedo(which is attenuatoin) , so if red light ,only 80% gets absorbed, we multiply red component of the light with 0.80(or whatever it is) and then that comes to our camera, hence, it means, red is 80% of the original light intensity of the original light source
			return true;
		}
	private:
		color albedo;//this tells how much light the material captures, or absorbs, which if you know physics, you will know that, this directly attributes to how it looks like in terms of color, if it absorbs more green light, it will look purple, if it absorbs , red and blue fully it will look green
};

class metal : public material {
	public:
		metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}//we chose range 0-1 for fuzz, 0 being not fuzzy at all, 1 being maximum fuzz

		bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
			vec3 reflected = reflect(r_in.direction(), rec.normal);
			reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
			scattered = ray(rec.p, reflected);//first we created normal reflected ray, then create a random unit vector, ie just choose a random direction, and the fuzz param decides how much this random direction decides the final scattered or reflected array, more the fuzz, more the affect of this random direction on the scattered/reflected light more the fuzz, if less,if 0 , this random direction gets converted to 0, and adding 0 to reflected ray doesnt make a difference, so it is as if it didnt fuzz at all, and is fully shiny unfuzzy 
			attenuation = albedo; //how much of the light incident on it , it reflects
			return (dot(scattered.direction(), rec.normal) > 0);//if the direction of the scattered light is toward the sphere, we just cancel it saying reflection didnt happen, if it went in direction of the normal at the point of sphere where ray hits, we say that reflection did happen, and calculate the next place where ray went to 
		}
	private:
		color albedo;
		double fuzz;
};

class dielectric : public material {
	public:
		dielectric(double refraction_index) : refraction_index(refraction_index) {}
	
		bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
			attenuation = color(1.0,1.0,1.0);
			double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

			vec3 unit_direction = unit_vector(r_in.direction());
			vec3 refracted = refract(unit_direction, rec.normal, ri);

			scattered = ray(rec.p, refracted);
			return true;
		}
	private:
		double refraction_index;
};

#endif
