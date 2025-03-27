#include "Common.hlsl"

[shader("miss")]
void Miss(inout HitInfo payload : SV_RayPayload)
{
    float4 gradientStart = float4(0.8f, 0.6f, 1.0f, 1.0f);
    float4 gradientEnd = float4(0.9f, 0.9f, 1.0f, 1.0f);
    float3 unitDir = normalize(WorldRayDirection());
    float t = 0.5 * unitDir.y + 0.5;
    
    payload.color = (1.0f - t) * gradientStart + t * gradientEnd;
}