#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

#include <xmmintrin.h>
#include <math/Vector.h>

namespace Thor 
{
	typedef D3DXMATRIXA16 Matrix;

	__declspec(align(16)) struct Mtx44
	{
		float xx, xy, xz, xw;
		float yx, yy, yz, yw;
		float zx, zy, zz, zw;
		float px, py, pz, pw;
	};

	inline void MtxIdentity(Mtx44& m) {
		m.xx	= 1.0f;	m.xy	= 0.0f;	m.xz	= 0.0f;	m.xw	= 0.0f;
		m.yx	= 0.0f;	m.yy	= 1.0f;	m.yz	= 0.0f;	m.yw	= 0.0f;
		m.zx	= 0.0f;	m.zy	= 0.0f;	m.zz	= 1.0f;	m.zw	= 0.0f;
		m.px	= 0.0f;	m.py	= 0.0f;	m.pz	= 0.0f;	m.pw	= 1.0f;
	}

	inline Vec4 MtxMul(const Mtx44& m, const Vec4& v)
	{
	}

	inline void MtxMul(Mtx44& out, const Mtx44& a, const Mtx44& b)
	{
		// rows times columns
	}

	void MtxSetRotationX(Mtx44& m, float a);
	void MtxSetRotationY(Mtx44& m, float a);
	void MtxSetRotationZ(Mtx44& m, float a);
}

#endif

