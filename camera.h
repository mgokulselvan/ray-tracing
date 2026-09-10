#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"

class camera {
	public:
		/*public camera params*/
		double aspect_ratio = 1.0; //ratio of image -width over height
		int image_width = 100; //randered image width in pixel count

		void render(const hittable& world) {
			initialize();
			//Rendering
			std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
			/*
			   P3  <--says its a ppm image
			   256 256  <-- width and height (in pixels)
			   255 <-- max intensity of RGB each
			 *image starts here*
			 */

			for(int j = 0; j < image_height; j++){
				std::clog << "\rScanlines remaining: " <<(image_height - j) <<  ' ' << std:: flush;
				for(int i  = 0;i < image_width;i++){
					auto pixel_center = pixel00_loc + (i*pixel_delta_u) + (j*pixel_delta_v);
					auto ray_direction = pixel_center - center;//not unit vector, cuz we need the magnitude or something? later , in some function, i dont remember which, i have written so many , vectors are all over my brain 
					ray r(center, ray_direction);

					color pixel_color = ray_color(r,world);
					write_color(std::cout,pixel_color);
				}
			}
			std::clog << "\rDone.                                 \n";


		}

	private:
		/* private cam param*/
		int image_height; //calculated using image_width and aspect_ratio
		point3 center; //camera center
		point3 pixel00_loc; //location of pixel 0,0
		vec3 pixel_delta_u; //distance from one pixel center to another sidewise (to the right)
		vec3 pixel_delta_v; //distance from one pixel center to another vertically (below)

		void initialize() {

			//Calculate the image height, and ensure that it's at least 1
			image_height = int(image_width / aspect_ratio);
			image_height = (image_height < 1) ? 1 : image_height;

			auto focal_length = 1.0;
			auto viewport_height = 2.0;
			auto viewport_width = viewport_height * (double(image_width)/image_height);//using image width and height to calculate view port width instead of aspect ratio, because aspect ratio of image is not exact, due to type casting and checking if its <1
																					   //
			center = point3(0,0,0);//x -> leftRight , y-> upDown, z->direction of viewing

			//calculate vectors across the horizontal and down the vertical viewport edges.
			auto viewport_u = vec3(viewport_width, 0,0);
			auto viewport_v = vec3(0,-viewport_height, 0);

			//calculate the horizontal and vertical delta vectors from pixel to pixel.
			pixel_delta_u = viewport_u / image_width;
			pixel_delta_v = viewport_v / image_height;

			//calculate location of upper left corner of immage with using center as reference(as we have calculated it according to our camera position)
			auto viewport_upper_left = center - vec3(0,0,focal_length) - viewport_u/2 - viewport_v/2;

			//calculate location of the upper left pixel which is the first pixel of image and hence also of that to be rendered
			pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);



		}

		color ray_color(const ray& r,const hittable& world) const {
			hit_record rec;

			if (world.hit(r,interval(0,infinity),rec)){
				return 0.5 * (rec.normal + color(1,1,1));
			}

			vec3 unit_direction = unit_vector(r.direction());
			auto a=0.5*(unit_direction.y()+1.0);
			return (1.0-a)*color(1.0,1.0,1.0) + a*color(0.5,0.7,1.0);
		}
};

#endif
