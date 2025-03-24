#include "Common.hlsl"

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

[shader("closesthit")]
void ClosestHit(inout HitInfo payload, Attributes attrib)
{
    float3 barycentrics = float3(1.f - attrib.bary.x - attrib.bary.y, attrib.bary.x, attrib.bary.y);

    uint vertId = 3 * PrimitiveIndex();
    float3 hitColor = 0;
    
    float2 texC = float3(gMeshVertex[gMeshIndex[vertId + 0]].TexC, 1) * barycentrics.x +
               float3(gMeshVertex[gMeshIndex[vertId + 1]].TexC, 1) * barycentrics.y +
               float3(gMeshVertex[gMeshIndex[vertId + 2]].TexC, 1) * barycentrics.z;
        
    hitColor = gAlbedo.SampleLevel(gsamPointWrap, texC, 0).rgb;
    
    payload.colorAndDistance = float4(hitColor, RayTCurrent());
}

[shader("closesthit")]
void PlaneClosestHit(inout HitInfo payload, Attributes attrib)
{
    float3 barycentrics = float3(1.f - attrib.bary.x - attrib.bary.y, attrib.bary.x, attrib.bary.y);

    uint vertId = 3 * PrimitiveIndex();
    float3 hitColor = 0;
    
    float2 texC = float3(gMeshVertex[gMeshIndex[vertId + 0]].TexC, 1) * barycentrics.x +
               float3(gMeshVertex[gMeshIndex[vertId + 1]].TexC, 1) * barycentrics.y +
               float3(gMeshVertex[gMeshIndex[vertId + 2]].TexC, 1) * barycentrics.z;
        
    hitColor = gAlbedo.SampleLevel(gsamPointWrap, texC, 0).rgb;
    
    payload.colorAndDistance = float4(hitColor, RayTCurrent());
}