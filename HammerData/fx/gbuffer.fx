
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
    float3 Tangent    : TEXCOORD2;
    float3 Bitangent  : TEXCOORD3;
    float2 Depth      : TEXCOORD4;
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
    
    //Out.Normal = mul(In.Normal, WorldViewProj); // transform Normal

	// create matrix mapping from tangent-space to view-space to transform the bump map
	Out.Tangent	  = mul( float4( In.Tangent, 0.0f),   WorldViewProj ).xyz;
	Out.Bitangent = mul( float4( In.Bitangent, 0.0f), WorldViewProj ).xyz;	
	Out.Normal    = mul( float4( -In.Normal, 0.0f),   WorldViewProj ).xyz;

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
    
    //float3 normal = -normalize( In.Normal );
    
    // perturb normal based on normal map
    float4 nrmMap = tex2D( NormalSampler, In.Texture );
    
    //float3 nrmIn = nrmMap.xyz;
    float3 nrmIn = float3(0,0,1);
    
    //float3 normal;
    //normal.x = dot( nrmIn, normalize(In.Tangent) );
    //normal.y = dot( nrmIn, normalize(In.Bitangent) );
    //normal.z = dot( nrmIn, normalize(In.Normal) );

    
    // transform bump from tangent space to viewspace
    float3x3 nrmMat = float3x3( normalize(In.Tangent), normalize(In.Bitangent), normalize(In.Normal) );
    float3 normal = mul( nrmIn, nrmMat );
    

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
