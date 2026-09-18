#ifndef TEXTURE_H
#define TEXTURE_H
#include "rtweekend.h"

class texture {
	public:
		virtual ~texture() = default;//default deconstructor, (asking cpp to do it itself)

		virtual color value(double u, double v, const point3& p) const = 0;
};

class solid_color : public texture {//all points on the objects of this texture are of same color
	public:
		solid_color(const color& albedo): albedo(albedo) {}

		solid_color(double red, double green, double blue) : solid_color(color(red,green,blue)) {}//calling another constructor

		color value(double u, double v, const point3& p) const override{//what is the color of this texture
			return albedo;
		}

	private:
		color albedo;
};

class checker_texture : public texture {//this is a solid(or spatial) texture i.e. the color depends on the 3d coordinates, it colors the space itself, instead of the surface of the object
	public:
		checker_texture(double scale, shared_ptr<texture> even, shared_ptr<texture> odd) : inv_scale(1.0/scale), even(even), odd(odd) {}

		checker_texture(double scale, const color& c1, const color& c2) : checker_texture(scale,make_shared<solid_color>(c1), make_shared<solid_color>(c2)) {}

		color value(double u, double v, const point3& p) const override {//the color based on the position(cuz its checkered)
			auto xInteger = int(std::floor(inv_scale * p.x()));
			auto yInteger = int(std::floor(inv_scale * p.y()));
			auto zInteger = int(std::floor(inv_scale * p.z()));

			bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

			return isEven ? even->value(u,v,p): odd->value(u,v,p);
		}

	private:
		double inv_scale;// scale tells how big the boxes in checker_texture be, inv_scale is inverse of scale, this is used to scale the position of the point on the object, and based on it,the color is assigned (using isEven) ,and indirectly this makes it so that the range in which isEven is True or False changes , and because of this the size of boxes differ
		shared_ptr<texture> even;
		shared_ptr<texture> odd;
};

#endif
