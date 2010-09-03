#ifndef THREAD_H_INCLUDED
#define THREAD_H_INCLUDED

namespace Thor
{
	typedef DWORD ThreadID;

	enum ThreadPriority
	{
		LOW			= THREAD_PRIORITY_LOWEST,
		NORMAL		= THREAD_PRIORITY_NORMAL,
		HIGH		= THREAD_PRIORITY_HIGHEST,
		CRITICAL	= THREAD_PRIORITY_TIME_CRITICAL
	};

	class Thread
	{
	public:
		Thread();
		~Thread();

		void start( AutoRef<IVoidDelegate> aDelegate,  ThreadPriority  priority = NORMAL, bool suspended = false, int stackSize = 64*1024 );

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
