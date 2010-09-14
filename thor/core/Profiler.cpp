#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "Profiler.h"

#include "Core.h"


// hack!
extern LPDIRECT3DDEVICE9 d3ddev;


namespace Thor
{
	
	Profiler* Profiler::Instance()
	{
		static Profiler profiler;
		return &profiler;
	}

	Profiler::Profiler()
		: mIsProfiling(false)
	{
		mMarkerStack.reserve( 16 );
		mEntries.reserve( 16 );
	}

	void Profiler::BeginFrame()
	{
		thorAssertNoMessage( !mIsProfiling );
		mIsProfiling = true;
		mMarkerStack.clear();
		mEntries.clear();

		PushMarker( "Total" );
	}

	void Profiler::EndFrame()
	{
		thorAssertNoMessage( mIsProfiling );
		PopMarker();
		mIsProfiling = false;
	}

	void Profiler::PushMarker(const char * marker)
	{
		thorAssertNoMessage( mIsProfiling );
		LARGE_INTEGER t;
		if( QueryPerformanceCounter(&t) )
		{
			mMarkerStack.push_back( Marker() );
			mMarkerStack.back().mName = marker;
			mMarkerStack.back().mBegin = t.QuadPart;
		}
		else
			thorError( "Failed to query performance counter" );
	}

	void Profiler::PopMarker()
	{
		thorAssertNoMessage( mIsProfiling );
		thorAssertNoMessage( !mMarkerStack.empty() );

		LARGE_INTEGER t;
		QueryPerformanceCounter(&t);

		const char* markerName = mMarkerStack.back().mName;
		Int64 begin = mMarkerStack.back().mBegin;
		Int64 elapsed = t.QuadPart - begin;
		mMarkerStack.pop_back();

		for(std::vector< FrameEntry >::iterator it = mEntries.begin(); it != mEntries.end(); ++it)
		{
			if( strcmp(markerName, it->mName) == 0 )
			{
				it->mTime += elapsed;
				return;
			}
		}

		// not found one so add it here
		mEntries.push_back( FrameEntry() );
		mEntries.back().mName = markerName;
		mEntries.back().mTime = elapsed;
		mEntries.back().mBegin = begin;
	}


	// display timings on screen
	void Profiler::ShowStats()
	{
		thorAssertNoMessage( !mIsProfiling );
		HRESULT res;

		// HACK!!
		static ID3DXFont* font = NULL;
		if(font==NULL) {
			res = D3DXCreateFont( d3ddev, 12, 0, FW_DONTCARE, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Verdana", &font );
			thorAssertNoMessage( SUCCEEDED( res ) );
		}

		res = d3ddev->BeginScene();

		static D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 255);
		static Int32 lineHeight = 12;
		static Int32 lineWidth = 500;
		static Int32 startPosX = 20;
		static Int32 startPosY = 20;

		LARGE_INTEGER freq;
		if( QueryPerformanceFrequency(&freq) )
		{

			for( UInt32 y=0; y<mEntries.size(); ++y )
			{
				char msg[256];
				double t = (double)(mEntries[y].mTime * 1000.0) / freq.QuadPart;
				sprintf_s( msg, 256, "%s :  %5.3fms", mEntries[y].mName, t );

				RECT rect;
				rect.top = startPosY + (y * lineHeight);
				rect.bottom = rect.top + lineHeight;
				rect.left = startPosX;
				rect.right = startPosX + lineWidth;

				res = font->DrawTextA( NULL, msg, strlen(msg), &rect, DT_LEFT | DT_NOCLIP, fontColor );
			}

		}

		res = d3ddev->EndScene();

	}



};
