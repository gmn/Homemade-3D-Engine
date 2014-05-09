// primitives - geometry primitives vertices sets

#include "geometry.h"


struct heightMap_t 
{
    float * map;

    int w;
    int h;
    float height;      // each point will be from 0 to height

    heightMap_t( int a, int b, float c ) ;

    ~heightMap_t() { delete[] map; }

    float& operator[] ( int i ) { return map[i]; }
};



// creates vertices for a tessellated plane from a heightmap
struct tessellatedPlane_t 
{
    vec3_t *    verts;
    int         num_verts;      // # of vertices
    float       w, h;           // width & height of plane in world-space 
    int         row, column;       // # verts in each direction
    float *     array;          // sz = num_verts * 6

    

    tessellatedPlane_t( heightMap_t& m, float iw, float ih ) ;

    void setColor( float r, float g, float b ) ;
    void randomColors( void );
    void gradedTones( float, float, float );

    ~tessellatedPlane_t() 
    { 
        delete[] verts; 
        delete[] array; 
    }

    vec3_t& operator[] ( int i ) { 
        return verts[i]; 
    } 

}; // tessellatedPlane_t

