#include "SimpleFX.h"

#include <core/Core.h>

extern IDirect3DTexture9 *LoadTexture(char *fileName);
extern int gRTRes[2];
extern IDirect3DTexture9 *pGBufferTexture;

namespace Thor
{

	IDirect3DTexture9* mParticleTex;

	SimpleFX::SimpleFX(IDirect3DDevice9 * d3ddev)
	{
		HRESULT ret;
		int numParticles = 100;
		mNumParticles = numParticles;

		// create particle data arrays and zero them
		mParticlePos	= (Vec4*)_mm_malloc(sizeof(Vec4) * numParticles, 16);
		mParticleVel	= (Vec4*)_mm_malloc(sizeof(Vec4) * numParticles, 16);
		mParticleAge	= (F32*)_mm_malloc(sizeof(F32) * numParticles, 16);
		mParticleLife	= (F32*)_mm_malloc(sizeof(F32) * numParticles, 16);
		mSpawn			= new UInt32[numParticles];
		memset( mParticlePos, 0, sizeof(Vec4) * numParticles );
		memset( mParticleVel, 0, sizeof(Vec4) * numParticles );
		memset( mParticleAge, 0, sizeof(float) * numParticles );
		memset( mParticleLife, 0, sizeof(float) * numParticles );
		for(int i=0; i<numParticles; ++i)
			mSpawn[i] = 1;


		// vertex declaration
		D3DVERTEXELEMENT9 decl[] = {
			{0, 0,  D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0},
			D3DDECL_END() };

		ret = d3ddev->CreateVertexDeclaration(decl, &mVertexDecl);
		mVertexSize = sizeof(Vec4) + sizeof(DWORD);

		// create vertex buffer
		ret = d3ddev->CreateVertexBuffer(numParticles * mVertexSize, D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, 0, 
										D3DPOOL_DEFAULT, &mVertexBuffer, NULL);

		// load FX
		LPD3DXBUFFER errors = NULL;
		HRESULT result = D3DXCreateEffectFromFileW( d3ddev, L"fx/particle.fx", NULL, NULL,
													0, NULL, &mFX, &errors );
		if( FAILED( result ) )
		{
			char* szErrors = (char*)errors->GetBufferPointer();
			errors->Release();
		}

		// load texture
		mParticleTex = LoadTexture("test.png");

	}

	// a float between 0 and 1
	static F32 RandNormalFloat()
	{
		return (F32)rand() / RAND_MAX;
	}

	// unit length vector (in xyz)
	static Vec4 RandUnitVec4()
	{
		return Vec4( (RandNormalFloat() * 2.0f) - 1.0f,
			(RandNormalFloat() * 2.0f) - 1.0f,
			(RandNormalFloat() * 2.0f) - 1.0f,
			1.0f);
	}


	void SimpleFX::Update(float ft)
	{
		const int numParticles = mNumParticles;
		int i=0;

		ft /= 10.0f;


		// move to SIMD register
		Thor::Vec4 deltaT(ft, ft, ft, 1.0f);

		const Thor::Vec4 gravVel = Thor::Vec4(0.0f, -10.0f, 0.0f, 0.0f) * deltaT;

		// update velocity and age
		for(i=0; i<numParticles; ++i)
		{
			mParticleVel[i] += gravVel;
			mParticleAge[i] += ft;
		}

		// lock buffer
		HRESULT res;
		void* buffer;
		res = mVertexBuffer->Lock( 0, 0, &buffer, D3DLOCK_DISCARD );
		thorAssertNoMessage( SUCCEEDED(res) );

		char* cp = (char*)buffer;
		const int vertSize = mVertexSize;

		// update position
		for(i=0; i<numParticles; ++i)
		{
			UInt8 alpha = mParticleAge[i] > mParticleLife[i] ? 0 : (UInt8)(255-(mParticleAge[i]*255) / mParticleLife[i]);
			//unsigned char alpha = (char)(255-(mParticleAge[i]*255) / mParticleLife[i]);
			mSpawn[i] = mParticleAge[i] > mParticleLife[i] ? 1 : 0;

			mParticlePos[i] += mParticleVel[i] * deltaT;

			// copy particle data streams into AoS format for shader
			const int offset = i * vertSize;
			memcpy( &cp[ offset ], &mParticlePos[i], sizeof(Vec4) );
			DWORD color = D3DCOLOR_RGBA(alpha, alpha, alpha, alpha);
			*(DWORD*)(&cp[ offset + sizeof(Vec4) ]) = color;

		}

		res = mVertexBuffer->Unlock();

		// spawn all the particles that need spawning!
		for(i=0; i<numParticles; ++i)
		{
			if(mSpawn[i])
			{
				mParticlePos[i] = Vec4(0.0f, 2.0f, 0.0f, 1.0f);
				mParticleVel[i] = Vec4(0.0f, 5.0f, 0.0f, 0.0f) + (RandUnitVec4() * Vec4(1.0f, 0.0f, 1.0f, 0.0f));
				//mParticleVel[i] = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
				mParticleAge[i] = 0.0f;
				mParticleLife[i] = 1.0f + RandNormalFloat();
				//mParticleLife[i] = 10000.0f;
			}
		}

	}


	void SimpleFX::Render(IDirect3DDevice9 * d3ddev)
	{
		HRESULT res;

		// setup vertex stream
		res = d3ddev->SetStreamSource(0, mVertexBuffer, 0, mVertexSize);
		res = d3ddev->SetVertexDeclaration( mVertexDecl );

		res = mFX->SetTechnique( "SoftParticle" );

		float gbufferSize[] = {(float)gRTRes[0], (float)gRTRes[1] };


		D3DXMATRIXA16 matView, matProj, matViewProj;
		d3ddev->GetTransform(D3DTS_VIEW, &matView);
		d3ddev->GetTransform(D3DTS_PROJECTION, &matProj);
		matViewProj = matView * matProj;

		mFX->SetFloatArray( "GBufferSize", &gbufferSize[0], 2 );
		mFX->SetMatrix( "ViewProj", &matViewProj );
		mFX->SetMatrix( "View", &matView );
		mFX->SetTexture( "DiffuseMap", mParticleTex );
		mFX->SetTexture( "GBufferTexture", pGBufferTexture );

		float vpDims[] = {800.0f, 600.0f};
		mFX->SetFloatArray( "ViewportDimensions", &vpDims[0], 2 );

		d3ddev->BeginScene();

		UINT cPasses=1, iPass=0;
		res = mFX->Begin( &cPasses, 0 );

		for( iPass = 0; iPass < cPasses; ++iPass )
		{
			mFX->BeginPass( iPass );

			res = d3ddev->DrawPrimitive(D3DPT_POINTLIST, 0, mNumParticles);

			mFX->EndPass();
		}

		res = mFX->End();

		d3ddev->EndScene();

	}

};
