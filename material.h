#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"
//the material class needs the time only because it creates a ray and sends it back to the one which called it, not really send it back , but its reference and it puts it to that and the calling function uses it and bla bla bla, so you know what im saying, the scatter logic doesnt depend on time, because, the scatter function only gets what ray hits it, angle and stuff

//think of concept of time as such: at time=0 the position of objects is where it would be when the camera shutter opens
//and at time = 1,the position of the objects is where it would be just before the camera shutter closes
//that is why t=0 to 1 is enough
class material {
	public:
		virtual ~material() = default;

		virtual color emitted(double u, double v, const point3& p) const {//overridable by light sources so that they give actual color instead of black
			return color(0,0,0);
		}

		virtual bool scatter(
				const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
				) const {
			return false;
		}
};

class lambertian : public material {//for the diffuse ( matte like) surfaces
	public:
		lambertian(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
		lambertian(shared_ptr<texture> tex) : tex(tex) {}

		bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
			auto scatter_direction = rec.normal + random_unit_vector();//this is just lambertian using spheres and their kind of distribution

			//catch degenerate scatter direction
			if(scatter_direction.near_zero())
				scatter_direction = rec.normal;//if scatter direction is 0 ,it means light goes nowhere, which doesnt make sense, so we just replace it with the normal at the point of intersection

			scattered = ray(rec.p, scatter_direction, r_in.time());//we have to even use time because we need rays position, because the scattering depends on when it was hit, because the object is at different position at different time

			//because of texture, the light absorbed by that point depends on what color that region is, which is given by the texture
			attenuation = tex->value(rec.u, rec.v, rec.p); //attenuation tells how much light's intensity decreases , but how is that equal to albedo?
								  //the ray we shoot goes in reverse manner as to how it acts in real life, so when you think about it, when light comes , it hits on this surface, we multiply it with albedo(which is attenuatoin) , so if red light ,only 80% gets absorbed, we multiply red component of the light with 0.80(or whatever it is) and then that comes to our camera, hence, it means, red is 80% of the original light intensity of the original light source
			return true;
		}
	private:
		shared_ptr<texture> tex;
		//its a texture, which tells the albedo of the object at different point in space
		//this tells how much light the material captures, or absorbs, which if you know physics, you will know that, this directly attributes to how it looks like in terms of color, if it absorbs more green light, it will look purple, if it absorbs , red and blue fully it will look green
};

class metal : public material {
	public:
		metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}//we chose range 0-1 for fuzz, 0 being not fuzzy at all, 1 being maximum fuzz

		bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
			vec3 reflected = reflect(r_in.direction(), rec.normal);
			reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
			scattered = ray(rec.p, reflected, r_in.time());//first we created normal reflected ray, then create a random unit vector, ie just choose a random direction, and the fuzz param decides how much this random direction decides the final scattered or reflected array, more the fuzz, more the affect of this random direction on the scattered/reflected light more the fuzz, if less,if 0 , this random direction gets converted to 0, and adding 0 to reflected ray doesnt make a difference, so it is as if it didnt fuzz at all, and is fully shiny unfuzzy 
		   //we have to even use time because we need rays position, because the scattering depends on when it was hit, because the object is at different position at different time
			attenuation = albedo; //how much of the light incident on it , it reflects
			return (dot(scattered.direction(), rec.normal) > 0);//if the direction of the scattered light is toward the sphere, we just cancel it saying reflection didnt happen, if it went in direction of the normal at the point of sphere where ray hits, we say that reflection did happen, and calculate the next place where ray went to 
		}
	private:
		color albedo;
		double fuzz;
};

class dielectric : public material {//the one that refracts
	public:
		dielectric(double refraction_index) : refraction_index(refraction_index) {}
	
		bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
			attenuation = color(1.0,1.0,1.0);
			double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

			vec3 unit_direction = unit_vector(r_in.direction());
			double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
			double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

			bool cannot_refract = ri * sin_theta > 1.0;
			vec3 direction;

			if(cannot_refract || reflectance(cos_theta, ri) > random_double())//when its more than acceptance angle, we find this out, by seeing if ri * sin_theta > 1.0, because thats what gives us the angle of refraction, if its > 1.0 then it means it isnt refraction, because sin_theta(here angle of refraction) = ri* sin_theta(angle of incidence) , and it cant be more than 1
				direction = reflect(unit_direction, rec.normal);
			else
				direction = refract(unit_direction, rec.normal, ri);


			scattered = ray(rec.p, direction, r_in.time());
		   //we have to even use time because we need rays position, because the scattering depends on when it was hit, because the object is at different position at different time
			return true;
		}
	private:
		double refraction_index;
		static double reflectance(double cosine, double refraction_index) {
			//schlick's approximation for reflectance
			auto r0 = (1 - refraction_index) / (1 + refraction_index);
			r0 = r0 * r0;
			return r0 + (1 - r0)*std::pow((1-cosine), 5);
		}
};

class diffuse_light : public material {
	public:

		diffuse_light(shared_ptr<texture> tex) : tex(tex) {}//if the emitting source has some specific texture i.e. like different coloured light at different points of the object

		diffuse_light(const color& emit) : tex(make_shared<solid_color>(emit)) {}//creating a solid color texture for sources which are emitting just one colored light

		color emitted(double u, double v, const point3& p) const override {//u and v are the texture mapping coordinates, i.e. 2d coordinates that tell the position of the point within the object(not the 3d vector space, but the object), and if we just refer to its color in the texture , it returns the color at that point in the texture which is the light source color
			return tex->value(u, v, p);
		}

	private:
		shared_ptr<texture> tex;
};

class isotropic : public material {
	public: 
		isotropic(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
		isotropic(shared_ptr<texture> tex) : tex(tex) {}

		bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
			scattered = ray(rec.p, random_unit_vector(), r_in.time());//scatter in a random direction
			attenuation = tex->value(rec.u, rec.v, rec.p);//attenuation is just from its texture
			return true;
		}

	private:
		shared_ptr<texture> tex;
};

#endif
