#ifndef SHADER_RAYTRACING_CLOSESTHIT_HLSL
#define SHADER_RAYTRACING_CLOSESTHIT_HLSL

#define HLSL
#include "Global.hlsli"
#include "RaytracingCompat.h"
#include "ClosestHit_Local.h"
#include "Helper.hlsli"

inline bool shadowCast( in Ray ray) {
    RayDesc shadowRay;
    shadowRay.Origin = ray.origin;
    shadowRay.Direction = ray.direction;
    shadowRay.TMin = 0.01;
    shadowRay.TMax = 10000.0;
    ShadowPayload shadowPayload = { false };
    TraceRay(
        g_scene,
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
        ~0,
        1,
        1,
        1,
        shadowRay,
        shadowPayload);
    return shadowPayload.hit;
}

[shader("closesthit")]
void ClosestHit_Cube(inout RayPayload payload, in MyAttr attr) {
    if (payload.recursion > MAX_RECURSION) {
        payload.color = float4(0, 0, 0, 0);
        return;
    }
    float3 N = getNormal(getIndices(), attr);

    float3 worldPos = hitWorldPosition();
    float3 L = normalize(g_sceneCB.lightPosition.xyz - worldPos);

    float2 UV = getUV(getIndices(), attr);
    Ray ray = { worldPos,L };
    bool shadow = shadowCast(ray);
    float4 color = l_texture.SampleLevel(samLinear, UV, 0.0);
    color = shadow ? color * 0.1 : color;
    payload.color = color;
}
#endif //! SHADER_RAYTRACING_CLOSESTHIT_HLSL