#include "Common.hlsl"
#include "HitBuffers.hlsl"
#include "LightingUtil.hlsl"

float3 HitAttribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
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
    
    float2 texC = HitAttribute(vertexTexC, attrib).xy;
    float3 triangleNormal = HitAttribute(vertexNormals, attrib);
    
    triangleNormal = normalize(mul(triangleNormal, (float3x3) gWorld));
    
    float2 texScale = InstanceID() == 0 ? 4 : 1;
    float4 diffuseAlbedo = gAlbedo.SampleLevel(gsamPointWrap, texC * texScale, 0) * float4(gDiffuseColor, 1);
    
    float4 color;
    if (payload.recursionDepth < 4)
    {
        // Shadow
        RayDesc shadowRay;
        shadowRay.Origin = worldOrigin;
        shadowRay.Direction = normalize(gLightPos - shadowRay.Origin);
        shadowRay.TMin = 0.01;
        shadowRay.TMax = 100000;
        ShadowHitInfo shadowPayload;
        TraceRay(gSceneBVH,
            0 /*rayFlags*/,
            0xFF,
            1 /* ray index*/,
            0 /* Multiplies */,
            1 /* Miss index (shadow) */,
            shadowRay,
            shadowPayload);

        // Reflection    
        RayDesc reflectionRay;
        reflectionRay.Origin = worldOrigin;
        reflectionRay.Direction = reflect(WorldRayDirection(), triangleNormal);
        reflectionRay.TMin = 0.01;
        reflectionRay.TMax = 100000;
        HitInfo reflectionPayload;
        reflectionPayload.recursionDepth = payload.recursionDepth + 1;
        
        TraceRay(gSceneBVH,
            0 /*rayFlags*/,
            0xFF,
            0 /* ray index*/,
            0 /* Multiplies */,
            0 /* Miss index (raytrace) */,
            reflectionRay,
            reflectionPayload);
        float4 reflectionColor = reflectionPayload.color;

        float3 fresnelR = FresnelReflectanceSchlick(WorldRayDirection(), triangleNormal, diffuseAlbedo.rgb);
        float4 reflectedColor = gReflectanceCoef * float4(fresnelR, 1) * reflectionColor;
        
        float4 phongColor = CalculatePhongLighting(gLightPos, diffuseAlbedo, triangleNormal, shadowPayload.IsHit, gDiffuseCoef, gSpecularCoef, gSpecularPower);
        color = phongColor + reflectedColor;
    }
    else
    {
        color = CalculatePhongLighting(gLightPos, diffuseAlbedo, triangleNormal, false, gDiffuseCoef, gSpecularCoef, gSpecularPower);
    }
    
    payload.color = color;
}