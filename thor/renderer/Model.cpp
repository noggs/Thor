#include "Model.h"

#include <iostream>
#include <fstream>

#include <math/Vector.h>

using namespace Thor;


struct D3DVERTEX
{
   Vec4 vec;
   DWORD color;
};

namespace bb 
{
	typedef enum VertexComponentBit
	{
		BIT_UV1			= 0x1,
		BIT_NRM			= 0x2,
	};
}


#define LENGTH_OF(a) ( sizeof( a ) / sizeof( a[0] ) )

D3DVERTEX gVerticesTriangle[] = {
	{Vec4(0.0f, 0.0f, 0.0f, 1.0f), 0xff00ff00},
	{Vec4(1.0f, 0.0f, 0.0f, 1.0f), 0xff0000ff},
	{Vec4(1.0f, 1.0f, 0.0f, 1.0f), 0xffff0000}
};

D3DVERTEX gVerticesCube[] = {
	{Vec4(-0.5f, -0.5f, -0.5f, 1.0f),	0xffffffff},
	{Vec4( 0.5f, -0.5f, -0.5f, 1.0f),	0xffffffff},
	{Vec4(-0.5f,  0.5f, -0.5f, 1.0f),	0xffffffff},
	{Vec4( 0.5f,  0.5f, -0.5f, 1.0f),	0xffffffff},
	{Vec4(-0.5f, -0.5f,  0.5f, 1.0f),	0xffffffff},
	{Vec4( 0.5f, -0.5f,  0.5f, 1.0f),	0xffffffff},
	{Vec4(-0.5f,  0.5f,  0.5f, 1.0f),	0xffffffff},
	{Vec4( 0.5f,  0.5f,  0.5f, 1.0f),	0xffffffff}
};


D3DMATERIAL9 ObjectMaterial; //material object (NEW)


extern LPDIRECT3DDEVICE9 d3ddev;    // the pointer to the device class


extern Matrix gWorldMatrices[];
extern int gNumWorldMatrices;
extern Matrix gLocalMatrices[];
extern int gNumLocalMatrices;


void Model::CreateTriangle()
{
	mName.assign("Triangle");
	VertexBuffer* pVertexBuffer;
	d3ddev->CreateVertexBuffer(3*sizeof(D3DVERTEX), 0, D3DFVF_XYZ|D3DFVF_DIFFUSE, 
		D3DPOOL_DEFAULT, &pVertexBuffer, NULL);

	void* buffer;
	pVertexBuffer->Lock(0, 3*sizeof(D3DVERTEX), &buffer, 0);
	memcpy(buffer, gVerticesTriangle, 3*sizeof(D3DVERTEX));
	pVertexBuffer->Unlock();

	mGeometry = new Geometry();
	mGeometry->mVertexSize = sizeof(D3DVERTEX);
	mGeometry->mVertexFormat = D3DFVF_XYZ|D3DFVF_DIFFUSE;
	mGeometry->mVertexBuffer = pVertexBuffer;
	mGeometry->mNumVertices = 3;
	mGeometry->mIndexBuffer = NULL;
	mGeometry->mNumIndices = 0;

	mWorldTransformID = gNumWorldMatrices++;
	D3DXMatrixIdentity( &gWorldMatrices[mWorldTransformID] );

	mLocalTransformID = gNumLocalMatrices++;
	D3DXMatrixIdentity( &gLocalMatrices[mLocalTransformID] );
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
			dxFormat |= D3DFVF_TEX0;
			vertSize += sizeof(float) * 2;
		}
		if( fmt & bb::BIT_NRM  ) {
			dxFormat |= D3DFVF_NORMAL;
			vertSize += sizeof(float) * 3;
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

		mGeometry = new Geometry();
		mGeometry->mVertexSize = vertSize;
		mGeometry->mVertexFormat = dxFormat;
		mGeometry->mVertexBuffer = pVertexBuffer;
		mGeometry->mNumVertices = numVerts;
		mGeometry->mIndexBuffer = pIndexBuffer;
		mGeometry->mNumIndices = numFaces;

		mWorldTransformID = gNumWorldMatrices++;
		D3DXMatrixIdentity( &gWorldMatrices[mWorldTransformID] );

		mLocalTransformID = gNumLocalMatrices++;
		D3DXMatrixIdentity( &gLocalMatrices[mLocalTransformID] );
	}

	in.close();

	ZeroMemory(&ObjectMaterial,sizeof(ObjectMaterial));
	ObjectMaterial.Diffuse.r = 1.0f;
	ObjectMaterial.Diffuse.g = 1.0f;
	ObjectMaterial.Diffuse.b = 1.0f;
}


void Model::Render()
{
	HRESULT ret;

	// set transform matrix
	d3ddev->SetTransform(D3DTS_WORLD, &gWorldMatrices[mWorldTransformID]);

	// render the model
	d3ddev->SetStreamSource(0, mGeometry->mVertexBuffer, 0, mGeometry->mVertexSize);
	d3ddev->SetFVF( mGeometry->mVertexFormat );

	if( mGeometry->mIndexBuffer ) {

		d3ddev->SetMaterial(&ObjectMaterial);

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




