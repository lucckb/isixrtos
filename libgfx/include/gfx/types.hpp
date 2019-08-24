 
/*
 * types.hpp
 *
 *  Created on: 10-06-2013
 *      Author: lucck
 */

#pragma once 
 
#include <cstdint>
#include <config/conf.h>
 
#define CONFIG_GFX_PIXEL_FORMAT_RGB565 1
#define CONFIG_GFX_PIXEL_FORMAT_BGR565 2
#define CONFIG_GFX_PIXEL_FORMAT_RGB8   3
#define CONFIG_GFX_PIXEL_FORMAT_BGR8   4

#ifndef CONFIG_GFX_PIXEL_FORMAT
#error Pixel format not defined
#endif
 
namespace gfx
{
#if CONFIG_GFX_PIXEL_FORMAT == CONFIG_GFX_PIXEL_FORMAT_RGB8
	/* Basic color type */
	typedef unsigned int color_t;
#elif CONFIG_GFX_PIXEL_FORMAT == CONFIG_GFX_PIXEL_FORMAT_BGR8
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
	static inline constexpr uint8_t color_t_R(color_t color)
	{
		return (color&0x1F) << 3;
	}
	static inline constexpr uint8_t color_t_G(color_t color)
	{
		return ((color>>(16-5-6))&0x3F) << 2;
	}
	static inline constexpr uint8_t color_t_B(color_t color)
	{
		return ((color>>(16-5))&0x1F) << 3;
	}
#elif  CONFIG_GFX_PIXEL_FORMAT == CONFIG_GFX_PIXEL_FORMAT_BGR565
	static inline constexpr color_t rgb( unsigned char R, unsigned char G, unsigned char B )
	{
		return ((R>>3)<<(16-5)) | ((G>>2)<<(16-5-6) | (B>>3) );
	}
	static inline constexpr uint8_t color_t_B(color_t color)
	{
		return (color&0x1F) << 3;
	}
	static inline constexpr uint8_t color_t_G(color_t color)
	{
		return ((color>>(16-5-6))&0x3F) << 2;
	}
	static inline constexpr uint8_t color_t_R(color_t color)
	{
		return ((color>>(16-5))&0x1F) << 3;
	}
#elif  CONFIG_GFX_PIXEL_FORMAT == CONFIG_GFX_PIXEL_FORMAT_RGB8
	static inline constexpr color_t rgb( unsigned char R, unsigned char G, unsigned char B )
	{
		return (B<<16) | (G<<8) | R;
	}
	static inline constexpr uint8_t color_t_R(color_t color)
	{
		return color&0xFF;
	}
	static inline constexpr uint8_t color_t_G(color_t color)
	{
		return (color>>8)&0xFF;
	}
	static inline constexpr uint8_t color_t_B(color_t color)
	{
		return (color>>16)&0xFF;
	}
#elif  CONFIG_GFX_PIXEL_FORMAT == CONFIG_GFX_PIXEL_FORMAT_BGR8
	static inline constexpr color_t rgb( unsigned char R, unsigned char G, unsigned char B )
	{
		return (B<<0) | (G<<8) | (R<<16);
	}
	static inline constexpr uint8_t color_t_R(color_t color)
	{
		return (color>>16)&0xFF;
	}
	static inline constexpr uint8_t color_t_G(color_t color)
	{
		return (color>>8)&0xFF;
	}
	static inline constexpr uint8_t color_t_B(color_t color)
	{
		return (color>>0)&0xFF;
	}

#else
#error "Unknown CONFIG_GFX_PIXEL_FORMAT"
#endif
	//Error code
	enum error : int
	{
		error_ok,					/* Error OK */
		error_lz_compress,			/* LZMA compress failed */
		error_img_not_supported,	/* Image type is not supported */
		error_not_supported         /* Syscall not supported */
	};
	//Color space utilities
	namespace colorspace
	{
		//! Convert to rgb565 format
		static inline constexpr color_t rgb565( uint16_t value )
		{
			return rgb( (value&0x1f)<<3, ((value>>5)&0x3f)<<2, ((value>>11)&0x1f)<<3 );
		}
		//! Convert to bgr565 format
		static inline constexpr color_t bgr565( uint16_t value )
		{
			return rgb( ((value>>11)&0x1f)<<3, ((value>>5)&0x3f)<<2, (value&0x1f)<<3 );
		}
		//! Convert to bgr332 format
		static inline constexpr color_t bgr332( uint8_t value )
		{
			return (value==0xff)?(rgb(255,255,255)):(rgb( (value&0x07)<<5, ((value>>3)&0x07)<<5, ((value>>6)&0x03)<<6 ));
		}
		//! Change brightness of color
		static inline color_t brigh( color_t color, int bright )
		{
			int r = color_t_R(color) + bright;
			int g = color_t_G(color) + bright;
			int b = color_t_B(color) + bright;
			if (r > 255) r = 255;
			else if(r < 0) r = 0;
			if (g > 255) g = 255;
			else if(g < 0) g = 0;
			if (b > 255) b = 255;
			else if(b < 0) b = 0;
			return rgb(r,g,b);
		}
		//! Convert color to luminance
		static inline constexpr uint8_t luminance( color_t color )
		{
			return ( color_t_R(color)*color_t_R(color) +
					 color_t_B(color) +
					 color_t_G(color)*color_t_G(color)*color_t_G(color)
				    );
		}
		//! Calculate average of two pixels format independent
		static inline constexpr color_t average( color_t color1, color_t color2 )
		{
			return rgb( ( int(color_t_R(color1)) + int(color_t_R(color2)) ) / 2,
						( int(color_t_G(color1)) + int(color_t_G(color2)) ) / 2,
						( int(color_t_B(color1)) + int(color_t_B(color2)) ) / 2
					  );
		}
	}
	//Standard html5 color defs
	struct color
	{
		enum	: color_t
		{
			AliceBlue  			=  rgb(0xF0,0xF8,0xFF),
			AntiqueWhite  		=  rgb(0xFA,0xEB,0xD7),
			Aqua  				=  rgb(0x00,0xFF,0xFF),
			Aquamarine  		=  rgb(0x7F,0xFF,0xD4),
			Azure  				=  rgb(0xF0,0xFF,0xFF),
			Beige  				=  rgb(0xF5,0xF5,0xDC),
			Bisque  			=  rgb(0xFF,0xE4,0xC4),
			Black  				=  rgb(0x00,0x00,0x00),
			BlanchedAlmond  	=  rgb(0xFF,0xEB,0xCD),
			Blue  				=  rgb(0x00,0x00,0xFF),
			BlueViolet  		=  rgb(0x8A,0x2B,0xE2),
			Brown  				=  rgb(0xA5,0x2A,0x2A),
			BurlyWood  			=  rgb(0xDE,0xB8,0x87),
			CadetBlue  			=  rgb(0x5F,0x9E,0xA0),
			Chartreuse  		=  rgb(0x7F,0xFF,0x00),
			Chocolate  			=  rgb(0xD2,0x69,0x1E),
			Coral  				=  rgb(0xFF,0x7F,0x50),
			CornflowerBlue  	=  rgb(0x64,0x95,0xED),
			Cornsilk  			=  rgb(0xFF,0xF8,0xDC),
			Crimson  			=  rgb(0xDC,0x14,0x3C),
			Cyan  				=  rgb(0x00,0xFF,0xFF),
			DarkBlue  			=  rgb(0x00,0x00,0x8B),
			DarkCyan  			=  rgb(0x00,0x8B,0x8B),
			DarkGoldenRod  		=  rgb(0xB8,0x86,0x0B),
			DarkGray  			=  rgb(0xA9,0xA9,0xA9),
			DarkGreen  			=  rgb(0x00,0x64,0x00),
			DarkKhaki  			=  rgb(0xBD,0xB7,0x6B),
			DarkMagenta  		=  rgb(0x8B,0x00,0x8B),
			DarkOliveGreen  	=  rgb(0x55,0x6B,0x2F),
			DarkOrange  		=  rgb(0xFF,0x8C,0x00),
			DarkOrchid  		=  rgb(0x99,0x32,0xCC),
			DarkRed  			=  rgb(0x8B,0x00,0x00),
			DarkSalmon  		=  rgb(0xE9,0x96,0x7A),
			DarkSeaGreen  		=  rgb(0x8F,0xBC,0x8F),
			DarkSlateBlue  		=  rgb(0x48,0x3D,0x8B),
			DarkSlateGray  		=  rgb(0x2F,0x4F,0x4F),
			DarkTurquoise  		=  rgb(0x00,0xCE,0xD1),
			DarkViolet  		=  rgb(0x94,0x00,0xD3),
			DeepPink  			=  rgb(0xFF,0x14,0x93),
			DeepSkyBlue  		=  rgb(0x00,0xBF,0xFF),
			DimGray  			=  rgb(0x69,0x69,0x69),
			DodgerBlue  		=  rgb(0x1E,0x90,0xFF),
			FireBrick  			=  rgb(0xB2,0x22,0x22),
			FloralWhite  		=  rgb(0xFF,0xFA,0xF0),
			ForestGreen  		=  rgb(0x22,0x8B,0x22),
			Fuchsia  			=  rgb(0xFF,0x00,0xFF),
			Gainsboro  			=  rgb(0xDC,0xDC,0xDC),
			GhostWhite  		=  rgb(0xF8,0xF8,0xFF),
			Gold  				=  rgb(0xFF,0xD7,0x00),
			GoldenRod  			=  rgb(0xDA,0xA5,0x20),
			Gray  				=  rgb(0x80,0x80,0x80),
			Green  				=  rgb(0x00,0x80,0x00),
			GreenYellow  		=  rgb(0xAD,0xFF,0x2F),
			HoneyDew  			=  rgb(0xF0,0xFF,0xF0),
			HotPink  			=  rgb(0xFF,0x69,0xB4),
			IndianRed   		=  rgb(0xCD,0x5C,0x5C),
			Indigo   			=  rgb(0x4B,0x00,0x82),
			Ivory  				=  rgb(0xFF,0xFF,0xF0),
			Khaki  				=  rgb(0xF0,0xE6,0x8C),
			Lavender  			=  rgb(0xE6,0xE6,0xFA),
			LavenderBlush  		=  rgb(0xFF,0xF0,0xF5),
			LawnGreen  			=  rgb(0x7C,0xFC,0x00),
			LemonChiffon  		=  rgb(0xFF,0xFA,0xCD),
			LightBlue  			=  rgb(0xAD,0xD8,0xE6),
			LightCoral  		=  rgb(0xF0,0x80,0x80),
			LightCyan  			=  rgb(0xE0,0xFF,0xFF),
			LightGoldenRodYellow=  rgb(0xFA,0xFA,0xD2),
			LightGray  			=  rgb(0xD3,0xD3,0xD3),
			LightGreen  		=  rgb(0x90,0xEE,0x90),
			LightPink  			=  rgb(0xFF,0xB6,0xC1),
			LightSalmon  		=  rgb(0xFF,0xA0,0x7A),
			LightSeaGreen  		=  rgb(0x20,0xB2,0xAA),
			LightSkyBlue  		=  rgb(0x87,0xCE,0xFA),
			LightSlateGray  	=  rgb(0x77,0x88,0x99),
			LightSteelBlue  	=  rgb(0xB0,0xC4,0xDE),
			LightYellow  		=  rgb(0xFF,0xFF,0xE0),
			Lime  				=  rgb(0x00,0xFF,0x00),
			LimeGreen  			=  rgb(0x32,0xCD,0x32),
			Linen  				=  rgb(0xFA,0xF0,0xE6),
			Magenta  			=  rgb(0xFF,0x00,0xFF),
			Maroon  			=  rgb(0x80,0x00,0x00),
			MediumAquaMarine  	=  rgb(0x66,0xCD,0xAA),
			MediumBlue  		=  rgb(0x00,0x00,0xCD),
			MediumOrchid  		=  rgb(0xBA,0x55,0xD3),
			MediumPurple  		=  rgb(0x93,0x70,0xDB),
			MediumSeaGreen  	=  rgb(0x3C,0xB3,0x71),
			MediumSlateBlue  	=  rgb(0x7B,0x68,0xEE),
			MediumSpringGreen  	=  rgb(0x00,0xFA,0x9A),
			MediumTurquoise  	=  rgb(0x48,0xD1,0xCC),
			MediumVioletRed  	=  rgb(0xC7,0x15,0x85),
			MidnightBlue  		=  rgb(0x19,0x19,0x70),
			MintCream  			=  rgb(0xF5,0xFF,0xFA),
			MistyRose  			=  rgb(0xFF,0xE4,0xE1),
			Moccasin  			=  rgb(0xFF,0xE4,0xB5),
			NavajoWhite  		=  rgb(0xFF,0xDE,0xAD),
			Navy  				=  rgb(0x00,0x00,0x80),
			OldLace  			=  rgb(0xFD,0xF5,0xE6),
			Olive  				=  rgb(0x80,0x80,0x00),
			OliveDrab  			=  rgb(0x6B,0x8E,0x23),
			Orange  			=  rgb(0xFF,0xA5,0x00),
			OrangeRed  			=  rgb(0xFF,0x45,0x00),
			Orchid  			=  rgb(0xDA,0x70,0xD6),
			PaleGoldenRod  		=  rgb(0xEE,0xE8,0xAA),
			PaleGreen  			=  rgb(0x98,0xFB,0x98),
			PaleTurquoise  		=  rgb(0xAF,0xEE,0xEE),
			PaleVioletRed  		=  rgb(0xDB,0x70,0x93),
			PapayaWhip  		=  rgb(0xFF,0xEF,0xD5),
			PeachPuff  			=  rgb(0xFF,0xDA,0xB9),
			Peru  				=  rgb(0xCD,0x85,0x3F),
			Pink  				=  rgb(0xFF,0xC0,0xCB),
			Plum  				=  rgb(0xDD,0xA0,0xDD),
			PowderBlue  		=  rgb(0xB0,0xE0,0xE6),
			Purple  			=  rgb(0x80,0x00,0x80),
			Red  				=  rgb(0xFF,0x00,0x00),
			RosyBrown  			=  rgb(0xBC,0x8F,0x8F),
			RoyalBlue  			=  rgb(0x41,0x69,0xE1),
			SaddleBrown  		=  rgb(0x8B,0x45,0x13),
			Salmon  			=  rgb(0xFA,0x80,0x72),
			SandyBrown  		=  rgb(0xF4,0xA4,0x60),
			SeaGreen  			=  rgb(0x2E,0x8B,0x57),
			SeaShell  			=  rgb(0xFF,0xF5,0xEE),
			Sienna  			=  rgb(0xA0,0x52,0x2D),
			Silver  			=  rgb(0xC0,0xC0,0xC0),
			SkyBlue  			=  rgb(0x87,0xCE,0xEB),
			SlateBlue  			=  rgb(0x6A,0x5A,0xCD),
			SlateGray  			=  rgb(0x70,0x80,0x90),
			Snow  				=  rgb(0xFF,0xFA,0xFA),
			SpringGreen  		=  rgb(0x00,0xFF,0x7F),
			SteelBlue  			=  rgb(0x46,0x82,0xB4),
			Tan  				=  rgb(0xD2,0xB4,0x8C),
			Teal  				=  rgb(0x00,0x80,0x80),
			Thistle  			=  rgb(0xD8,0xBF,0xD8),
			Tomato  			=  rgb(0xFF,0x63,0x47),
			Turquoise  			=  rgb(0x40,0xE0,0xD0),
			Violet  			=  rgb(0xEE,0x82,0xEE),
			Wheat  				=  rgb(0xF5,0xDE,0xB3),
			White  				=  rgb(0xFF,0xFF,0xFF),
			WhiteSmoke  		=  rgb(0xF5,0xF5,0xF5),
			Yellow  			=  rgb(0xFF,0xFF,0x00),
			YellowGreen  		=  rgb(0x9A,0xCD,0x32)
		};
	};
}
 
