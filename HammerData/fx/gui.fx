
////////////////////////////////////////////////////
// FX file for GUI rendering
//


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
    float4 Position   : POSITION0;
    float4 Color      : COLOR0;
    float2 Texture    : TEXCOORD0;
};


// Vertex shader output structure
struct VS_OUTPUT
{
    float4 Position   : POSITION;
    float4 Color      : COlOR0;
    float2 Texture    : TEXCOORD0;
};



VS_OUTPUT vs_main( in VS_INPUT In )
{
    VS_OUTPUT Out;                      //create an output vertex

    Out.Position = In.Position;
    Out.Color = In.Color;
    Out.Texture  = float2( In.Texture.x, 1-In.Texture.y);          //copy original texcoords

    return Out;                         //return output vertex
}


struct PS_OUTPUT
{
	float4 Color : COLOR;
};

PS_OUTPUT ps_main( in VS_OUTPUT In )
{
	PS_OUTPUT Out;

	float4 tex = tex2D( DiffuseSampler, In.Texture );

	Out.Color = tex;

	return Out;
}


///
///
Technique TexturedColor
{
	pass Pass0
	{
		VertexShader = compile vs_2_0 vs_main();
		PixelShader  = compile ps_2_0 ps_main();
	}
}

