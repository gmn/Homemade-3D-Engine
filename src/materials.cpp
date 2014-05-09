// materials.cpp

#include "materials.h"
#include "hgr.h"
#include "core.h"



materialString_t matTypeNames[] = {
	{ MTL_NONE, "MTL_NONE" },
	{ MTL_COLOR, "color" },
	{ MTL_COLOR_MASKED, "maskedColor" },
	{ MTL_COLOR_BORDERED, "borderedColor" },
	{ MTL_TEXTURE_STATIC, "textureStatic" },
	{ MTL_OSCILLATOR, "oscillator" },
	{ MTL_OSCILLATOR_MASKED, "oscillatorMasked" },
};

const int TOTAL_MATERIAL_TYPES = sizeof(matTypeNames) / sizeof(matTypeNames[0]);



/*
================================================================================
    material_t
    
        @deletes: name, image_t
================================================================================
*/
unsigned int material_t::mat_id = 0;

material_t::material_t() : name(0),color(1.f,1.f,1.f,1.f),img(0)
{   
    id = ++mat_id;
    type = MTL_TEXTURE_STATIC;
    osc_type = OSC_NONE;
}

material_t::material_t( const char *_name, image_t * _img ) 
{
    // id
    id = ++mat_id;

    // name
    const char * basename = strip_path_and_extension( _name );
    name = copy_string( basename );
    str_tolower( name );

    // color
    color.zero();
    color2.zero();

    // img
	img = _img;

    // types
	type = MTL_TEXTURE_STATIC;
    osc_type = OSC_NONE;

    // tex coords
	tex.setRow( 0, vec4_t(0.f, 1.f, 1.f, 0.f) );
	tex.setRow( 1, vec4_t(0.f, 0.f, 1.f, 1.f) );
}

material_t::material_t ( const material_t& M ) 
{
    Assert(0 && "cant use copy-constructor. would duplicate image_t* leading to double-free" );
   

    // id
    id = ++mat_id;

    // name
    name = (char *) malloc( strlen(M.name) + 1 );
    memset( name, 0, strlen(M.name) + 1 );
	strcpy ( name, M.name );

    // color
	color = M.color;
	color2 = M.color2;

    // img
	img = M.img;

    // types
    type = M.type;
    osc_type = M.osc_type;

    // tex coords
	tex.setRow( 0, M.tex );
	tex.setRow( 1, M.tex );
}

material_t::~material_t()
{
    if ( name )
        free( name );
    if ( img )
        delete( img );
}


/*
================================================================================
    animation_t

        @deletes: name
================================================================================
*/
unsigned int animation_t::anim_id = 0;

animation_t::animation_t( unsigned int _frames, const char * _name, int _dir ) : frames(_frames) 
{
    id = ++anim_id;

    if ( strstr( _name, "animation_" ) == _name ) {
        name = (char *) malloc( 18 );
        memset( name, 0, 18 );
		sprintf( name, "animation_%04u", id );
	} else if ( _name ) {
        name = copy_string( _name );
	} else {
        name = (char *) malloc( 18 );
        memset( name, 0, 18 );
		sprintf( name, "animation_%04u", id );
	}

	fps             = 10.0f;
	mspf            = 100.f; 
	index           = 0;
	direction       = _dir;
	t_lastframe     = 0;
}

animation_t::animation_t( const animation_t &A ) : frames(A.frames) 
{
    id          = ++anim_id;
    name        = copy_string( A.name );
	fps         = A.fps;
	mspf        = A.mspf;
	direction   = A.direction;
	index       = 0;
    t_lastframe = 0;
}

animation_t::~animation_t()
{
    if (name)
        free( name );
}

material_t * animation_t::advance( int frame ) 
{
	// request to start at a specific number
	if ( -1 != frame ) {
		index = frame;
		t_lastframe = get_milliseconds();
	}

	int time = get_milliseconds();
    int total = frames.length();

	if ( time - t_lastframe > mspf ) 
    {
		if ( ANIM_BACK_AND_FORTH & direction ) {
			// down
			if ( direction & 0x80 ) {
				if ( --index <= 0 ) {
					direction ^= 0x80;
					index = 0;
				}
			} else {
				if ( ++index > total - 2 ) {
					index = total - 1;
					direction ^= 0x80;
				}
			}
		} else if ( ANIM_LOOPING & direction ) {
			index = (index+1) % total;
		}
		t_lastframe = time;
	}

	return frames[ index ];
}



/*
================================================================================
	materialSet_t

        Singleton. Stores all materials in entire application.
        @deletes: material_t[]
================================================================================
*/
materialSet_t::~materialSet_t() 
{
    for ( unsigned int i = 0; i < mat.length(); i++ )
    { 
        delete mat.data[i];
        mat.data[i] = (material_t*)0;
    } 
}

material_t * materialSet_t::findByName( const char * name ) 
{
	for ( unsigned int i = 0; i < mat.length(); i++ ) {
        // get the first material name
        if ( !strcmp( name, mat.data[i]->name ) )
            return mat.data[i];

/*        // if there's not an image try the image syspath
        if ( !mat.data[i]->img ) 
			continue;

		if ( !strcmp( name, mat.data[i]->img->fullpath ) ) 
			return mat.data[i];
*/
	}
	return 0;
}

material_t * materialSet_t::prevMaterial( void ) {
    if ( 0 == mat.length() )
        return 0; // no materials
    current = ( current == 0 ) ? mat.length() - 1 : current - 1;
	return mat.data[ current ];
}

material_t * materialSet_t::nextMaterial( void ) {
    if ( 0 == mat.length() )
        return 0; // can not modulus by 0
	current = ( current + 1 ) % mat.length();
	return mat.data[ current ];
}

void materialSet_t::loadTextureDirectory( void ) 
{
    gvar_t * imgpath = core.gvar( "imgpath" );
    char ** dirlist = list_directory( imgpath->string );
    char ** pp = dirlist;
    core.printf( "Created Materials:\n" );

    while ( *pp ) 
    {
        if ( strcasestr( *pp, ".tga" ) || strcasestr( *pp, ".bmp" ) ) 
        {
            image_t *img_p;
            IMG_compileImageFile( *pp, &img_p );

            // build a texture material
			material_t * mat_p = new material_t( *pp, img_p );
			mat.add( mat_p );
            core.printf( " \"%s\"\n", mat_p->name );
        }
        ++pp;
    }

    free_listdir( dirlist );
}

/*
================================================================================
    colModel_t
================================================================================
*/
int colModel_t::check( const vec4_t& v ) {
    if ( !col || 0 == total ) 
        return 0;
    return 1;
}

#if 0
void materialSet_t::ReLoadTextures( void ) {
	unsigned int total_tex = 0;
    unsigned int asset_len = 0;

    char ** asset_names = GetDirectoryList( "zpak/gfx", &asset_len );

	if ( !asset_len || !asset_names ) { 
		Err_Fatal( "couldn't find data directory! Cannot run. Exiting.\n" );
        return;
	}

    // count how many files listed are actually loadable textures
    for ( unsigned int i = 0; i < asset_len; i++ ) {
        if ( O_strcasestr( asset_names[i], ".tga" ) || O_strcasestr( asset_names[i], ".bmp" ) )
            ++total_tex;
    }

	image_t *image = NULL;
	material_t *existing = NULL;
	
	// foreach asset name
    for ( unsigned int i = 0; i < asset_len; i++ ) {

		// if it appears to be an image file
        if ( O_strcasestr( asset_names[i], ".tga" ) || O_strcasestr( asset_names[i], ".bmp" ) ) {

			// compile a new image from the asset name
            IMG_compileImageFile( asset_names[i], &image );

			// see if we already have it
			if ( (existing=FindByName( strip_gamepath( asset_names[i] ) ) ) ) {
				// in that case replace the data in the image_t pointer in 
				//  the existing material with the new data, and free the 
				//  newly created, redundant image_t
				if ( !existing->img ) {
					existing->img = image;
				} else {
					memcpy( existing->img, image, sizeof(image_t) );
					V_Free( image );
				}
			} else {

				// otherwise: build a new texture material
				material_t *mat_p = material_t::NewTextureMaterial( asset_names[i], image );
				// and add it to the set
				this->add( mat_p );

			}
        }
    }

    // free array of string pointers
    for ( unsigned int i = 0; i < asset_len; i++ ) {
        V_Free( asset_names[i] );
    }
    V_Free( asset_names );
}
#endif


#if 0
/*
====================
    animSet_t
====================
*/
material_t * animSet_t::getMat( const char *setname, int frame ) {
	lastAccess = -1;
	if ( !setname || !setname[0] )
		return NULL;
	int i;
	for ( i = 0; i < length(); i++ ) {
		if ( !strcmp( setname, data[ i ].name ) ) {
			break;
		}
	}
	if ( length() == i )
		return NULL;
	
	lastAccess = i;
	return data[i].advance( frame );
}

// does not advance, just gets it
animation_t * animSet_t::getAnim( const char * aname ) {
	lastAccess = -1;
	if ( !aname || !aname[0] )
		return NULL;
	int i;
	for ( i = 0; i < length(); i++ ) {
		if ( !strcmp( aname, data[ i ].name ) ) {
			break;
		}
	}
	if ( length() == i )
		return NULL;
	
	lastAccess = i;
	return &data[i];
}

animSet_t::animSet_t( animSet_t const & A ) : started(0) {
	const unsigned int len = A.length();
	if ( len > 0 ) {

		// only allocing for the used slots
		init( len ); 
		
		for ( int i = 0; i < len; i++ ) 
		{
			// copy all local data members
			buffer_c<animation_t>::add( A.data[ i ] ); 

			/* this one is a little confusing. buffer_c<animation_t> stores animations
				by value, not pointer, so when we call add()^, some mystery mechanism that I
				have read about, but never seen before, calls the ctor() and makes a temporary
				copy of A.data[i], including the malloc and copying of its frames array.
				(it would be nice if didn't do this. do we have any control?)
				then after it has memory copied all of those elements over to the buffer<> array
				inside the add() func
				it calls the dtor which frees frames. so what you see below is data[i].frames
				with an invalid address that just got freed, but in a copy, so it hasn't been
				set to NULL.  THEN, we get another copy of frames via a roll-your-own function
				because C++ is a stupid mess */

			// but we must get our own copy of the frame pointers array
			data[i].frames = A.data[i].copy_frames();
			data[i].last = 0; // reset the last frame drawn , after copy
		}
		
		started = true;
	}
}

/*
====================
	mapTile_t
====================
*/
int mapTile_t::tile_id = 0;

mapTile_t * mapTile_t::New( colModel_t *cpycol ) {
	mapTile_t *n = new mapTile_t();

	if ( cpycol ) {
		n->col = cpycol->Copy();
	} else {
		n->col = NULL;
	}

	n->uid = ++nextMapTileUID;
	return n;
}
mapTile_t * mapTile_t::NewMaterialTile( void ) {
	mapTile_t *n = mapTile_t::New();
	n->mat = material_t::New();
	return n;
}

mapTile_t * mapTile_t::Copy( void ) {

	mapTile_t *n = mapTile_t::New( this->col );

	if ( anim ) {
		n->anim = new animSet_t( *this->anim );
	}
	if ( mat ) {
		n->mat = new material_t( *this->mat );
		//n->mat = mat->Copy();
	}

	n->uid = this->uid;
	n->snap = this->snap;
	n->lock = false; // copied tiles do not inherit lock
	n->do_sync = this->do_sync;
	n->poly = this->poly;
	n->layer = this->layer;
	return n;
}

void mapTile_t::Destroy( void ) {
	if ( anim ) {
		/*
		anim->destroy();
		V_Free( anim );
		*/
		delete anim;
		anim = NULL;
	}
	if ( mat ) {
		mat->Destroy();
		V_Free( mat );
		mat = NULL;
	}
	if ( col ) {
		col->Destroy();
		V_Free( col );
		col = NULL;
	}
}

mapTile_t::mapTile_t( mapTile_t const& M ) 
	: uid(M.uid), snap(M.snap), lock(M.lock), background(M.background), tile_t(M) {
}

#endif
