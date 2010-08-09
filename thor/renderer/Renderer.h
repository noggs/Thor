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


	class Camera
	{
	public:

	private:
	};


	class Scene
	{
	public:

		void AddModel(Model* );

		void Update(float t);

	private:
		Model*	mModels[32];
		int		mNumModels;
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
