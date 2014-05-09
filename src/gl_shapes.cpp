// gl_drawing.cpp - shapes
#ifdef __MAC__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif
#include <math.h>


void draw_spiral( float degrees, float radius, float climb ) 
{
    glBegin(GL_LINE_STRIP);
    float y = 0.0f; 
    for ( float angle = 0.0f; angle <= (2.0f*M_PI)*degrees; angle += 0.1f )
    { 
        float x = radius * sin(angle); 
        float z = radius * cos(angle);

        // Specify the point and move the z value up a little 
        glVertex3f(x, y, z); 
        y += climb;
    }
    // Done drawing points
    glEnd();
}

// circle in z=0 plane
void draw_circle( float x, float y, float radius )
{
    float pi = M_PI;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x,y);
    for (float ang = (2*pi)+(pi/24);ang>=0;ang-=pi/24)
    {
        float nx = (radius*sin(ang))+x;
        float ny = (radius*cos(ang))+y;
        glVertex2f(nx,ny);
    }
    glEnd();
}

// wire circle in z=0 plane
void draw_wire_circle( float x, float y, float radius )
{
    float pi = M_PI;
    glBegin(GL_LINE_STRIP);
    for (float ang = (2*pi)+(pi/24);ang>=0;ang-=pi/24)
    {
        float nx = (radius*sin(ang))+x;
        float ny = (radius*cos(ang))+y;
        glVertex2f( nx, ny );
    }
    glEnd();
}

void draw_tex_circle( float x, float y, float radius )
{
    float pi = M_PI;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x,y);
    for (float ang = (2*pi)+(pi/24);ang>=0;ang-=pi/24)
    {
        float nx = (radius*sin(ang))+x;
        float ny = (radius*cos(ang))+y;
        glTexCoord2f( sin(ang)*.5+.5, cos(ang)*.5+.5 );
        glVertex2f( nx, ny );
    }
    glEnd();
}


/*
The following Cartesian coordinates define the vertices of an icosahedron with edge-length 2, centered at the origin:
    (0, ±1, ±φ)
    (±1, ±φ, 0)
    (±φ, 0, ±1)
where φ = (1+√5)/2
*/


#define X .525731112119133606f
#define Z .850650808352039932f
static GLfloat vdata[12][3] = {    
    {-X, 0.0f, Z},  // 0
    {X, 0.0f, Z},   // 1
    {-X, 0.0f, -Z}, // 2
    {X, 0.0f, -Z},  // 3
    {0.0f, Z, X},   // 4
    {0.0f, Z, -X},  // 5
    {0.0f, -Z, X},  // 6
    {0.0f, -Z, -X}, // 7
    {Z, X, 0.0f},   // 8
    {-Z, X, 0.0f},  // 9
    {Z, -X, 0.0f},  // 10
    {-Z, -X, 0.0f}  // 11
};
#undef X
#undef Z

static GLuint tindices[20][3] = { 
    {1,4,0}, {4,9,0}, {4,5,9}, {8,5,4}, {1,8,4},    
    {1,10,8}, {10,3,8}, {8,3,5}, {3,2,5}, {3,7,2},    
    {3,10,7}, {10,6,7}, {6,11,7}, {6,0,11}, {6,1,0}, 
    {10,1,6}, {11,0,9}, {2,11,9}, {5,2,9}, {11,2,7} };

static void normalize(GLfloat *a) 
{
    GLfloat d=1.f/sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
    a[0]*=d; a[1]*=d; a[2]*=d;
}

static void drawtri( GLfloat *a, GLfloat *b, GLfloat *c, int div, float r, int serp =0 ) 
{
    if (div<=0) {
        glNormal3fv(a); glVertex3f(a[0]*r, a[1]*r, a[2]*r);
        glNormal3fv(b); glVertex3f(b[0]*r, b[1]*r, b[2]*r);
        glNormal3fv(c); glVertex3f(c[0]*r, c[1]*r, c[2]*r);
    } else {
        GLfloat ab[3], ac[3], bc[3];
        for (int i=0;i<3;i++) {
            ab[i]=(a[i]+b[i])/2;
            ac[i]=(a[i]+c[i])/2;
            bc[i]=(b[i]+c[i])/2;
        }
        normalize(ab); normalize(ac); normalize(bc);
        drawtri(a, ab, ac, div-1, r, serp);
        drawtri(b, bc, ab, div-1, r, serp);
        drawtri(c, ac, bc, div-1, r, serp);
        if ( !serp )
            drawtri(ab, bc, ac, div-1, r, serp);  
    }  
}

// windings or normals are incorrect. 
void draw_serpinski( int ndiv, float radius=1.0f ) 
{
    glBegin( GL_TRIANGLES );
    for ( int i = 0; i < 20; i++ )
    {
        drawtri (   vdata[tindices[i][0]], 
                    vdata[tindices[i][1]], 
                    vdata[tindices[i][2]], ndiv, radius, 1 );
    }
    glEnd();
}

void draw_sphere( int ndiv, float radius=1.0f ) 
{
    glBegin( GL_TRIANGLES );
    for ( int i = 0; i < 20; i++ )
    {
        drawtri (   vdata[tindices[i][0]], 
                    vdata[tindices[i][1]], 
                    vdata[tindices[i][2]], ndiv, radius, 0 );
    }
    glEnd();
}

void draw_icosahedron_wire( void )
{
    #define p 1.618033989f
    float top[] = {     0, p, 1, 
                        -p, 1, 0, 
                        -p, 1, 0, 
                        -1, 0, p,
                        0, p, 1,
                        -1, 0, p,
                        -1, 0, p,
                        1, 0, p,
                        0, p, 1,
                        1, 0, p,
                        1, 0, p,
                        p, 1, 0,
                        0, p, 1,
                        p, 1, 0,
                        p, 1, 0,
                        0, p, -1,
                        0, p, 1,
                        0, p, -1,
                        0, p, -1,
                        -p, 1, 0 };

    float middle[] = {  -p, 1, 0,
                        -p, -1, 0,
                        -1, 0, p,
                        0, -p, 1,
                        1, 0, p,
                        p, -1, 0,
                        p, 1, 0,
                        1, 0, -p,
                        0, p, -1,
                        -1, 0, -p,
                        -p, 1, 0 };
                        
    float bottom[] = {  0, -p, -1,
                        0, -p, 1, 
                        0, -p, 1, 
                        -p, -1, 0,
                        0, -p, -1,
                        -p, -1, 0,
                        -p, -1, 0,
                        -1, 0, -p, 
                        0, -p, -1,
                        -1, 0, -p, 
                        -1, 0, -p, 
                        1, 0, -p, 
                        0, -p, -1,
                        1, 0, -p, 
                        1, 0, -p, 
                        p, -1, 0,
                        0, -p, -1,
                        p, -1, 0,
                        p, -1, 0,
                        0, -p, 1 };
    #undef p

    unsigned int t_sz = 5 * 4 * 3;
    unsigned int m_sz = 11 * 3;

    glBegin( GL_LINES );
    for ( unsigned int i = 0; i < t_sz; i+=6 )
    {
        glVertex3f( top[i], top[i+1], top[i+2] );
        glVertex3f( top[i+3], top[i+4], top[i+5] );
    }
    for ( unsigned int i = 0; i < t_sz; i+=6 )
    {
        glVertex3f( bottom[i], bottom[i+1], bottom[i+2] );
        glVertex3f( bottom[i+3], bottom[i+4], bottom[i+5] );
    }
    glEnd();

    glBegin( GL_LINE_STRIP );
    for ( unsigned int i = 0; i < m_sz; i+=3 )
    {
        glVertex3f( middle[i], middle[i+1], middle[i+2] );
    }
    glEnd();
}

void draw_icosahedron( void ) // solid
{
    #define p 1.618033989f
    float top[] = {     0, p, 1, 
                        -p, 1, 0, 
                        -1, 0, p,
                        0, p, 1,
                        -1, 0, p,
                        1, 0, p,
                        0, p, 1,
                        1, 0, p,
                        p, 1, 0,
                        0, p, 1,
                        p, 1, 0,
                        0, p, -1,
                        0, p, 1,
                        0, p, -1,
                        -p, 1, 0 };

    float middle[] = {  -p, 1, 0,
                        -p, -1, 0,
                        -1, 0, p,
                        0, -p, 1,
                        1, 0, p,
                        p, -1, 0,
                        p, 1, 0,
                        1, 0, -p,
                        0, p, -1,
                        -1, 0, -p,
                        -p, 1, 0,
                        -p, -1, 0 };
                        
    float bottom[] = {  0, -p, -1,
                        0, -p, 1, 
                        -p, -1, 0,
                        0, -p, -1,
                        -p, -1, 0,
                        -1, 0, -p, 
                        0, -p, -1,
                        -1, 0, -p, 
                        1, 0, -p, 
                        0, -p, -1,
                        1, 0, -p, 
                        p, -1, 0,
                        0, -p, -1,
                        p, -1, 0,
                        0, -p, 1 };
    #undef p

    unsigned int t_sz = 5 * 3 * 3;
    unsigned int m_sz = 12 * 3;

    glBegin( GL_TRIANGLES );
    for ( unsigned int i = 0; i < t_sz; i+=3 )
    {
        glVertex3f( top[i], top[i+1], top[i+2] );
    }
    for ( unsigned int i = 0; i < t_sz; i+=3 )
    {
        glVertex3f( bottom[i], bottom[i+1], bottom[i+2] );
    }
    glEnd();

    glBegin( GL_TRIANGLE_STRIP );
    for ( unsigned int i = 0; i < m_sz; i+=3 )
    {
        glVertex3f( middle[i], middle[i+1], middle[i+2] );
    }
    glEnd();
}

void draw_triangle4( int mesh =1 )
{
    float p[] = {   0.f, 1.f, 0.f,
                    -0.75f, -0.5f, -0.433012702f,
                    0.75f, -0.5f, -0.433012702f,
                    0.f, -0.5f, 0.866025404f }; 
    unsigned int L[] = { 0, 1, 1, 3, 0, 3, 3, 2, 0, 2, 2, 1 };
    unsigned int T[] = { 1, 0, 3, 2, 1, 0 }; 
    if ( mesh )
    {
        glBegin( GL_LINES );
        for ( int i = 0; i < 12; i++ )
            glVertex3f( p[L[i]*3], p[L[i]*3+1], p[L[i]*3+2] );
    }
    else
    {
        glBegin( GL_TRIANGLE_STRIP );
        for ( int i = 0; i < 6; i++ )
            glVertex3f( p[T[i]*3], p[T[i]*3+1], p[T[i]*3+2] );
    }
    glEnd();
}

void draw_triangle4_2( int mesh =1 )
{
    float p[] = {   0.f, 0.f, 0.f,
                    0.f, 1.f, 0.f,
                    -0.75f, -0.5f, -0.433012702f,
                    0.75f, -0.5f, -0.433012702f,
                    0.f, -0.5f, 0.866025404f }; 
    unsigned int L[] = { 0, 1, 0, 2, 0, 3, 0, 4, 1, 2, 1, 3, 1, 4, 4, 2, 2, 3, 3, 4 }; 
    unsigned int T[] = { 0, 1, 2, 0, 1, 3, 0, 1, 4, 0, 2, 3, 0, 3, 4, 0, 4, 2 };
    
    if ( mesh )
    {
        glBegin( GL_LINES );
        for ( int i = 0; i < 20; i++ )
            glVertex3f( p[L[i]*3], p[L[i]*3+1], p[L[i]*3+2] );
        glEnd();
    }
    else
    {
        GLboolean cf; glGetBooleanv( GL_CULL_FACE, &cf );
        if ( cf )
            glDisable( GL_CULL_FACE );

        GLfloat c[4]; glGetFloatv( GL_CURRENT_COLOR, c );

        glBegin( GL_TRIANGLES );
        for ( int i = 0; i < 18; i++ ) {
            if ( i % 3 == 0 ) {
                float m = i / 18.0f * 0.2f - 0.1f;
                glColor4f( c[0] + m, c[1] + m, c[2] + m, c[3] );
            }
            glVertex3f( p[T[i]*3], p[T[i]*3+1], p[T[i]*3+2] );
        }
        glEnd();

        if ( cf )
            glEnable( GL_CULL_FACE );
        glColor4f( c[0], c[1], c[2], c[3] );
    }
}


void draw_triangle5( int mesh =1 )
{
    float p[] = {0.f, 2.f, 0.f,
                -1.f, 0.f, -1.f,
                -1.f, 0.f, 1.f,
                1.f, 0.f, 1.f, 
                1.f, 0.f, -1.f }; 

    unsigned int T[] = { 0, 1, 2, 3, 0, 4, 1, 3 };
    unsigned int L[] = { 0, 1, 2, 0, 3, 4, 0, 2, 3, 4, 1 };

    if ( mesh )
    {
        glBegin( GL_LINE_STRIP );
        for ( int i = 0; i < 7; i++ ) 
            glVertex3f( p[L[i]*3 ], p[L[i]*3 + 1], p[L[i]*3 + 2] );
        glEnd();
        glBegin( GL_LINES );
        for ( int i = 7; i < 11; i++ ) 
            glVertex3f( p[L[i]*3 ], p[L[i]*3 + 1], p[L[i]*3 + 2] );
        glEnd();
    }
    else
    {
        glBegin( GL_TRIANGLE_STRIP );
        for ( int i = 0; i < 8; i++ ) 
            glVertex3f( p[T[i]*3 ], p[T[i]*3 + 1], p[T[i]*3 + 2] );
        glEnd();                
    }
}

void draw_unitcube( int mesh =1 )
{
    float p[] = {   -1, -1, -1, 
                    -1, -1, 1, 
                    1, -1, 1, 
                    1, -1, -1,
                    1, 1, -1,   // 4
                    -1, 1, -1, 
                    -1, 1, 1,
                    1, 1, 1 };
    unsigned int L[] = { 3, 0, 1, 2, 3, 4, 5, 6, 7, 4, /**/ 0, 5, 1, 6, 2, 7 }; 
    unsigned int T[] = { 1, 2, 6, 7, 5, 4,  
                         7, 2, 4, 3, 5, 0,
                         6, 1, 0, 3, 1, 2 };

    if ( mesh )
    {
        glBegin( GL_LINE_STRIP );
        for ( unsigned int i = 0; i < 10; i++ )
            glVertex3f( p[L[i]*3], p[L[i]*3+1], p[L[i]*3+2] );
        glEnd();
        glBegin( GL_LINES );
        for ( unsigned int i = 10; i < 16; i++ )
            glVertex3f( p[L[i]*3], p[L[i]*3+1], p[L[i]*3+2] );
        glEnd();
    }
    else
    {
        glBegin( GL_TRIANGLE_STRIP );
        for ( unsigned int i = 0; i < 18; i++ )
            glVertex3f( p[T[i]*3], p[T[i]*3+1], p[T[i]*3+2] );
        glEnd();
    }
}

// takes angle 0-360, and translates frame both AROUND and UP-AND-DOWN
void trans_spiral_f( float angle, float radius, float rot_rate, float climb_rate, float ceiling ) 
{
    float x = radius * sinf( angle * rot_rate );
    float z = radius * cosf( angle * rot_rate );
    float y = ceiling + sinf( angle * climb_rate ) * ceiling * 2.f;
    glTranslatef( x, y, z );
}

