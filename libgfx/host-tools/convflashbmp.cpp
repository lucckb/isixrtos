/*
 * convflashbmp.cpp
 *
 *  Created on: 16-06-2013
 *      Author: lucck
 */

#include <cstddef>
#include <iostream>
#include <IL/il.h>
#include <IL/ilu.h>
#include <stdexcept>
#include "lz.h"
/* ----------------------------------------------------------------------------- */
inline void il_abort( ILboolean error )
{
    if( !error )
    {
        std::cerr << ilGetError() << std::endl;
        throw std::runtime_error("IL error");
    }
}

/* ----------------------------------------------------------------------------- */
typedef unsigned short color_t;
static inline constexpr color_t rgb( unsigned char R, unsigned char G, unsigned char B )
{
		return ((B>>3)<<(16-5)) | ((G>>2)<<(16-5-6) | (R>>3) );
}

/* ----------------------------------------------------------------------------- */
class pixel
{
public:
    pixel( unsigned char * buf )
      : m_data(buf)
      {}
      unsigned char R() const
      {
        return m_data[0];
      }
      unsigned char G() const
      {
        return m_data[1];
      }
      unsigned char B() const
      {
        return m_data[2];
      }
      void R( unsigned char v )
      {
        m_data[0] = v;
      }
      void G( unsigned char v )
      {
        m_data[1] = v;
      }
      void B( unsigned char v )
      {
        m_data[2] = v;
      }
      operator color_t() const
      {
         return rgb( R(), G(), B() );
      }
private:
    unsigned char *m_data;
};

class img_row
{
public:
    img_row( unsigned char *row )
        : m_data( row )
    {}
    pixel operator[](size_t index)
    {
        return pixel(&m_data[index*3]);
    }
private:
    unsigned char *m_data;
};

class img_array
{
public:
    img_array()
    {
        m_width = ilGetInteger(IL_IMAGE_WIDTH);
        const int height = ilGetInteger(IL_IMAGE_HEIGHT);
        m_data = new unsigned char[ m_width * height * 3 ];
        ilCopyPixels( 0, 0, 0, m_width, height, 1, IL_RGB, IL_UNSIGNED_BYTE ,m_data  );
    }
    img_row operator[](size_t index)
    {
        return img_row( &m_data[ index * m_width *3 ] );
    }
    ~img_array()
    {
        delete [] m_data;
    }
private:
    unsigned char *m_data;
    int m_width;
};

/* ----------------------------------------------------------------------------- */
int main()
{
	/* Init library */
    ilInit();
    iluInit();
    /* Load image */
    il_abort( ilLoadImage("/tmp/mkeia.jpg") );
    const int width = ilGetInteger(IL_IMAGE_WIDTH);
    const int height = ilGetInteger(IL_IMAGE_HEIGHT);
    std::cout << "Img size " << width << "x" << height << std::endl;
    img_array img;
    int cl = 0;
    color_t line[128];
    color_t linec[1024];
    size_t ll = 0;
    for(int y=0; y<height; ++y)
    for(int x=0; x<width; ++x)
    {
        line[cl++] = img[y][x];
        //std::cout << int(line[cl-1]) << " V "<< int(img[y][x]) << std::endl;
        if( cl == 128 )
        {
            const int clen = LZ_Compress((unsigned char*)line, (unsigned char*)linec, 256 );
            cl = 0;
            ll += clen + 1;
        }
    }
    std::cout << (width*height*2) << " " << ll << std::endl;
   	return 0;
}
