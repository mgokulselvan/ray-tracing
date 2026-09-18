#ifndef RTW_STB_IMAGE_H
#define RTW_STB_IMAGE_H

//disable strict warnings for this header file from microsoft visual cpp compiler
#ifndef _MSC_VER
	#pragma warning (push , 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "external/stb_image.h"

#include <cstdlib>
#include <iostream>


class rtw_image {
	public:
		rtw_image() {}

		rtw_image(const char* image_filename) {
			//loads image data form the specified file
			//if the RTW_IMAGES env is defined, this looks only in that directory for the image file.
			//if image was not found, searches for spcified image file first in current directory, then in the images/ subdirectory, then in the parent's images/ subdirectory, ad then its parent's images/ subdirectory and so on for six levels up. If image wasnt loaded successfully, width() and height() function calls will return 0

			auto filename = std::string(image_filename);
			auto imagedir = getenv("RTW_IMAGES");

			//search for the image file
			if(imagedir && load(std::string(imagedir)+"/"+image_filename)) return;
			if(load(filename)) return;
			if(load("images/"+filename))return;
			if(load("../images/"+filename))return;
			if(load("../../images/"+filename))return;
			if(load("../../../images/"+filename))return;
			if(load("../../../../images/"+filename))return;
			if(load("../../../../../images/"+filename))return;
			if(load("../../../../../../images/"+filename))return;

			std::cerr << "ERROR: Could not load image file '" << image_filename << "'.\n";
		}


		~rtw_image() {
			delete[] bdata;
			STBI_FREE(fdata);
		}


		bool load(const std::string& filename){
			//loads the linear space (gamma = 1) image data from the given file name.
			//returns True if succeeded
			//data buffer contains three [0.0, 1.0] floating-point values for red, green and blue, pixels are contiguous, going left to right for the width of image, followed by next row below, from top to bottom

			auto n = bytes_per_pixel;//how many bytes per pixel : R G B, each being 1 byte, therefore 3 bytes per pixel
			fdata = stbi_loadf(filename.c_str(), &image_width, &image_height, &n, bytes_per_pixel);
			if(fdata == nullptr) return false;

			bytes_per_scanline = image_width * bytes_per_pixel;//scanline is just one horizontal line in the image
			convert_to_bytes();
			return true;
		}


		int width() const { return (fdata == nullptr) ? 0 : image_width; }
		int height() const { return (fdata == nullptr) ? 0 : image_height; }

		const unsigned char* pixel_data(int x, int y) const {//this class contains an array containing elements which range from 0-255 each represent intensity of R/G/B of a pixel i.e. in the array 
															 // index 0 -> R of 1st pixel
															 // index 1 -> G of 1st pixel
															 // index 2 -> B of 1st pixel
															 // index 3 -> R of 2nd pixel
															 // index 4 -> G of 2nd pixel
															 // index 5 -> B of 2nd pixel
															 // and so on
			//return address of the rgb bytes of pixel at x,y . Returns magenta if no image data
			static unsigned char magenta[] = {255, 0, 255};
			if(bdata == nullptr) return magenta;

			x = clamp(x, 0, image_width);//if the user asks for some pixel doesnt even exist, so this check makes sure that the function doesnt try to access something out of the array
			y = clamp(y, 0, image_height);

			return bdata + y*bytes_per_scanline + x*bytes_per_pixel;//for 2nd scanline 1st pixel -> for width 1024 , the R, G, B is stored in array index 1*(1024*3)+ 0*3 th index
		}

	private:
		const int bytes_per_pixel = 3;//1 byte for R, G, B each
		float *fdata = nullptr;
		unsigned char *bdata = nullptr;
		int image_width = 0;
		int image_height = 0;
		int bytes_per_scanline = 0;

		static int clamp(int x, int low, int high) {
			//return the value clamped to the range [low , high)
			if (x<low) return low;
			if(x<high) return x;
			return high - 1;
		}

		static unsigned char float_to_byte(float value) {//converts the 0-1 data returned by the function loading the image to 0-255 range
			if(value <= 0.0)
				return 0;
			if(1.0 <= value)
				return 255;
			return static_cast<unsigned char>(256.0 * value);
		}

		void convert_to_bytes() {
			// convert the linear floating point pixel data to bytes, storing the resulting byte data in "bdata" member
			int total_bytes = image_width * image_height * bytes_per_pixel;
			bdata = new unsigned char[total_bytes];

			//Iterate through all pixel components, converting from [0.0, 1.0] float values to unsigned [0, 255] byte values

			auto *bptr = bdata;//stores the 0-255 for each R, G, B, for all pixels as one linear array
			auto *fptr = fdata;//contains the floating value (0 to 1) which we convert to bytes(0-255) and store in the above array
			for(auto i=0;i<total_bytes; i++, fptr++, bptr++)//just means to convert the data for all the pixels
				*bptr = float_to_byte(*fptr);
		}
};
//enable the warnings
#ifdef _MSC_VER
	#pragma warning (pop)
#endif

#endif
