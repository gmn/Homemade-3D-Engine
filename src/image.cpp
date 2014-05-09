/*
==================== 
image.c -  old image loading c code from pacman.  pretty archaic but works.
            bitmap and targa only.
==================== 
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef snprintf
#define snprintf _snprintf
#endif

#include "core.h"
#include "image.h"
#include "hgr.h"


#if 0
glTextureMode_t glTextureModes[] = {
	{ "GL_NEAREST", GL_NEAREST, GL_NEAREST, "Far Filtering off.  Mipmapping off" },
	{ "GL_LINEAR", GL_LINEAR, GL_LINEAR, "Bilinear filtering, no mipmap" },
	{ "GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST, "Far Filtering off, standard mipmap" },
	{ "GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, "Bilinear filtering with standard mipmap" },
	{ "GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST, "Far Filtering off.  Mipmaps use Trilinear" },
	{ "GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, "Bilinear Filtering.  Mipmaps use Trilinear" },
};

void GL_image_make_texture ( image_t *img )
{
	GLenum err = GL_NO_ERROR;

	err = glGetError();
	if ( err != GL_NO_ERROR ) {
		core.printf( "gl error: %s \n", gluErrorString(err) );
		console.Printf( "***GL error: %s\n", gluErrorString( err ) );
	}
	err = GL_NO_ERROR;

    gglPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

	err = gglGetError();
	if (err != GL_NO_ERROR) {
		core.printf( "gl error: %s \n", gluErrorString(err) );
		console.Printf( "***GL error: %s\n", gluErrorString( err ) );
	}
	err = GL_NO_ERROR;

    gglGenTextures ( 1, &img->texhandle );

	int BIG_STUPID_DEBUG_INT = img->texhandle;

	err = gglGetError();
	if (err != GL_NO_ERROR) {
		core.printf( "gl error: %s \n", gluErrorString(err) );
		console.Printf( "***GL error: %s\n", gluErrorString( err ) );
	}
	err = GL_NO_ERROR;

    gglBindTexture( GL_TEXTURE_2D, img->texhandle );

	err = gglGetError();
	if (err != GL_NO_ERROR) {
		core.printf( "gl error: %s \n", gluErrorString(err) );
		console.Printf( "***GL error: %s\n", gluErrorString( err ) );
	}
	err = GL_NO_ERROR;

/*	// vanilla bilinear filtering, no mipmap
	gglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    gglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); */

	// gvar specified filtering
	gglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, modes[tex_mode->integer()].minimize );
    gglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, modes[tex_mode->integer()].maximize );
	
	err = gglGetError();
	if (err != GL_NO_ERROR) {
		core.printf( "gl error: %s \n", gluErrorString(err) );
		console.Printf( "GL error: %s \"%s\"\n", gluErrorString( err ), img->name );
	}
	
	if ( img->bpp == 1 ) {
		gglTexImage2D ( GL_TEXTURE_2D, 0, GL_ALPHA, img->w, img->h, 
		              0, GL_ALPHA, GL_UNSIGNED_BYTE, img->compiled );
	} else {
		gglTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 
		              0, GL_RGBA, GL_UNSIGNED_BYTE, img->compiled );
	}

	err = gglGetError();
	if (err != GL_NO_ERROR) {
		core.printf( "gl error: %s \n", gluErrorString(err) );
		console.Printf( "GL error: %s \"%s\"\n", gluErrorString( err ), img->name );
	}
}
#endif


int IMG_getBmpInfo( image_t *img, bmp_info_t *info )
{
    img->w = info->biWidth;
    img->h = info->biHeight;
    img->bpp = info->biBitCount / 8;

    // check for stuff we dont want
    if ( img->bpp != 3 && img->bpp != 4 ) {
        core.printf( "bitmap implementation only supports 24 & 32-bit formats! : %d\n", img->bpp);
        return 0;
    }
    
    return 1;
}


int IMG_readfileBMP ( const char *fullpath, image_t *img )
{
    FILE *fp;
    int i, bytesread;
    int datasize;
    unsigned int infosize;
    unsigned char buf[128];
	bmp_header_t *h;
    bmp_info_t *info;
    const char * basename = strip_path(fullpath);

    // open
    if ((fp = fopen(fullpath, "rb")) == NULL) {
        core.warn("couldn't open: %s \n", basename );
        return -1;
    }

    // read header straight in
    if ( fread(buf, 14, 1, fp) != 1 ) {
        core.warn("%s: error reading header\n", basename );
        return -1;
    }

	h = &img->header.bmp;
	h->magic = (unsigned short) *((unsigned short *)buf);
	h->totalBytes = (unsigned int) *((unsigned int *)&buf[2]);
	h->reserved1 = h->reserved2 = 0;
	h->dataOffsetBytes = (unsigned int) *((unsigned int *)&buf[10]);

    // check magic number
    if (img->header.bmp.magic != 19778) {
        core.warn("%s Not a Bitmap File\n", basename );
        return -1;
    }

    // read info portion of header
    memset( buf , 0, sizeof(buf) );
    //infosize = img->header.bmp.dataOffsetBytes - sizeof(bmp_header_t);
    infosize = img->header.bmp.dataOffsetBytes - 14;

    info = (bmp_info_t *) buf;

    if ( fread( info, infosize, 1, fp ) != 1 ) {
        core.warn("error reading bitmap info from file\n");
        return -1;
    } 

    // record info
    if (! IMG_getBmpInfo(img, info) ) {
        core.warn( "couldn't get bitmap info\n" );
        return -1;
    }
    
    // malloc data portion
	
	//   2 bytes padding (0x00, 0x00) on the end of bmp
    datasize = img->header.bmp.totalBytes - img->header.bmp.dataOffsetBytes;
    img->data = (unsigned char *) malloc ( datasize );
    memset( img->data, 0, datasize );
    img->numbytes = datasize;
	img->type = IMG_BMP;

    // get data
    fseek ( fp, img->header.bmp.dataOffsetBytes, SEEK_SET );

	// read image data
    bytesread = 0;
	while ((i = fread(&img->data[bytesread], 1, 8192, fp)) > 0)
		bytesread += i;

    if ( bytesread != datasize ) {
        core.error("bitmap: incorrect datasize: %d\n", bytesread );
        return -1;
    }

    return 0; // success
}


void IMG_copyHeaderTGA( tga_header_t *tga, unsigned char *buf_p )
{
    tga->indentsize     = *buf_p++;
    tga->colormaptype   = *buf_p++;
    tga->imagetype      = *buf_p++;
    tga->colormapstart  = (unsigned short) *(short *)buf_p; buf_p += 2;
    tga->colormaplength = (unsigned short) *(short *)buf_p; buf_p += 2;
    tga->colormapbits   = *buf_p++;
    tga->xstart         = (unsigned short) *(short *)buf_p; buf_p += 2;
    tga->ystart         = (unsigned short) *(short *)buf_p; buf_p += 2;
    tga->width          = (unsigned short) *(short *)buf_p; buf_p += 2;
    tga->height         = (unsigned short) *(short *)buf_p; buf_p += 2;
    tga->pixel_size     = *buf_p++;
    tga->descriptor     = *buf_p;
}


int IMG_readfileTGA ( const char * fullpath, image_t *img )
{
    FILE *fp;
    unsigned char tga_header_buf[18];
    unsigned int sz;

    if ( (fp = fopen( fullpath, "rb" )) == 0 ) {
        core.warn ( "couldn't open file: %s\n", fullpath );
        return -1;
    }

    if ( 1 != fread( (void *)tga_header_buf, 18, 1, fp ) ) {
        core.warn ( "failure reading file: %s\n", fullpath );
        return -1;
    }

    IMG_copyHeaderTGA( &img->header.tga, tga_header_buf );

	sz = get_filesize( fullpath );

	if ( sz == 0 ) {
		core.warn( "file: %s returned bad size, file not found?\n", strip_path(fullpath) );
        return -1;
    }

	img->data = (unsigned char *) malloc( sz - 18 );

	img->numbytes = 0;
    int i = 0;
    while ( (i = fread( &img->data[img->numbytes], 1, 8192, fp )) )
        img->numbytes += i;

    if ( img->numbytes == 0 ) {
        core.warn ( "error reading rest of file: %s\n", strip_path(fullpath) );
        return -1;
    }
	if ( img->numbytes != sz-18 ) {
		core.error ( "%s: sizes read don't match %i %i\n", strip_path(fullpath), img->numbytes, sz );
        return -1;
    }

    img->type = IMG_TGA;
    
    img->h = img->header.tga.height;
    img->w = img->header.tga.width;
    img->bpp = img->header.tga.pixel_size / 8;

    fclose( fp );

	return 0; // success
}

/*
==================== 
  IMG_compile_image()
==================== 
*/
int IMG_compile_image( image_t *img )
{
    int i, j, rgba_sz;

    unsigned char *compiled;

	if ( img->bpp != 1 && img->bpp != 3 && img->bpp != 4 ) {
        core.warn( "IMG_compile_img: only handling 8, 24 or 32-bit right now. %s has unsupported bpp: %d\n", img->name, img->bpp );
		return -1;
	}

	if ( img->bpp == 1 ) {
		rgba_sz = img->numbytes;
	} else if ( img->bpp == 3 ) {
	    rgba_sz = img->numbytes * 4;
	    rgba_sz /= 3;
    } else {
        rgba_sz = img->numbytes;
    }

    rgba_sz += 16; // pad

    compiled = (unsigned char *) malloc ( rgba_sz );
    memset( compiled, 0, rgba_sz );

    if ( img->bpp == 1 ) {
		for ( i = 0; i < (int)img->numbytes; i++ ) {
			compiled[ i ] = img->data[ i ];
		}
	} else if (img->bpp == 3) {
        for (i = 0, j = 0; i < (int)img->numbytes; i += img->bpp, j+=4)
        {
            if (img->data[i] || img->data[i+1] || img->data[i+2]) {
                compiled[j+2] = img->data[i+0];
                compiled[j+1] = img->data[i+1];
                compiled[j+0] = img->data[i+2];
                compiled[j+3] = 255;
            }
        }
    } else {
        for (i = 0, j = 0; i < (int)img->numbytes; i += img->bpp, j+=4)
        {
            compiled[j+2] = img->data[i+0];
            compiled[j+1] = img->data[i+1];
            compiled[j+0] = img->data[i+2];
            compiled[j+3] = img->data[i+3];
        }
    }

    img->compiled = compiled;

	return 0;
}

static const char *IMG_Error( int typenum ) {
    switch (typenum) {
    case IMG_NONE:      return "IMG_NONE";
    case IMG_BMP:       return "IMG_BMP";
    case IMG_TGA:       return "IMG_TGA";
    case IMG_GIF:       return "IMG_TGA";
    case IMG_JPG:       return "IMG_JPG";
    case IMG_PNG:       return "IMG_PNG";
    }
    return "UNKNOWN";
}

void IMG_compileImageFile ( const char *path, image_t **ip )
{
    int type;
    int i;

    *ip = new image_t;

    // get type from filename extension
    for (i = 0; path[i] != '\0'; i++)
        ;

    do {
        --i;
    } while ( path[i] != '.' );
    ++i;


    type = IMG_NONE;
    if ( !strncasecmp( &path[i], "BMP", 3 ) )
        type = IMG_BMP;
    else if ( !strncasecmp( &path[i], "TGA", 3 ) )
        type = IMG_TGA;


    // get canonical game texture name
    const char * sname = strip_path_and_extension( &path[0] );
	(*ip)->name = copy_string( sname );

	// get the fully qualified path
	(*ip)->fullpath = realpath( &path[0], 0 );

    int err_cond = 0; // ok

    // 
    switch(type) {
    case IMG_TGA:
        err_cond = IMG_readfileTGA( (*ip)->fullpath, *ip ); 
        break;
    case IMG_BMP:
        err_cond = IMG_readfileBMP( (*ip)->fullpath, *ip );
        break;
    case IMG_GIF:
    case IMG_JPG:
    case IMG_PNG:
    default:
        core.warn( "image type: %s is unsupported\n", IMG_Error(type) );
read_fail:
        delete *ip;
        (*ip)=0;
        return;
    }

    if ( err_cond )
        goto read_fail;


	err_cond = IMG_compile_image( *ip ) ;

    // don't need any more. 
    free( (*ip)->data );
    (*ip)->data = NULL;

	if ( !err_cond ) 
    {
		GL_image_make_texture( *ip );

        free( (*ip)->compiled );
		(*ip)->compiled = NULL;
	}
}

image_t::~image_t()
{
    #define FreeIf(x) if ( x ) free(x)
    FreeIf( name );
    FreeIf( fullpath );
    FreeIf( data );
    FreeIf( compiled );
    #undef FreeIf
}


#if 0
// tex and mask should already be created.  we'll re-read in each of their
//  file's data, and then create another image and GL compile it, storing
//  all of the new info to ip.  Only supports textures that have an equal
//  width and height value, and that value must be a powerof 2.
int IMG_CombineTextureMaskPow2( image_t *tex, image_t *mask, image_t **ipp ) {
    int i;
	
	if ( !( tex && mask && ipp ) )
		return -1;
	if ( tex->h != tex->w || mask->h != mask->w )
		return -2;
	if ( !ISPOWEROF2( tex->h ) )
		return -3;

	// alloc the image
    image_t *ip = (image_t *) V_Malloc( sizeof(image_t) );
	image_t *m_ip = (image_t*) V_Malloc( sizeof(image_t) );
	memset( ip, 0, sizeof(image_t) );
	memset( m_ip, 0, sizeof(image_t) );

	char path[ 512 ];
	snprintf( path, 512, "%s\\%s", fs_gamepath->string(), tex->syspath );

    // read the TEX file 
    switch ( tex->type ) {
    case IMG_TGA:
        IMG_readfileTGA( path, ip ); 
        break;
    case IMG_BMP:
        IMG_readfileBMP( path, ip );
        break;
    case IMG_GIF:
    case IMG_JPG:
    case IMG_PCX:
    case IMG_PPM:
    case IMG_PNG:
    default:
        core.printf( "image type: %s not supported\n", IMG_Error(tex->type) );
        V_Free(ip);
        V_Free(m_ip);
        return -6;
    }

	// save path info of the texture image
	ip->name[0] = 0;
	ip->syspath[0] = 0;
	strcpy( ip->name, strip_path( &path[0] ) );
	strcpy( ip->syspath, strip_gamepath( &path[0] ) );

	// read in the MASK file
	snprintf( path, 512, "%s\\%s", fs_gamepath->string(), mask->syspath );
    switch ( mask->type ) {
    case IMG_TGA:
        IMG_readfileTGA( path, m_ip ); 
        break;
    case IMG_BMP:
        IMG_readfileBMP( path, m_ip );
        break;
    case IMG_GIF:
    case IMG_JPG:
    case IMG_PCX:
    case IMG_PPM:
    case IMG_PNG:
    default:
        core.printf( "image type: %s not supported\n", IMG_Error(mask->type) );
		if ( ip->data )
			V_Free( ip->data );
        V_Free(ip);
        V_Free(m_ip);
        return -7;
    }

	int err_cond = 0;

	// allocates compiled array & formats data into it
    if ( IMG_compile_image( ip ) )
		err_cond = 1;
    if ( IMG_compile_image( m_ip ) )
		err_cond = 1;

	unsigned char *rsmp = NULL;

	if ( !err_cond ) {

		int scale = ip->w / m_ip->w;	// difference between scales

		// single channel greyscale mask
		int r_sz = ip->h * ip->w; // 1 byte for each pixel
		rsmp = (unsigned char *) V_Malloc( ip->h * ip->w );
		memset( rsmp, 0, r_sz );

		// convert mask from it's resident format to be the same dimension
		//  as the texture image
		int h, i, j, k, rsmp_i, rsmp_j, pix;

		/// foreach Row in Mask

		// for each row in m_ip ==> j
		for ( j = 0; j < m_ip->h; j++ ) {

			/// foreach col in Mask

			// for each col in m_ip ==> i
			for ( i = 0; i < m_ip->w; i++ ) {

				/// extrapolate Mask Pixel to Cover all of it's corresponding pixels
				///  in the new re-sampled mask

				// ROWS: ( j * scale ) to ( j * scale + scale ) 
				for ( k = 0; k < scale; k++ ) {

					rsmp_j = j * scale + k;

					// COLS: set i to i + scale pixels to value of m_ip->compiled[ i ]
					for ( h = 0; h < scale; h++ ) {

						rsmp_i = i * scale + h;

						// index into new mask
						pix = rsmp_j * ip->w + rsmp_i;

						if ( pix < r_sz ) {
							rsmp[ pix ] = m_ip->compiled[ j * m_ip->w + i ];
						}
					}
				}
			}
		}

		if ( ip->bpp == 1 ) {
			for ( i = 0; i < (int)ip->numbytes; i += 1 ) {
				if ( rsmp[ i ] == 0 ) {
					ip->compiled[ i + 0 ] = 0;
				}
			}
		} else if ( 0 ) {
			// set to zero any texels that didn't pass
			for ( i = 0; i < (int)r_sz; i++ ) {
				if ( rsmp[ i ] == 0 ) {
					ip->compiled[ i * 4 + 0 ] = 0;
					ip->compiled[ i * 4 + 1 ] = 0;
					ip->compiled[ i * 4 + 2 ] = 0;
					ip->compiled[ i * 4 + 3 ] = 255;
				}
			}
		} // if bpp == 4
		
		IMG_MakeGLTexture( ip );
	}

	if ( ip->data ) {
    	V_Free( ip->data );
    	ip->data = NULL;
	}
	if ( ip->compiled ) {
		V_Free( ip->compiled );
		ip->compiled = NULL;
	}
	if ( m_ip->data ) {
    	V_Free( m_ip->data );
    	m_ip->data = NULL;
	}
	if ( m_ip->compiled ) {
		V_Free( m_ip->compiled );
		m_ip->compiled = NULL;
	}
	V_Free( m_ip );
	if ( rsmp )
		V_Free( rsmp );

	*ipp = NULL;
	if ( err_cond )
		return -4;

	*ipp = ip;
	return 0;
}
#endif 


