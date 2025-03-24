
struct ShadowHitInfo
{
    bool IsHit;
};

struct Attributes
{
    float2 UV;
};

[shader("closesthit")]void ShadowClosestHit(inout ShadowHitInfo hit, Attributes bary)
{
    hit.IsHit = true;
}

[shader("miss")]void ShadowMiss(inout ShadowHitInfo hit : SV_RayPayload)
{
    hit.IsHit = false;
}