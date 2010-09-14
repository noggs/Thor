#ifndef SIMPLEFX_H_INCLUDED
#define SIMPLEFX_H_INCLUDED

#include <core/Types.h>
#include <renderer/Renderer.h>

namespace Thor
{

	class SimpleFX
	{
	public:
		SimpleFX( IDirect3DDevice9* );
		~SimpleFX();

		void Update( float ft=0.066f );
		void Render( IDirect3DDevice9* );

	private:

		VertexBuffer*	mVertexBuffer;
		VertexDecl*		mVertexDecl;
		int 			mVertexSize;
		ID3DXEffect*	mFX;

		// particle data arrays
		Thor::Vec4*		mParticlePos;
		Thor::Vec4*		mParticleVel;
		Thor::F32*		mParticleAge;
		Thor::F32*		mParticleLife;
		UInt32*			mSpawn;
		int				mNumParticles;
	};

};

#endif
