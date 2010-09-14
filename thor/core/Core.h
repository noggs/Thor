#ifndef CORE_H_INCLUDED
#define CORE_H_INCLUDED

#define thorError( message ) Thor::error( __LINE__, __FILE__, message );
#define thorAssert( condition, message ) Thor::assert(condition, __LINE__, __FILE__, message, ... );
#define thorAssertNoMessage( condition ) Thor::assertNoMessage(condition, __LINE__, __FILE__ );

namespace Thor
{
	void error(long line, const char* file, const char* msg);
	void assert(bool condition, long line, const char* file, const char* msg, ...);
	void assertNoMessage(bool condition, long line, const char* file);

	inline void breakpoint()
	{
		__asm { int 3 };
	}
};

#endif
