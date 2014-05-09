#ifndef __DATATYPES_H__
#define __DATATYPES_H__

#include <string.h>
#include <stdlib.h> // malloc

#include "hgr.h"    // Assert

/*
------------------------------------------------------------------------------
    
    list_t: multipurpose, templated, doubly linked-list class with its own memory pool

    memPool_t: used by list_t; made up of 1 or more poolPage_t.

        1 poolPage_t is allocated ahead of time; more added if the list uses them.
        Items can be returned to the pool and used again at a very fast speed.

    vec_t: simple resizing array, good for repetitive work on sets of unknown size

    buffer_t: primitive, fast, automatically-growing data array 

------------------------------------------------------------------------------
*/

#define MAX_PAGE_SIZE (1024*1024*16)
#define DEF_POOL_PAGE_SIZE 128
#define BUFFER_DEFAULT_SIZE 128

/*
   - no linkage to these in .h
volatile static const unsigned int MaxPageSize = 1024 * 1024 * 16;
static volatile const int DefPoolPageSize = 1024;
volatile static const unsigned int BUFFER_DEFAULT_SIZE = 2048;
*/

/*
======================================
    poolPage_t
======================================
*/
template <typename type>
struct poolPage_t
{
    int pageSize;
    int inuse;                  // count how many used
    type *page;
    int returnEnd;              // last good index into returnList
    int returnStart;            // first good index into returnList
    poolPage_t<type> *next, *prev;
    unsigned int *returnList;

    // init is basic O(1). for use only on a page's initialization/1st use
    // dont alloc returnList until we need it
    poolPage_t( int sz =DEF_POOL_PAGE_SIZE ) : next(0), prev(0), returnList(0)
    {
        pageSize = sz;
        page = new type[ pageSize ];
        reset();
    }

    ~poolPage_t()
    {
        if ( page )
            delete[] page;
        if ( returnList )
            delete[] returnList;
    }

    //  O(1) 
    void reset( void ) 
    {
        returnEnd = returnStart = -1;
        inuse = 0;
    }

    void erase( void ) 
    {
        memset( page, 0, sizeof(type) * pageSize );
        reset();
    }

    void returnOne( unsigned int n ) 
    {
        // none returned at all
        if ( returnEnd == -1 ) {
            returnEnd = returnStart = 0;
            if ( !returnList )
                returnList = new unsigned int[ pageSize ];
            returnList[0] = n;
            return;
        }

        int i = (returnEnd + 1) % pageSize;

        // haven't hit returnStart, tack return onto End
        if ( i != returnStart ) {
            returnList[i] = n;
            returnEnd = i;
            return;
        }

        // returnEnd == returnStart, the returns rolled over,
        //  the entire list is free 
        reset();
    }

    // all are in use in memPool; see if there are any in returnList that we can use.
    int checkForReturned( void ) {
        if ( returnStart != -1 ) {
            // one open slot left in returnList
            if ( returnStart == returnEnd ) {
                int n = returnStart;
                returnStart = returnEnd = -1;
                return returnList[n];
            }
            int ind = returnStart;
            returnStart = (returnStart+1) % pageSize;
            return returnList[ ind ];
        }
        return -1;
    }

}; // memPage_t



/*
======================================
 memPool_t
======================================
*/
template <typename type>
struct memPool_t 
{
    poolPage_t<type> * page;
    unsigned int pageIndex;  // index of the current page 
    unsigned int numPages;
    unsigned int CreatedBaseSize;
    unsigned int currentPageSize;

    memPool_t() : pageIndex(0), numPages(1)
    {
        CreatedBaseSize = DEF_POOL_PAGE_SIZE;
        currentPageSize = CreatedBaseSize;
        page = new poolPage_t<type>( currentPageSize );
    }

    memPool_t( unsigned int sz ) : pageIndex(0), numPages(1)
    {
        // pages double in size, first one starts at DEF_POOL_PAGE_SIZE or what
        //  was requested. on reset this goes back to CreatedBaseSize
        CreatedBaseSize = sz;
        currentPageSize = CreatedBaseSize;
        page = new poolPage_t<type>( currentPageSize );

        // reset only first page.  if there are other pages, they are
        //  reset when they become the current page again
        // note: a page's size is only determined when it is first created,
        //  afterwards the size argument is ignored
        //page.reset( currentPageSize );
    }

    ~memPool_t()
    {
        drain();
        delete page;
    }

    void reset() {
        currentPageSize = CreatedBaseSize;
        pageIndex = 0;
        page->reset();
    }

    // completely wipes out and resets whole pool
    void clear()
    {
        poolPage_t<type> *p = page;
        while ( p->next ) {
            p = p->next;
            p->prev->erase();
        }
        p->erase();
    }


    poolPage_t<type> * newpage( void ) 
    {
        // get current page
        poolPage_t<type> *p = getPage( pageIndex );

        // if we're at the end of our pages...  create another page
        if ( pageIndex == numPages - 1 ) 
        {
            // heuristic: each new page is twice the size of the last
            if ( currentPageSize < MAX_PAGE_SIZE ) 
                currentPageSize *= 2;

            p->next = new poolPage_t<type>( currentPageSize );
            ++numPages;
        } 
        else 
        // get the pagesize from a page we already created
        {
            if ( p->next )
                currentPageSize = p->next->pageSize;
        }

        // this step serves to initialize a page, if it was created in the
        //  step above, it also resets pages that we have already allocated
        ++pageIndex;

        if ( p->next ) {
            p->next->reset();
            p->next->prev = p;
        }

        return p->next;
    }

    //
    type *getone ( void ) 
    {
        poolPage_t<type> *p;

        // check we are not off the end of the poolPage_t
        if ( !( p = getPage( pageIndex ) ) )
            return 0;
        if ( p->pageSize == p->inuse ) {
            // see if there were any returned that we can reuse them
            int i = p->checkForReturned();
            if ( i > -1 )
                return &p->page[i];
            // entire page in use, get new one
            p = newpage();
        }

        ++p->inuse;
        return &p->page[p->inuse-1];
    }

    void returnone( type *ret ) {
        // find which page
        poolPage_t<type> *p = page;
        while ( 1 ) {
            int diff; 
            if ( (diff = (int)(ret - p->page)) < p->pageSize ) {
                p->returnOne( diff ) ;
                return;
            }
            // hit the end and didn't find it
            if ( !p->next )
                return;
            p = p->next;
        }
    }

    poolPage_t<type> * getPage( unsigned int n ) 
    {
        poolPage_t<type> *p = page;
        if ( n == 0 ) 
            return page;
        
        do {
            p = p->next;
            if ( ! p )
                return 0;
        } while ( --n != 0 );

        return p;
    }

    // relinquishes all pages except the base page
    void drain( void ) 
    {
        poolPage_t<type> *p;
        poolPage_t<type> *tmp;
        for ( p = page->next ;  p  ; p = tmp ) {
            tmp = p->next;
            delete p;
        }
        numPages = 1;
        pageIndex = 0;
        currentPageSize = CreatedBaseSize;
    }

}; // memPool_t



/*
==============================================================================

    list_t 

 - linkedlist class, uses memPool_t.  

   pool might be shared with one or more other list_t
   (unless it is internally allocated).  

==============================================================================
*/
template <typename type>
struct node_t 
{
    type val;
    node_t *next, *prev;
    node_t() : next(0), prev(0) {}
};

template <typename type>
class list_t 
{

private:

    int pool_internal;
    type null_type;

protected:

    memPool_t<node_t<type> > *pool;
    node_t<type> *head;
    node_t<type> *tail;

    unsigned int _size;          // count of how many elts

public:    

    list_t() : pool_internal(1), head(0), tail(0), _size(0) 
    {
        pool = new memPool_t<node_t<type> >;
        memset( &null_type, 0, sizeof(type) );
    }

    list_t( int pool_page_sz ) : pool_internal(1), head(0), tail(0), _size(0) 
    {
        pool = new memPool_t<node_t<type> >( pool_page_sz );
        memset( null_type, 0, sizeof(type) );
    }

    // handed a pool that has been created created seperately
    list_t( memPool_t<node_t<type> >* pp ) : pool_internal(0), head(0), tail(0), _size(0)
    {
        pool = pp;
        memset( null_type, 0, sizeof(type) );
    }

    ~list_t()
    {
        if ( pool_internal && pool )
            delete pool;
    }

    void reset( void ) 
    {
        tail = head = 0; 
        _size       = 0; 
        if ( pool && pool_internal ) 
            pool->reset();
    }


    unsigned int size( void ) const { return _size; }
    node_t<type> *gethead( void ) const { return head; }
    node_t<type> *gettail( void ) const { return tail; }

    // adds onto the end (synonym for push)
    void add( type v ) 
    {
        node_t<type> *n = pool->getone();
        n->val = v;
        n->next = 0;

        if ( head ) {
            n->prev = tail;
            tail->next = n;
            tail = n;
        } else {
            head = tail = n;
            n->prev = 0;
        }
        ++_size;
    }
    
    // adds onto the end (synonym for add)
    void push( type v ) {
        add(v);
    }

    // pop specific node by passing node_t pointer
    type popnode( node_t<type> *node ) 
    {
        if ( !head || !tail || !node )
            return null_type;

        // first node in ll
        if ( node == head  ) {
            // ..and the only one in list
            if ( node == tail ) {
                head = tail = 0;
            } 
            // ..or else more than one
            else { 
                head = node->next;
                if ( head ) {
                    head->prev = 0;
                }
            }
        // last in ll and more than one in list
        } else if ( node == tail ) {
            tail->prev->next = 0;
            tail = tail->prev;
        // in middle, with at least one or more on either side
        } else {
            node->next->prev = node->prev;
            node->prev->next = node->next;
        }

        type v = node->val;
        // recycle it
        pool->returnone( node );
        --_size;
        return v;
    }

    // pop first node with matching value
    // returns success or failure of finding match
    bool popval( type value )
    {
        if ( !head || !tail ) 
            return false;

        node_t<type> *save = 0;

        // first node in ll
        if ( value == head->val ) {
            save = head;
            // ..and the only one in list
            if ( head == tail ) {
                head = tail = 0;
            } 
            // ..or else there is more than one in the list
            else { 
                head = head->next;
                if ( head ) {
                    head->prev = 0;
                }
            }
        // last in ll and more than one in list
        } else if ( value == tail->val ) {
            save = tail;
            tail->prev->next = 0;
            tail = tail->prev;

        // in middle, with at least one or more on either side
        } else {
            save = head->next;
            while ( save && save->val != value ) {
                save = save->next;
            }
            if ( !save )
                return false;

            save->next->prev = save->prev;
            save->prev->next = save->next;
        }

        // recycle it
        pool->returnone( save );
        --_size;
        return true;
    }

    // pop specific node by passing index (counted from head) // O(n/2)
    type popindex( unsigned int index ) {
        if ( !head || !tail ) {
            return null_type;
        }
        if ( index > _size-1 ) {
            return null_type;
        }

        // first node in ll
        if ( 0 == index  ) {
            return popnode( head );
        }

        if ( _size-1 == index ) {
            return popnode( tail );
        }

        node_t<type> *node;

        unsigned int i;
        if ( index < _size/2 ) {
            node = head;
            for ( i = 0; node && i < index; i++ ) {
                node = node->next;
            }
        } else {
            node = tail;
            for ( i = _size-1; node && i > index; i-- ) {
                node = node->prev;
            }
        }

        Assert( index == i );
        Assert( node );

        return popnode( node );
    }


    // pop off the end, return val
    type pop( void )
    {
        if ( !head || !tail ) {
            return null_type;
        }

        node_t<type> *p = 0;
        // more than one, do this..
        if ( tail->prev ) {
            p = tail->prev;
            p->next = 0;
        }
        type v = tail->val;
        pool->returnone( tail );
        tail = p;
        if ( !p )
            head = 0;
        
        --_size;
        return v;
    }

    // shift off the beginning, return the first val 
    type shift( void ) 
    {
        if ( !head || !tail ) {
            return null_type;
        }

        node_t<type> *p = 0;
        // more than one, do this..
        if ( head->next ) {
            p = head->next;
            p->prev = 0;
        }
        type v = head->val;
        pool->returnone( head );
        head = p;
        if ( !p )
            tail = 0;
        
        --_size;
        return v;
    }

    // adds onto the beginning.  
    void unshift( type v ) {
        node_t<type> *n = pool->getone();
        n->val = v;
        n->prev = 0;

        if ( head ) {
            n->next = head;
            head->prev = n;
            head = n;
        } else {
            head = tail = n;
            n->next = 0;
        }
        ++_size;
    }
    
    // peek at the last one put in
    type peek_last( void ) {
        if ( ! tail )
            return null_type;
        return tail->val;
    }

    // peek at the first one put in
    type peek_first( void ) {
        if ( ! head )
            return null_type;
        return head->val;
    }
    
    // O(n/2), use w/ care
    type peek ( unsigned int index ) {
        if ( index > _size-1 || !head )
            return null_type;
        node_t<type> *p;
        unsigned int i;
        if ( index < _size/2 ) {
            p = head; i = 0;
            while ( i < index && p ) 
                p = p->next, ++i;
        } else {
            p = tail; i = _size;
            while ( --i > index && p ) 
                p = p->prev;
        }
        if ( i == index && p ) 
            return p->val;
        return null_type;
    }

    node_t<type> *peek_node( unsigned int index ) {
        if ( index > _size-1 || !head )
            return 0;
        node_t<type> *p;
        unsigned int i;
        if ( index < _size/2 ) {
            p = head; i = 0;
            while ( i < index && p ) 
                p = p->next, ++i;
        } else {
            p = tail; i = _size;
            while ( --i > index && p ) 
                p = p->prev;
        }
        if ( i == index && p ) 
            return p;
        return 0;
    }

}; // list_t




/*
==============================================================================

    vec_t

    a simple array, that resizes dynamically by calling reset()

==============================================================================
*/
template<typename type>
struct vec_t 
{
protected:
    unsigned int _size;          // count of how many elts

public:

    type *vec;

    vec_t() : _size(256)
    {
        vec = new type[ _size ];
    }

    vec_t( unsigned int sz ) 
    {
        _size = sz;
        vec = new type[ _size ];
    }

    ~vec_t()
    {
        if ( vec )
            delete[] vec;
    }

    // resize dynamically by calling reset(), or keep mem we have already if enough
    void reset( unsigned int sz ) 
    {
        if ( sz > _size ) {
            _size = sz;
            if ( vec ) {
                delete[] vec;
            }
            vec = new type[ _size ];
        }
    }

    unsigned int size() { return _size; }

    void zero_out( void );

};  // vec_t

template <typename type>
void vec_t<type>::zero_out( void ) {
    unsigned int s = sizeof(type) * _size;
    if ( s > 0 ) { 
        memset( vec, 0, s );
    }
}




/*
==============================================================================

    buffer_t
        a buffer class that is an array and has the ability to grow dynamically
    when using the inline mechanism add() or push(), which inserts elts onto
    the end of the data.  if the maximum size is reached it is automatically
    increased

    * uses malloc instead of new & delete to avoid ctor dtor overhead.

    * intended for low-level storage of integral data.  not intended for use 
      with OO code relying on standard ctor(), dtor() calls

==============================================================================
*/
template <typename type>
struct buffer_t 
{
protected:
    type *                  free_p;                 // pointer to next free elt
    unsigned int            bytes;                  // byte amount held
    int                     delete_internal;        // state var, if set will call delete on array elts in dtor,
                                                    //  experimental. unused

public:

    type *                  data;       // master data pointer

    buffer_t() : delete_internal(0)
    {
        bytes = BUFFER_DEFAULT_SIZE * sizeof(type);
        data = (type *) malloc ( bytes );
        free_p = data;
    }
    
    buffer_t ( unsigned int sz ) : delete_internal(0)
    {
        bytes = sz * sizeof(type);
        data = (type *) malloc ( bytes );
        free_p = data;
    }

    buffer_t( buffer_t<type> const& obj ) : delete_internal(0)
    {
        bytes = obj.size_bytes();

        // next pow-2 above
        bytes = ( bytes + 7 ) & ~7;
        int s = bytes, c = 0;
        do {
            s >>= 1;
            ++c;
        }
        while ( s > 1 );
        bytes = 1 << c;

        data = (type *) malloc( bytes );
        free_p = data + obj.length();
        memset( free_p, 0, bytes - obj.size_bytes() ); 
        memcpy( data, obj.data, obj.size_bytes() );
    }

    ~buffer_t( void ) 
    {
/*
        if ( delete_internal )
            delete_contents();
*/
        if ( data )
            free( data );
    }


    void set_delete_internal( int set = 1 )
    {
        delete_internal = set;
    }

    // start adding at the beginning again
    void reset( void ) 
    {
        free_p = data;
    }

    void add( type t ) 
    {
        if ( (length()+1)*sizeof(type) > bytes ) {
            unsigned int newsz = bytes << 1;
            type *tmp = (type *) malloc( newsz );
            memcpy( tmp, data, bytes );
            bytes = newsz;
            int free_p_ofst = free_p - data;
            free( data );
            data = tmp;
            free_p = data + free_p_ofst;
        }

        *free_p++ = t;
    }

    void push( type t ) { add ( t ); }

    // copy in an array
    void copy_in( const type *a, unsigned int count ) 
    {
        unsigned int needed = ( length() + count ) * sizeof(type);

        if ( needed > bytes ) {
            unsigned int newsz = bytes << 1;
            while ( newsz < needed ) { 
                newsz <<= 1; 
            } 
            type *tmp = (type*) malloc ( newsz );
            memcpy( tmp, data, bytes );
            bytes = newsz;
            int free_p_ofst = free_p - data;
            free( data );
            data = tmp;
            free_p = data + free_p_ofst;
        }
        memcpy( free_p, a, count * sizeof(type) );
/* think I found why ^this wasn't working...
        for ( int i = 0; i < count ; i++ ) {
            free_p[ i ] = a[ i ];
        }
*/
        free_p += count;
    }

    void zero_out( void ) {
        if ( data )
            memset( data, 0, bytes );
        free_p = data;
    }

    // # of elts in use
    unsigned int length() const { return (unsigned int)(free_p-data); }

    // # of bytes alloc'd internally
    unsigned int size_bytes() const { return bytes; }


    type operator[]( unsigned int index ) {
        return data[ index ];
    }

    type getLast()
    {
        return data[ length() - 1 ];
    }

    void removeIndex( unsigned int index ) {
        if ( !data || index >= length() || length() == 0 )
            return;
        type * src = data + index + 1;
        type * dest = data + index;
        unsigned int mv_bytes = data + size_bytes() - src; 
        memmove( dest, src, mv_bytes );
        --free_p;
    }

#if 0
    // call delete on each element of the contents
    void delete_contents( void ) 
    {
        if ( !data )
            return;

        const unsigned int len = length();
        for ( unsigned int i = 0; i < len; i++ ) {
            if ( data[i] )
                delete data[i];
        }
    }

    void delete_reset() {
        delete_contents();
        reset();
    }
#endif

};

/*
    existential argument. using malloc() & free(), and dealing in bytes rather than
    objects essentially nullifies the OO ideology. buffer_t while effective and fast,
    is living on the edge of sane behavior, because it has abolished the OO notions.
    For instance, if you have a buffer full of objects whose ctor and dtor need to be
    called, neither the ctor nor the dtor will be called. although you can call 
    delete contents which will call the dtor for each object.  But if you were to 
    use the copy-constructor to create a buffer, or to use copy_in(), you would then
    have 2 or more buffer with objects whose dtor need to be called. This is a problem!

    In practice you will use a buffer_t for a dedicated task, such as for storing a
    massive array of vertex information, as such because these data structs are 
    primitives, and arguably don't need a ctor, not to mention their ctor (if there was
    one) will have been called already before they are copied in via add(). 
    Nor will they require a dtor call because they're composed of primitive data.

    Use with care for anything more than basic types. 
*/

#endif // ! __DATATYPES_H__
