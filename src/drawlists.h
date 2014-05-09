#ifndef __DRAWTYPES_H__
#define __DRAWTYPES_H__

#include "datastruct.h"
#include "geometry.h"

struct tessellation_t
{
    int type;
    mat16_t rotation;
    vec3_t  origin;
    tessellation_t * next;

    tessellation_t() : type(0), origin(0.f,0.f,0.f), next(0) { }
    virtual ~tessellation_t() {}
};

struct tesQuads_t : public tessellation_t
{
    buffer_t<vec3_t> buf;
    buffer_t<vec3_t> colors;

    tesQuads_t() {}
    tesQuads_t( int n ) : buf(n), colors(n) {}
    ~tesQuads_t() {}

    vec3_t cur; // current color

    void setColor( float r, float g, float b )
    {
        cur[0] = r; cur[1] = g; cur[2] = b;
    }

    void add( float i, float j, float k )
    {
        buf.push( vec3_t( i, j, k ) );
        colors.push( cur );
    }
};

enum {
    DT_QUADS,
    DT_TRIANGLES,
};

struct drawer_t 
{
    tessellation_t * head;
    drawer_t() : head(0) {}
    ~drawer_t() 
    {
        for ( tessellation_t * p = head, * tmp; p ; p = tmp ) {
            tmp = p->next;
            delete p;
        }
    }

    void setCube( float x, float y, float z, float r, float g, float b )
    {
        tesQuads_t * buf = new tesQuads_t( 3 * 6 );
        
        buf->type = DT_TRIANGLES;
        buf->setColor( r, g, b );
        buf->origin.set(x,y,z);

        buf->add( 0.f, 0.f, 0.f );
        buf->add( 10.f, 0.f, 0.f );
        buf->add( 0.f, 10.f, 0.f );
        buf->add( 10.f, 10.f, 0.f );
        buf->add( 0.f, 10.f, -10.f );
        buf->add( 10.f, 10.f, -10.f );

        buf->add( 10.f, 10.f, 0.f );
        buf->add( 10.f, 0.f, 0.f );
        buf->add( 10.f, 10.f, -10.f );
        buf->add( 10.f, 0.f, -10.f );
        buf->add( 0.f, 10.f, -10.f );
        buf->add( 0.f, 0.f, -10.f );

        buf->add( 0.f, 10.f, 0.f );
        buf->add( 0.f, 0.f, 0.f );
        buf->add( 0.f, 0.f, -10.f );
        buf->add( 10.f, 0.f, -10.f );
        buf->add( 0.f, 0.f, 0.f );
        buf->add( 10.f, 0.f, 0.f );

        push( buf );
    }

    void push( tessellation_t * tes )
    {
        if ( ! head ) {
            head = tes; 
            return;
        }

        tessellation_t * p = head;
        while ( p->next ) 
            p = p->next;

        p->next = tes;
    }

    tessellation_t * getLast() { 
        tessellation_t * p = head;
        while ( p->next ) 
            p = p->next;
        return p;
    }

    void drawLists( void );
    void drawQuads( tessellation_t * );
    void drawTriangles( tessellation_t * );
};

#endif // __DRAWTYPES_H__
