#ifndef MUTEX_H_INCLUDED
#define MUTEX_H_INCLUDED


// OS include for specifics
#include <windows.h>


namespace Thor
{

	//////////////////////////////////////////////////////////////////////////
	// Automatically calls enter() on creation and leave() on destruction
	template< typename T>
	class ScopedLock
	{
	public:
		ScopedLock(T& obj) : mObj(obj)
		{
			obj.enter();
		}
		~ScopedLock()
		{
			mObj.leave();
		}
	private:
		T& mObj;
	};


	//////////////////////////////////////////////////////////////////////////
	// Mutex wrapper for system calls
	class Mutex
	{
	public:
		Mutex();
		~Mutex();

		void enter();
		void leave();

	private:

		// OS specific information
		CRITICAL_SECTION mMutex;
	};

}

#endif
