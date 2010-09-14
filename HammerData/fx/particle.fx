
////////////////////////////////////////////////////
// FX file for Particle rendering
//

#include "utility.h"


float4x4 ViewProj;		// matrix from ObjectSpace to ClipSpace
float4x4 View;			// matrix from ObjectSpace to CameraSpace
float2   GBufferSize;	// dimensions of GBuffer texture

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

// G-Buffer texture lookup
texture GBufferTexture;
sampler GBufferSampler = 
sampler_state
{
	Texture = <GBufferTexture>;
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};



struct VS_OUTPUT
{
    float4 Position   : POSITION;
    float4 Color      : COLOR;
};

VS_OUTPUT vs_main( float4 pos : POSITION0, float4 color : COLOR0)
{
	VS_OUTPUT Out;
    Out.Position = mul(pos, ViewProj);  //apply vertex transformation
    Out.Color = color;
    return Out;
}


struct PS_INPUT
{
    float4 Color    : COLOR;
	float2 TexCoord : TEXCOORD0;
};

float4 ps_main( in PS_INPUT In ) : COLOR0
{
	return In.Color * tex2D(DiffuseSampler, In.TexCoord);
}



////////////////////////////////////
// soft particles

struct VS_OUTPUT_SOFT
{
    float4 Position   : POSITION;
    float4 Color      : COLOR0;
    float  PSize      : PSIZE;
    float4 ScreenPos  : COLOR1;		// lookup into G buffer
};

VS_OUTPUT_SOFT vs_soft( float4 pos : POSITION0, float4 color : COLOR0)
{
	VS_OUTPUT_SOFT Out;
    Out.Position = mul(pos, ViewProj);  //apply vertex transformation
    Out.Color = color;
    
    //Out.TexCoord = float2(0,0);
    
    ///////////
    // Particle size needs to be set to screenspace units. Transform it based
    // on the distance from camera.
    {
		float4 eyeSpacePos = mul(pos, View);
		float distSq = (eyeSpacePos.x*eyeSpacePos.x + eyeSpacePos.y*eyeSpacePos.y + eyeSpacePos.z*eyeSpacePos.z);
		float dist = sqrt(distSq);
		
		const float Vh = 600;
		const float Si = 1;
		
		const float A = 0.0f;
		const float B = 0.0f;
		const float C = 1.0f;
		
		float Ss = Vh * Si * sqrt( 1/( A + B*dist + C*distSq ) );
		Out.PSize = Ss;
    }
    
    //Out.PSize = 100.0f;
    
    float2 uv = (0.5f * (Out.Position.xy / Out.Position.w)) + 0.5f;
    
    Out.ScreenPos = float4(uv,Out.PSize,0);

    return Out;
}


struct PS_INPUT_SOFT
{
    float4 Color      : COLOR0;
    float  PSize      : PSIZE;
    float4 ScreenPos  : COLOR1;		// lookup into G buffer
	float2 TexCoord   : TEXCOORD0;
};

float4 ps_soft( in PS_INPUT_SOFT In ) : COLOR0
{
	float2 uv = In.ScreenPos.xy + (((2.0f*In.TexCoord)-1.0f));
	//uv *= 5;
    //return float4(uv, 0, 1);
    //return float4(In.TexCoord, 0, 1);

    float4 gvalue = tex2D( GBufferSampler, uv );
    return float4(gvalue.xy, 0, 1);

    float scenedepth = F32_Decompress(gvalue.zw);
    float particledepth = In.ScreenPos.z / In.ScreenPos.w;
    float scale = 1.0f;
    
    float f = saturate((scenedepth - particledepth) * scale);
    
    float4 color = float4(f,f,f,1);
    
    //float4 color = In.Color * tex2D(DiffuseSampler, In.TexCoord);
    //color.w *= f;

	return color;
}





///
///
Technique SimpleParticle
{
	pass Pass0
	{
		PointSpriteEnable = true;
		PointSize = 256;

		AlphaBlendEnable	= TRUE;  
		SrcBlend			= SrcAlpha;
		DestBlend			= InvSrcAlpha;

		VertexShader = compile vs_2_0 vs_main();
		PixelShader  = compile ps_2_0 ps_main();
	}
}

Technique SoftParticle
{
	pass Pass0
	{
		PointSpriteEnable = true;

		AlphaBlendEnable	= TRUE;  
		SrcBlend			= SrcAlpha;
		DestBlend			= InvSrcAlpha;

		VertexShader = compile vs_2_0 vs_soft();
		PixelShader  = compile ps_2_0 ps_soft();
	}
}
