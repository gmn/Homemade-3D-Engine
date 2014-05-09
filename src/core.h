#ifndef __CORE_H__
#define __CORE_H__
// 
// game's core systems
//
#include "logger.h"
#include "console.h"
#include "mixer.h"
#include "hash.h"
#include "camera.h"
#include "gvar.h"
#include "drawlists.h"
#include "materials.h"
#include "gameHandler.h"

struct core_t 
{
    int         myargc;
    char **     myargv;

    core_t();
    ~core_t();


    logger_t        log;
    console_t       console;
    mixer_t         mixer;
    hashtable_t     gvar_table;
    camera_t        camera;
    drawer_t        drawer;
    materialSet_t   materials;
    gameHandler_t   handler;


    void dump_console();
    void dump_gvar();
    void warn( const char *, ... );
    void error( const char *, ... );
    void printf( const char *, ... );

    const char ** check_cmdline( const char *parm ) ;
    const char * get_cmdline_arg( const char * cmd );

    gvar_t * gvar( const char * identifier ) {
        return (gvar_t *) gvar_table.retrieve( identifier );
    }

    void ChangeModule( const char * name ) { }

};

extern core_t core; // global handle



#endif /* __CORE_H__ */
