#ifndef COLOR_H
#define COLOR_H

#include "interval.h"
#include "vec3.h"


using color = vec3;

//images store colors in gamma space, some weird space, but what we are calculating is linear space, so we are using this function to just uk, map or transform whatever physically accurate mathematically accurate colors we got here to the gamma space, why? even tho the light intensity and color part are correct!, its because thats how stupid computers work, they are sassy and want it in their own preferred format, to accurately display that we actually want to display and what the physics and maths actually result to, its just that we are converting it form one unit to another, we arent modifying anything, just making it computer compatible from physics and math compatible 
inline double linear_to_gamma(double linear_component){
	if(linear_component > 0)
		return std::sqrt(linear_component);
	return 0;
}

void write_color(std::ostream& out,const color& pixel_color) {//writes ONE pixel to whatever standard output configured
	auto r = pixel_color.x();
	auto g = pixel_color.y();
	auto b = pixel_color.z();

	//apply linear to gamma transform from gamma 2
	r = linear_to_gamma(r);
	g = linear_to_gamma(g);
	b = linear_to_gamma(b);


	//Translate [0,1] component values to byte range [0.255]
	static const interval intensity(0.000,0.999);
	int rbyte = int(255.999 * intensity.clamp(r));
	int gbyte = int(255.999 * intensity.clamp(g));
	int bbyte = int(255.999 * intensity.clamp(b));

	//write out pixel color components
	out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';//this is how color pixel is shown in .ppm formax (Portable PixMap)

}
#endif
