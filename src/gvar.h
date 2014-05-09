#ifndef __GVAR_H__
#define __GVAR_H__
// gvar.h

#define GVAR_IDENTIFIER_SIZE   50

struct gvar_t {
    char    identifier [ GVAR_IDENTIFIER_SIZE ];
    char *  string;
    int     integer;
    float   fval;
    char *  comments;

    // name, value, comments
    gvar_t ( const char * , const char * , const char * =0 );
    ~gvar_t();

    // retrieve one by name
    gvar_t * find( const char * name );

    // reset value of already created
    void set( const char * , const char * =0 ) ;
};




#endif /* __GVAR_H__ */
