#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include <math/Vector.h>
#include <renderer/Model.h>		// for vertex buffer


struct IDirect3DDevice9;
struct IDirect3DTexture9;

namespace Thor
{
	typedef IDirect3DVertexDeclaration9 VertexDecl; 

	class Gui
	{
	public:

		Gui(IDirect3DDevice9* d3ddev);

		void DrawTexturedRect(int left, int top, int width, int height, IDirect3DTexture9* tex);

		void Render(IDirect3DDevice9* d3ddev);

	private:

		Thor::VertexDecl*	mVertexDecl;
		Thor::VertexBuffer*	mVertexBuffer;
		int					mMaxVerts;
		int					mNumVerts;
		int					mCurrentOffset;

		/////////////////
		// DrawRect adds vertices to buffer and also a command to the queue
		struct GuiCmd
		{
			int numVerts;
			IDirect3DTexture9* tex;
		};

		GuiCmd	mCommands[128];
		int		mNumCommands;

		struct GuiVertex
		{
			float		pos[4];
			DWORD		colour;
			float		UV[2];
		};

	};

}

#endif
