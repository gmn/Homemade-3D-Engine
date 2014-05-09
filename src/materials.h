#ifndef __MATERIALS_H__
#define __MATERIALS_H__

#include "geometry.h" //vec4
#include "image.h"
#include "datastruct.h" //buffer_t

typedef enum {
    MTL_NONE,
    MTL_COLOR,               // gl colored polygon
    MTL_COLOR_MASKED,
    MTL_COLOR_BORDERED,
    MTL_TEXTURE_STATIC,      // single texture
    MTL_OSCILLATOR,          // (sine | square | triangle) wave color oscillator
    MTL_OSCILLATOR_MASKED,   // oscillator color through a mask
} materialType_t;

enum oscillator_t {
    OSC_NONE,
    OSC_SAW,
    OSC_SINE,
    OSC_SQUARE,
    OSC_RANDOM_TIME,            // 2 colors 
    OSC_RANDOM_COLOR,
};

enum 
{
    ANIM_LOOPING            = 1,
    ANIM_BACK_AND_FORTH     = 2,
    ANIM_ONCE               = 4,
    ANIM_DOWNFLAG           = 0x80
};

struct materialString_t {
    int           type;
    const char *  string;
};

extern materialString_t matTypeNames[];
extern const int TOTAL_MATERIAL_TYPES;



/*
====================
    surface_t
        interface
====================
*/
struct surface_t
{
    virtual struct material_t * getMat() 
    {
        return 0; // null surface for materials with no display properties
    }
};


/*
====================
    material_t
====================
*/
struct material_t : public surface_t
{
    static unsigned int mat_id;

    int                 id;
    char *              name;
    vec4_t              color; 
    vec4_t              color2; 

    materialType_t      type;       
    oscillator_t        osc_type;

    image_t *           img;    

    mat16_t             tex;

    material_t();
    material_t( const char *, image_t * =0 );
    material_t( const material_t& );
    virtual ~material_t();

    material_t * getMat() { return this; }

}; // material_t


/*
====================
    animation_t
        set of materials, with book-keeping data
==================== 
*/
struct animation_t  
{
    static unsigned int     anim_id;
    unsigned int            id;

    char *          name;

    buffer_t<material_t *> frames;     // list of animation frames

    int             index;      // current frame

    float           fps;        // fps, real
    int             mspf;       // (int)(1000.f/fps)

    int             direction;  // low-bits determine looping or up-n-down,
                                // high bit is set on way back down

    int             t_lastframe;// time of last frame change

    animation_t() : name(0), frames(8), index(0), t_lastframe(0) { 
        id = ++anim_id; fps = 10.f; mspf = 100; direction=ANIM_LOOPING;
        t_lastframe = 0;
    }
    animation_t( unsigned int, const char * =0, int =ANIM_LOOPING );
    animation_t( animation_t const & ) ;
    ~animation_t() ;

    material_t * advance( int =-1 );

    void setFPS( float _f ) {
        fps = _f;
        mspf = (int)(1000.0f / fps);
    }

    void setMSPF( int _i ) {
        mspf = _i;
        fps = 1000.0f / mspf;
    }

}; // animation_t


/* 
====================
    materialSet_t
        set of all materials. 1 of each unique kind. materialSet_t is
        considered the 'owner' of image data. images will be deleted 
        in ~materSet_t() destructor, (and nowhere else)
====================
*/
struct materialSet_t 
{
    buffer_t<material_t*>   mat;

    int                     current;

    materialSet_t() : mat(64), current(0) { }
    ~materialSet_t() ;
    
    unsigned int size( void ) { return mat.length(); }

    material_t * findByName( const char * );

    void add ( material_t *new_mat ) 
    {
        if ( !new_mat )
            return;
        mat.push( new_mat );
    }

    void loadTextureDirectory( void );

    material_t * prevMaterial( void ) ;
    material_t * nextMaterial( void ) ;
    //void reloadTextures( void );
};


/* 
====================
    colModel_t

        set of 0 or more AABB (axially-aligned bounding boxes). 
        Any colModel can be checked for collision against any other.
====================
*/
struct colModel_t
{
    vec4_t *        col;
    unsigned int    total;

    colModel_t() : col(0), total(0) { // base colModel is empty: no collision
    } 

    colModel_t( unsigned int n ) : total(n) {
        col = new vec4_t[n];
    }

    colModel_t( colModel_t const& C ) : total(C.total) {
        if ( !C.col || C.total == 0 ) {
            col = 0; 
        } else {
            col = new vec4_t[ C.total ];
            for ( unsigned int i = 0; i < total; i++ ) {
                col[i].set( C.col[i] );
            }
        }
    }

    ~colModel_t() {
        if ( col ) 
            delete[] col;
    }

    int check( const vec4_t& v ) ;
};



#if 0
/*
==================== 
    animSet_t
        group of animations. owned
==================== 
*/
struct animSet_t : public surface_t, public buffer_t<animation_t>
{
    int lastAccess; // index of last animation accessed by getMat()

    animSet_t() : lastAccess(0) {}
    animSet_t( const animSet_t & );

    void add( animation_t *anim ) 
    {
        animation_t * cpy = new animation_t( *anim );
        buffer_t<animation_t>::add( *cpy );
    }

    // pre-allocate animations, return pointer to first
    animation_t * startAnim( unsigned int howmany =1 ) 
    {
        animation_t dummy;
        uint ofst = free_p - data;
        while ( howmany-- ) {
            buffer_t<animation_t>::add( dummy );
        //    data[ free_p - data - 1 ];
        }
        return &data[ ofst ];
    }

    ~animSet_t() 
    {
        const unsigned int len = length();
        for ( unsigned int i = 0; i < len; i++ ) 
        {
            delete data[ i ];
        }
        destroy();
    }
    
    // override operator to provide reference
    animation_t & operator[]( unsigned int index ) {
        return data[ index ];
    }

    material_t *getMat( void );

    // fetch animation by name. if no frame# provided, animation chooses frame
    material_t *getNameMat( const char *, int =-1 );

    // returns frame number the lastAccessed animation is currently on
    int frameNo() { if ( lastAccess >= 0 && lastAccess < length() ) return data[ lastAccess ].index; return 0; }

    // get handle to the animation
    animation_t *getAnim( const char * );

}; // animSet_t


// 2d visual description that each maptile or entity possess
struct tile_t 
{

    surface_t       surf;               // an animSet or material.  
    colModel_t      col;

    vec4_t          v;

    int             layer;              // layer tile is on

    tile_t() : surf(0), layer(0) {}
    tile_t( const animSet_t& s );
    tile_t( const material_t& s );
    tile_t( tile_t const& );            // actual copy-con
    virtual ~tile_t();

}; // tile_t




struct entTile_t : public tile_t 
{
};


struct mapTile_t : public tile_t 
{
    static unsigned int     tile_id;
    unsigned int            id; 
    int                     background;

    mapTile_t() : id(0), background(0) {}
    mapTile_t( mapTile_t const & );
    ~mapTile_t() {}

    // generate a copy of this
    static mapTile_t * NewMaterialTile( void );

}; // mapTile_t


// 3d object, possesses geometrical space only
struct brush_t
{
};
#endif 

#endif // __MATERIALS_H__

