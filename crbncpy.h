
#ifndef crbncpyHeader
#define crbncpyHeader

//
// crbncpy simply allocates space and copies a string into it
//   you'll need to delete the returned pointer when done with it
//
// C++ version
//

#include <string.h>

inline char *crbncpy( const char *S )
{
	if ( !S ) return NULL;
	char *R = new char[strlen(S) + 1];
	strcpy( R, S );
	return R;
}

inline char *crbncpy( const char *S1, const char *S2 )
{
	if ( !S1 ) return crbncpy( S2 );
	if ( !S2 ) return crbncpy( S1 );
	size_t L1 = strlen(S1);
	char *R = new char[L1 + strlen(S2) + 1];
	strcpy( R, S1 );
	strcpy( R + L1, S2 );
	return R;
}

#endif

