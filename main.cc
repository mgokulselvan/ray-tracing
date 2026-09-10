#include "rtweekend.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"

//PPM - R G B as values form 0 to 255 , \n at the end of a line, a group of 3 numbers make one pixel
int main(){
	
	// Za Warudo 
	hittable_list world;//world of "ray" hittable objects
	world.add(make_shared<sphere>(point3(0,0,-1),0.5));
	//world.add(make_shared<sphere>(point3(-0.5,0,-1),0.5));
	//world.add(make_shared<sphere>(point3(0.5,0,-1),0.5));
	world.add(make_shared<sphere>(point3(0,-100.5,-1),100));
	//world.add(make_shared<sphere>(point3(0,100.5,1),100));

	camera cam;

	cam.aspect_ratio = 16.0 / 9.0;
	cam.image_width = 400;

	cam.render(world);
}
