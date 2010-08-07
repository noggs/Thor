

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


// Global variables
float4x4 WorldViewProj;


// Name: Simple Vertex Shader
// Type: Vertex shader
// Desc: Vertex transformation and texture coord pass-through
//
VS_OUTPUT vs_main( in VS_INPUT In )
{
    VS_OUTPUT Out;                      //create an output vertex

    Out.Position = mul(In.Position,
                       WorldViewProj);  //apply vertex transformation

    Out.Texture  = In.Texture;          //copy original texcoords

    Out.Normal = -normalize(mul(In.Normal, WorldViewProj)); // transform Normal and normalize

	Out.Depth = 1-(Out.Position.z/Out.Position.w); 

    return Out;                         //return output vertex
}


