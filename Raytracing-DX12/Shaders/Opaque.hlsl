SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

Texture2D gAlbedo : register(t0);

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld; 
};

cbuffer cbPerPass : register(b1)
{
    float4x4 gViewProj;
};

struct VertexIn
{
    float3 PosL     : POSITION;
    float3 NormalL  : NORMAL;
    float3 TangentU : TANGENT;
    float2 TexC     : TEXCOORD;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float2 TexC : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    
    vout.PosH = mul(posW, gViewProj);
    vout.TexC = vin.TexC;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return gAlbedo.Sample(gsamLinearClamp, pin.TexC);
}


