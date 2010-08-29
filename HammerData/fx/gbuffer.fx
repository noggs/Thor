
#include "utility.h"


// Global variables
float4x4 WorldViewProj;		// matrix from ObjectSpace to ClipSpace
float4x4 WorldView;			// matrix from ObjectSpace to EyeSpace

float FarClip;

// normal map
texture NormalMap;
sampler NormalSampler = sampler_state {
	Texture = <NormalMap>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};


// Vertex shader input structure
struct VS_INPUT
{
    float4 Position   : POSITION;
	float3 Normal     : NORMAL;
	float3 Tangent    : TANGENT;
	float3 Bitangent  : BINORMAL;
    float2 Texture    : TEXCOORD0;
};


// Vertex shader output structure
struct VS_OUTPUT
{
    float4 Position   : POSITION;
    float2 Texture    : TEXCOORD0;
    float3 Normal     : TEXCOORD1;
    float2 Depth      : TEXCOORD2;
};



// Name: Simple Vertex Shader
// Type: Vertex shader
// Desc: Vertex transformation and texture coord pass-through
//
VS_OUTPUT vs_main( in VS_INPUT In )
{
    VS_OUTPUT Out;                      //create an output vertex

    Out.Position = mul(In.Position, WorldViewProj);  //apply vertex transformation

    Out.Texture  = In.Texture;          //copy original texcoords

    Out.Normal = -normalize(mul(In.Normal, WorldViewProj)); // transform Normal and normalize

	// non-linear depth
	//Out.Depth = (Out.Position.z/Out.Position.w); 
	Out.Depth = Out.Position.zw;
	
	// linear depth
	//float4 eyeSpacePos = mul(In.Position, WorldView);
	//Out.Depth = (eyeSpacePos.z / FarClip);

    return Out;                         //return output vertex
}




// Pixel shader output structure
struct PS_OUTPUT
{
    float4 Color   : COLOR0;
};


PS_OUTPUT ps_packNormalDepth( in VS_OUTPUT In )
{
    PS_OUTPUT Out;                             //create an output pixel
    
    float depth = In.Depth.x / In.Depth.y;	// z / w
    
    float3 normal = In.Normal;
    
    // perturb normal based on normal map
    float4 nrmMap = tex2D( NormalSampler, In.Texture );
    
    //normal = nrmMap;

	// pack normal and depth	
	Out.Color = float4( PackNormal(normal), F32_Compress(depth) );
	
	// test unpacking the depth
	//float unpacked = F32_Decompress(Out.Color.zw);
	//Out.Color = float4( unpacked, unpacked, unpacked, 1.0f );
	
	// test unpacking the normal
	//float3 un = UnpackNormal(Out.Color.xy);
	//Out.Color = float4( un, 1.0f );
	//Out.Color = float4( (un.x+1.0f)/2.0f, 0.0f, 0.0f, 1.0f );

    return Out;                                //return output pixel
}



Technique RenderToGBuffer
{
	pass Pass0
	{
		// enable stencil writes
		StencilEnable		= true;
		StencilMask			= 0x1;
		StencilWriteMask	= 0x1;
		StencilRef			= 0x1;		
		StencilPass			= REPLACE;
		StencilFunc			= ALWAYS;

        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_packNormalDepth(); 
	}
}
