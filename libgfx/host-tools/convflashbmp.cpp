/*
 * convflashbmp.cpp
 *
 *  Created on: 16-06-2013
 *      Author: lucck
 */

#include <cstring>
#include <cstddef>
#include <iostream>
#include <IL/il.h>
#include <IL/ilu.h>
#include <stdexcept>
#include <vector>
#include <string>
#include <algorithm>
#include "lz.h"
/* ----------------------------------------------------------------------------- */
namespace {
    constexpr auto lz_line_size_bytes = 256;
    constexpr auto lz_sub_size  = 3;
    constexpr auto vars_per_line = 20;
}
/* ----------------------------------------------------------------------------- */
inline void il_abort( ILboolean error )
{
    if( !error )
    {
        throw std::runtime_error( std::string("IL error ") + iluErrorString(ilGetError()));
    }
}

/* ----------------------------------------------------------------------------- */
typedef unsigned short color_t;
typedef unsigned char  color8_t;
static inline constexpr color_t rgb565_f( unsigned char R, unsigned char G, unsigned char B )
{
		return ((B>>3)<<(16-5)) | ((G>>2)<<(16-5-6) | (R>>3) );
}
static inline constexpr color_t bgr565_f( unsigned char R, unsigned char G, unsigned char B )
{
		return ((R>>3)<<(16-5)) | ((G>>2)<<(16-5-6) | (B>>3) );
}

static inline constexpr color8_t bgr233_f( unsigned char R, unsigned char G, unsigned char B )
{
    return (R>>5) | ((G>>5)<<3) | (B>>6)<<6;
}

static inline color_t color_le( color_t color )
{
#if __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
    return color;
#elif __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
    return __builtin_bswap16( color );
#else
#error Unknown endianess
#endif
}

static inline color_t color_be( color_t color )
{
#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
    return color;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
    return __builtin_bswap16( color );
#endif
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
      color_t rgb565() const
      {
         return rgb565_f( R(), G(), B() );
      }
      color_t bgr565() const
      {
          return bgr565_f( R(), G(), B() );
      }
      color8_t bgr233() const
      {
        return bgr233_f( R(), G(), B() );
      }
      color8_t grey8() const
      {
        return (int(R()) + int(G()) + int(B()))/3;
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
enum class img_fmt_t
{
    unknown,
    rgb565,         //RGB 565
    bgr565,         //BGR 565
    bgr332,         //RGB 332
    bw              //Monochrome
};
/* ----------------------------------------------------------------------------- */
/* Add data to linebuffer */
template <typename T>
    void add_to_linebuf( void* addr, size_t index, T value )
{
    *(reinterpret_cast<T*>(addr) + index) = value;
}
/* ----------------------------------------------------------------------------- */
//Prepare image in rgb565 format
size_t create_raw_bitmap( FILE *file, bool big_endian, img_fmt_t fmt, int bw_level )
{
    img_array img;
    size_t cl = 0;
    size_t bwcl = 0;
    uint8_t pixel_byte = 0;
    uint8_t ltmp_buf[ lz_line_size_bytes ];
    uint8_t linec[lz_line_size_bytes*2];
    const int width = ilGetInteger(IL_IMAGE_WIDTH);
    const int height = ilGetInteger(IL_IMAGE_HEIGHT); 
    fprintf( file, "static constexpr unsigned char image_data[] = {\n" );
    size_t tot_len = 0;
    size_t line_cnt = 0;
    const size_t lz_elem_size = ((fmt==img_fmt_t::rgb565||fmt==img_fmt_t::bgr565)?sizeof(uint16_t):sizeof(uint8_t));
    const size_t lz_line_size = lz_line_size_bytes/lz_elem_size;
    for(int y=0; y<height; ++y)
    for(int x=0; x<width; ++x)
    {
        if( fmt == img_fmt_t::rgb565 || fmt== img_fmt_t::bgr565 )
        {
            auto pixel = (fmt==img_fmt_t::bgr565)?(img[y][x].bgr565()):(img[y][x].rgb565());
            if( big_endian ) pixel = color_be(pixel);
            else pixel = color_le(pixel);
            add_to_linebuf<color_t>( ltmp_buf, cl++, pixel );
        }
        else if( fmt == img_fmt_t::bgr332 )
        {
            const auto pixel = img[y][x].bgr233();
            add_to_linebuf<color8_t>( ltmp_buf, cl++, pixel );
        }
        else if( fmt == img_fmt_t::bw )
        {
           const auto pixel = img[y][x].grey8()>bw_level?(1):(0);
           pixel_byte |= pixel << ( 7 - bwcl++ );
           if( bwcl == 8 || (y==height-1 && x==width-1) )
           {
               bwcl = 0;
               add_to_linebuf<uint8_t>( ltmp_buf, cl++, pixel_byte );
               pixel_byte = 0;
           }
        }
        if( cl==lz_line_size || (y==height-1 && x==width-1) )
        {
            const size_t clen = LZ_Compress(ltmp_buf, linec, cl*lz_elem_size);
            tot_len += clen + 1;
            cl = 0;
            fprintf(file, "\n/* LEN */ 0x%02lx, \n", clen - lz_sub_size );
            line_cnt=0;
            for( size_t i = 0; i<clen; i++ )
            {
                fprintf(file, "0x%02hhx, ", linec[i] );
                if( (line_cnt++ % vars_per_line == vars_per_line-1) && (clen-1!=i) )
                    fputc('\n', file );
            }
        }
    }
    fputs("/* EOF */ 0\n\n};\n", file);
    return tot_len;
}

/* ----------------------------------------------------------------------------- */
//Create CPP header
void create_cpp_header( FILE *file, const std::vector<std::string> &namespaces,
       const char *filename, size_t size, const char *fmt, bool big_endian )
{
    fputs("/* Static bitmap array created by convflash for ISIXRTOS\n", file );
    fprintf(file," Oryginal filename: %s, filesize: %lu format %s Endianess %s */\n\n",
            filename, size, fmt, big_endian?"BIG":"LITTLE" );
    fputs("#include <gfx/disp/static_bitmap.hpp>\n\n", file );
    for( const auto &ns: namespaces )
    {
        fprintf(file, "namespace %s {\n", ns.c_str() );
    }
}
/* ----------------------------------------------------------------------------- */
//Create namespace footer
void create_cpp_footer( FILE *file, size_t ns_cnt, const char* filename, img_fmt_t type )
{
    const int width = ilGetInteger(IL_IMAGE_WIDTH);
    const int height = ilGetInteger(IL_IMAGE_HEIGHT);
    std::string fname( filename );
    std::replace(fname.begin(), fname.end(), '.','_');
    unsigned found = fname.find_last_of("/\\");
    const char *fmt_desc="";
    switch(type)
    {
        case img_fmt_t::rgb565: fmt_desc = "rgb565"; break;
        case img_fmt_t::bgr565: fmt_desc = "bgr565"; break;
        case img_fmt_t::bw: fmt_desc = "bpp1"; break;
        case img_fmt_t::bgr332: fmt_desc = "bgr332"; break;
        default: fmt_desc = "error"; break;
    }
    const auto struct_name = fname.substr(found + 1);
    fputs("\n/* Exported structure */\n", file );
    fprintf(file,"extern const gfx::disp::cmem_bitmap_t %s;\n", struct_name.c_str() );
    fprintf(file,"\n\nconst gfx::disp::cmem_bitmap_t %s = { \n",   struct_name.c_str() );
    fprintf(file, "\tnullptr, \n");
    fprintf(file, "\t%i,\n", width );
    fprintf(file, "\t%i,\n",  height );
    fprintf(file, "\timage_data, \n" );
    fprintf(file, "\tgfx::disp::cmem_bitmap_t::%s \n};\n\n", fmt_desc);
    for( size_t i=0; i<ns_cnt; ++i)
    {
        fputc('}',file);
    }
    fputc('\n', file );
}

/* ----------------------------------------------------------------------------- */
//Load the Image
void load_image( const char *filename )
{
    /* Init library */
    ilInit();
    iluInit();
    /* Load image */
    ilLoadImage(filename);
    if( ilGetError() == IL_COULD_NOT_OPEN_FILE )
    {
         throw std::logic_error( "Unable to open file");
    }
    else
    {
        const int width = ilGetInteger(IL_IMAGE_WIDTH);
        const int height = ilGetInteger(IL_IMAGE_HEIGHT);
        const int data_size = ilGetInteger(IL_IMAGE_SIZE_OF_DATA);
        std::cout << "Img size: [" << width << "x" << height << "] data: "<< (data_size/1024.0) << "KB" << std::endl;
    }
}


/* ----------------------------------------------------------------------------- */
void usage( const char *reason )
{
    std::cout << "Usage: convflashbmp -in FILEIN -out FILEOUT -fmt IMGFORMAT [-be] [-ns NAMESPACE1] ... [-ns NAMESPACEn] " << std::endl;
    std::cout << "Convert image to the ixix gfx library source file format" << std::endl;
    std::cout << "Copyright (c) Lucjan Bryndza BoFF 2008-2013" << std::endl;
    std::cout << "Mandatory arguments: " << std::endl;
    std::cout << "\t-in FILEIN\tInput image file to convert" << std::endl;
    std::cout << "\t-out FILEOUT\tOutput image file name in C++ isix format" << std::endl;
    std::cout << "\t-fmt IMGFORMAT\tOutput image isix format" << std::endl;
    std::cout << "\tIMGFORMATS options:" << std::endl;
    std::cout << "\t\tRGB565\tOutput image in 16-bit RGB565 format"  << std::endl;
    std::cout << "\t\tBGR565\tOutput image in 16-bit BGR565 format"  << std::endl;
    std::cout << "\t\tBGR332\tOutput image in 8-bit BGR332 format"  << std::endl;
    std::cout << "\t\tBW[Level]\tMonochrome image 8-bit packed format with conversion level" << std::endl;
    std::cout << "\t-ns NAMESPACE\tOptional namespace in image source file" << std::endl;
    std::cout << "\t-be\tBig endian byte order (default is little)" << std::endl;
}

/* ----------------------------------------------------------------------------- */
/** 
 * usage  convflashimg -in ala.jpg -out fff.cpp -ns xx -ns yy [-be] -fmt
 * RGB565, BGR565, BW
 */
int main(int argc, const char * const * const argv)
{
    const char *in_filename = nullptr;
    const char *out_filename = nullptr;
    const char *fmt_string = nullptr;
    std::vector< std::string > namespaces;
    bool big_endian = false;
    img_fmt_t format { img_fmt_t::unknown };
    int bw_level = 127;
    for( int a=1; a<argc; ++a)
    {
        if(argv[a][0] != '-')
        {
            usage("Invalid parameter");
            return -1;
        }
        if( !std::strcmp(argv[a], "-in" ) && a+1<argc )
        {
            in_filename = argv[++a];
            std::cout << "Source image: " << in_filename << std::endl;
        }
        if( !std::strcmp(argv[a], "-out" ) && a+1<argc )
        {
            out_filename = argv[++a];
            std::cout << "Destination file: " << out_filename << std::endl;
        }
        if( !std::strcmp(argv[a], "-ns" ) && a+1<argc )
        {
              namespaces.push_back( std::string( argv[++a] ) );
        }
        if( !std::strcmp(argv[a], "-fmt" ) && a+1<argc )
        {
            std::string fmt( argv[++a] );
            {
                if( fmt=="RGB565")
                {
                    format = img_fmt_t::rgb565;
                }
                else if( fmt=="BGR565")
                {
                    format = img_fmt_t::bgr565;
                }
                else if( fmt[0]=='B' && fmt[1]=='W' )
                {
                    format = img_fmt_t::bw;
                    const int val = std::atoi( &fmt[2] );
                    if( val > 0 && val < 256 )
                        bw_level = val;
                    std::cout << "Black and wait trigger level: " << bw_level << std::endl;
                }
                else if( fmt=="BGR332")
                {
                    format = img_fmt_t::bgr332;
                }
                else
                {
                    usage("Format not supported");
                    return -1;
                }
                fmt_string = argv[a];
            }
        }
        if( !std::strcmp(argv[a], "-be" ))
        {
                big_endian = true;
        }
    }
    if( !in_filename )
    {
        usage("Input filename not provided");
        return -1;
    }
    if( !out_filename )
    {
        usage("Output filename not provided");
        return -1;
    }
    if( format == img_fmt_t::unknown )
    {
        usage("Image format not defined");
        return -1;
    }
    try
    {
        load_image( in_filename );
    }
    catch( std::exception &e )
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    FILE *outf = fopen(out_filename,"wt");
    if( !outf )
    {
        std::cerr << "Unable to create file" << std::endl;
    }
    create_cpp_header( outf , namespaces, in_filename, ilGetInteger(IL_IMAGE_SIZE_OF_DATA), fmt_string, big_endian );
    size_t tot_len = 0; 
    if( format==img_fmt_t::rgb565 || format==img_fmt_t::bgr565 || 
        format==img_fmt_t::bgr332 || format==img_fmt_t::bw )
        tot_len = create_raw_bitmap( outf, big_endian, format, bw_level );
    create_cpp_footer( outf , namespaces.size(), in_filename, format );
    std::cout << "Dest image size: " << (tot_len/1024.0) << "KB" << std::endl;
    fclose(outf);
    return 0;
}
