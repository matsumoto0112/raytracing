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

inline bool refract(in float3 V, in float3 N, float ni, out float3 ref) {
    float3 nV = normalize(V);
    float dt = dot(nV, N);
    float D = 1.0 - pow(ni, 2.0) * (1.0 - pow(dt, 2.0));
    if (D > 0.0) {
        ref = -ni * (nV - N * dt) - N * sqrt(D);
        return true;
    }
    return false;
}

[shader("closesthit")]
void ClosestHit_Sphere(inout RayPayload payload, in MyAttr attr) {
    if (payload.recursion > MAX_RECURSION) {
        return;
    }

    float3 N = getNormal(getIndices(), attr);

    float3 worldPos = hitWorldPosition();

    float3 L = normalize(g_sceneCB.lightPosition.xyz - worldPos);
    RayDesc shadowRay;
    shadowRay.Origin = worldPos;
    shadowRay.Direction = L;
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

    float3 reflected = reflect(WorldRayDirection(), N);
    float3 refracted;
    float3 R;
    N = dot(WorldRayDirection(), N) > 0 ? -N : N;
    if (refract(-WorldRayDirection(), N, 1.0 / 1.5f, refracted)) {
        R = refracted;
    }
    else {
        return;
    }

    //float2 UV = getUV(getIndices(), attr);
    RayPayload secondPayload = { payload.color,payload.recursion + 1 };
    RayDesc secondRay;
    secondRay.Origin = hitWorldPosition();
    secondRay.Direction = R;
    secondRay.TMin = 0.01;
    secondRay.TMax = 10000.0;
    TraceRay(
        g_scene,
        RAY_FLAG_NONE,
        ~0,
        0,
        1,
        0,
        secondRay,
        secondPayload);

    float4 color = l_material.color * secondPayload.color;
    payload.color = color;

    //float factor = shadowPayload.hit ? 0.1 : 1.0;
    //float4 color = float4(Lambert(N, L, l_material.color.rgb * g_sceneCB.lightDiffuse.rgb), 1);

    //float3 V = normalize(g_sceneCB.cameraPosition.xyz - worldPos);
    //color.rgb += Specular(N, L, float3(1, 1, 1));
    //color += g_sceneCB.lightAmbient;

    //color = color * secondPayload.color;
    //color = color * factor;
    //payload.color = color;
}
#endif //! SHADER_RAYTRACING_CLOSESTHIT_HLSL