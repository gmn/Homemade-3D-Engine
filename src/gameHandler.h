#ifndef __GAME_HANDLER_H__
#define __GAME_HANDLER_H__

// gameHandler.h
//
// implements: gameModule_t, gameHandler_t
//


struct gameModule_t
{
    char * name; 
    virtual void keyDown() = 0;
    virtual void keyUp() = 0;
    virtual void mouseMotion() = 0;
    virtual void mouseClick() = 0;
    virtual void update() = 0;
    virtual void drawFrame() = 0;

    gameModule_t * next;

    gameModule_t() : name(0), next(0) {}
    gameModule_t( const char * );
    virtual ~gameModule_t();
};

struct gameHandler_t
{
    void keyDown();
    void keyUp();
    void mouseMotion();
    void mouseClick();
    void update();
    void drawFrame();

    void do_module( const char * );
    void registerModule( gameModule_t * );

    gameModule_t * current_module;
    gameModule_t * modules;

    gameHandler_t() : current_module(0), modules(0) {}
    ~gameHandler_t(); // deletes modules
};


#endif //__GAME_HANDLER_H__
