#include "Core.h"

#include <stdio.h>
#include <stdarg.h>

namespace Thor
{

	void error(long line, const char* file, const char* msg)
	{
		breakpoint();
	}

	void assert(bool condition, long line, const char* file, const char* msg, ...)
	{
		if(!condition)
		{
			char buffer[256];
			va_list args;
			va_start (args, msg);
			vsprintf_s (buffer, 256, msg, args);

			puts( buffer );

			breakpoint();
		}
	}

	void assertNoMessage(bool condition, long line, const char* file)
	{
		if(!condition)
		{
			breakpoint();
		}
	}

};
