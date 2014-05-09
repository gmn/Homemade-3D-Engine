// obj.cpp

#include "mmfile.h" 
#include "hgr.h"

inline int obj_t::TOLOWER( int x ) {
    if ( x >= 65 && x <= 90 ) {
        return x + 'a' - 'A';
    }
    return x;
}

inline int obj_t::check_int()
{
    if ( data[index] >= '0' && data[index] <= '9' ) 
        return 1;
    if (( data[index] == '+' || data[index] == '-' ) && 
        ( data[index+1] >= '0' && data[index+1] <= '9' )) 
        return 1;
    return 0;
}

// merely checks a string. doesn't advanced file index
int obj_t::check_name( const char * test )
{
    int i = index;
    int k = 0;

    while ( test[k] )
    {
        if ( TOLOWER( test[k++] ) != TOLOWER( data[i++] ) ) 
            return 0;

        if ( i >= size )
            return 0; // hit file end
    }

    // still another character left in the data string, voids the match
    if ( data[i] != ' ' && data[i] != '\r' && 
         data[i] != '\t' && data[i] != '\n' )
        return 0;

    return 1;
}

// consumes comments and white space 
inline void obj_t::strip_white_space ( void )
{
    // consume all comments and whitespace 
    // until we come to a meaningful character
    while ( 1 ) 
    {
        // comment(s)
        while ( data[index] == '#' ) {
            while ( data[index++] != '\n' && index < size-1 )
                ;
        }
        if ( index >= size - 1 )
            break;

        // white space proper
        while ((data[index] == ' '  ||
                data[index] == '\t' ||
                data[index] == '\n' ||
                data[index] == '\r' ) && index < size-1 )
            ++index;

        // exit conditions
        if ( index >= size - 1 )
            break;

        if ( data[index] != '#' && data[index] != ' ' && data[index] != '\t' &&
             data[index] != '\n' && data[index] != '\r' )
            break;
    }
}

// consume 1 line, irregardless of what's in it
void obj_t::consume_line( void )
{
    while ( index < size && data[index] != '\n' )
    {
        ++index;
    }
    strip_white_space();
}

float obj_t::get_float()
{
    char    fname[ 128 ];
    int     j = 0;

    while ( ( index < size ) &&
            (   ( data[index] >= '0' && data[index] <= '9' ) ||
                ( data[index] == '.' ) ||
                ( data[index] == 'e' ) ||
                ( data[index] == 'f' ) ||
                ( data[index] == '+' ) ||
                ( data[index] == '-' ) ) ) {
        fname[ j++ ] = data[ index++ ];
    }

    fname[j] = 0;
    return (float)atof( fname );
}

int obj_t::get_int()
{
    char    name[ 128 ];
    int     j = 0;

    while ( ( index < size ) &&
            (   ( data[index] >= '0' && data[index] <= '9' ) ||
                ( data[index] == 'u' ) ||
                ( data[index] == 'x' ) ||
                ( data[index] == '+' ) ||
                ( data[index] == '-' ) ) ) {
        fname[ j++ ] = data[ index++ ];
    }

    name[j] = 0;
    return atoi( name );
}

void obj_t::add_vertex()
{
    if ( data[index] != 'v' )
        return;
    ++index; // eat v
    strip_white_space();
    vec3_t v;
    v[0] = get_float();
    strip_white_space();
    v[1] = get_float();
    strip_white_space();
    v[2] = get_float();
    verts.add( v );
}

void obj_t::add_normal()
{
    if ( data[index] != 'v' && data[index] != 'n' )
        return;
    index += 2; // eat vn
    strip_white_space();
    vec3_t v;
    v[0] = get_float();
    strip_white_space();
    v[1] = get_float();
    strip_white_space();
    v[2] = get_float();
    normals.add( v );
}

void obj_t::add_texcoord()
{
    if ( data[index] != 'v' && data[index] != 't' )
        return;
    index += 2; // eat vt
    strip_white_space();
    vec3_t v;
    v[0] = get_float();
    strip_white_space();
    v[1] = get_float();
    strip_white_space();
    v[2] = get_float();
    texcoords.add( v );
}


void obj_t::add_face()
{
    if ( data[index] != 'f' || data[index+1] != ' ' )
        return;

    ++index;
    
    // spaces only
    while ( data[index] == ' ' )
        ++index;

    face_t f;
    
    while ( data[index] != '\n' )
    {
        point_t p;

        if ( check_int() )
            p.v = get_int();
        else if ( data[index] == '/' ) 
            ++index; 
        if ( check_int() )
            p.t = get_int();
        else if ( data[index] == '/' ) 
            ++index; 
        if ( check_int() )
            p.n = get_int();
        else if ( data[index] == '/' ) 
            ++index; 

        f.add( p );

        // spaces only
        while ( data[index] == ' ' )
            ++index;
    }

    faces.add( f );
}

obj_t::obj_t( const char * objname ) 
{
    // long name
    fullpath = realname( objname ); 

    // model handle
    const char * shortname = strip_path_and_extension( objname );
    name = copy_string( shortname );

    // memory map file
    mmfile_t mm( fullpath ); 
    index = 0;
    size = mm.size;
    data = mm.data;
    
    
    while ( index < size ) 
    {
        strip_white_space();

        switch( data[i] ) 
        {
        case 'm':
            if ( check_name( "mtllib" ) )
                consume_line();
            break;
        case 'u':
            if ( check_name( "usemtl" ) )
                consume_line();
            break;
        case 'g':
            consume_line();
            break;
        case '#':
            strip_white_space();
            break;
        case 'v':
            if ( check_name( "vn" ) )
                add_normal();
            else if ( check_name( "vt" ) )
                add_texcoord();
            else if ( data[index+1] == ' ' )
                add_vertex();
            else
                core.warn( "obj_t: straggling v in objfile: %s", name );
            break;
        case 'f':
            if ( data[index+1] == ' ' ) 
                add_face();
            break;
        case 's':
        default:
            ++index;
            break;
        }
    }
}

obj_t::~obj_t()
{
    if ( fullpath )
        free( fullpath );
    if ( name )
        free( name );
}

