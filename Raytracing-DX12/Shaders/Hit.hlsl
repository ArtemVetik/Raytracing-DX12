#include "Common.hlsl"

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
    float4x4 gWorld;
}

cbuffer gMaterialCB : register(b1)
{
    float4 gLightAmbientColor;
    float4 gLightDiffuseColor;
}

float3 HitAttribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

float4 CalculateDiffuseLighting(float3 hitPosition, float3 normal)
{
    float3 pixelToLight = normalize(gLightPos - hitPosition);
    
    float fNDotL = max(0.0f, dot(pixelToLight, normal));
    return gLightAmbientColor + gLightDiffuseColor * fNDotL;
}

[shader("closesthit")]
void ClosestHit(inout HitInfo payload, BuiltInTriangleIntersectionAttributes attrib)
{
    uint vertId = 3 * PrimitiveIndex();
    float3 worldOrigin = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
    
    float3 vertexTexC[3] =
    {
        float3(gMeshVertex[gMeshIndex[vertId + 0]].TexC, 1),
        float3(gMeshVertex[gMeshIndex[vertId + 1]].TexC, 1),
        float3(gMeshVertex[gMeshIndex[vertId + 2]].TexC, 1),
    };
    
    float3 vertexNormals[3] =
    {
        gMeshVertex[gMeshIndex[vertId + 0]].Normal,
        gMeshVertex[gMeshIndex[vertId + 1]].Normal,
        gMeshVertex[gMeshIndex[vertId + 2]].Normal,
    };
    
    float3 triangleNormal = HitAttribute(vertexNormals, attrib);
    float2 texC = HitAttribute(vertexTexC, attrib).xy;
    
    triangleNormal = mul(triangleNormal, (float3x3)gWorld);
    
    float4 diffuseColor = CalculateDiffuseLighting(worldOrigin, triangleNormal);
    float4 diffuseAlbedo = gAlbedo.SampleLevel(gsamPointWrap, texC, 0);
    
    float4 color = diffuseAlbedo * diffuseColor;
    
    payload.colorAndDistance = float4(color.rgb, RayTCurrent());
}

[shader("closesthit")]
void PlaneClosestHit(inout HitInfo payload, BuiltInTriangleIntersectionAttributes attrib)
{
    uint vertId = 3 * PrimitiveIndex();
    float3 worldOrigin = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
    float3 lightDir = normalize(gLightPos - worldOrigin);

    float3 vertexTexC[3] =
    {
        float3(gMeshVertex[gMeshIndex[vertId + 0]].TexC, 1),
        float3(gMeshVertex[gMeshIndex[vertId + 1]].TexC, 1),
        float3(gMeshVertex[gMeshIndex[vertId + 2]].TexC, 1),
    };
    
    float2 texC = HitAttribute(vertexTexC, attrib).xy;
    
    RayDesc ray;
    ray.Origin = worldOrigin;
    ray.Direction = lightDir;
    ray.TMin = 0.001;
    ray.TMax = 100000;

    ShadowHitInfo shadowPayload;
    shadowPayload.IsHit = false;
    
    TraceRay(gSceneBVH, RAY_FLAG_NONE, 0xFF, 1, 0, 1, ray, shadowPayload);

    float factor = shadowPayload.IsHit ? 0.3 : 1.0;
    float3 hitColor = gAlbedo.SampleLevel(gsamPointWrap, texC, 0).rgb * factor;
    
    payload.colorAndDistance = float4(hitColor, RayTCurrent());}