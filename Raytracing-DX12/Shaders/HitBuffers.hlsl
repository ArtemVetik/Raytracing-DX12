#ifndef HIT_BUFFERS
#define HIT_BUFFERS

struct ShadowHitInfo
{
    bool IsHit;
};

struct SVertex
{
    float3 Pos;
    float3 Normal;
    float3 Tangent;
    float2 TexC;
};

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

StructuredBuffer<SVertex> gMeshVertex : register(t0);
StructuredBuffer<uint> gMeshIndex : register(t1);
Texture2D gAlbedo : register(t2);
RaytracingAccelerationStructure gSceneBVH : register(t3);

cbuffer gPassCB : register(b0)
{
    float3 gLightPos;
    uint gPadding;
    float3 gCamPos;
    uint gPadding1;
}

cbuffer gObjectCB : register(b1)
{
    float4x4 gWorld;
}

cbuffer gMaterialCB : register(b2)
{
    float3 gDiffuseColor;
    float gDiffuseCoef;
    float gSpecularCoef;
    float gSpecularPower;
    float gReflectanceCoef;
    float gInShadowRadiance;
};

#endif