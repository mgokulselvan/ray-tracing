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
		for(int i  = 0;i < image_width;i++){
			auto r = double(i) / (image_width-1);//basically goes slowly from 0 to 1 (it acts like percent - 0 to 100)(i am very proud of this, i figured it out on my own after looking at the image)
			auto b = double(j) / (image_height-1);
			auto g = 0.0;

			int ir = int(255.999 * r);
			int ig = int(255.999 * g);
			int ib = int(255.999 * b);


			std::cout << ir << ' ' << ig << ' ' << ib << '\n';
		}
	}

}
