#ifndef __HGR_H__
#define __HGR_H__

// main.cpp
int get_milliseconds();

// misc.cpp
char * va ( const char *, ... );
void _hidden_Assert( int, const char *, const char *, int );
#ifdef _DEBUG
#define Assert( v ) _hidden_Assert( ((v)), #v, __FILE__, __LINE__ )
#else
#define Assert( v )
#endif
//int block_for_any_key( void );
//int poll_for_any_key( int );
unsigned int get_filesize( const char * path );
const char * strip_path ( const char * );
const char * strip_extension ( const char * );
const char * strip_path_and_extension ( const char * );
char * str_tolower( char * );
char * str_toupper( char * );
char * copy_string( const char * );
char ** list_directory( const char * );
void free_listdir( char ** );
void chr_replace( const char s, const char r, char * q );
int is_little_endian( void );
unsigned long FlipULong( unsigned long x );

// gl_drawing.cpp
void GL_camera_projection( int, int ); 
void GL_ortho_projection( int, int ); 
void GL_init_state( void );
void GL_draw_frame( void * );
void GL_free_buffers();

// gl_macros.cpp
void GL_print_strings( void );
const char * GL_err_string( int );
void GL_print_matrix();
const char * GL_extensions_string();
const char * GL_vendor_string();
const char * GL_version_string();
struct image_t;
void GL_image_make_texture ( image_t *img );

// gl_shapes.cpp
void draw_spiral( float degrees, float radius, float climb );
void draw_circle( float, float, float );
void draw_wire_circle( float, float, float );
void draw_tex_circle( float, float, float );
void draw_serpinski( int subdiv, float radius );
void draw_sphere( int subdiv, float radius );
void draw_icosahedron_wire( void );
void draw_icosahedron( void );
void draw_triangle4( int );
void draw_triangle4_2( int );
void draw_triangle5( int );
void draw_unitcube( int );
void trans_spiral_f( float, float, float, float, float );

#endif /* __HGR_H__ */
