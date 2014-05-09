// primitives - geometry primitives vertices sets

#include "primitives.h"
#include <stdlib.h>


static float colors_t[ ] = 
{
1.f, 0.f, 0.f, 
0.f, 1.f, 0.f, 
0.f, 0.f, 1.f,
1.f, 1.f, 0.f,
1.f, 0.5f, 0.f, 
0.5f, 0.f, 0.f, 
0.f, 0.5f, 0.f,
0.f, 0.f, 0.5f,
1.f, 0.f, 1.f, 
1.f, 1.f, 1.f, 
0.5f, 0.5f, 0.5f, 
0.7f, 0.f, 0.7f,
0.7f, 0.3f, 0.7f, 
0.3f, 0.5f, 1.f,
0.5f, 0.f, 0.5f
};

static const int tot_colors = sizeof(colors_t)/(sizeof(colors_t[0])*3); 



heightMap_t::heightMap_t( int a, int b, float c ) : w(a), h(b), height(c)
{
    // width promoted to next odd number above
    if ( ! (w & 0x1) ) { w++; }

    map = new float[ w * h ];

    int iheight = (int)ceilf(height);

    for ( int j = 0; j < h; j++ ) 
    {
        for ( int i = 0; i < w; i++ ) 
        {
            map[ j * w + i ] = (random() % (1000*iheight)) / 1000.f;
        }
    }
}


    

tessellatedPlane_t::tessellatedPlane_t( heightMap_t& m, float iw, float ih ) 
            : w(iw), h(ih) 
{
    num_verts = m.h * 2 * (m.w-1);

    verts = new vec3_t[ num_verts ];

    /* prototype tess field is on y=0 plane: 
            X: 0 <= x < w, 
            Y: given by height map
            Z: 0 >= z > -h, */

    float col_width = w / ((float)(m.w-1)) ;
    float row_height = -h / ((float)(m.h));

    int v_num = 0;

    // 0 < col
    for ( int col = 0; col < m.w-1; col++ ) 
    {
        for ( int j = 0; j < m.h; j++ )
        {
            verts[ v_num ][ 0 ] = col_width * col;
            verts[ v_num ][ 1 ] = m[ ((int)(j * m.w + col)) ];
            verts[ v_num ][ 2 ] = row_height * j;
            ++v_num;

            verts[ v_num ][ 0 ] = col_width * ( col + 1 ) ;
            verts[ v_num ][ 1 ] = m[ ((int)(j * m.w + col + 1)) ];
            verts[ v_num ][ 2 ] = row_height * j;
            ++v_num;
        }

        // next column
        ++col;

        for ( int j = m.h-1; j >= 0; j-- )
        {
            verts[ v_num ][ 0 ] = col_width * ( col + 1 ) ;
            verts[ v_num ][ 1 ] = m[ ((int)(j * m.w + col + 1)) ];
            verts[ v_num ][ 2 ] = row_height * j;
            ++v_num;

            verts[ v_num ][ 0 ] = col_width * col;
            verts[ v_num ][ 1 ] = m[ ((int)(j * m.w + col)) ];
            verts[ v_num ][ 2 ] = row_height * j;
            ++v_num;
        }
    }

    row = m.h;
    column = m.w;

    // generate the interleaved array

    // 
    int array_sz = num_verts * 6; // 3 vert, 3 color

    array = new float[ array_sz ];

    // format: glInterleavedArrays GL_C3F_V3F
    for ( int i = 0, j = 0; j < num_verts; i+=6, j++ ) {
        int c = random() % tot_colors; // defaults to random rgb colors
        float * p = &colors_t[ c * 3 ];
        array[i+0] = p[0];
        array[i+1] = p[1];
        array[i+2] = p[2];

        array[i+3] = verts[j][0];
        array[i+4] = verts[j][1];
        array[i+5] = verts[j][2];
    }
}

void tessellatedPlane_t::setColor( float r, float g, float b )
{   
    for ( int i = 0; i < num_verts * 6; i+=6 )
    {  
        array[i+0] = r;
        array[i+1] = g;
        array[i+2] = b;
    }
}

void tessellatedPlane_t::randomColors( void )
{  
    for ( int i = 0; i < num_verts * 6; i+=6 )
    {  
        int c = random() % tot_colors;
        float * p = &colors_t[ c * 3 ];
        array[i+0] = p[0];
        array[i+1] = p[1];
        array[i+2] = p[2];
    }
}

void tessellatedPlane_t::gradedTones( float r, float g, float b )
{  
    for ( int i = 0; i < num_verts * 6; i+=6 )
    {  
        float grade = ( random() % 1000 ) / 1000.f;
        array[i+0] = r * grade;
        array[i+1] = g * grade;
        array[i+2] = b * grade;
    }
}



