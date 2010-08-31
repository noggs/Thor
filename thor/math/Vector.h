#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include <xmmintrin.h>

namespace Thor {

	class Vec4
	{
	public:
		Vec4() {}
		Vec4(float x, float y, float z)				{ mData = _mm_set_ps(1.0f,z,y,x);	}
		Vec4(float x, float y, float z, float w)	{ mData = _mm_set_ps(w,z,y,x);	}
		Vec4(const Vec4& v)							{ mData = v.mData; }
		Vec4(const __m128& v)						{ mData = v; }

		float GetX() const						{ return ((float*)&mData)[0]; }
		float GetY() const						{ return ((float*)&mData)[1]; }
		float GetZ() const						{ return ((float*)&mData)[2]; }
		float GetW() const						{ return ((float*)&mData)[3]; }

		void SetX(float f)						{ ((float*)&mData)[0] = f; }
		void SetY(float f)						{ ((float*)&mData)[1] = f; }
		void SetZ(float f)						{ ((float*)&mData)[2] = f; }
		void SetW(float f)						{ ((float*)&mData)[3] = f; }

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

