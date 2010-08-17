
// Global variables
float4x4 WorldViewProj;		// matrix from ObjectSpace to ClipSpace
float4x4 WorldView;			// matrix from ObjectSpace to EyeSpace

float FarClip;



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
    float3 Normal     : TEXCOORD1;
    float  Depth      : TEXCOORD2;
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
	Out.Depth = (Out.Position.z/Out.Position.w); 
	
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


PS_OUTPUT ps_packNormalDepth( in VS_OUTPUT In )
{
    PS_OUTPUT Out;                             //create an output pixel

	//Out.Color = float4(In.Normal, 1.0f);
	//Out.Color = float4(In.Depth, In.Depth, In.Depth, 1.0f);
	
	// pack normal into the first 16 bits and depth into next two
	//Out.Color = float4(In.Normal.x, In.Normal.y, 0.0f, 0.0f);
	//Out.Color = float4(F32_Compress(In.Depth), 0.0f, 1.0f );
	
	Out.Color = float4( PackNormal(In.Normal.xyz), F32_Compress(In.Depth) );
	
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
