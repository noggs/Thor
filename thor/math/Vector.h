#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include <xmmintrin.h>

namespace Thor {

	class Vec4
	{
	public:
		Vec4() {}
		Vec4(float x, float y, float z)				{ mData = _mm_set_ps(x,y,z,1.0f);	}
		Vec4(float x, float y, float z, float w)	{ mData = _mm_set_ps(x,y,z,w);	}
		Vec4(const Vec4& v)							{ mData = v.mData; }
		Vec4(const __m128& v)						{ mData = v; }

		float GetX() const						{ return ((float*)&mData)[3]; }
		float GetY() const						{ return ((float*)&mData)[2]; }
		float GetZ() const						{ return ((float*)&mData)[1]; }
		float GetW() const						{ return ((float*)&mData)[0]; }

		Vec4 operator* (const Vec4 &v) const	{ return Vec4(_mm_mul_ps(mData, v.mData)); }
		Vec4 operator+ (const Vec4 &v) const	{ return Vec4(_mm_add_ps(mData, v.mData)); }
		Vec4 operator- (const Vec4 &v) const	{ return Vec4(_mm_sub_ps(mData, v.mData)); }
		Vec4 operator/ (const Vec4 &v) const	{ return Vec4(_mm_div_ps(mData, v.mData)); }
		
		void operator*= (const Vec4 &v)			{ mData = _mm_mul_ps(mData, v.mData); }
		void operator+= (const Vec4 &v)			{ mData = _mm_add_ps(mData, v.mData); }
		void operator-= (const Vec4 &v)			{ mData = _mm_sub_ps(mData, v.mData); }
		void operator/= (const Vec4 &v)			{ mData = _mm_div_ps(mData, v.mData); }

	private:

		__m128 mData;
	};

}

#endif

