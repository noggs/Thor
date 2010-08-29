
#include "utility.h"

float4x4	WorldViewProj;		// matrix from ObjectSpace to ClipSpace
float2		GBufferSize;		// dimensions of GBuffer texture


// Lighting buffer texture lookup
texture LightBufferTexture;
sampler LightBufferSampler = sampler_state
{
	Texture = <LightBufferTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};


// diffuse colour map
texture DiffuseMap;
sampler DiffuseSampler = sampler_state {
	Texture = <DiffuseMap>;
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
    float2 Texture    : TEXCOORD0;
	float3 Normal     : NORMAL;
};


// Vertex shader output structure
struct VS_OUTPUT
{
    float4 Position   : POSITION;
    float2 Texture    : TEXCOORD0;
    float2 LookupUV   : TEXCOORD1;		// lookup into Lighting buffer
};



// Name: Simple Vertex Shader
// Type: Vertex shader
// Desc: Vertex transformation and texture coord pass-through
//
VS_OUTPUT vs_main( in VS_INPUT In )
{
    VS_OUTPUT Out;                      //create an output vertex

    Out.Position = mul(In.Position, WorldViewProj);  //apply vertex transformation

	Out.LookupUV.x = Out.Position.x / Out.Position.w;
	Out.LookupUV.y = Out.Position.y / Out.Position.w;

    Out.Texture  = float2(In.Texture.x, 1-In.Texture.y);          //invert Y for some reason?

    return Out;                         //return output vertex
}




// Pixel shader output structure
struct PS_OUTPUT
{
    float4 Color   : COLOR0;
};



PS_OUTPUT ps_modelTexDiffuse( in VS_OUTPUT In )
{
    PS_OUTPUT Out;                             //create an output pixel
    
    float2 uv = float2(
		(0.5f * In.LookupUV.x) + 0.5f,
		(0.5f * -In.LookupUV.y) + 0.5f);
		
	uv += 0.5f / GBufferSize;

    //Out.Color = float4( uv, 0.0f, 1.0f );

    // grab value from the Lighting Buffer (diffuse xyz, specular w)
    float4 lighting = MapInvExp( tex2D( LightBufferSampler, uv ) );
    
	float3 diffCol = float3( lighting.xyz );
	
	float SpecularPower_0 = 5.0f;
	float specular = pow( lighting.w, SpecularPower_0);
	
	//Out.Color = float4( specular, specular, specular, 1.0f );

	float specIntens = 0.2f;
	float diffIntens = 1.0f;
	
	// get diffuse colour here
	float4 baseCol = float4( tex2D(DiffuseSampler, In.Texture) );
	
	//Out.Color = baseCol;
	
	Out.Color = float4( baseCol.xyz * diffCol * diffIntens + specular * specIntens, 1.0f );

    return Out;                                //return output pixel
}



Technique ModelTexDiffuse
{
	pass Pass0
	{
		StencilEnable		= false;

        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_modelTexDiffuse(); 
	}
}
