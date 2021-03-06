#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <d3d9.h>
#include <d3dx9math.h>
#include <string>
#include <math/Matrix.h>
#include <renderer/Renderer.h>

namespace Thor {

	struct Geometry
	{
		VertexBuffer*	mVertexBuffer;
		VertexDecl*		mVertexDecl;
		int 			mVertexSize;
		int 			mVertexFormat;		// internal
		int 			mVertexFormatDX;	// DX9 format
		int 			mNumVertices;

		IndexBuffer*	mIndexBuffer;
		int				mNumIndices;

		int				mNumBones;
		float*			mBoneMatrices;
	};



	class Model
	{
	public:
		void LoadModel( const char* filename );

		void Render();
		void Update();

		void SetLocalMatrix(const Matrix& );

		Geometry* GetGeometry()		{ return mGeometry; }

	private:
		std::string		mName;	// always handy for debug
		int				mWorldTransformID;		// index into World matrix array
		int				mLocalTransformID;		// index into Local matrix array
		Geometry*		mGeometry;					// pointer to geometry instance
		float			mScale;
	};

}


#endif
