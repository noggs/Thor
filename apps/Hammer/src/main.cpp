// include the basic windows header files and the Direct3D header file
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>

#include <renderer/model.h>
#include <math/Vector.h>
#include <math/Matrix.h>
#include <gui/Gui.h>

static const float PI = 3.1415f;

// include the Direct3D Library file
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

// global declarations
LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;    // the pointer to the device class

// function prototypes
void initD3D(HWND hWnd);    // sets up and initializes Direct3D
void render_frame(void);    // renders a single frame
void cleanD3D(void);    // closes Direct3D and releases memory

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Some global variables used to measure the time
float  timeAtGameStart;
UINT64 ticksPerSecond;

float GetGameTime()
{
  UINT64 ticks;
  float time;
  // This is the number of clock ticks since start
  QueryPerformanceCounter((LARGE_INTEGER *)&ticks);

  // Divide by frequency to get the time in seconds
  time = (float)(__int64)ticks/(float)(__int64)ticksPerSecond;
  // Subtract the time at game start to get
  // the time since the game started
  time -= timeAtGameStart;
  return time;
}

// Global variables for measuring fps
float lastUpdate        = 0;
float fpsUpdateInterval = 0.5f;
UINT  numFrames         = 0;
float fps               = 0;

// Called once for every frame
void UpdateFPS()
{
  numFrames++;
  float currentUpdate = GetGameTime();
  if( currentUpdate - lastUpdate > fpsUpdateInterval )
  {
    fps = numFrames / (currentUpdate - lastUpdate);
    lastUpdate = currentUpdate;
    numFrames = 0;
  }
}


// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    hWnd = CreateWindowEx(NULL,
                          L"WindowClass",
                          L"Thor",
                          WS_OVERLAPPEDWINDOW,
                          80, 0,
                          800, 600,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hWnd, nCmdShow);

    // set up and initialize Direct3D
    initD3D(hWnd);

    // enter the main loop:


	if( !QueryPerformanceFrequency((LARGE_INTEGER *)&ticksPerSecond) )
		ticksPerSecond = 1000;
	// If timeAtGameStart is 0 then we get the time since
	// the start of the computer when we call GetGameTime()
	timeAtGameStart = 0;
	timeAtGameStart = GetGameTime();


    MSG msg;

    while(TRUE)
    {
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if(msg.message == WM_QUIT)
            break;

        render_frame();

		UpdateFPS();
    }

    // clean up DirectX and COM
    cleanD3D();

    return msg.wParam;
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } break;
    }

    return DefWindowProc (hWnd, message, wParam, lParam);
}


LPDIRECT3DVERTEXSHADER9      vertexShader = NULL; //VS (NEW)
LPD3DXCONSTANTTABLE          constantTable = NULL; //ConstantTable (NEW)
LPDIRECT3DPIXELSHADER9       pixelShader = NULL; //PS (NEW)
LPDIRECT3DPIXELSHADER9       GBufferShader = NULL; //PS (NEW)

ID3DXEffect*				pFX_GBuffer = NULL;
ID3DXEffect*				pFX_Lighting = NULL;
ID3DXEffect*				pFX_Model = NULL;

LPDIRECT3DTEXTURE9 pGBufferTexture = NULL, pLightBufferTexture = NULL;
LPDIRECT3DSURFACE9 pGBufferSurface = NULL, pLightBufferSurface = NULL, pBackBuffer = NULL;
LPDIRECT3DSURFACE9 pZBuffer = NULL;

Thor::Gui* gui = NULL;

struct Camera
{
	Thor::Vec4	mPosition;
	float		mRotation;
};
Camera gCamera;


Thor::Model* gModel = NULL;
Thor::Model* gSphere = NULL;


Thor::Matrix gWorldMatrices[10];
int gNumWorldMatrices = 0;
Thor::Matrix gLocalMatrices[10];
int gNumLocalMatrices = 0;


// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd)
{
	HRESULT result;
    d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

	D3DCAPS9 caps;
	result = d3d->GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps);

    D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

    ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
    d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
    d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;//D3DFMT_D16;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;

    // create a device class using this information and information from the d3dpp stuct
    result = d3d->CreateDevice(	D3DADAPTER_DEFAULT,
								D3DDEVTYPE_HAL,
								hWnd,
								D3DCREATE_SOFTWARE_VERTEXPROCESSING,
								&d3dpp,
								&d3ddev);

	result = d3ddev->CreateDepthStencilSurface( 800, 600, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE, &pZBuffer, NULL );

	d3ddev->SetRenderState(D3DRS_AMBIENT,RGB(0,0,0));
	d3ddev->SetRenderState(D3DRS_LIGHTING, false);
	d3ddev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
	d3ddev->SetRenderState(D3DRS_ZENABLE,D3DZB_TRUE);
	d3ddev->SetRenderState(D3DRS_ZWRITEENABLE,D3DZB_TRUE);
	d3ddev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	gui = new Thor::Gui(d3ddev);

	LPD3DXBUFFER code = NULL; //Temporary buffers
	LPD3DXBUFFER errors = NULL;

	LPCSTR vshProf = D3DXGetVertexShaderProfile( d3ddev );
	LPCSTR pshProf = D3DXGetPixelShaderProfile( d3ddev );


	//////
	// Create G Buffer

	result = d3ddev->CreateTexture( 
				512, 512, 1,
				D3DUSAGE_RENDERTARGET,
				D3DFMT_A8R8G8B8,
				D3DPOOL_DEFAULT,
				&pGBufferTexture,
				NULL);

	result = pGBufferTexture->GetSurfaceLevel(0, &pGBufferSurface);


	//////
	// Create Light buffer

	result = d3ddev->CreateTexture(
				512, 512, 1,
				D3DUSAGE_RENDERTARGET,
				D3DFMT_A8R8G8B8,
				D3DPOOL_DEFAULT,
				&pLightBufferTexture,
				NULL);

	result = pLightBufferTexture->GetSurfaceLevel(0, &pLightBufferSurface);


	gModel = new Thor::Model();
	//gModel->CreateTriangle();
	gModel->LoadModel( "duck.bbg" );

	gSphere = new Thor::Model();
	gSphere->LoadModel( "sphere.bbg" );


	gCamera.mPosition = Thor::Vec4(0.0f, 100.0f, 300.0f );
	gCamera.mRotation = 0.0f;


	result = D3DXCreateEffectFromFile( d3ddev, L"gbuffer.fx", NULL, NULL, 
										0, NULL, &pFX_GBuffer, &errors );

	if(FAILED(result))
	{
		char* szErrors = (char*)errors->GetBufferPointer();
		errors->Release();
	}


	result = D3DXCreateEffectFromFile( d3ddev, L"lbuffer.fx", NULL, NULL,
										0, NULL, &pFX_Lighting, &errors );
	if( FAILED( result ) )
	{
		char* szErrors = (char*)errors->GetBufferPointer();
		errors->Release();
	}


	result = D3DXCreateEffectFromFile( d3ddev, L"model.fx", NULL, NULL,
										0, NULL, &pFX_Model, &errors );
	if( FAILED( result ) )
	{
		char* szErrors = (char*)errors->GetBufferPointer();
		errors->Release();
	}

}


float gNearClip = 100.0f;
float gFarClip = 900.0f;

void SetupCamera(void)
{
	D3DXMATRIXA16 ProjectionMatrix;
	D3DXMatrixPerspectiveFovLH(&ProjectionMatrix, PI/4, 800.0f/600.0f, gNearClip, gFarClip);
	d3ddev->SetTransform(D3DTS_PROJECTION, &ProjectionMatrix);

	Thor::Matrix ViewMatrix;
  
	// set the view matrix
	D3DXVECTOR3 EyePoint(gCamera.mPosition.GetX(),gCamera.mPosition.GetY(),gCamera.mPosition.GetZ());
	D3DXVECTOR3 LookAt(0.0f, 70.0f, 0.0f);
	D3DXVECTOR3 UpVector(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&ViewMatrix, &EyePoint, &LookAt, &UpVector);

	d3ddev->SetTransform(D3DTS_VIEW, &ViewMatrix);
}


// Sweep through all local matrices, transform them
void TransformAllMatrices(const int start, const int end, const Thor::Matrix& m)
{
	for(int i=start; i<end; ++i)
	{
		D3DXMatrixMultiply( &gWorldMatrices[i], &gLocalMatrices[i], &m );
		//Thor::MtxMul( gWorldMatrices[i], gLocalMatrices[i], m );
	}
}


//Load texture from file with D3DX
//Supported formats: BMP, PPM, DDS, JPG, PNG, TGA, DIB
IDirect3DTexture9 *LoadTexture(char *fileName)
{
  IDirect3DTexture9 *d3dTexture;
  D3DXIMAGE_INFO SrcInfo;      //Optional

  //Use a magenta colourkey
  D3DCOLOR colorkey = 0xFFFF00FF;

  // Load image from file
  if (FAILED(D3DXCreateTextureFromFileExA (d3ddev, fileName, 0, 0, 1, 0, 
        D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, 
        colorkey, &SrcInfo, NULL, &d3dTexture)))
  {
    return NULL;
  }

  //Return the newly made texture
  return d3dTexture;
}

IDirect3DTexture9* duckTexture = NULL;

Thor::Vec4 gDirLightPos(0.7f, 0.0f, -0.7f, 0.0f);
float gDirLightColour[] = {0.5f, 0.5f, 0.5f};

struct Light
{
	Thor::Vec4 pos;
	float radius;
	float colour[3];
};

Light gPointLights[] = {
	{ Thor::Vec4(100.0f, 100.0f, 0.0f, 1.0f), 250.0f, {0.6f, 0.0f, 0.0f} },
	{ Thor::Vec4(-100.0f, 100.0f, 0.0f, 1.0f), 250.0f, {0.0f, 0.6f, 0.0f} },
};
int gPointLightsNum = 2;



//////////////////////////////////////////////////////////////////////////
// Clear lbuffer to white
// Write out exp(-colour)
//
// When reading do -log2(light_buffer)
// Using blend: 
//	SrcBlend = DestColor
//	DestBlend = Zero
//

//////////////////////////////////////////////////////////////////////////
// Shadows:
//	Parallel split?
//	exponential filtering?
//


// this is the function used to render a single frame
void render_frame(void)
{
	if(duckTexture==NULL)
	{
		duckTexture = LoadTexture("duckCM.png");
	}

	// update scene
	gModel->Update();



	Thor::Matrix identity;
	//Thor::MtxIdentity( identity );
	D3DXMatrixIdentity( &identity );
	TransformAllMatrices(0, gNumWorldMatrices, identity);

	// set view parameters (World, View and Proj matrices)
	SetupCamera();

	// calculate WorldViewProj matrix
    D3DXMATRIXA16 matWorld, matView, matProj;
    d3ddev->GetTransform(D3DTS_WORLD, &matWorld);
    d3ddev->GetTransform(D3DTS_VIEW, &matView);
    d3ddev->GetTransform(D3DTS_PROJECTION, &matProj);

    D3DXMATRIXA16 matWorldViewProj = matWorld * matView * matProj;

	// calculate WorldView matrix
	D3DXMATRIXA16 matWorldView = matWorld * matView;

	D3DXMATRIXA16 matInvProj;
	D3DXMatrixInverse( &matInvProj, NULL, &matProj );

	////////////////////
	// Step 1 - render the scene into GBuffer storing normals and depth

	// grab the back buffer
	d3ddev->GetRenderTarget(0, &pBackBuffer);

	// set G buffer rt
	d3ddev->SetRenderTarget(0, pGBufferSurface);
	// clear the rt
	d3ddev->Clear(	0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 
					D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);

	d3ddev->BeginScene();

	// setup shader constants
	pFX_GBuffer->SetMatrix( "WorldViewProj", &matWorldViewProj);
	pFX_GBuffer->SetMatrix( "WorldView", &matWorldView);
	pFX_GBuffer->SetFloat( "FarClip", gFarClip );

	// Apply the technique contained in the effect 
	UINT cPasses, iPass;
	pFX_GBuffer->Begin(&cPasses, 0);

	for (iPass = 0; iPass < cPasses; iPass++)
	{
		pFX_GBuffer->BeginPass(iPass);

		// Render the mesh with the applied technique
		gModel->Render();

		pFX_GBuffer->EndPass();
	}
	pFX_GBuffer->End();

	d3ddev->EndScene();


	/////////////////////
	// Step 2 - render the lighting into the light buffer sampling the GBuffer

	// set lighting buffer rt
	d3ddev->SetRenderTarget(0, pLightBufferSurface);
	// clear the rt
	d3ddev->Clear(	0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
					D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);

	d3ddev->BeginScene();

	pFX_Lighting->SetMatrix( "WorldViewProj", &matWorldViewProj);
	pFX_Lighting->SetMatrix( "WorldView", &matWorldView);
	pFX_Lighting->SetMatrix( "InvProj", &matInvProj );
	pFX_Lighting->SetFloat( "FarClip", gFarClip );

	
	float gbufferSize[] = {512.0f, 512.0f };
	//float gbufferSize[] = {256.0f, 256.0f };
	pFX_Lighting->SetFloatArray( "GBufferSize", &gbufferSize[0], 2 );

	
	// transform directional light vector into view space
	{
		D3DXVECTOR4 lightDir( gDirLightPos.GetX(), gDirLightPos.GetY(), gDirLightPos.GetZ(), gDirLightPos.GetW() );
		D3DXVec4Transform( &lightDir, &lightDir, &matView );
		pFX_Lighting->SetFloatArray( "LightDirVS", (FLOAT*)&lightDir, 3 );
	}


	//////////////////////////////////////////////////////////////////////////
	// Render directional/ambient first

	pFX_Lighting->SetTechnique("DirectionalLight");

	pFX_Lighting->SetFloatArray( "LightColourDif", &gDirLightColour[0], 3 );

#if 0
	// Apply the technique contained in the effect 
	pFX_Lighting->Begin(&cPasses, 0);

	for (iPass = 0; iPass < cPasses; iPass++)
	{
		pFX_Lighting->BeginPass(iPass);

		// Render a full screen quad for the directional light pass
		// use Gui system!
		gui->DrawTexturedRect(0, 0, 512, 512, pGBufferTexture );
		gui->Render(d3ddev);

		pFX_Lighting->EndPass();
	}
	pFX_Lighting->End();
#endif

	d3ddev->EndScene();

	//////////////////////////////////////////////////////////////////////////
	// Now render all the point lights

	// Setup lighting parameters
	{
		const int MaxLights = 8;
		D3DXVECTOR3 lightPos[MaxLights];
		float lightColour[3*MaxLights];
		float lightRadius[MaxLights];

		int i;
		for(i=0; i<gPointLightsNum && i<MaxLights; ++i)
		{
			const Light& light = gPointLights[i];

			// transform pos into view space
			D3DXVECTOR4 tempPos( light.pos.GetX(), light.pos.GetY(), light.pos.GetZ(), 1.0f );
			D3DXVECTOR4 xformPos;
			D3DXVec4Transform( &xformPos, &tempPos, &matView );

			lightPos[i] = D3DXVECTOR3(xformPos.x, xformPos.y, xformPos.z);

			lightColour[0 + (i*3)] = light.colour[0];
			lightColour[1 + (i*3)] = light.colour[1];
			lightColour[2 + (i*3)] = light.colour[2];

			lightRadius[i] = light.radius;
		}

		pFX_Lighting->SetFloatArray( "LightPosVS", (FLOAT*)&lightPos[0], 3*i );
		pFX_Lighting->SetFloatArray( "LightColourDif", &lightColour[0], 3*i );
		pFX_Lighting->SetFloatArray( "LightRadius", &lightRadius[0], i );
		pFX_Lighting->CommitChanges();
	}


	pFX_Lighting->SetTechnique("PointLight");

	d3ddev->BeginScene();

	// Apply the technique contained in the effect 
	pFX_Lighting->Begin(&cPasses, 0);
	for (iPass = 0; iPass < cPasses; iPass++)
	{
		pFX_Lighting->BeginPass(iPass);

		// for each positional light...
		//for(int i=0; i<gPointLightsNum; ++i)
		//{
		//	const Light& light = gPointLights[i];

			// position in ViewSpace
			//D3DXVECTOR4 lightPos( light.pos.GetX(), light.pos.GetY(), light.pos.GetZ(), light.pos.GetW() );
			//D3DXVec4Transform( &lightPos, &lightPos, &matView );
			//pFX_Lighting->SetFloatArray( "LightPosVS", (FLOAT*)&lightPos, 3 );
			//pFX_Lighting->SetFloatArray( "LightColourDif", &light.colour[0], 3 );
			//pFX_Lighting->SetFloatArray( "LightRadius", &light.radius,1 );

			//pFX_Lighting->CommitChanges();

			// Render a full screen quad for the light - room for improvement here :)
			gui->DrawTexturedRect(0, 0, 512, 512, pGBufferTexture );
			gui->Render(d3ddev);
		//}

		pFX_Lighting->EndPass();
	}
	pFX_Lighting->End();

	d3ddev->EndScene();


	/////////////////////
	// Step 3 - render the scene into back buffer sampling from lighting/G buffer

	// restore the back buffer
	d3ddev->SetRenderTarget(0, pBackBuffer);

	// clear the window to a deep blue
	d3ddev->Clear(	0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
					D3DCOLOR_RGBA(0, 40, 100, 0), 1.0f, 0);
	d3ddev->BeginScene();    // begins the 3D scene

		pFX_Model->SetMatrix( "WorldViewProj", &matWorldViewProj);
		pFX_Model->SetFloatArray( "GBufferSize", &gbufferSize[0], 2 );
		
		
		pFX_Model->SetTexture( "LightBufferTexture", pLightBufferTexture );
		//pFX_Model->SetTexture( "LightBufferTexture", pGBufferTexture );

		pFX_Model->SetTexture( "DiffuseMap", duckTexture );

		pFX_Model->Begin(&cPasses, 0);
			pFX_Model->BeginPass(0);

			gModel->Render();

			pFX_Model->EndPass();
		pFX_Model->End();

		//gui->DrawTexturedRect(0, 0, 256, 256, pGBufferTexture );
		gui->DrawTexturedRect(0, 0, 256, 256, pLightBufferTexture );

		// render 2D overlay
		gui->Render(d3ddev);


	d3ddev->EndScene();
	d3ddev->Present(NULL, NULL, NULL, NULL);    // displays the created frame

}


// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
    d3ddev->Release();    // close and release the 3D device
    d3d->Release();    // close and release Direct3D
}
