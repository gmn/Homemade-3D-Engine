#include <ctype.h> // tolower
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// simplest possible hash table. takes a string, returns a void *
#include "hash.h"
#include <stdlib.h>

hashnode_t::hashnode_t() : fullhash(0),str(0),val(0),next(0) { }

hashnode_t::hashnode_t( int fh, const char * name, void * store ) : next(0)
{
    fullhash = fh;
    str = (char*) malloc( strlen( name ) + 1 );
    strcpy( str, name );
    val = store;
}

hashnode_t::~hashnode_t() {
    if ( str )
        free( str );
    str = 0;
}

int hash_string( const char *str ) 
{
	int hash = *str;
	
	if ( hash ) 
    {
		for ( str += 1; *str != '\0'; ++str ) 
        {
			hash = (hash << 5) - hash + tolower(*str);
		}
	}
	return hash;
}


hashtable_t::hashtable_t( void ) : size(0),mask(0),nodes(0),stored(0)
{
    init( HASH_TABLE_DEFAULT_SZ );
}

hashtable_t::hashtable_t( int _size ) : size(0),mask(0),nodes(0),stored(0)
{
    if ( _size <= 0 )
        _size = HASH_TABLE_DEFAULT_SZ;
    init( _size );
}

void hashtable_t::init( int _size )
{
    _size = ( _size + 7 ) & ~7;
    
    int s = _size, c = 0;
    if ( s < 0 ) 
        { s = 8; }

    do {
        s >>= 1;
        ++c;
    }
    while ( s > 1 );

    size = 1 << c;
    mask = size - 1;

    nodes = (hashnode_t**) calloc( size, sizeof(hashnode_t*) );
}


hashtable_t::~hashtable_t()
{
    for ( int i = 0; i < size; i++ ) 
    {
        if ( nodes[i] ) {
            for ( hashnode_t * tmp, * p = nodes[i]; p; p = tmp ) {
                tmp = p->next;
                delete p;
            }
            nodes[i] = 0;
        }
    }

    free( nodes );
}


void hashtable_t::insert( const char * str, void * store )
{
    int fullhash = hash_string( str );
    int index = fullhash & mask;

    hashnode_t * node = new hashnode_t( fullhash, str, store );
    
    if ( ! nodes[index] )
    {
        nodes[ index ] = node;
    }
    else
    {
        hashnode_t * p = nodes[ index ];
        while ( p->next )
            p = p->next;

        p->next = node;
    }
    
    ++stored;
}

void * hashtable_t::retrieve( const char * str )
{
    int fullhash = hash_string( str );
    int index = fullhash & mask;
    
    hashnode_t * p = nodes[ index ];
    while ( p ) 
    {
        // maybe found it...
        if ( p->fullhash == fullhash )
        {
            // definitely found it
            if ( strcmp( p->str, str ) == 0 )
            {
                return p->val;
            }
        }

        p = p->next;
    }

    return (void*)0;
}

