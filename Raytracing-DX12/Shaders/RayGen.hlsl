#include "Common.hlsl"

cbuffer gCameraCB : register(b0)
{
    float4 gCamPos;
    float4x4 gViewProjectionInv;
}

RWTexture2D<float4> gOutput : register(u0);
RaytracingAccelerationStructure gSceneBVH : register(t0);

[shader("raygeneration")]
void RayGen()
{
    HitInfo payload;
    payload.colorAndDistance = float4(0, 0, 0, 0);

    uint2 launchIndex = DispatchRaysIndex().xy;
    
    float2 xy = launchIndex + 0.5f;
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    screenPos.y = -screenPos.y;
    
    float4 world = mul(float4(screenPos, 0, 1), gViewProjectionInv);
    world.xyz /= world.w;
    
    RayDesc ray;
    ray.Origin = gCamPos;
    ray.Direction = normalize(world.xyz - ray.Origin);
    ray.TMin = 0.001;
    ray.TMax = 100000;
    
    TraceRay(gSceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);
    
    gOutput[launchIndex] = float4(payload.colorAndDistance.rgb, 1.f);
}
