#ifndef __FTIMER_H__
#define __FTIMER_H__

int get_milliseconds( void );

//==================================================================
//   ftimer_t
//==================================================================
struct ftimer_t
{
    int flags;
    int start;
    int length;

    ftimer_t() : flags(0),start(0),length(0) {}
    ftimer_t( int t ) { set( t ); }
    ftimer_t( ftimer_t const& t ) { set( t ); }

    void reset ( void ) {
        flags = start = length = 0;
    }

    void stop( void ) {
        flags = start = length = 0;
    }

    int check( void ) {
        return get_milliseconds() - start > length;
    }

    int timeup( void ) {
        return check();
    }

    void set( int len =0 ) {
        length = len;
        start = get_milliseconds();
        flags = 1;
    }

    void set( ftimer_t const& t ) {
		flags = t.flags;
		start = t.start;
		length = t.length;
    }

    void set( int len, int flag ) {
        length = len;
        start = get_milliseconds();
        flags = flag;
    }

	int delta( void ) {
		return get_milliseconds() - start;
	}

    int time( void ) {
        return delta();
    }

	float ratio( void ) {
		return (float) delta() / (float) length;
	}

    // create new start, from time left on timer (or now if time's up)
    void increment( int inc ) {
        int left = length - delta();
        if ( left < 0 ) left = 0;
        start = get_milliseconds();
        length = left + inc;
        if ( 0 == flags ) flags = 1;
    }
};

#endif
