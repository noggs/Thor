// include the basic windows header files and the Direct3D header file
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>

#include <renderer/model.h>
#include <math/Vector.h>
#include <math/Matrix.h>
#include <gui/Gui.h>

#include <core/Mutex.h>
#include <core/Thread.h>

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


int gMainRes[] = {800, 600};
int gRTRes[] = {512,512};


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
                          gMainRes[0], gMainRes[1],
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

bool keys[256] = { false };

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



		case WM_LBUTTONDOWN:
			{
				SetCapture(hWnd);
			} break;

		case WM_LBUTTONUP:
			{
				ReleaseCapture();
			} break;

		case WM_MOUSEMOVE:
			{
			} break;



		case WM_KEYDOWN:
			{
				keys[ wParam ] = true;
			} break;

		case WM_KEYUP:
			{
				keys[ wParam ] = false;
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
ID3DXEffect*				pFX_Forward = NULL;

LPDIRECT3DTEXTURE9 pGBufferTexture = NULL, pLightBufferTexture = NULL;
LPDIRECT3DSURFACE9 pGBufferSurface = NULL, pLightBufferSurface = NULL, pBackBuffer = NULL;
LPDIRECT3DSURFACE9 pZBuffer = NULL;

Thor::Gui* gui = NULL;

struct Camera
{
	Thor::Vec4	mPosition;
	float		mRotationX;
	float		mRotationY;

	Thor::Matrix	mMatrix;
};
Camera gCamera;


Thor::Model* gModel = NULL;
Thor::Model* gPlane = NULL;
Thor::Model* gTiny = NULL;



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

	result = d3ddev->CreateDepthStencilSurface( gMainRes[0], gMainRes[1], D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE, &pZBuffer, NULL );

	d3ddev->SetRenderState(D3DRS_AMBIENT,RGB(0,0,0));
	d3ddev->SetRenderState(D3DRS_LIGHTING, false);
	d3ddev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
	d3ddev->SetRenderState(D3DRS_ZENABLE,D3DZB_TRUE);
	d3ddev->SetRenderState(D3DRS_ZWRITEENABLE,D3DZB_TRUE);
	d3ddev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	gui = new Thor::Gui(d3ddev);

	LPD3DXBUFFER errors = NULL;

	LPCSTR vshProf = D3DXGetVertexShaderProfile( d3ddev );
	LPCSTR pshProf = D3DXGetPixelShaderProfile( d3ddev );


	//////
	// Create G Buffer

	result = d3ddev->CreateTexture( 
				gRTRes[0], gRTRes[1], 1,
				D3DUSAGE_RENDERTARGET,
				D3DFMT_A8R8G8B8,
				D3DPOOL_DEFAULT,
				&pGBufferTexture,
				NULL);

	result = pGBufferTexture->GetSurfaceLevel(0, &pGBufferSurface);


	//////
	// Create Light buffer

	result = d3ddev->CreateTexture(
				gRTRes[0], gRTRes[1], 1,
				D3DUSAGE_RENDERTARGET,
				D3DFMT_A8R8G8B8,
				D3DPOOL_DEFAULT,
				&pLightBufferTexture,
				NULL);

	result = pLightBufferTexture->GetSurfaceLevel(0, &pLightBufferSurface);


	result = D3DXCreateEffectFromFile( d3ddev, L"fx/gbuffer.fx", NULL, NULL, 
										0, NULL, &pFX_GBuffer, &errors );

	if(FAILED(result))
	{
		char* szErrors = (char*)errors->GetBufferPointer();
		errors->Release();
	}


	result = D3DXCreateEffectFromFile( d3ddev, L"fx/lbuffer.fx", NULL, NULL,
										0, NULL, &pFX_Lighting, &errors );
	if( FAILED( result ) )
	{
		char* szErrors = (char*)errors->GetBufferPointer();
		errors->Release();
	}


	result = D3DXCreateEffectFromFile( d3ddev, L"fx/model.fx", NULL, NULL,
										0, NULL, &pFX_Model, &errors );
	if( FAILED( result ) )
	{
		char* szErrors = (char*)errors->GetBufferPointer();
		errors->Release();
	}

	result = D3DXCreateEffectFromFile( d3ddev, L"fx/forward.fx", NULL, NULL,
										0, NULL, &pFX_Forward, &errors );
	if( FAILED( result ) )
	{
		char* szErrors = (char*)errors->GetBufferPointer();
		errors->Release();
	}


	gModel = new Thor::Model();
	gModel->LoadModel( "duck.bbg" );

	gPlane = new Thor::Model();
	gPlane->LoadModel( "plane_xz_200.bbg" );

	gTiny = new Thor::Model();
	gTiny->LoadModel( "tiny.bbg" );

	gCamera.mPosition = Thor::Vec4(0.0f, 1.5f, -6.0f );
	gCamera.mRotationX = 0.0f;
	gCamera.mRotationY = 0.0f;//3.142f/2.0f;


}


float gNearClip = 0.1f;
float gFarClip = 100.0f;

void SetupCamera(void)
{
	D3DXMATRIXA16 ProjectionMatrix;
	D3DXMatrixPerspectiveFovLH(&ProjectionMatrix, PI/4, (float)gMainRes[0]/(float)gMainRes[1], gNearClip, gFarClip);
	d3ddev->SetTransform(D3DTS_PROJECTION, &ProjectionMatrix);

	Thor::Matrix ViewMatrix;
  
	// set the view matrix
	D3DXVECTOR3 EyePoint(gCamera.mPosition.GetX(),gCamera.mPosition.GetY(),gCamera.mPosition.GetZ());
	D3DXVECTOR3 LookAt(gCamera.mPosition.GetX(), gCamera.mPosition.GetY(), 0.0f);
	D3DXVECTOR3 UpVector(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&ViewMatrix, &EyePoint, &LookAt, &UpVector);

	d3ddev->SetTransform(D3DTS_VIEW, &gCamera.mMatrix);
	//d3ddev->SetTransform(D3DTS_VIEW, &ViewMatrix);
}


void FlyCam(float frameTime)
{
	static float moveSpeed = 0.8f;
	static float rotSpeed = 0.2f;

	// process input 
	float angleX=0.0f, angleY=0.0f;
	Thor::Vec4 movement(0.0f, 0.0f, 0.0f);
	if(keys['W'])
		movement.SetZ( moveSpeed * frameTime );
	if(keys['S'])
		movement.SetZ( -moveSpeed * frameTime );
	if(keys['A'])
		movement.SetX( -moveSpeed * frameTime );
	if(keys['D'])
		movement.SetX( moveSpeed * frameTime );

	// 37 left 38 up 39 right 40 down
	if(keys[37])
		angleY -= rotSpeed * frameTime;
	if(keys[39])
		angleY += rotSpeed * frameTime;
	if(keys[38])
		angleX -= rotSpeed * frameTime;
	if(keys[40])
		angleX += rotSpeed * frameTime;

	gCamera.mRotationX += angleX;
	gCamera.mRotationY += angleY;


	D3DXVECTOR3 right	(1.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up		(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 forward	(0.0f, 0.0f, 1.0f);

	// create matrix which maps from world space to view space

	// YAW
	D3DXMATRIX matYaw;
	D3DXMatrixRotationY( &matYaw, gCamera.mRotationY );
	D3DXVec3TransformCoord( &forward, &forward, &matYaw );
	D3DXVec3TransformCoord( &right, &right, &matYaw );

	// PITCH
	D3DXMATRIX matPitch;
	D3DXMatrixRotationAxis( &matPitch, &right, gCamera.mRotationX );
	D3DXVec3TransformCoord( &forward, &forward, &matPitch );
	D3DXVec3TransformCoord( &up, &up, &matPitch );

	// create matrix
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);
	mat._11 = right.x;		mat._12 = up.x;		mat._13 = forward.x;
	mat._21 = right.y;		mat._22 = up.y;		mat._23 = forward.y;
	mat._31 = right.z;		mat._32 = up.z;		mat._33 = forward.z;

	// transform movement vector
	D3DXVECTOR3 rightM = right * movement.GetX();
	D3DXVECTOR3 forwardM = forward * movement.GetZ();
	gCamera.mPosition += Thor::Vec4( rightM.x, rightM.y, rightM.z, 0.0f );
	gCamera.mPosition += Thor::Vec4( forwardM.x, forwardM.y, forwardM.z, 0.0f );

	D3DXVECTOR3 position = D3DXVECTOR3(gCamera.mPosition.GetX(), gCamera.mPosition.GetY(), gCamera.mPosition.GetZ());

	// inverse translation
	mat._41 = - D3DXVec3Dot( &position, &right );
	mat._42 = - D3DXVec3Dot( &position, &up );
	mat._43 = - D3DXVec3Dot( &position, &forward );

	gCamera.mMatrix = mat;
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
IDirect3DTexture9* duckNrmTexture = NULL;
IDirect3DTexture9* skinTexture = NULL;

Thor::Vec4 gDirLightPos(0.577f, -0.577f, 0.577f, 0.0f);
float gDirLightColour[] = {0.3f, 0.3f, 0.3f};

struct Light
{
	Thor::Vec4 pos;
	float radius;
	float colour[3];
};

Light gPointLights[] = {
	{ Thor::Vec4(-1.0f, 1.0f, 0.0f, 1.0f), 5.0f, {0.0f, 0.6f, 0.0f} },
	{ Thor::Vec4(0.0f, 3.0f, 0.0f, 1.0f), 5.0f, {1.0f, 1.0f, 1.0f} },
	{ Thor::Vec4(1.0f, 1.0f, 0.0f, 1.0f), 5.0f, {0.6f, 0.0f, 0.0f} },
};
int gPointLightsNum = 3;



//////////////////////////////////////////////////////////////////////////
// Clear lbuffer to white
// Write out exp(-colour)
//
// When reading do -log2(light_buffer)
// Using blend: 
//	SrcBlend = DestColor
//	DestBlend = Zero
//


static float gAngle = 0.0f;


// this is the function used to render a single frame
void render_frame(void)
{
	if(duckTexture==NULL)
	{
		duckTexture = LoadTexture("duckCM.png");
	}
	if(duckNrmTexture==NULL)
	{
		duckNrmTexture = LoadTexture("brickwork_nrm.png");
	}
	if(skinTexture==NULL)
	{
		skinTexture = LoadTexture("Tiny_skin.dds");
	}

	// update scene
	{
		gAngle += 0.01f;

		///
		// DUCK

		// set local transform
		Thor::Matrix mat;
		D3DXMatrixRotationY( &mat, gAngle );

		// offset
		static float radius = 2.0f;
		mat._41 = radius * sin(gAngle);
		mat._43 = radius * -cos(gAngle);

		gModel->SetLocalMatrix( mat );

		///
		// skinned mesh
		static float scale = 0.005f;
		static float skinHeight = 2.0f;
		D3DXMatrixScaling( &mat, scale, scale, scale );
		mat._42 = skinHeight;
		gTiny->SetLocalMatrix( mat );

	}


	//gModel->Update();
	//gPlane->Update();
	//gTiny->Update();

	// update camera



	Thor::Matrix identity;
	//Thor::MtxIdentity( identity );
	D3DXMatrixIdentity( &identity );
	TransformAllMatrices(0, gNumWorldMatrices, identity);

	// set view parameters (World, View and Proj matrices)
	FlyCam(0.066f);
	SetupCamera();

	// get matrices
    D3DXMATRIXA16 matWorld, matView, matProj;
    d3ddev->GetTransform(D3DTS_WORLD, &matWorld);
    d3ddev->GetTransform(D3DTS_VIEW, &matView);
    d3ddev->GetTransform(D3DTS_PROJECTION, &matProj);


	// calculate all WorldView and WorldViewProj matrices now on CPU...
	D3DXMATRIXA16 matArrayWorldView[10];
	D3DXMATRIXA16 matArrayWorldViewProj[10];
	D3DXMATRIXA16 matArrayInvWorldView[10];

	int i;
	for(i=0; i<gNumWorldMatrices; ++i)
	{
		matArrayWorldView[i] = gWorldMatrices[i] * matView;
		matArrayWorldViewProj[i] = gWorldMatrices[i] * matView * matProj;
		D3DXMatrixInverse( &matArrayInvWorldView[i], NULL, &matArrayWorldView[i] );
	}

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
	pFX_GBuffer->SetFloat( "FarClip", gFarClip );
	pFX_GBuffer->SetTexture( "NormalMap", duckNrmTexture );
	pFX_GBuffer->CommitChanges();

	// Apply the technique contained in the effect 
	UINT cPasses, iPass;
	pFX_GBuffer->Begin(&cPasses, 0);

	for (iPass = 0; iPass < cPasses; iPass++)
	{
		pFX_GBuffer->BeginPass(iPass);

		pFX_GBuffer->SetMatrix( "WorldViewProj", &matArrayWorldViewProj[0]);
		pFX_GBuffer->SetMatrix( "WorldView", &matArrayWorldView[0]);
		pFX_GBuffer->CommitChanges();
		gModel->Render();

		pFX_GBuffer->SetMatrix( "WorldViewProj", &matArrayWorldViewProj[1]);
		pFX_GBuffer->SetMatrix( "WorldView", &matArrayWorldView[1]);
		pFX_GBuffer->CommitChanges();
		gPlane->Render();

		pFX_GBuffer->SetMatrix( "WorldViewProj", &matArrayWorldViewProj[2]);
		pFX_GBuffer->SetMatrix( "WorldView", &matArrayWorldView[2]);
		pFX_GBuffer->CommitChanges();
		gTiny->Render();

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
//					D3DCOLOR_RGBA(255, 255, 255, 255), 1.0f, 0);
					D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);

	d3ddev->BeginScene();

	pFX_Lighting->SetMatrix( "InvProj", &matInvProj );
	pFX_Lighting->SetFloat( "FarClip", gFarClip );

	
	float gbufferSize[] = {(float)gRTRes[0], (float)gRTRes[1] };
	//float gbufferSize[] = {256.0f, 256.0f };
	pFX_Lighting->SetFloatArray( "GBufferSize", &gbufferSize[0], 2 );

	
	// transform directional light vector into view space
	{
		D3DXVECTOR4 lightDir( gDirLightPos.GetX(), gDirLightPos.GetY(), gDirLightPos.GetZ(), gDirLightPos.GetW() );
		D3DXVec4Transform( &lightDir, &lightDir, &matView );
		pFX_Lighting->SetFloatArray( "LightDirVS", (FLOAT*)&lightDir, 3 );
		pFX_Forward->SetFloatArray( "LightDirVS", (FLOAT*)&lightDir, 3 );
	}


	//////////////////////////////////////////////////////////////////////////
	// Render directional/ambient first

	pFX_Lighting->SetTechnique("DirectionalLight");

	pFX_Lighting->SetFloatArray( "LightColourDif", &gDirLightColour[0], 3 );
	pFX_Forward->SetFloatArray( "LightColourDifDir", &gDirLightColour[0], 3 );

#if 1
	// Apply the technique contained in the effect 
	pFX_Lighting->Begin(&cPasses, 0);

	for (iPass = 0; iPass < cPasses; iPass++)
	{
		pFX_Lighting->BeginPass(iPass);

		// Render a full screen quad for the directional light pass
		// use Gui system!
		gui->DrawTexturedRect(0, 0, gRTRes[0], gRTRes[1], pGBufferTexture );
		gui->RenderUsingCurrentFX(d3ddev);

		pFX_Lighting->EndPass();
	}
	pFX_Lighting->End();
#endif

	d3ddev->EndScene();

	//////////////////////////////////////////////////////////////////////////
	// Now render all the point lights

	if( gPointLightsNum > 0 )
	{

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
			pFX_Lighting->SetInt( "CurNumLights", gPointLightsNum-1 );
			pFX_Lighting->CommitChanges();

			pFX_Forward->SetFloatArray( "LightPosVS", (FLOAT*)&lightPos[0], 3*i );
			pFX_Forward->SetFloatArray( "LightColourDif", &lightColour[0], 3*i );
			pFX_Forward->SetFloatArray( "LightRadius", &lightRadius[0], i );
			pFX_Forward->SetInt( "CurNumLights", gPointLightsNum-1 );
			pFX_Forward->CommitChanges();
		}


		pFX_Lighting->SetTechnique("PointLight");

		d3ddev->BeginScene();

		// Apply the technique contained in the effect 
		pFX_Lighting->Begin(&cPasses, 0);
		for (iPass = 0; iPass < cPasses; iPass++)
		{
			pFX_Lighting->BeginPass(iPass);

				// Render a full screen quad for the light - room for improvement here :)
				gui->DrawTexturedRect(0, 0, gRTRes[0], gRTRes[1], pGBufferTexture );
				gui->RenderUsingCurrentFX(d3ddev);

			pFX_Lighting->EndPass();
		}
		pFX_Lighting->End();

		d3ddev->EndScene();

	}


	/////////////////////
	// Step 3 - render the scene into back buffer sampling from lighting/G buffer

	// restore the back buffer
	d3ddev->SetRenderTarget(0, pBackBuffer);

	// clear the window to a deep blue
	d3ddev->Clear(	0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
					D3DCOLOR_RGBA(0, 40, 100, 0), 1.0f, 0);

	if( !keys['F'] )
	{

	d3ddev->BeginScene();    // begins the 3D scene

		pFX_Model->SetFloatArray( "GBufferSize", &gbufferSize[0], 2 );
		pFX_Model->SetTexture( "LightBufferTexture", pLightBufferTexture );
		pFX_Model->CommitChanges();

		pFX_Model->Begin(&cPasses, 0);
			pFX_Model->BeginPass(0);

			pFX_Model->SetTexture( "DiffuseMap", duckTexture );
			pFX_Model->SetMatrix( "WorldViewProj", &matArrayWorldViewProj[0] );
			pFX_Model->CommitChanges();
			gModel->Render();

			pFX_Model->SetMatrix( "WorldViewProj", &matArrayWorldViewProj[1] );
			pFX_Model->CommitChanges();
			gPlane->Render();

			pFX_Model->SetTexture( "DiffuseMap", skinTexture );
			pFX_Model->SetMatrix( "WorldViewProj", &matArrayWorldViewProj[2]);
			pFX_Model->CommitChanges();
			gTiny->Render();

			pFX_Model->EndPass();
		pFX_Model->End();

		gui->DrawTexturedRect(0, 0, 256, 256, pGBufferTexture );
		gui->DrawTexturedRect(0, 256, 256, 256, pLightBufferTexture );

		// render 2D overlay
		gui->Render(d3ddev);


	d3ddev->EndScene();


	} else {



	
	pFX_Forward->CommitChanges();

	d3ddev->BeginScene();

	// Apply the technique contained in the effect 
	pFX_Forward->Begin(&cPasses, 0);
	for (iPass = 0; iPass < cPasses; iPass++)
	{
		pFX_Forward->BeginPass(iPass);

			pFX_Forward->SetTexture( "NormalMap", duckNrmTexture );
			pFX_Forward->SetTexture( "DiffuseMap", duckTexture );
			pFX_Forward->SetMatrix( "WorldViewProj", &matArrayWorldViewProj[0]);
			pFX_Forward->SetMatrix( "WorldView", &matArrayWorldView[0]);
			pFX_Forward->SetMatrix( "InvWorldView", &matArrayInvWorldView[0] );
			pFX_Forward->CommitChanges();
			gModel->Render();

			pFX_Forward->SetMatrix( "WorldViewProj", &matArrayWorldViewProj[1]);
			pFX_Forward->SetMatrix( "WorldView", &matArrayWorldView[1]);
			pFX_Forward->SetMatrix( "InvWorldView", &matArrayInvWorldView[1] );
			pFX_Forward->CommitChanges();
			gPlane->Render();

			pFX_Forward->SetTexture( "DiffuseMap", skinTexture );
			pFX_Forward->SetMatrix( "WorldViewProj", &matArrayWorldViewProj[2]);
			pFX_Forward->SetMatrix( "WorldView", &matArrayWorldView[2]);
			pFX_Forward->SetMatrix( "InvWorldView", &matArrayInvWorldView[2] );
			pFX_Forward->CommitChanges();
			gTiny->Render();


		pFX_Forward->EndPass();

		gui->DrawTexturedRect(0, 256, 256, 256, pLightBufferTexture );

		// render 2D overlay
		gui->Render(d3ddev);
	};

	d3ddev->EndScene();

	}



	d3ddev->Present(NULL, NULL, NULL, NULL);    // displays the created frame

}


// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
    d3ddev->Release();    // close and release the 3D device
    d3d->Release();    // close and release Direct3D
}












