#include "color.h"
#include "vec3.h"

#include<iostream>

//PPM - R G B as values form 0 to 255 , \n at the end of a line, a group of 3 numbers make one pixel
int main(){

	//Image
	int image_width = 256;
	int image_height = 256;

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

			auto pixel_color = color(double(i)/(image_width-1), double(j)/(image_height-1), 0);
			write_color(std::cout,pixel_color);
		}
	}
	std::clog << "\rDone.                                 \n";
}
