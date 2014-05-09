// mixer.cpp

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include "mixer.h"
#include "hash.h"
#include "core.h"


/**
 *  sfxchunk_t
 */
sfxchunk_t::sfxchunk_t() : chunk(0), next(0)
{
    memset( name, 0, CHUNK_NAME_SIZE );
}

sfxchunk_t::sfxchunk_t( void * c, const char * _name, int _type ) : next(0)
{
    chunk = c;
    int len = strlen( _name ) + 1;
    if ( len > CHUNK_NAME_SIZE ) {
        len = CHUNK_NAME_SIZE;
    }
    strncpy( name, _name, len );
    name[ CHUNK_NAME_SIZE - 1 ] = '\0';
    type = _type;
}

sfxchunk_t::~sfxchunk_t()
{
    if ( type == CHUNK_SFX ) {
        Mix_Chunk * p = (Mix_Chunk*) chunk;
        Mix_FreeChunk( p );
    } else if ( type == CHUNK_MUSIC ) {
        Mix_Music * p = (Mix_Music*) chunk;
	    Mix_FreeMusic( p );
    }
}


/**
 *  mixer_t
 */
mixer_t::mixer_t( void )
{
    system_string_len = 8000;
    
    frequency = 44100;
    audio_format = MIX_DEFAULT_FORMAT;
    audio_format = AUDIO_S16SYS;
    audio_format = AUDIO_S16LSB;
    channels = 2;
    buffer_sz = 4096;
    music_volume = MIX_MAX_VOLUME;
    sfx_volume = MIX_MAX_VOLUME;

    snd_table = new hashtable_t( 128 );
}



mixer_t::~mixer_t ( void )
{
    // free sfx and music
    freeSounds( &sfxChunkHead );

    delete snd_table;

    // return system_string
    if ( system_string )
        delete system_string;

    // shut down SDL audio
	Mix_CloseAudio();
}


// returns system_string on success, or NULL
char * mixer_t::system_info()
{
    if ( ! system_string )
        system_string = new char[ system_string_len ];

    memset ( system_string, 0, system_string_len );

	audio_device_times_opened = Mix_QuerySpec( &frequency, &audio_format, &channels );

    if ( ! audio_device_times_opened )
    {
        core.warn( "Audio device couldn't be opened: \"%s\"\n", SDL_GetError() );
        return (char*)0;
    }

    int bits = audio_format & 0xFF;

    const char *format_str="Unknown";
    switch ( audio_format ) 
    {
        case AUDIO_U8:
            format_str="U8";
            break;
        case AUDIO_S8:
            format_str="S8";
            break;
        case AUDIO_U16LSB:
            format_str="U16LSB";
            break;
        case AUDIO_S16LSB:
            format_str="S16LSB";
            break;
        case AUDIO_U16MSB:
            format_str="U16MSB";
            break;
        case AUDIO_S16MSB:
            format_str="S16MSB";
            break;
        default:
            break;
    }

    snprintf( system_string, system_string_len, "got %d Hz, %d format_bits (%s), %d channels, %d bytes audio buffer. Opened %d times. Music volume: %d, sfx volume: %d", 
            frequency, 
            bits, 
            format_str, 
            channels, 
            buffer_sz, 
            audio_device_times_opened, 
            music_volume, 
            sfx_volume );

    return system_string;
}


// read datadir, load music & load sfx
void mixer_t::start()
{
    Mix_AllocateChannels(4); // 

	if ( -1 == Mix_OpenAudio(frequency, audio_format, channels, buffer_sz) ) {
        core.warn( "unable to open audio device: \"%s\"\n", SDL_GetError() );
        return;
	} 

    Mix_SetPanning( MIX_CHANNEL_POST, 127, 127 );


    // report audio setup 
    system_info();
    core.printf( "" );
    core.printf( "SDL_mixer started: %s\n", system_string );


    music = 0; // unset so mixer doesn't think it's already playing
}

// 
void mixer_t::play ( const char * name )
{
    Mix_Music * mus = (Mix_Music*) snd_table->retrieve( name );
    if ( ! mus ) {
        core.printf( "requested music file: %s not found\n", name );
        return;
    }

    if ( mus == music )
        return; // already playing, ignore call

    // stop current music if playing
    stop();

    // set music equal to current playing
    music = mus;

    // play it
    if ( Mix_PlayingMusic() )
        Mix_HaltMusic();

    Mix_PlayMusic( music, -1 );

    core.printf( "playing music: \"%s\"\n", name );
}

// stop music
void mixer_t::stop( void )
{
    if ( Mix_PlayingMusic() )
        Mix_HaltMusic();
    music = 0;
}

// returns play-state after this call
//  - 1 for playing 
//  - 0 for paused 
int mixer_t::pause ( void )
{
    if ( Mix_PausedMusic() ) {
        Mix_ResumeMusic();
        return 1;
    }

    Mix_PauseMusic();
    return 0;
}


void mixer_t::sfx ( const char * name )
{
    Mix_Chunk * sfx = (Mix_Chunk*) snd_table->retrieve( name );
    if ( ! sfx ) {
        core.printf( "requested sfx chunk: %s not found\n", name );
        return;
    }

    // -1 play on first free channel
    //  0 play exactly once
    Mix_PlayChannel( -1, sfx, 0 ); 

    core.printf( "sfx: \"%s\"\n", name );
}

void mixer_t::fadein ( int ms )
{
    if ( Mix_PlayingMusic() )
        Mix_FadeInMusic( music, -1, ms );
}

void mixer_t::fadeout ( int ms )
{
    if ( Mix_PlayingMusic() )
        Mix_FadeOutMusic( ms );
}

// 0 <= pan <= 254, 127 is centered
void mixer_t::pan ( int p )
{
    if ( p < 0 ) 
        p = 0;
    else if ( p > 254 )
        p = 254;

    int left = p;
    int right = 254 - p; 

    Mix_SetPanning( MIX_CHANNEL_POST, left, right );
}

// 
void mixer_t::set_volume( int vol, int type )
{
    if ( vol < MIX_MAX_VOLUME && vol >= 0 ) 
    {
        const char * which = "Neither";
        switch ( type ) {
        case VOL_SFX: 
            sfx_volume = vol; 
            Mix_Volume ( -1, vol ); // -1 = set for all channels (same as chunks?)
            which = "SFX";
            break;
        case VOL_MUSIC: 
            music_volume = vol; 
            Mix_VolumeMusic( vol );
            which = "Music";
            break;
        case VOL_BOTH:
            sfx_volume = vol; 
            Mix_Volume ( -1, vol ); // -1 = set for all channels (same as chunks?)
            music_volume = vol; 
            Mix_VolumeMusic( vol );
            which = "Music and SFX";
            break;
        }  

        // Mix_VolumeChunk( Mix_Chunk *, int ); // per-chunk volume
        core.printf( "%s volume set to: %d\n", which, vol );
    } else
        core.warn( "tried to set volume: %d, out of range\n", vol );
}


// store a sample already loaded by SDL, into hashtable
void mixer_t::push_sfx( Mix_Chunk * c, const char *name )
{
    sfxchunk_t * p = new sfxchunk_t( c, name, CHUNK_SFX );

    if ( !sfxChunkHead )
    {
        sfxChunkHead = p;
    }
    else
    {
        sfxchunk_t * r = sfxChunkHead;
        while ( r->next )
        {
            r = r->next;
        }
        r->next = p;
    }
}

void mixer_t::push_music( Mix_Music * mus, const char * name )
{
    sfxchunk_t * p = new sfxchunk_t( mus, name, CHUNK_MUSIC );

    if ( !sfxChunkHead )
    {
        sfxChunkHead = p;
    }
    else
    {
        sfxchunk_t * r = sfxChunkHead;
        while ( r->next )
        {
            r = r->next;
        }
        r->next = p;
    }
}

void mixer_t::freeSounds( sfxchunk_t **p_head )
{
    if ( *p_head )
    {
        sfxchunk_t * p = *p_head;
        sfxchunk_t * tmp;
        while ( p )
        {
            tmp = p->next;
            delete p;
            p = tmp;
        }
    }
    *p_head = 0;
}

void mixer_t::loadMusic()
{
    Mix_Music * m;
    gvar_t * mdir = core.gvar( "musicpath" );
    char ** tunes = list_directory( mdir->string );
    char ** tune = tunes;

    while ( *tune )
    {

        if ( (m = Mix_LoadMUS( *tune )) == 0 ) {
            core.warn("didn't load music: %s\n", Mix_GetError() );
            ++tune; continue;
        }

        const char * canonical_name = strip_path_and_extension( *tune );
        push_music( m, canonical_name ); // 
        snd_table->insert( canonical_name, m );
        core.printf( " \"%s\" loaded\n", canonical_name );

        ++tune;
    }

    free_listdir( tunes );
}

void mixer_t::loadSFX()
{
    gvar_t * sfxdir = core.gvar( "sfxpath" );
    char ** sfxlist = list_directory( sfxdir->string );
    char ** sfx = sfxlist;
    Mix_Chunk * sample1;

    while ( *sfx )
    {
        if ( (sample1 = Mix_LoadWAV( *sfx )) == NULL ) {
            core.warn( "didn't load sfx: %s\n", *sfx );
        } else if ( sample1 ) {
            const char * sfxname = strip_path_and_extension( *sfx );
            push_sfx( sample1, sfxname );
            snd_table->insert( sfxname, sample1 );
            core.printf( " \"%s\" loaded\n", sfxname );
        }
        ++sfx;
    }

    free_listdir( sfxlist );
}


