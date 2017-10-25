
#include "mutex.h"

void
lock( mutex& x1, mutex& x2 )
{
	if ( &x1 < &x2 )	// defined order
	{
		x1.lock();
		x2.lock();
	}
	else
	{
		x2.lock();
		x1.lock();
	}
}

