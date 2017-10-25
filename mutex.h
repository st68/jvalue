
#ifndef mutexHeader
#define mutexHeader

#include <pthread.h>

// wrapper for mutex functions

class mutex
{
		mutex( const mutex& );            // not implemented
		mutex& operator=( const mutex& ); // not implemented
	public:
		mutex()
			{ pthread_mutex_init( &mMutex, NULL ); }
		~mutex()
			{ pthread_mutex_destroy( &mMutex ); }
		void lock()
			{ pthread_mutex_lock( &mMutex ); }
		void unlock()
			{ pthread_mutex_unlock( &mMutex ); }
		bool trylock()	// returns true if got the lock
			{ return pthread_mutex_trylock( &mMutex ) == 0; }
	protected:
		pthread_mutex_t mMutex;
};

extern void lock( mutex& x1, mutex& x2 );
static inline void unlock( mutex& x1, mutex& x2 )
	{ x1.unlock(); x2.unlock(); }

#endif

