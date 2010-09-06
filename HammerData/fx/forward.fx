
////////////////////////////////////////////////////
// FX file for forward rendering models
//

#include "utility.h"

float4x4	WorldViewProj;		// matrix from ObjectSpace to ClipSpace
float4x4	WorldView;			// matrix from ObjectSpace to ViewSpace
float4x4	InvWorldView;		// matrix from ViewSpace to ObjectSpace


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

// Shared light parameters
float3 LightColourAmb[8];
float3 LightColourDif[8];

// Point Light parameters
float3 LightPosVS[8];	// need to be in view space
float  LightRadius[8];

// Directional Light parameters
float3 LightDirVS;
float3 LightColourDifDir;



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
    float3 PosVS      : TEXCOORD4;
};



VS_OUTPUT vs_main( in VS_INPUT In )
{
    VS_OUTPUT Out;                      //create an output vertex

    Out.Position = mul(In.Position, WorldViewProj);  //apply vertex transformation
    Out.Texture  = float2( In.Texture.x, 1-In.Texture.y);          //copy original texcoords

	// create matrix mapping from tangent-space to view-space to transform the bump map
	Out.Tangent	  = mul( float4( In.Tangent, 0.0f),   WorldView ).xyz;
	Out.Bitangent = mul( float4( In.Bitangent, 0.0f), WorldView ).xyz;	
	Out.Normal    = mul( float4( -In.Normal, 0.0f),   WorldView ).xyz;
	
	Out.PosVS = mul(In.Position, WorldView).xyz;

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
	
	float3 normal = normalize( In.Normal );
	
	// get normal from surface/texture
	//float3 nrmTS = tex2D(NormalSampler, In.Texture).xyz;
	//float3 nrmTS = float3(0,0,1);

    // transform bump from tangent space to viewspace
    //float3x3 nrmMat = float3x3( normalize(In.Tangent), normalize(In.Bitangent), normalize(In.Normal) );
    //float3 normal = mul( nrmTS, nrmMat );

	// is this a faster method? need to test for correctness
    //float3 normal;
    //normal.x = dot( nrmIn, normalize(In.Tangent) );
    //normal.y = dot( nrmIn, normalize(In.Bitangent) );
    //normal.z = dot( nrmIn, normalize(In.Normal) );
    
    float specular=0;
    float3 lightColour = float3(0,0,0);

	// Directional light
	//if( DirLight )
	{
		// light direction in ViewSpace
		float3 lightDir = LightDirVS;
	
		// surface normal to light angle
		float NL = dot( lightDir, normal );

		// now calculate specular component
		float3 eyeVec = float3(0.0f, 0.0f, 1.0f);	// in ViewSpace so camera is always here!
		specular = pow( saturate( dot( reflect(eyeVec, normal), lightDir) ), 1 );

   		// write colour
		lightColour = float4(LightColourDifDir * NL, 1);
	}

	// Point lights
	for( int i = 0; i != NumPointLights; ++i )
	{
		// calculate distance and direction to light
		float3 lightDir = In.PosVS - LightPosVS[i];
		float lightDist = length( lightDir.xyz );
		lightDir = normalize( lightDir );
	    
		// could use 1D texture to encode falloff upto light radius
		// for now use linear falloff: 1-(lightDist/LightRadius[i])
	    
		float att = saturate( 1.0f - (lightDist/LightRadius[i]) );
		float NL = saturate( dot( lightDir, normal ) ) * att;
		
		lightColour += float4(LightColourDif[i] * NL, 0);
	}
	
	Out.Color = float4( baseCol.xyz * lightColour, 1.0f );

	
	return Out;
}


int CurNumLights = 2;
PixelShader psArrayDir[] = {	compile ps_2_0 ps_main(1), 
								compile ps_2_0 ps_main(2),
								compile ps_2_0 ps_main(3),
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
