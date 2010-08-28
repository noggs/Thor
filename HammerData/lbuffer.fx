
// Global variables
float4x4	WorldViewProj;		// matrix from ObjectSpace to ClipSpace
float4x4	WorldView;			// matrix from ObjectSpace to EyeSpace
float4x4	InvWorldViewProj;	// matrix to unproject a point from ClipSpace back to ObjectSpace
float4x4	InvProj;			// matrix to unproject from ClipSpace to ViewSpace
float		FarClip;			// farclip plane distance
float2		GBufferSize;		// dimensions of GBuffer texture


// Shared light parameters
float3 LightColourAmb = float3(0.2f, 0.2f, 0.2f);
float3 LightColourDif = float3(0.0f, 1.0f, 0.0f);

// Point Light parameters
float3 LightPosVS;	// need to be in view space
float  LightRadius;

// Directional Light parameters
float3 LightDirVS;


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
	Out.PositionVS.x = In.Position.x;
	Out.PositionVS.y = In.Position.y;
	

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
	
	// surface normal to light angle
	float NL = dot( LightDirVS, nrm );
	
	// now calculate specular component
	float3 eyeVec = float3(0.0f, 0.0f, -1.0f);	// in ViewSpace so camera is always here!
	float specular = saturate( dot( reflect(eyeVec, nrm), LightDirVS)) / 10;
	
	//Out.Color = float4( specular, specular, specular, 1.0f );
    
   	// write diffuse colour in xyz and specular term in w channel
	Out.Color = float4(LightColourDif * NL, specular);
	
	// add specular in here for debug
	//Out.Color += float4( specular, specular, specular, 0.0f );
	
	//Out.Color = float4( specular, specular, specular, specular );
    
    return Out;
}


PS_OUTPUT ps_light( in VS_OUTPUT In )
{
    PS_OUTPUT Out;                             //create an output pixel
    
    // lookup this pixel in the GBuffer to get the surface normal at this point
    // use the depth to calculate the distance from the light
    float2 uv = In.PositionVS.xy;
    
    //Out.Color = float4(uv, 0.0f, 1.0f);

    // grab value from the GBuffer (packed normal/depth)
    float4 gvalue = tex2D( GBufferSampler, uv );
    //Out.Color = gvalue;
    
    //return Out;
    
    // extract normal
    float3 nrm = UnpackNormal( gvalue.xy );	// gets the normal in viewspace
    //Out.Color = float4( nrm, 1.0f );
    
    // extract depth
    float depth = F32_Decompress(gvalue.zw);
	Out.Color = float4( depth, depth, depth, 1.0f );

	// get the pixel position in ViewSpace
	float x = In.PositionVS.x * 2 - 1;
    float y = (1 - In.PositionVS.y) * 2 - 1;
	float4 projPixelPos = mul( float4(x, y, depth, 1.0f), InvProj );	// EXPENSIVE MATRIX MULTIPLY!

	float3 pixelPosVS = float3( projPixelPos.xyz / projPixelPos.w ); 	

    // calculate distance and direction to light
    float3 lightDir = pixelPosVS - LightPosVS;
    float lightDist = length( lightDir.xyz );
    lightDir = normalize( lightDir );
    
    // could use 1D texture to encode falloff upto light radius
    // for now use linear falloff: 1-(lightDist/LightRadius)
    
    float att = saturate( 1.0f - (lightDist/LightRadius) );
    float NL = dot( lightDir, nrm ) * att;

	//Out.Color = float4(LightColourDif * NL, 1.0f);

	// now calculate specular component
	float3 eyeVec = float3(0.0f, 0.0f, -1.0f);	// in ViewSpace so camera is always here!
	float specular = saturate( dot( reflect(eyeVec, nrm), LightDirVS)) / 10;
	
	//Out.Color = float4( specular, specular, specular, 1.0f );
	
	// write diffuse colour in xyz and specular term in w channel
	Out.Color = float4(LightColourDif * NL, specular);
	
	// add specular in here for debug
	//Out.Color += float4( specular, specular, specular, 0.0f );

	//Out.Color = float4( specular, specular, specular, specular );

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
		
		AlphaBlendEnable	= TRUE;  
		BlendOp				= ADD;
		SrcBlend			= ONE;
		DestBlend			= ONE;

		VertexShader = compile vs_2_0 vs_light();
		PixelShader  = compile ps_2_0 ps_light();
	}
}

