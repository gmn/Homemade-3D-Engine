
#include "mmfile.h"
#include "hgr.h"
#include "core.h"


mmfile_t::mmfile_t( const char * _filename ) 
{
    filename = copy_string( _filename );

#ifdef _WIN

	LPSTR lpfilename = (LPSTR) filename;
	hFile = CreateFile(lpfilename, GENERIC_READ, FILE_SHARE_READ, NULL,
		       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if ( hFile == INVALID_HANDLE_VALUE ) {
		core.error( "couldn't open file: \"%s\" for reading\n", filename );
	}
	hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hFileMapping == 0) {
		CloseHandle( hFile );
		core.error( "couldn't map file: \"%s\" \n", filename );
	}
	
	lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if (lpFileBase == 0) {
		CloseHandle( hFileMapping );
		CloseHandle( hFile );
		core.error( "couldn't map file: \"%s\"\n", filename );
	}

	// filesize
	size = GetFileSize( hFile, NULL );

	// save 
	data = (unsigned char *) lpFileBase;

#else /* Unix */

	// open
	if ( !(fp = fopen( filename, "rb" )) ) {
		core.error( "couldn't open input: \"%s\" for reading\n", filename );
	}

	// fileno
	this->fileno = ::fileno( fp );

	// filesize
	struct stat st;
	if ( fstat( this->fileno, &st ) == -1 || st.st_size == 0 ) {
		core.error( "couldn't stat input file\n" );
	}
	size = st.st_size;

	// mmap the file
	data = (unsigned char *) mmap( 0, st.st_size, PROT_READ, MAP_SHARED, this->fileno, 0 );

#endif /* windows or Unix mmap methods */

	Assert( data != NULL );
}

mmfile_t::~mmfile_t()
{
#ifdef _WIN
    UnmapViewOfFile(lpFileBase);
    CloseHandle(hFileMapping);
    CloseHandle(hFile);

#else /* Unix|Mac */
    if ( data )
        if ( -1 == munmap( data, size ) )
            core.warn( "failed to unmap file: %s\n", filename );
    if ( fp )
        fclose( fp );
    if ( filename )
        free( filename );
#endif
}

