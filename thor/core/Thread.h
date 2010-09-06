#ifndef THREAD_H_INCLUDED
#define THREAD_H_INCLUDED

namespace Thor
{
	typedef DWORD ThreadID;


	class Thread
	{
	public:
		Thread();
		~Thread();

		//void start( AutoRef<IVoidDelegate> aDelegate, int priority = 2, bool suspended = false, int stackSize = 64*1024 );

		void suspend( void );
		void resume( void );
		bool isAlive( void );
		void join();

		ThreadID getThreadID( void );

		void setName( char* name );
		const char* getName(void);

		static void sleep( DWORD aTime );

	};
}

#endif
