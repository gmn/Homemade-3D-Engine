#ifndef __IMAGE_H__
#define __IMAGE_H__


typedef struct {
	const char *name;
	int	minimize, maximize;
	const char *desc;
} glTextureMode_t;


typedef enum {
    IMG_NONE,
    IMG_BMP,
    IMG_TGA,
    IMG_GIF,
    IMG_JPG,
    IMG_PNG,
    TOTAL_IMAGE_TYPES
} imagetype_t;


typedef struct
{
    // size of ID field that follows 18 byte header (0 usually)
    unsigned char indentsize;
    // type of color map 0=none, 1=has palette
    unsigned char colormaptype;
    // type of image 0=none, 1=indexed, 2=rgb, 3=grey, +8=rle packed
    unsigned char imagetype;
    // first color map entry in palette
    unsigned short colormapstart;
    // number of colors in palette
    unsigned short colormaplength;
    // number of bits per palette entry 15, 16, 24, 32
    unsigned char colormapbits;
    // image x origin
    unsigned short xstart;
    // image y origin
    unsigned short ystart;
    // image width in pixels
    unsigned short width;
    // image height in pixels
    unsigned short height;
    // image bits per pixel (8, 16, 24, 32)
    unsigned char pixel_size;
    // image descriptor in bits (vh flip bits)
    unsigned char descriptor;
} tga_header_t;

/*
// Windows GDI Bitmap header for reference
typedef struct tagBITMAP {
  LONG   bmType; 
  LONG   bmWidth; 
  LONG   bmHeight; 
  LONG   bmWidthBytes; 
  WORD   bmPlanes; 
  WORD   bmBitsPixel; 
  LPVOID bmBits; 
} BITMAP, *PBITMAP; 
*/

typedef struct { 
  unsigned int      biSize; 
  int               biWidth; 
  int               biHeight; 
  unsigned short    biPlanes; 
  unsigned short    biBitCount; 
  unsigned int      biCompression; 
  unsigned int      biSizeImage; 
  int               biXPelsPerMeter; 
  int               biYPelsPerMeter; 
  unsigned int      biClrUsed; 
  unsigned int      biClrImportant; 
} bmp_info_t;


typedef struct { 
  unsigned short    magic; 
  unsigned int      totalBytes; 
  unsigned short    reserved1; 
  unsigned short    reserved2; 
  unsigned int      dataOffsetBytes; 
} bmp_header_t;


struct image_t
{
    char *name;             // basename( fullpath )
    char *fullpath;         
    imagetype_t type;

    union
    {
        tga_header_t tga;
        bmp_header_t bmp;
    } header;

    unsigned int h, w;
    unsigned int numbytes;  // filesize - sizeof(header) (tga=18,bmp=..)
    unsigned int bpp;       // bytes per pixel
    unsigned char *data;    // raw file read, minus header

    unsigned char *compiled;
    unsigned int texhandle;

    image_t() : name(0), fullpath(0), data(0), compiled(0), texhandle(-1) {}
    ~image_t();
};


void IMG_compileImageFile ( const char *path, image_t **ip );


//int IMG_CombineTextureMaskPow2( image_t *tex, image_t *mask, image_t **ip );

#endif /* !__IMAGE_H__ */
