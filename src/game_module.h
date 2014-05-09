// game_module.h

class game_module_t 
{
    virtual ~game_module_t() = 0;
    virtual void update_screen() = 0;
    virtual void update_logic() = 0;
    virtual void handle_keydown( int ) = 0;
    virtual void handle_keyup( int ) = 0;
    virtual void handle_mouseclick( int, int ) = 0;
    virtual void handle_mousemovement() = 0;
    
    virtual void shutdown() = 0;
    virtual void init() = 0;
};

class MainGame_t : public game_module_t
{
    ~MainGame_t();

    void update_screen( void ) 
    {
    }

    void update_logic( void )
    {
    }

    void handle_keydown( int k )
    {
    }

    void handle_keyup( int k )
    {
    }

    void handle_mouseclick( int k )
    {
    }

    void handle_mousemovement( int k )
    {
    }

    void shutdown()
    {
    }

    void init()
    {
    }
};



