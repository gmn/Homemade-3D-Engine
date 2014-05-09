#ifndef __GAME_H__
#define __GAME_H__
// game.h

#include "gameHandler.h"

struct tunnelModule_t : public gameModule_t
{
    void keyDown();
    void keyUp();
    void mouseMotion();
    void mouseClick();
    void update();
    void drawFrame();

    tunnelModule_t( const char * );
    ~tunnelModule_t();
};

#endif //__GAME_H__
