/*
 * static_bitmap.hpp
 *
 *  Created on: 16-06-2013
 *      Author: lucck
 */

#ifndef STATIC_BITMAP_HPP_
#define STATIC_BITMAP_HPP_
 
namespace gfx {
namespace disp {
 
/* Flash memory bitmap */
struct cmem_bitmap_t
{
	enum img_type {
		bpp1, bgr332, rgb565, bgr565
	};
	const char *name;				/* Image name if presents */
	unsigned short width;			/* Image width */
	unsigned short height;			/* Image height */
	const void *img_data;			/* Image data */
	img_type type;
};
 
}
}

 
#endif /* STATIC_BITMAP_HPP_ */
