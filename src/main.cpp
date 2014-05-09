#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include <SDL/SDL.h>

#include "core.h"
#include "datastruct.h"
#include "gvar.h"
#include "primitives.h"
#include "game_module.h"
#include "image.h"


#include "model_obj.h"
extern class ModelOBJ model;


const int ms_per_logic_frame = 80;
const int ms_per_render_frame = 80;
static int quit_time = 0;

SDL_Surface * surface = 0;

/////////////////////////////////////////////
//
// get_tick -- SDL dependant calls 
//
int get_ticks (void)
{
    return (SDL_GetTicks() / ms_per_logic_frame);
}

int get_milliseconds( void )
{
    return SDL_GetTicks();
}

/*
void friz_sleep( void )
{
    if ( sleep_amt < 0 )
        sleep_amt = 0;
    if ( sleep_amt > 0 )
        SDL_Delay( sleep_amt );
}
*/

void friz_cleanup( void )
{
    static int called;

    if ( called )
        return;

    called = 1;

    GL_free_buffers(); 

    SDL_Quit(); /* SDL */
}

void atexit_frizcleanup( void )
{
    static int called;

    if ( called )
        return;

    called = 1;

    //push_msg( "shutdown initiated by system: atexit()" );
    friz_cleanup();
}

void friz_init( void )
{
    srandom( (unsigned)time( NULL ) );                            

#ifdef _DEBUG
    core.log.start(); // writes console output to file
#endif
}

void initiate_quit( char * msg )
{
    // user pressed 'q' 2 or more times, expedite
    if ( quit_time ) {
        quit_time = get_milliseconds() - 1;
        return; 
    }

    quit_time = 2000 + get_milliseconds();
    //push_msg( msg );
}

void get_game_paths( void )
{
    // cwd: directory exe invoked from, or exists in
    int sz = 2048;
    char * p = 0;
    while ( 1 ) 
    {
        p = (char*) malloc( sz );
        if ( getcwd( p, sz ) != 0 )
            break;
        free( p );
        if ( errno != ERANGE )
            core.error( "getcwd failure" );
        sz *= 2;
        if ( sz > 10000000 )
            break;
    }
    Assert( sz < 10000000 );
    gvar_t * cwd = new gvar_t( "cwd", p, "directory game invoked from" );
    free( p );

    // current user name
    gvar_t * uname = 0;
    if ( (p = getenv( "USER" )) )
        uname = new gvar_t( "username", p );
    else if ( (p = getlogin()) )
        uname = new gvar_t( "username", p );

    // system type: 1, 2, 3, 4
    // ...
    // user's home directory
    // 1: linux: /home/<username>
    // 2: mac: /Users/<username>
    // 3: win7: /Users/<username>
    // 4: winnt: /Documents\ and\ Settings/<username>
    DIR * dp = 0;
    char buf[2048];
    sprintf( buf, "/Users/%s", uname->string );
    if ( !(dp = opendir( buf )) ) {
        sprintf( buf, "/home/%s", uname->string );
        dp = opendir( buf );
    }
    gvar_t * udir = 0;
    if ( dp ) {
        closedir( dp );
        udir = new gvar_t( "userdir", buf );
    }

    // config path
    if ( udir )
        sprintf( buf, "%s/.frizl.cfg", udir->string );
    else
        sprintf( buf, "%s/.frizl.cfg", cwd->string );
    new gvar_t( "cfgpath", buf );
    

    // datapath: datadir fullpath
    sprintf( buf, "%s/%s", cwd->string, core.gvar( "datadir" )->string ); 
    gvar_t * datapath = new gvar_t( "datapath", buf );

    // imgdir: image dir fullpath
    sprintf( buf, "%s/img", datapath->string );
    new gvar_t( "imgpath", buf );

    // musicdir: music dir fullpath
    sprintf( buf, "%s/music", datapath->string );
    new gvar_t( "musicpath", buf );

    // sfxdir: sfx dir fullpath
    sprintf( buf, "%s/sfx", datapath->string );
    new gvar_t( "sfxpath", buf );
}

void cmdline_args_and_gvar_inits ( int argc, char **argv )
{
    core.myargc = argc;
    core.myargv = argv;

    new gvar_t( "tex_mode", "1", "Bi-Linear Filtering No Mipmap" );

    // cmdline args override static init values generally
    new gvar_t( "datadir", "data", 0 );

    get_game_paths();

#ifdef _DEBUG
    new gvar_t( "debug", "1", 0 );
#else
    new gvar_t( "debug", "0", 0 );
#endif

    if ( core.check_cmdline( "-f" ) || core.check_cmdline("--fullscreen") ) 
        new gvar_t( "fullscreen", "1", 0 );
    else
        new gvar_t( "fullscreen", "0", 0 );
    

    // TODO: mechanism for setting gvars from command-line

    // TODO: .frizl.cfg support which saves default settings on exit
    //                  and loads them on boot. can be overridden w/
    //                  cmdline switches, which will then be saved

/*
    else if ( check_cmdline( "-map" ) ) // set game map
    {
        do_mapeditor = gvar_create( "mapeditor", "0", 0 );
        if ( (p=get_cmdline_arg( "-map" )) )
        {
            map_name = gvar_create( "map_name", p, 0 );
*/
}


// build data after gvar_t instantiation, and after SDL/OpenGL init
void Load_Assets( void )
{
    core.materials.loadTextureDirectory();
    core.mixer.start();
    core.mixer.loadMusic();
    //core.mixer.loadSFX();

    // load a model
    if (!model.import("data/models/GlobalHawkOBJ.obj"))
        core.error( "couldn't load model\n" );
    model.normalize();
}


void handle_keyup( SDL_keysym *keysym )
{
    switch ( keysym->sym )
    {
    case SDLK_w: // forward
        core.camera.keyUpForward();
        break;
    case SDLK_s: // backward
        core.camera.keyUpBackward();
        break;
    case SDLK_a: // strafe-left
        core.camera.keyUpLeft();
        break;
    case SDLK_d: // strafe-right
        core.camera.keyUpRight();
        break;
    default:
        break;
    }
}


bool done = false;


void handle_keydown( SDL_keysym *keysym )
{
    switch ( keysym->sym )
    {
    case SDLK_q:
        done = true;
        break;
    case SDLK_f:
        //SDL_WM_ToggleFullScreen( surface );
        break;
    case SDLK_w: // forward
        core.camera.keyDownForward();
        break;
    case SDLK_s: // backward
        core.camera.keyDownBackward();
        break;
    case SDLK_a: // strafe-left
        core.camera.keyDownLeft();
        break;
    case SDLK_d: // strafe-right
        core.camera.keyDownRight();
        break;
    case SDLK_TAB: // alt+tab frees the mouse, active event captures it again
        if ( KMOD_LALT == keysym->mod  ) {
            SDL_GrabMode state = SDL_WM_GrabInput( SDL_GRAB_QUERY );
            state = state == SDL_GRAB_ON ? SDL_GRAB_OFF : state;
            SDL_WM_GrabInput( state );
        }
    case SDLK_ESCAPE:
        break;
    case SDLK_SPACE:
    default:
        break;
    }
}

void mouse_motion( SDL_MouseMotionEvent * motion )
{
    static int first_call;
    if ( 0 == first_call ) { ++first_call; return; }
    core.camera.mouseCursorMove( motion->yrel , motion->xrel );
}

void mouse_click( int x, int y, SDL_MouseButtonEvent * b_evt )
{
    #define P(x) core.printf( " --> %s\n", #x ); break
    if ( b_evt->type == SDL_MOUSEBUTTONDOWN )
    {
        switch( b_evt->button ) 
        { 
        case SDL_BUTTON_LEFT:       P(leftclick);
        case SDL_BUTTON_MIDDLE:     P(middleclick);
        case SDL_BUTTON_RIGHT:      P(rightclick);
        case SDL_BUTTON_WHEELUP:    P(mousewheelup);
        case SDL_BUTTON_WHEELDOWN:  P(mousewheeldown);
            break;
        }
    }
    else if ( b_evt->type == SDL_MOUSEBUTTONUP )
    {
        core.printf( " <-- UP\n" );
    }
    #undef P
}


void calc_fps()
{
    static int frames;
    static int t0;

    frames++;

    int t = SDL_GetTicks();

    if (t - t0 >= 5000) 
    {
        float seconds = (t - t0) / 1000.0;
        float fps = frames / seconds;
        core.printf("%d frames in %g seconds = %g FPS\n", frames, seconds, fps);
        t0 = t;
        frames = 0;
    }
}


void Start_SDL_GL( void )
{

    if ( SDL_Init( SDL_INIT_VIDEO /* | SDL_INIT_AUDIO */ ) < 0 )
    {
        core.error( "Video/Audio initialization failed: %s\n", SDL_GetError( ) );
    }
    
    const SDL_VideoInfo * videoInfo = SDL_GetVideoInfo();

    if ( !videoInfo )
    {
        core.error( "Video query failed: %s\n", SDL_GetError( ) );
    }

    
    int videoFlags  = SDL_OPENGL;          
    videoFlags |= SDL_GL_DOUBLEBUFFER; 
    videoFlags |= SDL_HWPALETTE;       
    videoFlags |= SDL_RESIZABLE;       

    gvar_t * fullscreen = core.gvar( "fullscreen" );
    if ( 1 == fullscreen->integer ) {
        videoFlags |= SDL_FULLSCREEN;
        core.printf( "Going Fullscreen\n" );
    } else {
        core.printf( "Going Windowed\n" );
    }

    

    if ( videoInfo->hw_available ) {
        videoFlags |= SDL_HWSURFACE;
        core.printf( "got Hardware surface\n" );
    } else {
        videoFlags |= SDL_SWSURFACE;
        core.printf( "got Software surface\n" );
    }
    
    if ( videoInfo->blit_hw )
        videoFlags |= SDL_HWACCEL;

    // set GL attributes before call to SDL_SetVideoMode
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    
    int bpp = videoInfo->vfmt->BitsPerPixel;

    struct {
        const char * name;
        unsigned int flags;
        SDL_Rect ** modes;
    } sdl_modes[4] = {
        { "SDL_FULLSCREEN|SDL_HWSURFACE|SDL_OPENGL", SDL_FULLSCREEN|SDL_HWSURFACE|SDL_OPENGL, 0 },
        { "SDL_FULLSCREEN|SDL_SWSURFACE|SDL_OPENGL", SDL_FULLSCREEN|SDL_SWSURFACE|SDL_OPENGL, 0 }, 
        { "SDL_HWSURFACE|SDL_OPENGL", SDL_HWSURFACE|SDL_OPENGL, 0 },
        { "SDL_SWSURFACE|SDL_OPENGL", SDL_SWSURFACE|SDL_OPENGL, 0 },
    };

    gvar_t * g_vid[2] = { 0, 0 };

    for ( int i = 0; i < 4; i++ )
    {
        sdl_modes[i].modes = SDL_ListModes( 0, sdl_modes[i].flags );

        if ( !sdl_modes[i].modes ) {
            core.printf( " %d: %s: no modes\n", i, sdl_modes[i].name );
        } else if ( sdl_modes[i].modes == (SDL_Rect**)-1 ) {
            core.printf( " %d: %s: all modes available\n", i, sdl_modes[i].name );
        } 
        else 
        {
            for ( int j = 0; sdl_modes[i].modes[j]; j++ ) 
            {
                core.printf( " %d: %s: %d: %d x %d\n", i, sdl_modes[i].name, j+1, sdl_modes[i].modes[j]->w, sdl_modes[i].modes[j]->h );

                // get first mode (should be largest) 
                if ( g_vid[0] )
                    continue;

                int w = fullscreen->integer ? sdl_modes[i].modes[j]->w : 1024;
                int h = fullscreen->integer ? sdl_modes[i].modes[j]->h : 768;

                char buf[20];
                sprintf( buf, "%d", w );
                g_vid[0] = new gvar_t ( "v_width", buf );
                sprintf( buf, "%d", h );
                g_vid[1] = new gvar_t ( "v_height", buf );
            } 
        }
    }
    


    Assert( 0 != core.gvar( "v_width" ) );
    Assert( 0 != core.gvar( "v_height" ) );

    int v_width = core.gvar("v_width")->integer;
    int v_height = core.gvar("v_height")->integer;

    core.printf( "Setting resolution: \"%sx%s\"\n", core.gvar("v_width")->string, core.gvar("v_height")->string );


    surface = SDL_SetVideoMode( v_width, v_height, bpp, videoFlags );
    
    if ( !surface )
    {
        core.error( "Video mode set failed: %s\n", SDL_GetError( ) );
    }

    //GL_print_strings();
    const char * glstr = GL_extensions_string();
    char * glstr2 = (char *) alloca( strlen(glstr) + 10 ); 
    memset( glstr2, 0, strlen(glstr) + 10 );
    strcpy( glstr2, glstr );
    chr_replace( ' ', '\n', glstr2 );
    new gvar_t( "GL_extensions:", glstr2 );
    
    const char * ext[] = {
        "GL_ARB_vertex_buffer_object", 
        "GL_EXT_vertex_buffer_object", 
        "GL_ARB_point_sprite",
        "GL_ARB_transpose_matrix",
        "GL_ARB_occlusion_query",
        "GL_ARB_texture_non_power_of_two",
        "GL_ARB_texture_compression_rgtc",
        "GL_ARB_framebuffer_object",
        "GL_EXT_texture_compression_s3tc",
        "GL_EXT_texture_compression_dxt1",
        "GL_EXT_texture_array",
        0
    };

    gvar_t * vend = new gvar_t( "GL_Vendor", GL_vendor_string() );
    gvar_t * vers = new gvar_t( "GL_Version", GL_version_string() );
    core.printf( "%-34s %s\n%-34s %s\n", vend->identifier, vend->string, vers->identifier, vers->string );
    
    for ( int i = 0; ext[i]; i++ )
    {
        core.printf( "%-34s %s\n", ext[i], strstr( glstr, ext[i] ) ? "is available" : "NOT available" );
    }
    
return;
    if ( SDL_WM_GrabInput( SDL_GRAB_ON ) != SDL_GRAB_ON )
    {
        core.warn( "SDL unable to grab window input" );
    }
}



void set_a_buncha_cubes() 
{
    float y = 0.f;
    int c = 0;

    for ( int j = 0; j < 49; j++ ) 
    {
        for ( int i = 0; i < 49; i++ ) 
        {
            core.drawer.setCube( i * 15, y, j * 15, 1.0f, (float)c, 0.f );
            c ^= 1;
        }
    }

    core.drawer.setCube( -10, 0.f, -50.0f, 0.7f, 0.8f, 0.7f );
    // vThis works!
    //core.drawer.getLast()->rotation.rotate( 15.f, 0.f, 1.f, 0.f );
    core.drawer.getLast()->rotation.scale( 1.f, 15.f, 250.f );

    core.drawer.setCube( -61.f, 0.f, -50.f, 0.33f, 0.f, 0.f );
    core.drawer.getLast()->rotation.scale( 5.f, 20.f, 2.f );
    core.drawer.setCube( -61.f, 0.f, -90.f, 0.33f, 0.f, 0.f );
    core.drawer.getLast()->rotation.scale( 5.f, 24.f, 2.f );
    core.drawer.setCube( -61.f, 0.f, -130.f, 0.33f, 0.f, 0.f );
    core.drawer.getLast()->rotation.scale( 5.f, 18.f, 2.f );
}

void Game_Main_Loop( tessellatedPlane_t * tes )
{
    SDL_Event event;

    set_a_buncha_cubes();


    while ( !done )
    {
        
        while ( SDL_PollEvent( &event ) )
        {
            switch( event.type )
            {
            case SDL_ACTIVEEVENT:
                if ( event.active.gain )
                {
                    SDL_WM_GrabInput( SDL_GRAB_ON );
                }
                else
                {
                    SDL_WM_GrabInput( SDL_GRAB_OFF );
                }
                break;
            case SDL_VIDEORESIZE:
                /*surface = SDL_SetVideoMode( event.resize.w, event.resize.h, 16, videoFlags );
                if ( !surface )
                {
                    fprintf( stderr, "Could not get a surface after resize: %s\n", SDL_GetError( ) );
                }
                GL_reset_camera_projection( SCREEN_WIDTH, SCREEN_HEIGHT );
                */
                break;
            case SDL_KEYDOWN:
                core.printf( " --> KEYDOWN\n" );
                handle_keydown( &event.key.keysym ) ;
                break;
            case SDL_KEYUP:
                core.printf( " --> KEYUP\n" );
                handle_keyup( &event.key.keysym ) ;
                break;
            case SDL_QUIT:
                done = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
                mouse_click( event.button.x, event.button.y, &event.button );
                break;
            case SDL_MOUSEMOTION:            
                mouse_motion( &event.motion );
                break;            
            default:
                break;
            }
        }

//printf( "heading: %1.2f, pitch: %1.2f\n", core.camera.heading, core.camera.pitch );
//printf( "speed: %f\n", core.camera.move[camera_t::MV_FORWARD] );

        core.camera.update();
        
        GL_draw_frame( tes );

        calc_fps();
    }
}


int main( int argc, char **argv )
{

//    int tic, lasttic = 0;
//    int now, draw_ms = 0;


    friz_init();

    // 
    cmdline_args_and_gvar_inits( argc, argv );


/*
    if ( do_mapeditor->integer )
    {
        editor_main();
    }
*/

    // TODO: catch signals ? - "caught signal, cleaning up..."
    //atexit( atexit_frizcleanup );


//    lasttic = tic = get_ticks();
//    now = get_milliseconds() / ms_per_render_frame;



    // tesselate a heightmap into array of world-space vertices: tes->verts
    heightMap_t         map( 24, 32, 32 );
    tessellatedPlane_t  tes ( map, 1000.f, 1000.f );
    tes.gradedTones( 0.0f, 0.88f, 0.0f );

    

    Start_SDL_GL();

    // build data after gvar_t instantiation, and after SDL/OpenGL init
    Load_Assets();
    
    GL_init_state();

    GL_camera_projection( core.gvar("v_width")->integer, core.gvar("v_height")->integer );
    
    SDL_ShowCursor(0);

    core.mixer.play( "Underground" );

    core.dump_gvar();

    Game_Main_Loop( &tes );
    

    friz_cleanup(); 
    
    return( 0 );
}

