#ifndef __HASH_H__
#define __HASH_H__

// hash.h

//#define HASH_TABLE_DEFAULT_SZ       1024
//#define HASH_STRING_SZ              128

struct hashnode_t {
    int                 fullhash;
    char *              str;
    void *              val;
    hashnode_t *        next;
    hashnode_t();
    hashnode_t( int, const char *, void * );
    ~hashnode_t();
};

struct hashtable_t 
{
    static const int HASH_TABLE_DEFAULT_SZ = 1024;

    int                 size;
    int                 mask;               
    hashnode_t **       nodes;
    int                 stored;

    hashtable_t();
    hashtable_t( int size ) ;
    ~hashtable_t();

    void    insert( const char * str, void * store ) ;
    void *  retrieve( const char * str ) ;

private:
    void    init( int );
};

int hash_string( const char *str ) ;

#endif /* __HASH_H__ */
