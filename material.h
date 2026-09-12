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
		metal(const color& albedo) : albedo(albedo) {}

		bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
			vec3 reflected = reflect(r_in.direction(), rec.normal);
			scattered = ray(rec.p, reflected);
			attenuation = albedo;
			return true;
		}
	private:
		color albedo;
};

#endif
