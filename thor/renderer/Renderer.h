#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <d3d9.h>
#include <math/Matrix.h>


namespace Thor
{

	// for now!
	typedef IDirect3DVertexBuffer9 VertexBuffer;
	typedef IDirect3DIndexBuffer9 IndexBuffer;
	typedef IDirect3DVertexDeclaration9 VertexDecl; 



	class Skeleton
	{
	public:
		static const unsigned int MaxBones = 64;
		static const unsigned int MaxHierachy = 16;

		Matrix	mLocalOffset[ MaxBones ];		// local offset of this bone (constant) should be a pointer to geometry maybe?

		Matrix	mLocalTransform[ MaxBones ];	// local transform of this bone (from animation)
		Matrix	mCombined[ MaxBones ];			// model space transform of this bone (multiplied down from parents)

		//////////////////////////////////////////////////////////////////////////
		// given a hierarchy like this:
		//   pelvis
		//     spine                    legL      legR
		//       neck   shoulderL         calfL     calfR
		//
		// looks like this in flat hierarchy:
		//   pelvis | spine | legL | legR | neck | calfL | calfR
		//

		int		mNumBonesAtEachLevel[ MaxHierachy ];
	};


	class Texture
	{
	public:
	private:
		//std::string			mName;	// always handy for debug
		IDirect3DTexture9*	mTexture;
	};

	class TextureCache
	{
	public:
		Texture*	Load(const char* fileName);
	};


	class Effect
	{
	public:
	private:
		//std::string		mName;	// always handy for debug
		ID3DXEffect*	mEffect;
		Texture*		mTextures[4];	// upto 4 texture stages
		UINT			mNumTextures;
	};


	class RenderTarget
	{
	public:
	private:
		IDirect3DTexture9*	mTexture;
		IDirect3DSurface9*	mSurface;
	};


	class BoundingSphere
	{
	public:
		Vec4	mPosition;
		float	mRadius;
	};


	class Node
	{
	public:
	private:
		int	mLocalMatrixID;
		int	mWorldMatrixID;
	};


	class Camera : public Node
	{
	public:

	private:
	};


	class Colour
	{
	public:
		void GetAsFloatArray(float* out);
	private:
		DWORD mData;
	};


	///
	class Light : public Node
	{
	public:
		Colour	mDiffuse;
		Colour	mSpecular;
	};

	class PointLight : public Light
	{
	public:
		Vec4	mPosition;
		float	mRadius;
	};




	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		bool Init(HWND hWnd);
		void RenderScene();


	private:

		enum RTType { RT_BACKBUFFER, RT_GBUFFER, RT_LIGHTBUFFER, RT_NUM };
		RenderTarget*	mRenderTargets[RT_NUM];

		Effect*			mFX_GBuffer;

	};


	//Renderer*	RendererInstance;	// global variable \o/
}

#endif
