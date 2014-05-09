// gvar.cpp

#include <stdlib.h>
#include <string.h>

#include "gvar.h"
#include "hash.h"
#include "hgr.h"
#include "core.h"




// create, and store in hashtable 
gvar_t::gvar_t ( const char * name, const char * value, const char * _comments )
                    : string(0), comments(0)
{
    /*
     * init internal state
     */
    memset( identifier, 0, sizeof( identifier ) );

    // name
    identifier[ 0 ] = '\0';
    if ( name && *name )
    {
        strncpy( identifier, name, GVAR_IDENTIFIER_SIZE );
        identifier[ GVAR_IDENTIFIER_SIZE - 1 ] = '\0';
        if ( strlen( name )-1 >= GVAR_IDENTIFIER_SIZE )
            core.warn( "string: \"%s\" is longer than gvar identifier size: %d", name, GVAR_IDENTIFIER_SIZE );
    }

    // string
    if ( value && *value ) 
    {
        int a_size  = strlen( value ) + sizeof( char ) ;
        string = new char[ a_size ];
        strncpy( string, value, a_size );
        string[ a_size - 1 ] = '\0';
        // i, f 
        integer  = atoi( value );
        fval     = atof( value );
    }
    else
    {
        string = 0;
        integer = 0;
        fval = 0.0f;
    }

    // comments
    if ( _comments && *_comments )
    {
        int a_size  = strlen( _comments ) + sizeof( char ) ;
        this->comments = new char[ a_size ];
        strncpy( this->comments, _comments, a_size );
        this->comments[ a_size - 1 ] = '\0';
    }

    /* 
     * save in table
     */
    core.gvar_table.insert( name, this );
}

gvar_t::~gvar_t() 
{
    if ( comments )
        delete comments;
    if ( string )
        delete string;
}

gvar_t * gvar_t::find( const char * name )
{
    gvar_t * g = (gvar_t *) core.gvar_table.retrieve( name );
    if ( ! g )
        return (gvar_t *)0;
    return g;
}

// change the value of a gvar that already exists
void gvar_t::set( const char * newval, const char * new_comments )
{
    int nlen = strlen( newval ) + 1;
    int olen = strlen( string ) + 1;
    int len;
    if ( nlen > olen )
    {
        delete string;
        string = new char[ nlen ];
        len = nlen;
    }
    else
    {
        len = olen;
    }

    memset( string, 0, len );
    strncpy( string, newval, len );
    string[ len - 1 ] = '\0';
    integer  = atoi( newval );
    fval     = atof( newval );

    if ( ! new_comments )
    {
        return;
    }

    nlen = strlen( new_comments ) + 1;
    if ( ! comments )
        olen = 0;
    else
        olen = strlen( comments );

    if ( nlen > olen ) {
        delete comments;
        comments = new char[ nlen ];
        len = nlen;
    } else {
        len = olen;
    }
    memset( comments, 0, len );
    strcpy( comments, new_comments );
    comments[ len - 1 ] = '\0';
}

