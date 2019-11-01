#ifndef SHADER_RAYTRACING_CLOSESTHIT_HLSL
#define SHADER_RAYTRACING_CLOSESTHIT_HLSL

#define HLSL
#include "Global.hlsli"
#include "RaytracingCompat.h"
#include "ClosestHit_Local.h"
#include "Helper.hlsli"

inline float random(in float2 p) {
    return frac(sin(dot(p, float2(12.9898, 78.233))) * 43758.5453);
}

inline float3 randomSphere(in float2 p) {
    float x = random(p) * 2.0 - 1.0;
    float y = random(p + 1.0) * 2.0 - 1.0;
    float z = random(p + 2.0f) * 2.0 - 1.0;
    return normalize(float3(x, y, z));
}

[shader("closesthit")]
void ClosestHit_Sphere(inout RayPayload payload, in MyAttr attr) {
    if (payload.recursion > MAX_RECURSION) {
        return;
    }

    float3 N = normalize(hitWorldPosition() - l_material.color.xyz);
    float3 worldPos = hitWorldPosition();

    float3 L = normalize(g_sceneCB.lightPosition.xyz - worldPos);

    float3 dir =  WorldRayDirection();
    float2 UV = getUV(getIndices(), attr);
    RayPayload secondPayload = { payload.color,payload.recursion };
    RayDesc secondRay;
    secondRay.Origin = worldPos;
    secondRay.Direction =dir;
    secondRay.TMin = 0.01;
    secondRay.TMax = 10000.0;
    TraceRay(
        g_scene,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
        ~0,
        0,
        1,
        0,
        secondRay,
        secondPayload);

    float4 color =  secondPayload.color * 0.5f;
    float dotNL = saturate(dot(N, L));
    color.rgb += float3(1, 1, 1) * pow(dotNL, 50.0);
    payload.color = color;
}
#endif //! SHADER_RAYTRACING_CLOSESTHIT_HLSL