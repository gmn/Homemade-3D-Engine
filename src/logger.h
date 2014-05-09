#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdio.h> // FILE*


#define DEF_LOGNAME "fz.log"


struct logger_t 
{
private:

    FILE * fp;

public:
    static const char * def_logname;

    void start( int append=0 );                   // must call start() to use
    void close();                           // called in destructor
    void msg( const char * );
    void printf( const char *, ... );

    logger_t() : fp(0) {}
    ~logger_t();
};


#endif /* __LOGGER_H__ */
