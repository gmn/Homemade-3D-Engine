// logger.cpp

#include <stdarg.h>
#include <string.h> // strlen


#include "logger.h"



const char * logger_t::def_logname = DEF_LOGNAME;

void logger_t::start( int append )
{
    if ( fp )
        return; // already started

    if ( append )
        fp = fopen ( logger_t::def_logname, "a" );
    else
        fp = fopen ( logger_t::def_logname, "w" );
}

void logger_t::close ( void )
{
    if ( ! fp )
        return;
    fclose( fp );
    fp = 0x0;
}

void logger_t::msg( const char * msg )
{
    if ( ! fp )
        return;

    fwrite( msg, 1, strlen( msg ), fp );
}

void logger_t::printf( const char * format, ... ) 
{
    if ( ! fp )
        return;

	va_list	argptr;
	va_start( argptr, format );
    vfprintf( fp, format, argptr );
	va_end( argptr );
}

logger_t::~logger_t()
{
    close(); // safe to call, started or not
}


