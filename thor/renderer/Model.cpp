#include "Model.h"

#include <iostream>
#include <fstream>

#include <math/Vector.h>

using namespace Thor;



namespace bb 
{
	typedef enum VertexComponentBit
	{
		BIT_UV1			= 0x1,
		BIT_NRM			= 0x2,
		BIT_TAN			= 0x4,
	};
}


#define LENGTH_OF(a) ( sizeof( a ) / sizeof( a[0] ) )



extern LPDIRECT3DDEVICE9 d3ddev;    // the pointer to the device class


extern Matrix gWorldMatrices[];
extern int gNumWorldMatrices;
extern Matrix gLocalMatrices[];
extern int gNumLocalMatrices;


void CreateVertexDeclaration(int fmt, Thor::VertexDecl** declOut )
{
	D3DVERTEXELEMENT9 decl[] = {
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
		{0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,  0},
		{0, 36, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0},
		{0, 48, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END() };

	d3ddev->CreateVertexDeclaration(decl, declOut);
}




void Model::LoadModel(const char* filename)
{
	HRESULT ret = 0;
	mName.assign(filename);

	// open file
	std::fstream in( filename, std::ios::in | std::ios::binary);

	// read number of meshes
	unsigned int iNumMeshes;
	in.read( reinterpret_cast<char*>( &iNumMeshes ), sizeof(unsigned int) );

	for (unsigned int iMesh=0; iMesh < iNumMeshes; ++iMesh)
	{
		// only use the first mesh!
		if(iMesh>0)
			continue;

		int fmt;
		in.read( reinterpret_cast<char*>( &fmt ), sizeof(int) );

		unsigned int numVerts;
		in.read( reinterpret_cast<char*>( &numVerts ), sizeof(unsigned int) );

		unsigned int dxFormat = D3DFVF_XYZ;

		int vertSize = sizeof(float) * 3;
		if( fmt & bb::BIT_UV1 ) {
			dxFormat |= D3DFVF_TEX1;
			vertSize += sizeof(float) * 2;
		}
		if( fmt & bb::BIT_NRM  ) {
			dxFormat |= D3DFVF_NORMAL;
			vertSize += sizeof(float) * 3;
		}
		if( fmt & bb::BIT_TAN ) {
			// no dx thingy for a TANGENT+BITANGENT
			vertSize += sizeof(float) * 6;
		}


		// create vertex buffer
		VertexBuffer* pVertexBuffer;
		ret = d3ddev->CreateVertexBuffer(
			numVerts * vertSize, D3DUSAGE_WRITEONLY, dxFormat, 
			D3DPOOL_DEFAULT, &pVertexBuffer, NULL);

		void* buffer;
		ret = pVertexBuffer->Lock(0, 0, &buffer, 0);

			// read the entire stream in
			in.read( reinterpret_cast<char*>( buffer ), numVerts * vertSize );

		ret = pVertexBuffer->Unlock();

		// load face information
		unsigned int numFaces;
		in.read( reinterpret_cast<char*>( &numFaces ), sizeof(unsigned int) );

		IndexBuffer* pIndexBuffer;
		ret = d3ddev->CreateIndexBuffer( 
			numFaces * 3 * sizeof(short), 
			D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			D3DPOOL_DEFAULT,
			&pIndexBuffer, 
			NULL );

		ret = pIndexBuffer->Lock( 0, 0, &buffer, 0 );
			// read the entire stream in
			in.read( reinterpret_cast<char*>( buffer ), numFaces * 3 * sizeof(short) );
		ret = pIndexBuffer->Unlock();

		// create vertex declaration

		mGeometry = new Geometry();
		mGeometry->mVertexSize = vertSize;
		mGeometry->mVertexFormat = fmt;
		mGeometry->mVertexFormatDX = dxFormat;
		mGeometry->mVertexBuffer = pVertexBuffer;
		mGeometry->mNumVertices = numVerts;
		mGeometry->mIndexBuffer = pIndexBuffer;
		mGeometry->mNumIndices = numFaces;
		CreateVertexDeclaration( fmt, &mGeometry->mVertexDecl );

		mWorldTransformID = gNumWorldMatrices++;
		D3DXMatrixIdentity( &gWorldMatrices[mWorldTransformID] );

		mLocalTransformID = gNumLocalMatrices++;
		D3DXMatrixIdentity( &gLocalMatrices[mLocalTransformID] );
	}

	in.close();

}


void Model::Render()
{
	HRESULT ret;

	// set transform matrix
	d3ddev->SetTransform(D3DTS_WORLD, &gWorldMatrices[mWorldTransformID]);

	// render the model
	d3ddev->SetStreamSource(0, mGeometry->mVertexBuffer, 0, mGeometry->mVertexSize);
	d3ddev->SetFVF( mGeometry->mVertexFormatDX );

	if( mGeometry->mIndexBuffer ) {

		ret = d3ddev->SetVertexDeclaration( mGeometry->mVertexDecl );

		ret = d3ddev->SetIndices( mGeometry->mIndexBuffer );
		ret = d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
			0, 0, mGeometry->mNumVertices, 
			0, mGeometry->mNumIndices );
	}
	else
		ret = d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, mGeometry->mNumVertices);
}


static float gAngle = 0.0f;

void Model::Update()
{
	gAngle += 0.01f;

	// set local transform
	D3DXMatrixRotationY( &gLocalMatrices[mWorldTransformID], gAngle );

	static float scale = 1.0f;
	Thor::Matrix scaleMat;
	D3DXMatrixScaling( &scaleMat, scale, scale, scale );

	D3DXMatrixMultiply( &gLocalMatrices[ mWorldTransformID ], &gLocalMatrices[ mWorldTransformID ], &scaleMat );
}




