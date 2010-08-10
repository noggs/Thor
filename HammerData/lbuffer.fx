
// Global variables
float4x4 WorldViewProj;

// Light parameters
float4 LightPos;
float4 LightColour;
float  LightAttenuation;
float  LightIntensity;

texture GBufferTexture;

sampler GBufferSampler = 
sampler_state
{
	Texture = <GBufferTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};


// Vertex shader input structure
struct VS_INPUT
{
    float4 Position		: POSITION;
};




// Vertex shader output structure
struct VS_OUTPUT
{
    float4 Position		: POSITION;
    float2 ScreenUV		: TEXCOORD0;
};



// Name: Simple Vertex Shader
// Type: Vertex shader
// Desc: Vertex transformation and texture coord pass-through
//
VS_OUTPUT vs_light( in VS_INPUT In )
{
    VS_OUTPUT Out;                      //create an output vertex

    Out.Position = mul(In.Position, WorldViewProj);  //apply vertex transformation
    
    Out.ScreenUV = ?????;

    return Out;                         //return output vertex
}




// Pixel shader output structure
struct PS_OUTPUT
{
    float4 Color   : COLOR0;
};


float2 F32_Compress(float f)
{
	float u,v;
	float res_u;

	u = floor(f*256.0);
	res_u = f*256.0 - u;
	v = floor(res_u * 256.0);

	return (1/256.0*float2(u,v));
}

float F32_Decompress(float2 vec)
{
	return (vec.x+vec.y*1.0/256.0);
}

float2 PackNormal(float3 nrm)
{
	return float2(nrm.x, nrm.y);
}

float3 UnpackNormal(float2 nrm)
{
	return float3( nrm.x, nrm.y, sqrt(1-(nrm.x*nrm.x)-(nrm.y*nrm.y)) );
}


PS_OUTPUT ps_light( in VS_OUTPUT In )
{
    PS_OUTPUT Out;                             //create an output pixel
    
    // lookup this pixel in the GBuffer to get the surface normal at this point
    // use the depth to calculate the distance from the light
    
    // grab value from the GBuffer (packed normal/depth)
    float4 gvalue = tex2D( GBufferSampler, In.ScreenUV );
    
    // extract normal and depth value
    float3 nrm = UnpackNormal( gvalue.xy );
    float depth = F32_Decompress( gvalue.zw );
    
    // reconstruct world space position of pixel from screenUV, depth and WorldViewProj
    float3 pixelPos = ;
    
    // calculate distance and direction to light
    float3 lightDir = sub(PixelPos, LightPos);
    float lightDist = length( lightDir );
    lightDir = normalize( lightDir );
    
    // for directional light:
    // diffuse = dot(N,L) * Di * Dc			( N=surface normal,  L=light direction,  Di=light intensity,  Dc=light colour
    
    // for point light
    // diffuse = dot(N,L) * S * Att			( S=distance to light,  Att=Attenuation )

    return Out;                                //return output pixel
}




Technique RenderToLightBuffer
{
	pass Pass0
	{
		VertexShader = compile vs_2_0 vs_light();
		PixelShader  = compile ps_2_0 ps_light();
	}
}
