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
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    
    vout.PosH = mul(posW, gViewProj);
    vout.Color = float4(vin.TexC, 0, 1);
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}


