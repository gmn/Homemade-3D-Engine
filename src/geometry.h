#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <math.h>
//#include <string.h>
#include <stdlib.h> //malloc
#include <stdio.h> // sprintf

// TODO: detect 64-bit architecture, copy in 8-byte segs instead
static inline void l_memset( void * vp, unsigned char byte, unsigned int len_bytes )
{
    const unsigned int val = byte | byte<<8 | byte<<16 | byte<<24;
    
    int rem = len_bytes % 4;
    const int len_32 = len_bytes / 4;

    // copy in 4-byte vals
    register unsigned int * p = (unsigned int *) vp;
    const unsigned int * D = p + len_32;
    while ( p != D )
        *p++ = val;

    if ( rem ) {
        unsigned char * c = (unsigned char *) p;
        do 
        {
            *c++ = byte;
        }
        while ( --rem >= 0 );
    }
}
// l_memset

// TODO
typedef float vec2_t[2];


/********************************************************
 * vec3_t
 */
struct vec3_t 
{
    float vec[3];
    
    // ctors
    vec3_t() { } 
    vec3_t( float a ) { vec[0] = vec[1] = vec[2] = a; }
    vec3_t( float a, float b, float c ) { 
        vec[0] = a;
        vec[1] = b;
        vec[2] = c; 
    }  
    vec3_t( float * a ) { 
        vec[0] = a[0]; 
        vec[1] = a[1]; 
        vec[2] = a[2]; 
    }
    vec3_t( const vec3_t& v ) { 
        vec[0]=v.vec[0]; 
        vec[1]=v.vec[1]; 
        vec[2]=v.vec[2]; 
    }

    //
    // operators
    //

    // asgn op
    vec3_t& operator= ( vec3_t& v ) {
        vec[0] = v.vec[0];
        vec[1] = v.vec[1];
        vec[2] = v.vec[2];
        return *this;
    }

    float& operator[] ( int i ) { return vec[i]; }
    
    vec3_t operator+ ( const vec3_t& v ) {
        return vec3_t( vec[0] + v.vec[0], vec[1] + v.vec[1], vec[2] + v.vec[2] );
    }
    
    vec3_t operator- ( const vec3_t& v ) {
        return vec3_t( vec[0] - v.vec[0], vec[1] - v.vec[1], vec[2] - v.vec[2] );
    }
    vec3_t operator- () { //  unary -
        return vec3_t( -vec[0], -vec[1], -vec[2] );
    }

    void operator-= ( const vec3_t& v ) {
        vec[0] -= v.vec[0];
        vec[1] -= v.vec[1];
        vec[2] -= v.vec[2];
    }

    void operator+= ( const vec3_t& v ) {
        vec[0] += v.vec[0];
        vec[1] += v.vec[1];
        vec[2] += v.vec[2];
    }

    void operator*= ( float f ) {
        vec[0] *= f;
        vec[1] *= f;
        vec[2] *= f;
    }

    void operator/= ( float f ) {
        float v = 1.0f/f;
        vec[0] *= v;
        vec[1] *= v;
        vec[2] *= v;
    }

    vec3_t operator* ( float f ) {
        return vec3_t( vec[0] * f, vec[1] * f, vec[2] * f );
    }
    vec3_t operator/ ( float f ) {
        float oof = 1.0f/f;
        return vec3_t( vec[0] * oof, vec[1] * oof, vec[2] * oof );
    }

    void set( float a, float b, float c ) { 
        vec[0] = a;
        vec[1] = b;
        vec[2] = c; 
    }  

    float magnitude() {
        return sqrtf( vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] );
    }

    void normalize() {
        float mag = vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];
        mag = 1.f / sqrtf( mag );
        vec[0] *= mag;
        vec[1] *= mag;
        vec[2] *= mag;
    }

    vec3_t crossWith( const vec3_t& v )
    {
        return vec3_t(  vec[1] * v.vec[2] - vec[2] * v.vec[1], 
                        vec[2] * v.vec[0] - vec[0] * v.vec[2], 
                        vec[0] * v.vec[1] - vec[1] * v.vec[0] );
    }

    float dot( const vec3_t& v )
    {
        return v.vec[0] * vec[0] + v.vec[1] * vec[1] + v.vec[2] * vec[2];
    }
    
    int operator==( const vec3_t& v )
    {
        return vec[0] == v.vec[0] && vec[1] == v.vec[1] && vec[2] == v.vec[2];
    }

    int operator!=( const vec3_t& v )
    {
        return vec[0] != v.vec[0] || vec[1] != v.vec[1] || vec[2] != v.vec[2];
    }

    void zero() { vec[0] = vec[1] = vec[2] = 0.f; }

    float distance( const vec3_t& a ) {
        float v[3];
        v[0] = vec[0] - a.vec[0];
        v[1] = vec[1] - a.vec[1];
        v[2] = vec[2] - a.vec[2];
        return sqrtf( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
    }


}; // vec3_t


/********************************************************
 * vec4_t
 */
struct vec4_t 
{
    float vec[4];
    
    // ctors
    vec4_t() {} 
    vec4_t( float a ) { vec[0] = vec[1] = vec[2] = vec[3] = a; }
    vec4_t( float a, float b, float c, float d ) { 
        vec[0] = a; vec[1] = b; vec[2] = c; vec[3] = d;
    }  
    vec4_t( float * a ) { 
        vec[0] = a[0]; vec[1] = a[1]; vec[2] = a[2]; vec[3] = a[3];
    }
    vec4_t( const vec4_t& v ) { 
        vec[0]=v.vec[0]; vec[1]=v.vec[1]; vec[2]=v.vec[2]; vec[3] = v.vec[3];
    }

    //
    // operators
    //

    // assignment 
    // the compiler provides vec4_t operator= (void), 
    // (this how the compiler would write it anyway)
    vec4_t& operator= ( const vec4_t& v ) {
        vec[0] = v.vec[0];
        vec[1] = v.vec[1];
        vec[2] = v.vec[2];
        vec[3] = v.vec[3];
        return *this;
    }

    float& operator[] ( int i ) { return vec[i]; }
    
    vec4_t operator+ ( const vec4_t& v ) {
        return vec4_t( vec[0] + v.vec[0], vec[1] + v.vec[1], vec[2] + v.vec[2], vec[3] + v.vec[3] );
    }
    
    vec4_t operator- ( const vec4_t& v ) {
        return vec4_t( vec[0] - v.vec[0], vec[1] - v.vec[1], vec[2] - v.vec[2], vec[3] - v.vec[3] );
    }

    vec4_t operator- () { //  unary, eg 'a = -b'
        return vec4_t( -vec[0], -vec[1], -vec[2], -vec[3] );
    }

    void operator-= ( const vec4_t& v ) {
        vec[0] -= v.vec[0];
        vec[1] -= v.vec[1];
        vec[2] -= v.vec[2];
        vec[3] -= v.vec[3];
    }

    void operator+= ( const vec4_t& v ) {
        vec[0] += v.vec[0];
        vec[1] += v.vec[1];
        vec[2] += v.vec[2];
        vec[3] += v.vec[3];
    }

    void operator*= ( float f ) {
        vec[0] *= f;
        vec[1] *= f;
        vec[2] *= f;
        vec[3] *= f;
    }

    void operator/= ( float f ) {
        float v = 1.0f/f;
        vec[0] *= v;
        vec[1] *= v;
        vec[2] *= v;
        vec[3] *= v;
    }

    vec4_t operator* ( float f ) 
    {
        return vec4_t( vec[0] * f, vec[1] * f, vec[2] * f, vec[3] * f );
    }

    vec4_t operator/ ( float f ) 
    {
        float oof = 1.0f/f;
        return vec4_t( vec[0]*oof, vec[1]*oof, vec[2]*oof, vec[3]*oof );
    }

    void set( float a, float b, float c, float d ) { 
        vec[0] = a;
        vec[1] = b;
        vec[2] = c; 
        vec[3] = d;
    }  

    void set( const vec4_t& v ) {
        vec[0] = v.vec[0];
        vec[1] = v.vec[1];
        vec[2] = v.vec[2];
        vec[3] = v.vec[3];
    }

    float magnitude() 
    {
        return sqrtf( vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2] + vec[3]*vec[3] );
    }

    void normalize() 
    {
        float mag = vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3];
        mag = 1.f / sqrtf( mag );
        vec[0] *= mag;
        vec[1] *= mag;
        vec[2] *= mag;
        vec[3] *= mag;
    }

    vec4_t crossWith( const vec4_t& v )
    {
        return vec4_t(  vec[1] * v.vec[2] - vec[2] * v.vec[1], 
                        vec[2] * v.vec[3] - vec[3] * v.vec[2], 
                        vec[3] * v.vec[0] - vec[0] * v.vec[3], 
                        vec[0] * v.vec[1] - vec[1] * v.vec[0] );
    }

    float dot( const vec4_t& v )
    {
        return  v.vec[0] * vec[0] + 
                v.vec[1] * vec[1] + 
                v.vec[2] * vec[2] + 
                v.vec[3] * vec[3];
    }
    
    int operator==( const vec4_t& v )
    {
        return  vec[0] == v.vec[0] && 
                vec[1] == v.vec[1] && 
                vec[2] == v.vec[2] && 
                vec[3] == v.vec[3];
    }

    int operator!=( const vec4_t& v )
    {
        return  vec[0] != v.vec[0] || 
                vec[1] != v.vec[1] || 
                vec[2] != v.vec[2] || 
                vec[3] != v.vec[3];
    }

    void zero() { vec[0] = vec[1] = vec[2] = vec[3] = 0.f; }

    float distanceFrom( const vec4_t& a ) 
    {
        float v[4];
        v[0] = vec[0] - a.vec[0];
        v[1] = vec[1] - a.vec[1];
        v[2] = vec[2] - a.vec[2];
        v[3] = vec[3] - a.vec[3];
        return sqrtf( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] + v[3]*v[3] );
    }
};

//struct mat9_t { };

/******************************************************** 
 * mat16_t
 *
 *      4 x 4 float matrix
 */
struct mat16_t 
{
    float mat[16];

    static const float p180;
    static const float ipi;

    // constructors
    mat16_t() { 
        identity(); 
    }

    mat16_t( const float n ) {
        for ( register int i = 0; i < 16; i++ )
            mat[i] = n;
    }
    
    mat16_t( const float * f ) 
    {
        for ( register int i = 0; i < 16; i++ )
            mat[i] = f[i];
    }
    
    mat16_t(float x1, float x2, float x3, float x4, 
            float y1, float y2, float y3, float y4,
            float z1, float z2, float z3, float z4,
            float w1, float w2, float w3, float w4 )
    {
        register float * f = mat;
        *f++ = x1;
        *f++ = x2;
        *f++ = x3;
        *f++ = x4;
        *f++ = y1;
        *f++ = y2;
        *f++ = y3;
        *f++ = y4;
        *f++ = z1;
        *f++ = z2;
        *f++ = z3;
        *f++ = z4;
        *f++ = w1;
        *f++ = w2;
        *f++ = w3;
        *f   = w4;
    }

    mat16_t( mat16_t const& m ) 
    {
        for ( register int i = 0; i < 16; i++ )
            mat[i] = m.mat[i];
    }
    
    void copy_in( const float * f ) 
    {
        for ( register int i = 0; i < 16; i++ )
            mat[i] = f[i];
    }

    void zero() 
    {
        l_memset(mat,0,sizeof(mat));
    }

    void identity() 
    {
        l_memset(mat,0,sizeof(mat));
        mat[0] = mat[5] = mat[10] = mat[15] = 1.0f;
    }

    void setRow( int _index, const vec4_t& v ) 
    {
        if ( _index >= 0 && _index < 4 ) { 
            register int index = _index * 4;
            mat[ index + 0 ] = v.vec[0];
            mat[ index + 1 ] = v.vec[1];
            mat[ index + 2 ] = v.vec[2];
            mat[ index + 3 ] = v.vec[3];
        }
    }

    void setRow( int _index, const float * f ) 
    {
        if ( _index >= 0 && _index < 4 ) { 
            register int index = _index * 4;
            mat[ index + 0 ] = f[0];
            mat[ index + 1 ] = f[1];
            mat[ index + 2 ] = f[2];
            mat[ index + 3 ] = f[3];
        }
    }

    void setRow( int _index, const mat16_t& m ) 
    {
        if ( _index >= 0 && _index < 4 ) { 
            register int index = _index * 4;
            mat[ index + 0 ] = m.mat[ index + 0 ];
            mat[ index + 1 ] = m.mat[ index + 1 ];
            mat[ index + 2 ] = m.mat[ index + 2 ];
            mat[ index + 3 ] = m.mat[ index + 3 ];
        }
    }

    void setCol( int index, const vec4_t& v ) 
    {
        if ( index >= 0 && index < 4 ) { 
            mat[ index + 0 ] = v.vec[0];
            mat[ index + 4 ] = v.vec[1];
            mat[ index + 8 ] = v.vec[2];
            mat[ index + 12] = v.vec[3];
        }
    }

    void setCol( int index, const float * f ) 
    {
        if ( index >= 0 && index < 4 ) { 
            mat[ index + 0 ] = f[0];
            mat[ index + 4 ] = f[1];
            mat[ index + 8 ] = f[2];
            mat[ index + 12] = f[3];
        }
    }

    void setCol( int index, mat16_t const& m ) 
    {
        if ( index >= 0 && index < 4 ) { 
            mat[ index + 0 ] = m.mat[ index + 0 ];
            mat[ index + 4 ] = m.mat[ index + 4 ];
            mat[ index + 8 ] = m.mat[ index + 8 ];
            mat[ index + 12] = m.mat[ index + 12 ];
        }
    }


    void left( float x )
    {
        vec4_t left(mat[0],mat[1],mat[2],0.f);
        left.normalize();
        left *= x;

        mat16_t mult(   1.f, 0.f, 0.f, left[0],
                        0.f, 1.f, 0.f, left[1],
                        0.f, 0.f, 1.f, left[2],
                        0.f, 0.f, 0.f, 1.f );
            
        *this = *this * mult;
    }

    void right( float x )
    {
        vec4_t right(-mat[0],-mat[1],-mat[2],0.f);
        right.normalize();
        right *= x;

        mat16_t mult(   1.f, 0.f, 0.f, right[0],
                        0.f, 1.f, 0.f, right[1],
                        0.f, 0.f, 1.f, right[2],
                        0.f, 0.f, 0.f, 1.f );
            
        *this = *this * mult;
    }

    void forward( float z )
    {
        vec4_t ahead ( mat[8],mat[9],mat[10],0.f );
        ahead.normalize();
        ahead *= z;

        mat16_t mult(   1.f, 0.f, 0.f, ahead[0],
                        0.f, 1.f, 0.f, ahead[1],
                        0.f, 0.f, 1.f, ahead[2],
                        0.f, 0.f, 0.f, 1.f );
            
        *this = *this * mult;
    }

    void backward( float z )
    {
        vec4_t back ( -mat[8],-mat[9],-mat[10],0.f );
        back.normalize();
        back *= z;

        mat16_t mult(   1.f, 0.f, 0.f, back[0],
                        0.f, 1.f, 0.f, back[1],
                        0.f, 0.f, 1.f, back[2],
                        0.f, 0.f, 0.f, 1.f );
            
        *this = *this * mult;
    }

    void up( float y )
    {
    }

    void down( float y )
    {
    }

    void translate( float x, float y, float z )
    {
/*      1 0 0 x 
        0 1 0 y
        0 0 1 z 
        0 0 0 1 */
    }

    void rotate ( float degrees, float x, float y, float z )
    {
        const float s = sinf( p180 * degrees );
        const float c = cosf( p180 * degrees );
        vec3_t v(x,y,z);
        v.normalize();

        mat16_t m (
        v[0]*v[0]*(1-c)+c, v[0]*v[1]*(1-c)-v[2]*s, v[0]*v[2]*(1-c)+v[1]*s, 0.f, 
        v[1]*v[0]*(1-c)+v[2]*s, v[1]*v[1]*(1-c)+c, v[1]*v[2]*(1-c)-v[0]*s, 0.f, 
        v[2]*v[0]*(1-c)-v[1]*s, v[2]*v[1]*(1-c)+v[0]*s, v[2]*v[2]*(1-c)+c, 0.f,
        0.f, 0.f, 0.f, 1.f );

        *this = *this * m;
    }

    void rotateX( float degrees )
    {
        //rotate( degrees, mat[0], mat[1], mat[2] );
        rotate( degrees, 1.f, 0.f, 0.f );
    }

    void rotateY( float degrees )
    {
        //rotate( degrees, mat[4], mat[5], mat[6] );
        rotate( degrees, 0.f, 1.f, 0.f );
    }

    void rotateZ( float degrees )
    {
        rotate( degrees, mat[8], mat[9], mat[10] );
    }

    // return matrix with location column zeroed out
    mat16_t getRotationMatrix()
    {
        mat16_t b( mat[0], mat[1], mat[2], 0.f, 
                   mat[4], mat[5], mat[6], 0.f,
                   mat[8], mat[9], mat[10], 0.f,
                   0.f,    0.f,    0.f,    1.f );
        return b;
    }

    float& operator[] ( int i ) { return mat[i]; }

    mat16_t operator* ( mat16_t const& m ) // FIXME: could be better
    {
        float a[16];

        a[0] = mat[0]*m.mat[0] + mat[1]*m.mat[4] + mat[2]*m.mat[8] + mat[3]*m.mat[12];
        a[1] = mat[0]*m.mat[1] + mat[1]*m.mat[5] + mat[2]*m.mat[9] + mat[3]*m.mat[13];
        a[2] = mat[0]*m.mat[2] + mat[1]*m.mat[6] + mat[2]*m.mat[10] + mat[3]*m.mat[14];
        a[3] = mat[0]*m.mat[3] + mat[1]*m.mat[7] + mat[2]*m.mat[11] + mat[3]*m.mat[15];

        a[4] = mat[4]*m.mat[0] + mat[5]*m.mat[4] + mat[6]*m.mat[8] + mat[7]*m.mat[12];
        a[5] = mat[4]*m.mat[1] + mat[5]*m.mat[5] + mat[6]*m.mat[9] + mat[7]*m.mat[13];
        a[6] = mat[4]*m.mat[2] + mat[5]*m.mat[6] + mat[6]*m.mat[10] + mat[7]*m.mat[14];
        a[7] = mat[4]*m.mat[3] + mat[5]*m.mat[7] + mat[6]*m.mat[11] + mat[7]*m.mat[15];

        a[8] = mat[8]*m.mat[0] + mat[9]*m.mat[4] + mat[10]*m.mat[8] + mat[11]*m.mat[12];
        a[9] = mat[8]*m.mat[1] + mat[9]*m.mat[5] + mat[10]*m.mat[9] + mat[11]*m.mat[13];
        a[10] = mat[8]*m.mat[2] + mat[9]*m.mat[6] + mat[10]*m.mat[10] + mat[11]*m.mat[14];
        a[11] = mat[8]*m.mat[3] + mat[9]*m.mat[7] + mat[10]*m.mat[11] + mat[11]*m.mat[15];

        a[12] = mat[12]*m.mat[0] + mat[13]*m.mat[4] + mat[14]*m.mat[8] + mat[15]*m.mat[12]; 
        a[13] = mat[12]*m.mat[1] + mat[13]*m.mat[5] + mat[14]*m.mat[9] + mat[15]*m.mat[13]; 
        a[14] = mat[12]*m.mat[2] + mat[13]*m.mat[6] + mat[14]*m.mat[10] + mat[15]*m.mat[14];
        a[15] = mat[12]*m.mat[3] + mat[13]*m.mat[7] + mat[14]*m.mat[11] + mat[15]*m.mat[15] ;
        //mat16_t * p = new mat16_t( a );
        //return * p;
        mat16_t b(a);
        return b;
    }


    mat16_t& scale( float x, float y, float z ) 
    {
        mat[0] *= x; mat[1] *= y; mat[2] *= z;
        mat[4] *= x; mat[5] *= y; mat[6] *= z;
        mat[8] *= x; mat[9] *= y; mat[10] *= z;
        mat[12] *= x; mat[13] *= y; mat[14] *= z;
        return *this;
    }

    mat16_t& scale( float A ) 
    {
        mat[0] *= A; mat[1] *= A; mat[2] *= A;
        mat[4] *= A; mat[5] *= A; mat[6] *= A;
        mat[8] *= A; mat[9] *= A; mat[10] *= A;
        mat[12] *= A; mat[13] *= A; mat[14] *= A;
        return *this;
    }

    mat16_t& transpose( void )  // FIXME: could be better
    {
        mat16_t b( mat[0], mat[4], mat[8], mat[12],
                   mat[1], mat[5], mat[9], mat[13],
                   mat[2], mat[6], mat[10], mat[14],
                   mat[3], mat[7], mat[11], mat[15] );
        *this = b;
        return *this;
    }

/*  --TODO--
    operator=
    operator==
    operator!=
    operator* f
    operator/ f
    operator/ ( mat16_t& m )
    operator*=
    determinant
    normalize
*/

    char * tostring()
    {
        char * str = (char*) malloc( 5 * 16 + 10 );
        sprintf( str, "%1.2f %1.2f %1.2f %1.2f\n%1.2f %1.2f %1.2f %1.2f\n%1.2f %1.2f %1.2f %1.2f\n%1.2f %1.2f %1.2f %1.2f\n", mat[0], mat[1], mat[2], mat[3], mat[4], mat[5], mat[6], mat[7], mat[8], mat[9], mat[10], mat[11], mat[12], mat[13], mat[14], mat[15] );
        return str;
    }

}; // mat16_t


#endif /* !__GEOMETRY_H__ */

