
////////////////////////////////////////////////////
// FX file for forward rendering models
//

#include "utility.h"

float4x4	WorldViewProj;		// matrix from ObjectSpace to ClipSpace


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
};



VS_OUTPUT vs_main( in VS_INPUT In )
{
    VS_OUTPUT Out;                      //create an output vertex

    Out.Position = mul(In.Position, WorldViewProj);  //apply vertex transformation
    Out.Texture  = In.Texture;          //copy original texcoords

	// create matrix mapping from tangent-space to view-space to transform the bump map
	Out.Tangent	  = mul( float4( In.Tangent, 0.0f),   WorldView ).xyz;
	Out.Bitangent = mul( float4( In.Bitangent, 0.0f), WorldView ).xyz;	
	Out.Normal    = mul( float4( In.Normal, 0.0f),    WorldView ).xyz;

    return Out;                         //return output vertex
}


struct PS_OUTPUT
{
	float4 Color : COLOR;
};

PS_OUTPUT ps_main( uniform const int NumPointLights, in VS_OUTPUT In )
{
	PS_OUTPUT Out;
	
	// get diffuse colour here
	float4 baseCol = tex2D(DiffuseSampler, In.Texture);
	Out.Color = baseCol;
	
	// get normal from surface/texture
	//float3 nrmTS = tex2D(NormalSampler, In.Texture).xyz;
	float3 nrmTS = float3(0,0,1);

    // transform bump from tangent space to viewspace
    float3x3 nrmMat = float3x3( normalize(In.Tangent), normalize(In.Bitangent), normalize(In.Normal) );
    float3 normal = mul( nrmTS, nrmMat );

	// is this a faster method? need to test for correctness
    //float3 normal;
    //normal.x = dot( nrmIn, normalize(In.Tangent) );
    //normal.y = dot( nrmIn, normalize(In.Bitangent) );
    //normal.z = dot( nrmIn, normalize(In.Normal) );
    
    float specular=0;
    float3 colour = float3(0,0,0);

	// Directional light
	if( DirLight )
	{
		// light direction in WorldSpace
		float3 lightDir = LightDirWS;
	
		// surface normal to light angle
		float NL = dot( lightDir, nrm );

		// now calculate specular component
		float3 eyeVec = float3(0.0f, 0.0f, 1.0f);	// in ViewSpace so camera is always here!
		specular = pow( saturate( dot( reflect(eyeVec, nrm), lightDir)), 1 );
		specular = 0;

	}
	
	
	return Out;
}


int CurNumLights = 2;
PixelShader psArrayDir[] = {	compile ps_2_0 ps_main(true, 1), 
								compile ps_2_0 ps_main(true, 2),
								compile ps_2_0 ps_main(true, 3),
							};

///
/// 1 Directional and n Point Lights
///
Technique DirPointLights
{
	pass Pass0
	{
		VertexShader = compile vs_2_0 vs_main();
		PixelShader  = (psArrayDir[CurNumLights]);
	}
}

