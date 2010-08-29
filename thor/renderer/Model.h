#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <d3d9.h>
#include <d3dx9math.h>
#include <string>
#include <math/Matrix.h>

namespace Thor {

	// for now!
	typedef IDirect3DVertexBuffer9 VertexBuffer;
	typedef IDirect3DIndexBuffer9 IndexBuffer;
	typedef IDirect3DVertexDeclaration9 VertexDecl; 

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
	};



	class Model
	{
	public:
		void LoadModel( const char* filename );

		void Render();
		void Update();

		void SetLocalMatrix(const Matrix& );

	private:
		std::string		mName;	// always handy for debug
		int				mWorldTransformID;		// index into World matrix array
		int				mLocalTransformID;		// index into Local matrix array
		Geometry*		mGeometry;					// pointer to geometry instance
	};

}


#endif
