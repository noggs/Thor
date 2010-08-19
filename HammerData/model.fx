
float4x4	WorldViewProj;		// matrix from ObjectSpace to ClipSpace
float2		GBufferSize;		// dimensions of GBuffer texture


// Lighting buffer texture lookup
texture LightBufferTexture;
sampler LightBufferSampler = sampler_state
{
	Texture = <LightBufferTexture>;
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
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

	// Offset the position by half a pixel to correctly
	// align texels to pixels. Only necessary for D3D9 or XNA
	Out.LookupUV.x = Out.Position.x - (1.0f/GBufferSize.x);
	Out.LookupUV.y = Out.Position.y + (1.0f/GBufferSize.y);

    Out.Texture  = In.Texture;          //copy original texcoords

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
    
    // grab value from the GBuffer (packed normal/depth)
    float4 lighting = tex2D( LightBufferSampler, In.LookupUV );

	Out.Color = float4( lighting.xyz, 1.0f );

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
