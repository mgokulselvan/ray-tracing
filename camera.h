#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"

class camera {
	public:
		/*public camera params*/
		double aspect_ratio = 1.0; //ratio of image -width over height
		int image_width = 100; //randered image width in pixel count
		int samples_per_pixel = 10; //how many rays to send through each pixel into the world
		int max_depth = 10; //max number of ray bounce, which controls the number of recursions when the ray hits a surface
		double vfov = 90; //vertical view angle(field of view)
		point3 lookfrom = point3(0,0,0);//point from where we are looking from (this is generally the camera position)
		point3 lookat = point3(0,0,-1);//point at which we are looking at
		vec3 vup = vec3(0,1,0);//vector pointing towards what is considered as up for the camera

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
					color pixel_color(0,0,0);

					for(int sample = 0; sample < samples_per_pixel; sample++) {
						ray r = get_ray(i,j);
						pixel_color+=ray_color(r,max_depth,world);
					}
					/*
					 * this is point sampling, which leads to aliasing, the jagged boundaries on our rendered images, in real life , this doesnt happen, because image is continues, and instead of harsh jaggies, we have smooth blend between foreground and background, we do this by sending multiples samples through a single pixel for all pixels on the image
					auto pixel_center = pixel00_loc + (i*pixel_delta_u) + (j*pixel_delta_v);
					auto ray_direction = pixel_center - center;//not unit vector, cuz we need the magnitude or something? later , in some function, i dont remember which, i have written so many , vectors are all over my brain 
					ray r(center, ray_direction);

					color pixel_color = ray_color(r,world);
					*/
					write_color(std::cout,pixel_samples_scale*pixel_color);
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
		double pixel_samples_scale; //color scale factor for a sum of pixel samples
		vec3 u, v, w;//u is vector pointing to the  right of the camera, w is vector pointing back direction of the camera from the point we are looking at, v is vector pointing to the up of the camera

		void initialize() {

			//Calculate the image height, and ensure that it's at least 1
			image_height = int(image_width / aspect_ratio);
			image_height = (image_height < 1) ? 1 : image_height;

			pixel_samples_scale = 1.0 / samples_per_pixel;

			auto focal_length = (lookfrom - lookat).length();
			auto theta = degrees_to_radians(vfov);//converting degrees to radians cuz tan function needs radians
			auto h = std::tan(theta/2);//we need this for calculating height
			auto viewport_height = 2 * h * focal_length;//we know the angle, tan of that angle, base length, to get the perpendicular length, we do tan(angle)*base_length(which is focal length) , we multiply this by 2 as a whole, because this only gives half the viewport height , above the camera, for below the camera, we need to add that once more, hence the multiplication by 2
			auto viewport_width = viewport_height * (double(image_width)/image_height);//using image width and height to calculate view port width instead of aspect ratio, because aspect ratio of image is not exact, due to type casting and checking if its <1

			center = lookfrom;//x -> leftRight , y-> upDown, z->direction of viewing
			w = unit_vector(lookfrom - lookat);
			u =  unit_vector(cross(vup, w));
			v = cross(w, u);//cross product produces vector perpendicular to both, its used to make both u and v vector, if you interchange the order, they produce opposite vectors

			//calculate vectors across the horizontal and down the vertical viewport edges.
			auto viewport_u = viewport_width * u;
			auto viewport_v = viewport_height * -v;

			//calculate the horizontal and vertical delta vectors from pixel to pixel.
			pixel_delta_u = viewport_u / image_width;
			pixel_delta_v = viewport_v / image_height;

			//calculate location of upper left corner of immage with using center as reference(as we have calculated it according to our camera position)
			auto viewport_upper_left = center - (focal_length * w) - viewport_u/2 - viewport_v/2;

			//calculate location of the upper left pixel which is the first pixel of image and hence also of that to be rendered
			pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

		}

		ray get_ray(int i, int j) const {
			//construct a ray originating from origin and directed at randomly sampled point around the pixel location given by i and j
			auto offset = sample_square();

			auto pixel_sample = pixel00_loc + ((i+offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);//previously since we only took middle of each pixel , we were doing pixel_loc +(i*pixel_delta_u) + (j*pixel_delta_v), now we are adding offset to that i and j, wihch can only be between -0.5 and 0.5, and hence it means, it will stay within the pixel

			auto ray_origin = center;
			auto ray_direction = pixel_sample - ray_origin;

			return ray(ray_origin, ray_direction);
		}

		vec3 sample_square() const {//returns offset between -0.5 to 0.5 for each dimension
			//Returns the vector to a random point in the [-0.5,-0.5] - [0.5,0.5] unit square
			return vec3(random_double() - 0.5, random_double() - 0.5, 0);
		}

		color ray_color(const ray& r,int depth, const hittable& world) const {

			if(depth <=0 )//if ray has bounced the max limit and still hasnt hit anything, we just consider it as black, pretty sure after these many bounces, all the energy has been absorbed by something anyway
				return color(0,0,0);

			hit_record rec;

			if (world.hit(r,interval(0.001,infinity),rec)){//using 0.001 instead of 0 , because a random generated scattered ray might not start flush from the surface of sphere, and might be a bit below or above the sphere surface(this happens because of floating point rounding errors , so its not accurately flush), if its below, then it immediately hits the sphere again, to avoid this , we are giving some min value of t that the t has to be greater than, why we avoid this? , because it keeps hitting within the sphere, and then the outside of the sphere from where this supposed ray inside sphere was a scattering from , gets a dark color, a black spot, as it keeps hitting within the walls of the sphere, this is knows as shadow acne
				ray scattered;
				color attenuation;
				if(rec.mat -> scatter(r, rec, attenuation, scattered))//if the ray hit some other surface, then this ray color is attenuation i.e. how much light it reflects in each color * ray color of ray of light from the surface that is coming to this spot from other place, (that is why we send a ray in that directoin getting to know the color of ray coming from that source, or surface, or whatever it is that the ray is coming from)
					return attenuation * ray_color(scattered, depth-1, world);
				return color(0,0,0);
/*
				//vec3 direction = random_on_hemisphere(rec.normal);
				vec3 direction = rec.normal + random_unit_vector();//this ensure lambertian spheres distribution which is close to how "diffuse" materials usually scatter light, rather than uniformly in any direction , its more likely , in direction near to the normal of the point of intersection
				return 0.5 * ray_color(ray(rec.p,direction), depth-1, world);//the 0.5 is for the color, a gray color, we are telling that gray absorbes only .5 of the light that hits it, (thats why its gray),(thats how physics work)
*/
			}

			vec3 unit_direction = unit_vector(r.direction());
			auto a=0.5*(unit_direction.y()+1.0);
			return (1.0-a)*color(1.0,1.0,1.0) + a*color(0.5,0.7,1.0);
		}
};

#endif
