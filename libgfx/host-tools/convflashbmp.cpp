/*
 * convflashbmp.cpp
 *
 *  Created on: 16-06-2013
 *      Author: lucck
 */

#include <vector>
#include <cstddef>
#include <iostream>
#include <IL/il.h>
#include <IL/ilu.h>
#include <stdexcept>
#include "lz.h"
/* ----------------------------------------------------------------------------- */
namespace {
    constexpr auto lz_line_size = 128;
    constexpr auto lz_sub_size  = 3;
}
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
//Prepare image in rgb565 format
void create_rgb16_bitmap( FILE *file )
{
    img_array img;
    int cl = 0;
    color_t line[lz_line_size];
    color_t linec[lz_line_size*2];
    const int width = ilGetInteger(IL_IMAGE_WIDTH);
    const int height = ilGetInteger(IL_IMAGE_HEIGHT); 

    for(int y=0; y<height; ++y)
    for(int x=0; x<width; ++x)
    {
        line[cl++] = img[y][x];
        if( cl == lz_line_size )
        {
            const int clen = LZ_Compress(line, linec, sizeof line );
            cl = 0;
            fprintf(file, "/*s*/ 0x%02hhx, ", clen - lz_sub_size );
            bool new_line = false;
            for( int i = 0; i< clen; i++ )
            {
                fprintf(file, "0x%02hhx, ", linec[i] );
                if( (i % 25)==24 ) 
                {
                    fprintf(file, "\n" );
                    new_line = true;
                }
                else
                {
                    new_line = false;
                }
            }
            if( !new_line ) 
                fprintf(file, "\n" );
        }
    }
}

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
    FILE *f = fopen("/tmp/test.cpp","wt");
    create_rgb16_bitmap( f );
    fclose(f);
    return 0;
}
