// console.cpp

#include <string.h>
#include <stdarg.h>
#include <stdio.h> // vsnprintf

//#include <sys/types.h> // fstat
#include <sys/stat.h>

#include "console.h"
#include "hgr.h"


/**
 * message_t
 */
message_t::message_t( const char * _msg, int ttl ) : next(0)
{
    msg = new char[ strlen( _msg ) + 10 ];

    memset( msg, 0, strlen( _msg ) + 10 );

    strcpy( msg, _msg );

    expire_time = get_milliseconds() + ttl;
}

message_t::~message_t() {
    if ( msg )
        delete msg;
}


/**
 * console_t
 */
console_t::~console_t()
{
    // free_messages
    message_t * p, * tmp; 
    p = messages;
    while ( p ) 
    {
        tmp = p->next;
        delete p;
        p = tmp;
    }
}

void console_t::push_msg( const char *msg )
{
    message_t * p;

    if ( ! messages )
    {
        lastmsg_p = messages = new message_t( msg, default_msg_ttl );
    }
    else
    {
        p = messages;
        while ( p->next ) 
            p = p->next;

        p->next = new message_t( msg, default_msg_ttl );
    }

    // send messages to stdout if it is available
    int fd = fileno( stdout );
    struct stat ss;
    if ( 0 == fstat( fd, &ss ) )
    {
        if ( S_IWOTH | ss.st_mode )
            fprintf( stdout, "%s", msg );
    }
}

void console_t::printf( const char *format, ... )
{
	va_list	argptr;
	char	msg[ 1024 ];
	
	va_start( argptr, format );
	vsnprintf( msg, sizeof( msg ), format, argptr );
	va_end( argptr );
	
	msg[ sizeof( msg ) - 1 ] = '\0';

    push_msg( msg );
}


