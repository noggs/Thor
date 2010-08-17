
// Global variables
float4x4	WorldViewProj;		// matrix from ObjectSpace to ClipSpace
float4x4	WorldView;			// matrix from ObjectSpace to EyeSpace
float4x4	InvWorldViewProj;	// matrix to unproject a point from ClipSpace back to ObjectSpace
float		FarClip;			// farclip plane distance
float2		GBufferSize;		// dimensions of GBuffer texture


// Shared light parameters
float3 LightColourAmb = float3(0.2f, 0.2f, 0.2f);
float3 LightColourDif = float3(0.0f, 1.0f, 0.0f);

// Point Light parameters
float4 LightPosVS;	// need to be in view space
float  LightRadius;

// Directional Light parameters
float3 LightDirVS;


// G-Buffer texture lookup
texture GBufferTexture;
sampler GBufferSampler = 
sampler_state
{
	Texture = <GBufferTexture>;
	MipFilter = NONE;
	MinFilter = NONE;
	MagFilter = NONE;
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
    float4 PositionVS   : TEXCOORD0;
};



// Name: Simple Vertex Shader
// Type: Vertex shader
// Desc: Vertex transformation and texture coord pass-through
//
VS_OUTPUT vs_light( in VS_INPUT In )
{
    VS_OUTPUT Out;                      //create an output vertex

    //Out.Position = mul(In.Position, WorldViewProj);  //apply vertex transformation
    
	// Offset the position by half a pixel to correctly
	// align texels to pixels. Only necessary for D3D9 or XNA
	Out.Position.x = In.Position.x - (1.0f/GBufferSize.x);
	Out.Position.y = In.Position.y + (1.0f/GBufferSize.y);
	Out.Position.z = In.Position.z;
	Out.Position.w = 1.0f;
	
	Out.PositionVS = Out.Position;
	//Out.PositionVS.x = In.Position.x - (1.0f/GBufferSize.x);
	//Out.PositionVS.y = In.Position.y + (1.0f/GBufferSize.y);
	

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
	return float2((nrm.x+1.0f)/2.0f, (nrm.y+1.0f)/2.0f);
}

float3 UnpackNormal(float2 nrm)
{
	float x = (nrm.x*2.0f)-1.0f;
	float y = (nrm.y*2.0f)-1.0f;
	return float3( x, y, sqrt(1-(x*x)-(y*y)) );
}



PS_OUTPUT ps_DirLight( in VS_OUTPUT In )
{
    PS_OUTPUT Out;                             //create an output pixel
    
    float2 uv = In.PositionVS.xy;

    // grab value from the GBuffer (packed normal/depth)
    float4 gvalue = tex2D( GBufferSampler, uv );
    Out.Color = gvalue;
    
    // extract normal
    float3 nrm = UnpackNormal( gvalue.xy );	// gets the normal in viewspace
    //Out.Color = float4( nrm, 1.0f );
    //Out.Color = float4( (nrm.y+1.0f)/2.0f, 0.0f, 0.0f, 1.0f );
    
    // extract depth
    float depth = F32_Decompress(gvalue.zw);
	//Out.Color = float4( depth, depth, depth, 1.0f );
    
    // dot product light vector and normal gives us the light at that point/pixel
    float lightValue = saturate( dot( nrm, LightDirVS ) );
    Out.Color = float4(lightValue * LightColourDif, 1.0f);
    Out.Color += float4( LightColourAmb, 1.0f );
    
    return Out;
}


PS_OUTPUT ps_light( in VS_OUTPUT In )
{
    PS_OUTPUT Out;                             //create an output pixel
    
    // lookup this pixel in the GBuffer to get the surface normal at this point
    // use the depth to calculate the distance from the light
    float2 uv = In.PositionVS.xy;

    // grab value from the GBuffer (packed normal/depth)
    float4 gvalue = tex2D( GBufferSampler, uv );
    Out.Color = gvalue;
    
    // extract normal
    float3 nrm = UnpackNormal( gvalue.xy );	// gets the normal in viewspace
    //Out.Color = float4( nrm, 1.0f );
    
    // extract depth
    float depth = F32_Decompress(gvalue.zw);
	//Out.Color = float4( depth, depth, depth, 1.0f );

	// need the pixel position in ViewSpace
	float4 pixelPosVS = float4(0.0f, 0.0f, 0.0f, 1.0f);


    // calculate distance and direction to light
    float4 lightDir = pixelPosVS - LightPosVS;
    float lightDist = length( lightDir.xyz );
    lightDir = normalize( lightDir );
    
    // could use 1D texture to encode falloff upto light radius
    // for now use linear falloff: 1-(lightDist/LightRadius)
    
    float lightFalling = saturate( 1.0f - (lightDist/LightRadius) );

	Out.Color = float4(LightColourDif * lightFalling, 1.0f);

    return Out;                                //return output pixel
}




Technique DirectionalLight
{
	pass Pass0
	{
		// only write pixel if it passes the test
		StencilEnable		= true;
		StencilMask			= 0x1;
		StencilRef			= 0x1;		
		StencilPass			= KEEP;
		StencilFunc			= EQUAL;

		VertexShader = compile vs_2_0 vs_light();
		PixelShader  = compile ps_2_0 ps_DirLight();
	}
}


Technique PointLight
{
	pass Pass0
	{
		// only write pixel if it passes the test
		StencilEnable		= true;
		StencilMask			= 0x1;
		StencilRef			= 0x1;		
		StencilPass			= KEEP;
		StencilFunc			= EQUAL;

		VertexShader = compile vs_2_0 vs_light();
		PixelShader  = compile ps_2_0 ps_light();
	}
}

