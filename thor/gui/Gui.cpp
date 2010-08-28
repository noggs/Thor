#include "gui.h"

#include <d3d9.h>

using namespace Thor;

Gui::Gui(IDirect3DDevice9* d3ddev)
{
	HRESULT ret;

	unsigned int dxFormat = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	int vertSize = sizeof(GuiVertex);

	mNumVerts = 0;
	mNumCommands = 0;
	mCurrentOffset = 0;
	mMaxVerts = 1024;

	// create vertex buffer
	ret = d3ddev->CreateVertexBuffer(mMaxVerts * vertSize, D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, dxFormat, 
									 D3DPOOL_DEFAULT, &mVertexBuffer, NULL);
}


void Gui::DrawTexturedRect(int left, int top, int width, int height, IDirect3DTexture9 *tex)
{
	using namespace Thor;

	HRESULT res;
	void* buffer;
	res = mVertexBuffer->Lock( mCurrentOffset, 6*sizeof(GuiVertex), &buffer, D3DLOCK_NOOVERWRITE );
	if( SUCCEEDED(res) )
	{
		float fLeft = (float)left;
		float fRight = (float)(left + width);
		float fTop = (float)top;
		float fBottom = (float)(top + height);

		GuiVertex tl = { {fLeft, fTop, 0.0f, 1.0f},		0xffffffff,	{0.0f, 0.0f} };
		GuiVertex bl = { {fLeft, fBottom, 0.0f, 1.0f},	0xffffffff,	{0.0f, 1.0f} };
		GuiVertex tr = { {fRight, fTop, 0.0f, 1.0f},	0xffffffff,	{1.0f, 0.0f} };
		GuiVertex br = { {fRight, fBottom, 0.0f, 1.0f},	0xffffffff,	{1.0f, 1.0f} };

		// construct two triangles
		GuiVertex vertices[] = {tl, br, bl,
								tl, tr, br };

		memcpy(buffer, vertices, 6*sizeof(GuiVertex));

		mVertexBuffer->Unlock();

		// record we've added these verts to the buffer
		mCurrentOffset += 6*sizeof(GuiVertex);

		// add command to the queue
		GuiCmd& cmd = mCommands[mNumCommands++];
		cmd.numVerts = 6;
		cmd.tex = tex;
	}
}


void Gui::Render(IDirect3DDevice9 *d3ddev)
{
	HRESULT res;

	if( mNumCommands > 0 )
	{
		// setup vertex stream
		unsigned int dxFormat = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
		res = d3ddev->SetStreamSource(0, mVertexBuffer, 0, sizeof(GuiVertex));
		res = d3ddev->SetFVF( dxFormat );

		int vert = 0;

		// loop the commands
		for(int i=0; i<mNumCommands; ++i)
		{
			const GuiCmd& cmd = mCommands[i];

			res = d3ddev->SetTexture(0, cmd.tex);
			res = d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, vert, cmd.numVerts / 3);

			vert += cmd.numVerts;
		}

		// clear commands
		mNumVerts = 0;
		mNumCommands = 0;
		mCurrentOffset = 0;
	}

}

