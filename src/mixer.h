#ifndef __MIXER_H__
#define __MIXER_H__
// mixer.h

#include <SDL/SDL_mixer.h> // Mix_Music, Mix_Chunk

#include "hash.h"


enum {
    VOL_SFX,
    VOL_MUSIC,
    VOL_BOTH,

    // sfxchunk_t::type
    CHUNK_MUSIC,
    CHUNK_SFX,
};


// basically all these are for it turns out is to provide the destructor mechanism
// to automatically free Mix_Chunk*, Mix_Music*
struct sfxchunk_t {
    static const int CHUNK_NAME_SIZE = 128;

    char name[ CHUNK_NAME_SIZE ];
    void *          chunk;
    sfxchunk_t *    next;
    int             type;

    sfxchunk_t() ;
    sfxchunk_t( void *, const char *, int type );
    ~sfxchunk_t();
};



// sound controller, master struct
struct mixer_t 
{
    int frequency;
    unsigned short audio_format;
    int channels;
    int buffers;
    char * system_string;
    int system_string_len;
    int audio_device_times_opened;
    int buffer_sz;

    int music_volume;
    int sfx_volume;

    Mix_Music * music;                  // pointer to current playing music
    
    // deletion list
    sfxchunk_t * sfxChunkHead;  

    // look-up table, stores pointers to all loaded sounds (music & sfx)
    hashtable_t * snd_table;

    mixer_t();
    ~mixer_t();

    char *  system_info ();             // returns string with system sound info
    void    start ();                   // read datadir, load music & load sfx
    void    play ( const char * );      // starts a music by name, returns 0 on failure
    void    stop ( void );              // stop music
    int     pause ();                   // toggle, return state after toggle
    void    sfx ( const char * );
    void    fadein ( int );
    void    fadeout ( int );
    void    pan ( int );
    void    set_volume ( int, int =VOL_BOTH );
    void    loadMusic();
    void    loadSFX();

private:
    void    freeSounds( sfxchunk_t ** );
    void    push_sfx( Mix_Chunk * c, const char *name );
    void    push_music( Mix_Music * c, const char *name );
};


#endif /* __MIXER_H__ */
