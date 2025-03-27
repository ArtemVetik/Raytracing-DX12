#include "Common.hlsl"

[shader("miss")]
void Miss(inout HitInfo payload : SV_RayPayload)
{
    float4 gradientStart = float4(0.4, 0.4, 0.4, 1.0);
    float4 gradientEnd = float4(0.8, 0.8, 0.8, 1.0);
    float3 unitDir = normalize(WorldRayDirection());
    float t = 0.5 * unitDir.y;
    payload.color = (1.0 - t) * gradientStart + t * gradientEnd;
}