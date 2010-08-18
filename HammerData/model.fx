


// G-Buffer texture lookup
texture GBufferTexture;
sampler GBufferSampler = sampler_state
{
	Texture = <GBufferTexture>;
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
    float4 PositionVS : TEXCOORD1;		// lookup into Lighting buffer
};



// Name: Simple Vertex Shader
// Type: Vertex shader
// Desc: Vertex transformation and texture coord pass-through
//
VS_OUTPUT vs_main( in VS_INPUT In )
{
    VS_OUTPUT Out;                      //create an output vertex

    Out.Position = mul(In.Position, WorldViewProj);  //apply vertex transformation
    Out.PositionVS = Out.Position;		// maybe we only need xy for gbuffer lookup?

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
