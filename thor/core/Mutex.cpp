#include "Mutex.h"

namespace Thor
{

	Mutex::Mutex()
	{
		InitializeCriticalSection( &mMutex );
	}

	Mutex::~Mutex()
	{
		DeleteCriticalSection( &mMutex );
	}

	void Mutex::enter()
	{
		EnterCriticalSection( &mMutex );
	}

	void Mutex::leave()
	{
		LeaveCriticalSection( &mMutex );
	}

}
