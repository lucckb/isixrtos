/*
 * convflashbmp.cpp
 *
 *  Created on: 16-06-2013
 *      Author: lucck
 */
#include <CImg.h>
#include <cstddef>
#include <iostream>


typedef unsigned short color_t;
static inline constexpr color_t rgb( unsigned char R, unsigned char G, unsigned char B )
{
		return ((B>>3)<<(16-5)) | ((G>>2)<<(16-5-6) | (R>>3) );
}

int main()
{
	using namespace cimg_library;
	CImg<unsigned char> image("/tmp/mkeia.jpg");
	const auto width = image.width();
	const auto height = image.height();
	color_t *bmp = new color_t[width*height*sizeof(color_t)];
	color_t compressed[256];
	for( size_t x=0; x<width; ++x)
	for( size_t y=0,lp=0; y<height; ++lp,++y )
	{
		const auto R = image[image.offset( x,y,0,0 )];
		const auto G = image[image.offset( x,y,0,1 )];
		const auto B = image[image.offset( x,y,0,2 )];
		const auto color = rgb(R,G,B);
		bmp[lp] = color;
		if( lp == 127 )
		{
			//std::cout << Rice_Compress(bmp,compressed,lp*sizeof(color_t), RICE_FMT_INT16) << std::endl;
			lp = 0;
		}
		//std::cout << color << std::endl;
	}
	return 0;
}
