#include "rtweekend.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "material.h"

//PPM - R G B as values form 0 to 255 , \n at the end of a line, a group of 3 numbers make one pixel
int main(){

	// Za Warudo 
	hittable_list world;//world of "ray" hittable objects
	
	auto material_ground = make_shared<lambertian>(color(0.8,0.8,0.0));
	auto material_center = make_shared<lambertian>(color(0.1,0.2,0.5));
	auto material_left = make_shared<metal>(color(0.8,0.8,0.8));
	auto material_right = make_shared<metal>(color(0.8,0.6,0.2));

	world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
	world.add(make_shared<sphere>(point3(0.0, 0.0, -1.2), 0.5, material_center));
	world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
	world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));

	camera cam;

	cam.aspect_ratio = 16.0 / 9.0;
	cam.image_width = 1920;
	cam.samples_per_pixel = 1000;
	cam.max_depth = 40;

	cam.render(world);
}
