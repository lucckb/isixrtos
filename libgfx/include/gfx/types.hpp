/* ------------------------------------------------------------------ */
/*
 * types.hpp
 *
 *  Created on: 10-06-2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef ISIX_GFX_TYPES_HPP_
#define ISIX_GFX_TYPES_HPP_
/* ------------------------------------------------------------------ */
#include <cstdint>
/* ------------------------------------------------------------------ */
#define CONFIG_GFX_PIXEL_FORMAT_RGB565 1
#define CONFIG_GFX_PIXEL_FORMAT_BGR565 2
#define CONFIG_GFX_PIXEL_FORMAT_RGB8   3
#ifndef CONFIG_GFX_PIXEL_FORMAT
#define CONFIG_GFX_PIXEL_FORMAT CONFIG_GFX_PIXEL_FORMAT_RGB565

/* ------------------------------------------------------------------ */
#endif
/* ------------------------------------------------------------------ */
namespace gfx
{
#if CONFIG_GFX_PIXEL_FORMAT == CONFIG_GFX_PIXEL_FORMAT_RGB
	/* Basic color type */
	typedef unsigned int color_t;
#else
	typedef unsigned short color_t;
#endif
	/* Basic coordinate type */
	typedef unsigned short coord_t;
#if CONFIG_GFX_PIXEL_FORMAT == CONFIG_GFX_PIXEL_FORMAT_RGB565
	//RGB definition
	static inline constexpr color_t rgb( unsigned char R, unsigned char G, unsigned char B )
	{
		return ((B>>3)<<(16-5)) | ((G>>2)<<(16-5-6) | (R>>3) );
	}
#elif  CONFIG_GFX_PIXEL_FORMAT == CONFIG_GFX_PIXEL_FORMAT_BGR565
	static inline constexpr color_t rgb( unsigned char R, unsigned char G, unsigned char B )
	{
		return ((R>>3)<<(16-5)) | ((G>>2)<<(16-5-6) | (B>>3) );
	}
#elif  CONFIG_GFX_PIXEL_FORMAT == CONFIG_GFX_PIXEL_FORMAT_RGB8
	static inline constexpr color_t rgb( unsigned char R, unsigned char G, unsigned char B )
	{
		return (B<<16) | (G<<8) | R );
	}
#else
#error "Unknown CONFIG_GFX_PIXEL_FORMAT"
#endif
	//Error code
	enum error : int
	{
		error_ok,					/* Error OK */
		error_lz_compress,			/* LZMA compress failed */
		error_img_not_supported		/* Image type is not supported */
	};
	//Color space utilities
	namespace colorspace
	{
		inline color_t rgb565( uint16_t value )
		{
			return rgb( (value&0x1f)<<3, ((value>>5)&0x3f)<<2, ((value>>11)&0x1f)<<3 );
		}
		inline color_t bgr565( uint16_t value )
		{
			return rgb( ((value>>11)&0x1f)<<3, ((value>>5)&0x3f)<<2, (value&0x1f)<<3 );
		}
		inline color_t bgr332( uint8_t value )
		{
			return (value==0xff)?(rgb(255,255,255)):(rgb( (value&0x07)<<5, ((value>>3)&0x07)<<5, ((value>>6)&0x03)<<6 ));
		}
	}
}
/* ------------------------------------------------------------------ */
#endif /* TYPES_HPP_ */
