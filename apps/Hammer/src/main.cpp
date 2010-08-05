// include the basic windows header files and the Direct3D header file
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>

#include <renderer/model.h>
#include <math/Vector.h>
#include <math/Matrix.h>

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
                          300, 300,
                          640, 480,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hWnd, nCmdShow);

    // set up and initialize Direct3D
    initD3D(hWnd);

    // enter the main loop:

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



struct Camera
{
	Thor::Vec4	mPosition;
	float		mRotation;
};
Camera gCamera;


Thor::Model* gModel = NULL;


Thor::Matrix gWorldMatrices[10];
int gNumWorldMatrices = 0;
Thor::Matrix gLocalMatrices[10];
int gNumLocalMatrices = 0;


// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd)
{
	HRESULT result;
    d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

    D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

    ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
    d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
    d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D

    // create a device class using this information and information from the d3dpp stuct
    d3d->CreateDevice(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      hWnd,
                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                      &d3dpp,
                      &d3ddev);

	d3ddev->SetRenderState(D3DRS_AMBIENT,RGB(0,0,0));
	d3ddev->SetRenderState(D3DRS_LIGHTING, true);
	d3ddev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
	d3ddev->SetRenderState(D3DRS_ZENABLE,D3DZB_TRUE);


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
	d3ddev->LightEnable(0,true); //enables the light (NEW)

	gCamera.mPosition = Thor::Vec4(0.0f, 0.0f, 1000.0f );
	gCamera.mRotation = 0.0f;

	gModel = new Thor::Model();
	//gModel->CreateTriangle();
	gModel->LoadModel( "duck.bbg" );

	D3DVERTEXELEMENT9 decl[] = {
		{0,0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,0},
		{0,12,D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,0},
		{0,20,D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,0},
		D3DDECL_END()};
	result = d3ddev->CreateVertexDeclaration(decl, &vertexDecl);

	LPD3DXBUFFER code = NULL; //Temporary buffer (NEW)
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
				&errors,            //errors
				&constantTable); //constants

	if(SUCCEEDED(result))
	{
		d3ddev->CreateVertexShader((DWORD*)code->GetBufferPointer(), &vertexShader);
		code->Release();
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
				&errors,          //errors
				NULL);         //constants

	if(SUCCEEDED(result))
	{
		d3ddev->CreatePixelShader((DWORD*)code->GetBufferPointer(), &pixelShader);
		code->Release();
	}



}


void SetupCamera(void)
{
	D3DXMATRIXA16 ProjectionMatrix;
	D3DXMatrixPerspectiveFovLH(&ProjectionMatrix, PI/4, 1.0f, 100.0f, 2000.0f);
	d3ddev->SetTransform(D3DTS_PROJECTION, &ProjectionMatrix);

	
	Thor::Matrix ViewMatrix;
  
   // set the view matrix
	D3DXVECTOR3 EyePoint(gCamera.mPosition.GetX(),
                        gCamera.mPosition.GetY(),
                        gCamera.mPosition.GetZ());
   D3DXVECTOR3 LookAt(0.0f, 0.0f, 0.0f);
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


// this is the function used to render a single frame
void render_frame(void)
{
	Thor::Matrix identity;
	//Thor::MtxIdentity( identity );
	D3DXMatrixIdentity( &identity );
	TransformAllMatrices(0, gNumWorldMatrices, identity);

	// clear the window to a deep blue
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);

	d3ddev->BeginScene();    // begins the 3D scene

		SetupCamera();

		//communicate with shader (NEW)
        D3DXMATRIXA16 matWorld, matView, matProj;
        d3ddev->GetTransform(D3DTS_WORLD, &matWorld);
        d3ddev->GetTransform(D3DTS_VIEW, &matView);
        d3ddev->GetTransform(D3DTS_PROJECTION, &matProj);

        D3DXMATRIXA16 matWorldViewProj = matWorld * matView * matProj;
        constantTable->SetMatrix(d3ddev, "WorldViewProj", &matWorldViewProj);

		// setup vertex shader and pixel shader
		d3ddev->SetVertexDeclaration(vertexDecl);
        d3ddev->SetVertexShader(vertexShader);
        d3ddev->SetPixelShader(pixelShader);

		// do 3D rendering on the back buffer here
		gModel->Update();
		gModel->Render();

	d3ddev->EndScene();    // ends the 3D scene

	d3ddev->Present(NULL, NULL, NULL, NULL);    // displays the created frame
}


// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
    d3ddev->Release();    // close and release the 3D device
    d3d->Release();    // close and release Direct3D
}

