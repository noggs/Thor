#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

namespace Thor
{

	class Texture
	{
	public:
	private:
		std::string			mName;	// always handy for debug
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
		std::string		mName;	// always handy for debug
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


	class Scene
	{
	public:

		void AddModel(Model* );

		void Update(float t);

	private:
		Model*	mModels[32];
		int		mNumModels;

		PointLight	mPointLights[32];
		int			mNumPointLights;
	};


	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		bool Init(HWND hWnd);
		void RenderScene();


	private:

		Scene*			mScene;

		enum RTType { RT_BACKBUFFER, RT_GBUFFER, RT_LIGHTBUFFER, RT_NUM };
		RenderTarget*	mRenderTargets[RT_NUM];

		Effect*			mFX_GBuffer;

	};


	Renderer*	RendererInstance;	// global variable \o/
}

#endif
