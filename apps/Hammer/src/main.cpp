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
                          L"Our First Direct3D Program",
                          WS_OVERLAPPEDWINDOW,
                          80, 0,
                          640, 480,
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


LPDIRECT3DVERTEXDECLARATION9 vertexDecl = NULL; //VertexDeclaration (NEW)
LPDIRECT3DVERTEXSHADER9      vertexShader = NULL; //VS (NEW)
LPD3DXCONSTANTTABLE          constantTable = NULL; //ConstantTable (NEW)
LPDIRECT3DPIXELSHADER9       pixelShader = NULL; //PS (NEW)
LPDIRECT3DPIXELSHADER9       GBufferShader = NULL; //PS (NEW)

ID3DXEffect*				pEffect_GBuffer = NULL;

LPDIRECT3DTEXTURE9 pGBufferTexture = NULL, pLightBufferTexture = NULL;
LPDIRECT3DSURFACE9 pGBufferSurface = NULL, pLightBufferSurface = NULL, pBackBuffer = NULL;

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
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    // create a device class using this information and information from the d3dpp stuct
    d3d->CreateDevice(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      hWnd,
                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                      &d3dpp,
                      &d3ddev);

	d3ddev->SetRenderState(D3DRS_AMBIENT,RGB(0,0,0));
	d3ddev->SetRenderState(D3DRS_LIGHTING, false);
	d3ddev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
	d3ddev->SetRenderState(D3DRS_ZENABLE,D3DZB_TRUE);
	d3ddev->SetRenderState(D3DRS_ZWRITEENABLE,D3DZB_TRUE);
	d3ddev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	gui = new Thor::Gui(d3ddev);

	// create a light
	D3DLIGHT9 Light;
	ZeroMemory(&Light,sizeof(Light));
	Light.Type = D3DLIGHT_DIRECTIONAL;
	Light.Diffuse.r = 1.0f;
	Light.Diffuse.g = 1.0f;
	Light.Diffuse.b = 1.0f;
	Light.Direction.x = 1.0f;
	Light.Direction.y = 0.0f;
	Light.Direction.z = 1.0f;
	Light.Range = 1000.0f;

	d3ddev->SetLight(0,&Light);  //set the light (NEW)
	//d3ddev->LightEnable(0,true); //enables the light (NEW)


	D3DVERTEXELEMENT9 decl[] = {
		{0,0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,0},
		{0,12,D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,0},
		{0,20,D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,0},
		D3DDECL_END()};
	result = d3ddev->CreateVertexDeclaration(decl, &vertexDecl);

	LPD3DXBUFFER code = NULL; //Temporary buffers
	LPD3DXBUFFER errors = NULL;

	LPCSTR vshProf = D3DXGetVertexShaderProfile( d3ddev );
	LPCSTR pshProf = D3DXGetPixelShaderProfile( d3ddev );


	//set up Vertex Shader (NEW)
	result = D3DXCompileShaderFromFile(
				L"vertex.vsh",   //filepath
				NULL,            //macro's
				NULL,            //includes
				"vs_main",       //main function
				"vs_2_0",        //shader profile
				0,               //flags
				&code,           //compiled operations
				&errors,         //errors
				&constantTable); //constants

	if(SUCCEEDED(result))
	{
		d3ddev->CreateVertexShader((DWORD*)code->GetBufferPointer(), &vertexShader);
		code->Release();
	}
	else
	{
		char* szErrors = (char*)errors->GetBufferPointer();
		errors->Release();
	}

	//set up Pixel Shader (NEW)
	result = D3DXCompileShaderFromFile(
				L"pixel.psh",  //filepath
				NULL,          //macro's            
				NULL,          //includes           
				"ps_main",     //main function      
				"ps_2_0",      //shader profile     
				0,             //flags              
				&code,         //compiled operations
				&errors,       //errors
				NULL);         //constants

	if(SUCCEEDED(result))
	{
		d3ddev->CreatePixelShader((DWORD*)code->GetBufferPointer(), &pixelShader);
		code->Release();
	}
	else
	{
		char* szErrors = (char*)errors->GetBufferPointer();
		errors->Release();
	}


	

	//set up Pixel Shader (NEW)
	result = D3DXCompileShaderFromFile(
				L"gbuffer.psh",  //filepath
				NULL,          //macro's            
				NULL,          //includes           
				"ps_main",     //main function      
				"ps_2_0",      //shader profile     
				0,             //flags              
				&code,         //compiled operations
				&errors,       //errors
				NULL);         //constants

	if(SUCCEEDED(result))
	{
		d3ddev->CreatePixelShader((DWORD*)code->GetBufferPointer(), &GBufferShader);
		code->Release();
	}
	else
	{
		char* szErrors = (char*)errors->GetBufferPointer();
		errors->Release();
	}


	//////
	// Create G Buffer

	result = d3ddev->CreateTexture( 
				256, 256, 1,
				D3DUSAGE_RENDERTARGET,
				D3DFMT_A8R8G8B8,
				D3DPOOL_DEFAULT,
				&pGBufferTexture,
				NULL);

	result = pGBufferTexture->GetSurfaceLevel(0, &pGBufferSurface);


	//////
	// Create Light buffer

	result = d3ddev->CreateTexture(
				256, 256, 1,
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
										0, NULL, &pEffect_GBuffer, &errors );

	if(FAILED(result))
	{
		char* szErrors = (char*)errors->GetBufferPointer();
		errors->Release();
	}


}


void SetupCamera(void)
{
	D3DXMATRIXA16 ProjectionMatrix;
	D3DXMatrixPerspectiveFovLH(&ProjectionMatrix, PI/4, 1.0f, 100.0f, 500.0f);
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

IDirect3DTexture9* guiTexture = NULL;


// this is the function used to render a single frame
void render_frame(void)
{
	if(guiTexture==NULL)
	{
		guiTexture = LoadTexture("test.png");
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

	////////////////////
	// Step 1 - render the scene into GBuffer storing normals and depth

	// grab the back buffer
	d3ddev->GetRenderTarget(0, &pBackBuffer);

	// set G buffer rt
	d3ddev->SetRenderTarget(0, pGBufferSurface);
	// clear the rt to red
	d3ddev->Clear(	0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
					D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);


	// Apply the technique contained in the effect 
	UINT cPasses, iPass;
	pEffect_GBuffer->Begin(&cPasses, 0);

	for (iPass = 0; iPass < cPasses; iPass++)
	{
		pEffect_GBuffer->BeginPass(iPass);

		pEffect_GBuffer->SetMatrix( "WorldViewProj", &matWorldViewProj);

		// Only call CommitChanges if any state changes have happened
		// after BeginPass is called
		pEffect_GBuffer->CommitChanges();

		// Render the mesh with the applied technique
		gModel->Render();

		pEffect_GBuffer->EndPass();
	}
	pEffect_GBuffer->End();


#if 0
	d3ddev->BeginScene();    // begins the 3D scene

        constantTable->SetMatrix(d3ddev, "WorldViewProj", &matWorldViewProj);

		// setup vertex shader and pixel shader
		d3ddev->SetVertexDeclaration(vertexDecl);
        d3ddev->SetVertexShader(vertexShader);
        d3ddev->SetPixelShader(GBufferShader);

		// do 3D rendering on the back buffer here
		gModel->Render();

	d3ddev->EndScene();    // ends the 3D scene
#endif


	/////////////////////
	// Step 2 - render the lighting into the light buffer sampling the GBuffer


	/////////////////////
	// Step 3 - render the scene into back buffer sampling from lighting/G buffer

	// restore the back buffer
	d3ddev->SetRenderTarget(0, pBackBuffer);

	// clear the window to a deep blue
	d3ddev->Clear(	0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
					D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);
	d3ddev->BeginScene();    // begins the 3D scene

		//gui->DrawTexturedRect(0, 0, 128, 128, guiTexture );
		gui->DrawTexturedRect(0, 0, 256, 256, pGBufferTexture );
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




