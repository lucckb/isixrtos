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
#define CONFIG_GFX_PIXEL_FORMAT_RGB565 1
#define CONFIG_GFX_PIXEL_FORMAT_BGR565 2
#define CONFIG_GFX_PIXEL_FORMAT_RGB8   3
#ifndef CONFIG_GFX_PIXEL_FORMAT
#define CONFIG_GFX_PIXEL_FORMAT CONFIG_GFX_PIXEL_FORMAT_RGB565
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
}
/* ------------------------------------------------------------------ */
#endif /* TYPES_HPP_ */
