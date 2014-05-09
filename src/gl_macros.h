#ifndef __GL_MACROS__
#define __GL_MACROS__

// simplistic square macro
#ifndef GL_SQUARE
#define GL_SQUARE(x,y,z,s) {\
        glBegin(GL_QUADS); \
        glVertex3f((GLfloat)(x),(GLfloat)(y),(GLfloat)(z)); \
        glVertex3f((GLfloat)(x)+(s),(GLfloat)(y),(GLfloat)(z)); \
        glVertex3f((GLfloat)(x)+(s),(GLfloat)(y)+(s),(GLfloat)(z)); \
        glVertex3f((GLfloat)(x),(GLfloat)(y)+(s),(GLfloat)(z)); \
        glEnd(); }
#endif


// draw a white outline around a poly in z=0 plane
#define POLY_OUTLINE(x,y,w,h) { \
    glPushAttrib( GL_CURRENT_BIT ); \
    glColor3f( 1.0f, 1.0f, 1.0f ); \
    glBegin( GL_LINE_STRIP ); \
    glVertex3i( x, y, 0 ); \
    glVertex3i( x, y+h, 0 ); \
    glVertex3i( x+w, y+h, 0 ); \
    glVertex3i( x+w, y, 0 ); \
    glVertex3i( x, y, 0 ); \
    glEnd(); \
    glPopAttrib(); }


#ifndef GL_TEX_SQUARE
#define GL_TEX_SQUARE(x,y,z,d) {\
        glBegin(GL_QUADS); \
        GLfloat s=0.f,t=0.f,r=1.f;\
        glTexCoord2f((GLfloat)(s),(GLfloat)(t)); \
        glVertex3f((GLfloat)(x),(GLfloat)(y),(GLfloat)(z)); \
        glTexCoord2f((GLfloat)(s)+(r),(GLfloat)(t)); \
        glVertex3f((GLfloat)(x)+(d),(GLfloat)(y),(GLfloat)(z)); \
        glTexCoord2f((GLfloat)(s)+(r),(GLfloat)(t)+(r)); \
        glVertex3f((GLfloat)(x)+(d),(GLfloat)(y)+(d),(GLfloat)(z)); \
        glTexCoord2f((GLfloat)(s),(GLfloat)(t)+(r)); \
        glVertex3f((GLfloat)(x),(GLfloat)(y)+(d),(GLfloat)(z)); \
        glEnd(); }
#endif


// GL_CUBE
// GL_TRIANGLE_4
// GL_TRIANGLE_5


#define GL_print_errors() { \
    int __err = glGetError(); \
    if ( 0 != __err ) \
        core.warn( "GL error (line %d): %s\n", __LINE__, GL_err_string(__err) ); \
}


#endif // __GL_MACROS__
