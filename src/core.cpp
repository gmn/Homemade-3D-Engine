// core.cpp

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h> // vsnprintf

#include "core.h"
#include "console.h"
#include "hgr.h"
#include "gvar.h"
#include "game.h"

// global handle to system core functions
core_t core;


core_t::core_t() : myargc(0), myargv(0), gvar_table(256)
{
    gameModule_t * mod_p = new tunnelModule_t("tunnel");
    handler.registerModule( mod_p );
    handler.do_module( "tunnel" );
}

core_t::~core_t()
{
    //
    // delete gvar_t in gvar_table 
    //
    for ( int i = 0; i < gvar_table.size; i++ )
    {
        if ( gvar_table.nodes[i] )
        {
            for ( hashnode_t * tmp, * p = gvar_table.nodes[i]; p; p = tmp ) {
                tmp = p->next;
                if ( p->val )
                    delete ((gvar_t*) p->val);
                p->val = 0;
            }
        }
    }

}



void core_t::printf( const char *format, ... )
{
	va_list	argptr;
	char	msg[ 1024 ];
	
	va_start( argptr, format );
	vsnprintf( msg, sizeof( msg ), format, argptr );
	va_end( argptr );
	
	msg[ sizeof( msg ) - 1 ] = '\0';

    console.push_msg( msg );
    log.msg( msg );
}


// same as core::printf, but 
//  - starts the debug log output
//  - prepends the word "warning: " 
void core_t::warn( const char *format, ... )
{
	va_list	argptr;
	char	msg[ 1024 ];
	
	va_start( argptr, format );
	vsnprintf( msg, sizeof( msg ), format, argptr );
	va_end( argptr );
	
	msg[ sizeof( msg ) - 1 ] = '\0';

    console.printf( "warning: %s", msg );
    //dump_console();
}


void core_t::error( const char *fmt, ... ) 
{
	va_list		argptr;
	
	va_start( argptr, fmt );
	
	char buffer[1024];
	vsnprintf( buffer, sizeof( buffer ), fmt, argptr );
	va_end( argptr );
    
    console.printf( "fatal error: %s", buffer );
    //dump_console();
	
    // FIXME: call shutdown handler directly
	exit( 1 ); // cleanup handled in atexit(); 
}


void core_t::dump_console( void )
{
    log.start(); // dump called, make sure log started 

    message_t * p = console.messages;
    while ( p )
    {
        log.printf( "%s", p->msg );
        p = p->next;
    }
}


void core_t::dump_gvar( void )
{
    log.start();
    int len = 0;
    int n;

    for ( int i = 0; i < gvar_table.size; i++ )
    {
        hashnode_t * p = gvar_table.nodes[i];
        while ( p )
        {
            n = strlen( ((gvar_t*)p->val)->identifier );
            if ( n > len )
                len = n;
            p = p->next;
        }
    }

    char buf[50];
    sprintf( buf, " %%-%ds%%s\n", len + 2 );

    if ( gvar_table.size > 0 )
        log.msg( "gvar_t: \n" );
    for ( int i = 0; i < gvar_table.size; i++ )
    {
        hashnode_t * p = gvar_table.nodes[i];
        while ( p )
        {
            log.printf( (const char *)buf, ((gvar_t*)p->val)->identifier, ((gvar_t*)p->val)->string );
            p = p->next;
        }
    }
}


// returns pointer to it if it exists
const char ** core_t::check_cmdline( const char *parm )
{
    int i;
    for ( i = 1; i < myargc; i++ ) {
        if ( ! strcmp( parm, myargv[i] ) ) {
            return (const char **) &myargv[i];
        }
    }
    return (const char **)0;
}

// get argument to a particular cmd switch, if any...
const char * core_t::get_cmdline_arg( const char * cmd )
{
    int i;
    for ( i = 1; i < myargc; i++ ) {
        if ( ! strcmp( cmd, myargv[i] ) ) {
            if ( (i+1) < myargc )
            {  
                // check for leading '-', if none, return it
                const char * p = myargv[ i + 1 ];
                if ( *p != '-' )
                    return p;
                else
                    return (const char *)0;
            }
        }
    }
    return (const char *)0;
}



