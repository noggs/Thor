
///////////////////////////////////////////////////////////
// Pack / unpack a float into two 8 bit channels 
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


///////////////////////////////////////////////////////////
// Pack / unpack a normal into 2 float channels - assumes positive Z

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


///////////////////////////////////////////////////////////
// Used to map specular on an exponential scale to help avoid saturation

float4 MapExp(float4 colour)
{
	return colour;
	//return exp( -colour );
	//return exp2( -colour );
}

float4 MapInvExp(float4 col)
{
	return col;
	//return -log( col );
	//return -log2( col );
}

