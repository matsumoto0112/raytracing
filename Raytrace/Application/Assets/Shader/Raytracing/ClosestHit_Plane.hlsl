#ifndef SHADER_RAYTRACING_CLOSESTHIT_PLANE_HLSL
#define SHADER_RAYTRACING_CLOSESTHIT_PLANE_HLSL

#define HLSL
#include "Global.hlsli"
#include "RaytracingCompat.h"
#include "ClosestHit_Local.h"
#include "Helper.hlsli"

[shader("closesthit")]
void ClosestHit_Plane(inout RayPayload payload, in MyAttr attr) {
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

    RayPayload secondPayload = { payload.color,payload.recursion + 1 };
    RayDesc secondRay;
    secondRay.Origin = hitWorldPosition();
    secondRay.Direction = reflect(WorldRayDirection(), N);
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



    float factor = shadowPayload.hit ? 0.1 : 1.0;
    float4 color = float4(Lambert(N, L,l_material.color.rgb * g_sceneCB.lightDiffuse.rgb), 1);

    float3 V = normalize(g_sceneCB.cameraPosition.xyz - worldPos);
    color.rgb += Specular(N, L, float3(1, 1, 1));
    color += g_sceneCB.lightAmbient;

    color = color * secondPayload.color;
    color = color * factor;
    payload.color = color;
}
#endif //! SHADER_RAYTRACING_CLOSESTHIT_PLANE_HLSL