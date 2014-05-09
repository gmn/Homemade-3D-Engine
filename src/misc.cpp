#include <ctype.h> // tolower
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <SDL/SDL.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "hgr.h"
#include "core.h"

char *va( const char *format, ... ) 
{
	va_list	argptr;
	static char	string[ 8192 ];
	
	va_start( argptr, format );
	vsnprintf( string, sizeof( string ), format, argptr );
	va_end( argptr );
	
	string[ sizeof( string ) - 1 ] = '\0';
	
	return string;	
}


// disabled when compiler _DEBUG not set
void _hidden_Assert( int die_if_false, const char *exp, const char *file, int line ) 
{
	if ( !die_if_false ) 
    {
        fprintf( stderr, "Assert failed with expression: \"%s\" at: %s, line: %d\n", exp, file, line );
		fflush( stderr );

#ifdef _WINDOWS
		__asm {
			int 0x3;
		}
#endif

        core.error( "Assert failed with expression: \"%s\" at: %s, line: %d\n", exp, file, line );
	}
}

#if 0
// blocks until ANY key is pressed
// returns key
int block_for_any_key()
{
    int c;
    do
    {
        if ( (c=getch()) != ERR )
            return c;

        SDL_Delay( 10 );

    } while(1);
}


// stalls for N ms, but exits if (any) keypress detected
// returns key
int poll_for_any_key( int ms )
{
    int fin = get_milliseconds() + ms;
    int c;

    do
    {
        if ( get_milliseconds() > fin )
            return 0;
        if ( (c=getch()) != ERR )
            return c;

        SDL_Delay( 10 );

    } while(1);
}
#endif


char ** list_directory( const char * dir )
{
    char * fullpath = realpath( dir, 0 );

    DIR * dirp = opendir( dir );
    if ( !dirp ) {
        core.warn( "bad dir: %s\n", dir );
        return 0;
    }

    struct dirent * ent = 0;
    int files = 0;

    while ( (ent = readdir( dirp )) )
    {
        if ( *ent->d_name != '.' && (ent->d_type == DT_DIR || ent->d_type == DT_REG) )
            ++files;
    }
    closedir( dirp );

    unsigned int sz = (files+1) * sizeof(char*) ;
    char ** pp = (char**) malloc ( sz );
    memset( pp, 0, sz );

    dirp = opendir( dir );
    int i = 0;
    while ( (ent = readdir( dirp )) )
    {
        if ( *ent->d_name != '.' && (ent->d_type == DT_DIR || ent->d_type == DT_REG) )
        {
            char buf[2048];
            sprintf( buf, "%s/%s", fullpath, ent->d_name );
            pp[i] = copy_string( buf );
            if ( ++i >= files )
                break;
        }
    }
    closedir( dirp );
    free( fullpath );
    return pp;
}

void free_listdir( char ** pp )
{
    char ** save_p = pp;
    while ( *pp ) 
        free( *pp++ );
    free ( save_p );
}

unsigned int get_filesize( const char * path )
{
    struct stat ss;
    stat( path, &ss );
    return (unsigned int)ss.st_size;   
}

// looks for both kinds of path seperator
const char * strip_path( const char *s ) 
{
    const char *w = strrchr ( s, '\\' );
    const char *u = strrchr ( s, '/' );

    if ( !w && !u )
        return s;

    if ( w && u )
        return s;

    if ( w && !u ) {
        return (const char *)(w+1);
    }

    if ( !w && u ) {
        return (const char *)(u+1);
    }

    return s;
}

const char * strip_extension( const char *s ) 
{
    static char buf[ 1000 ];

    if ( !s || !s[0] ) {
        buf[0] = 0;
        return buf;
    }

    memset( buf, 0, sizeof(buf) );
    strcpy( buf, s );

    char * ne = strrchr( buf, '.' );
    if ( !ne )
        return buf;

    *ne = '\0';
    return buf;
}

const char * strip_path_and_extension( const char * s ) 
{
    static char buf[ 100 ];

    if ( !s || !s[0] ) {
        buf[0] = 0;
        return buf;
    }

    const char * basename = strip_path( s );
    memset( buf, 0, sizeof(buf) );
    strcpy( buf, basename );

    char * ne = strrchr( buf, '.' );
    if ( !ne )
        return basename;

    *ne = '\0';
    return buf;
}

#define TO_LOWER( x ) \
    (( (x) >= 65 && (x) <= 90 ) ?  (x) + 'a' - 'A' : (x) )
#define TO_UPPER( x ) \
    (( (x) >= 97 && (x) <= 122 ) ? (x) - 'a' + 'A' : (x) )

// converts a whole string, in place
char * str_toupper( char * s ) {
    char * p = s;
    while ( *p ) {
        int n = *p;
        *p++ = TO_UPPER( n );
    }
    return s;
}

char * str_tolower( char * s ) {
    char * p = s;
    while ( *p ) {
        int n = *p;
        *p++ = TO_LOWER( n );
    }
    return s;
}

char * copy_string( const char * s ) 
{
    if ( !s || !s[0] ) 
        return '\0';
    char * str = (char *) malloc( strlen( s ) + sizeof(char) );
    memset( str, 0, strlen( s ) + sizeof(char) );
    strcpy( str, s );
    return str;
}

/* replaces instance of "search" with "replace" in subject, in place */
void chr_replace( const char s, const char r, char * q )
{
    while ( *q++ )
        if ( *q == s )
            *q = r;
}

/*
====================
 FlipULong

    endian utility
====================
*/
unsigned long FlipULong( unsigned long x ) {
    return ((x>>24)&255) | (((x>>16)&255)<<8) | (((x>>8)&255)<<16) | ((x&255)<<24);
}

/*
====================
 little_endian

    helper function to tell if system is little endian
====================
*/
int is_little_endian( void ) {
    short _short_int = 0x0001;
    return (int)((char*)&_short_int)[0];
}

