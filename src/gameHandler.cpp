#include "core.h"
#include "hgr.h"
#include "gameHandler.h"

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    gameModule_t

        virtual interface for game modules. generic functions implemented here
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
gameModule_t::gameModule_t( const char * _name ) : next(0)
{
    Assert ( _name && _name[0] && "gameModule Must have a name!" );

    name = copy_string( _name );
}

gameModule_t::~gameModule_t()
{
    if ( name )
        free( name );
    name = 0;
}

/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    gameHandler_t
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
gameHandler_t::~gameHandler_t() 
{
    gameModule_t * p = modules;
    gameModule_t * tmp;
    while ( p )
    { 
        tmp = p->next;
        delete p;
        p = tmp;
    }
}


void gameHandler_t::do_module( const char * modname )
{
    Assert( modname && modname[0] && "Module must have a name." );

    if ( ! modules ) {
        core.warn( "No game modules registered!\n" );
        return;
    }

    gameModule_t * p = modules;
    
    while ( p )
    {
        if ( strcmp( p->name, modname ) == 0 )
        {
            core.printf( "doing game module: \"%s\"\n", modname );
            current_module = p;
            return;
        }
        p = p->next;
    }

    core.warn( "game module: \"%s\" not found!!!\n", modname );
}

void gameHandler_t::registerModule( gameModule_t * mod_p )
{
    if ( modules )
    {
        gameModule_t * p = modules;
        while ( p->next ) {
            p = p->next;
        }
        p->next = mod_p;
        mod_p->next = 0; // make sure
    } 
    else
    {
        modules = mod_p;
        mod_p->next = 0; 
    }
}


void gameHandler_t::keyDown()
{
    if ( ! current_module ) {
        core.warn( "No game module assigned!\n" );
        return;
    }

    current_module->keyDown();
}

void gameHandler_t::keyUp()
{
    if ( ! current_module ) {
        core.warn( "No game module assigned!\n" );
        return;
    }

    current_module->keyUp();
}

void gameHandler_t::mouseMotion()
{
    if ( ! current_module ) {
        core.warn( "No game module assigned!\n" );
        return;
    }

    current_module->mouseMotion();
}

void gameHandler_t::mouseClick()
{
    if ( ! current_module ) {
        core.warn( "No game module assigned!\n" );
        return;
    }

    current_module->mouseClick();
}

void gameHandler_t::update()
{
    if ( ! current_module ) {
        core.warn( "No game module assigned!\n" );
        return;
    }

    current_module->update();
}

void gameHandler_t::drawFrame()
{
    if ( ! current_module ) {
        core.warn( "No game module assigned!\n" );
        return;
    }

    current_module->drawFrame();
}


