//

#ifdef __MAC__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <stdio.h> // 
// FIXME: these should return strings, NOT print them

#define GL_ERR(x) { \
    int __err = glGetError(); \
    if ( 0 != __err )\
        core.warn( "GL error (line %d): %s\n", __LINE__, GL_err_string(__err) ); }

#include "core.h"
#include "image.h"
#include "gvar.h"




void GL_print_strings()
{
    char * s = (char*) glGetString( GL_VENDOR );
    core.printf( "VENDOR: %s\n\n", s );
    s = (char*) glGetString( GL_RENDERER );
    core.printf( "RENDERER: %s\n\n", s );
    s = (char*) glGetString( GL_VERSION );
    core.printf( "VERSION: %s\n\n", s );
    s = (char*) glGetString( GL_EXTENSIONS );
    core.printf( "EXTENSIONS: %s\n\n", s );
}

const char * GL_extensions_string()
{
    // (is const GLubyte*)
    return (const char *) glGetString( GL_EXTENSIONS );
}

const char * GL_vendor_string()
{
    return (const char*) glGetString( GL_VENDOR );
}

const char * GL_version_string()
{
    return (const char*) glGetString( GL_VERSION );
}

const char * GL_err_string( int e )
{
    switch ( e ) {
    case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
    case GL_TABLE_TOO_LARGE: return "GL_TABLE_TOO_LARGE";
    default:
    case GL_NO_ERROR: return "GL_NO_ERROR";
    }
}

void GL_print_matrix() {
    float mat[16];
    glGetFloatv( GL_MODELVIEW_MATRIX, mat );
    for ( int i = 0; i < 4; i++ ) {
        core.printf( "%.1f %.1f %.1f %.1f\n", mat[i],mat[i+4],mat[i+8],mat[i+12] );
    }
    core.printf( "\n");
}


glTextureMode_t glTextureModes[] = {
	{ "GL_NEAREST", GL_NEAREST, GL_NEAREST, "Far Filtering off. Mipmapping off" },
	{ "GL_LINEAR", GL_LINEAR, GL_LINEAR, "Bilinear filtering, no mipmap" },
	{ "GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST, "Far Filtering off, standard mipmap" },
	{ "GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, "Bilinear filtering with standard mipmap" },
	{ "GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST, "Far Filtering off. Mipmaps use Trilinear" },
	{ "GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, "Bilinear Filtering. Mipmaps use Trilinear" },
};


void GL_image_make_texture ( image_t *img )
{
	GLenum err = GL_NO_ERROR;

	err = glGetError();
	if ( err != GL_NO_ERROR ) {
		core.printf( "***GL error: %s \n", GL_err_string( err ) );
	}

	err = GL_NO_ERROR;

    glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

	err = glGetError();
	if (err != GL_NO_ERROR) {
		core.warn( "GL error: %s \n", GL_err_string(err) );
	}
	err = GL_NO_ERROR;

    glGenTextures ( 1, &img->texhandle );

	Assert( img->texhandle > 0 );

	err = glGetError();
	if (err != GL_NO_ERROR) {
		core.warn( "GL error: %s \n", GL_err_string(err) );
	}
	err = GL_NO_ERROR;

    glBindTexture( GL_TEXTURE_2D, img->texhandle );

	err = glGetError();
	if (err != GL_NO_ERROR) {
		core.warn( "GL error: %s \n", GL_err_string(err) );
	}
	err = GL_NO_ERROR;

/*	// vanilla bilinear filtering, no mipmap
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); */

    gvar_t * tex_mode = core.gvar( "tex_mode" );

	// gvar specified filtering
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glTextureModes[ tex_mode->integer ].minimize );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glTextureModes[ tex_mode->integer ].maximize );
	
	err = glGetError();
	if (err != GL_NO_ERROR) {
		core.warn( "GL error: %s \n", GL_err_string(err) );
	}
	
	if ( img->bpp == 1 ) {
		glTexImage2D ( GL_TEXTURE_2D, 0, GL_ALPHA, img->w, img->h, 
		              0, GL_ALPHA, GL_UNSIGNED_BYTE, img->compiled );
	} else {
		glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 
		              0, GL_RGBA, GL_UNSIGNED_BYTE, img->compiled );
	}

	err = glGetError();
	if (err != GL_NO_ERROR) {
		core.warn( "GL error: %s \n", GL_err_string(err) );
	}
}

