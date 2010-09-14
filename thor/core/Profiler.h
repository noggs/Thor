#ifndef PROFILER_H_INCLUDED
#define PROFILER_H_INCLUDED

#include "core/Types.h"
#include <vector>

namespace Thor
{

	class Profiler
	{
	public:
		Profiler();

		void BeginFrame();
		void EndFrame();
		void ShowStats(/*IDirect3DDevice* d3ddev*/);

		void PushMarker(const char*);
		void PopMarker();

		static Profiler* Instance();

	private:

		struct Marker
		{
			const char* mName;
			Int64		mBegin;
		};
		std::vector< Marker > mMarkerStack;

		struct FrameEntry
		{
			const char*	mName;
			Int64		mBegin;
			Int64		mTime;
		};
		std::vector< FrameEntry > mEntries;

		bool mIsProfiling;
		Int64		mBegin;
		Int64		mEnd;

	};


	class ProfileScope
	{
	public:
		ProfileScope(const char* marker)
		{
			Profiler::Instance()->PushMarker(marker);
		}
		~ProfileScope()
		{
			Profiler::Instance()->PopMarker();
		}
	};


};

#endif
