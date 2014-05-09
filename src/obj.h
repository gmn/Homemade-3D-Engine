#ifndef __OBJ_H__
#define __OBJ_H__


struct obj_t
{
    char * fullpath;
    char * name;

    obj_t( const char * );
    ~obj_t();


    struct point_t {
        int v, t, n;
        point_t() : v(-1), t(-1), n(-1) {} 
        int& operator[]( int i ) {
            if ( 0 == i ) {
                return v;
            } else if ( 1 == i ) {
                return t;
            } else if ( 2 == i ) {
                return n;
            }
            return -1;
        }
    };
    struct face_t { 
        buffer<point_t> f;
        face_t() : f(4) { }
        face_t( unsigned int sz ) : f(sz) {}
        void add( const point_t& p ) { 
            f.add( p ); 
        }
        void v( int i ) {
            point_t b; b.v = i; f.add(b);
        }
        void vt( int i, int j ) {
            point_t b; b.v = i; b.t = j; f.add(b);
        }
        void vtn( int i, int j, int k ) {
            point_t b; b.v = i; b.t = j; b.n = k; f.add(b);
        }
        void vn( int i, int k ) {
            point_t b; b.v = i; b.k = n; f.add(b);
        }
    };
    buffer_t<vec3_t>    verts;
    buffer_t<vec2_t>    texcoords;
    buffer_t<vec3_t>    normals;
    buffer_t<face_t>    faces;

private:
    int index;              // file index
    int size;               // file size
    unsigned char * data;   // file bytes
    
    inline void     strip_white_space();
    float           get_float();
    int             get_int();
    int             check_name( const char * );
    void            consume_line( void );

    void add_normal();  
    void add_vertex();
    void add_texcoord();
    void add_face();
    
    inline int TOLOWER( int );

}; // obj_t


#endif // __OBJ_H__
