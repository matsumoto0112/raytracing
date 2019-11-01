#ifndef SHADER_RAYTRACING_CLOSESTHIT_HLSL
#define SHADER_RAYTRACING_CLOSESTHIT_HLSL

#define HLSL
#include "Global.hlsli"
#include "RaytracingCompat.h"
#include "ClosestHit_Local.h"
#include "Helper.hlsli"

[shader("closesthit")]
void ClosestHit_Sphere(inout RayPayload payload, in MyAttr attr) {
    if (payload.recursion > MAX_RECURSION) {
        return;
    }

    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride + l_material.indexOffset;

    uint3 indices = loadIndices(baseIndex, Indices) + l_material.vertexOffset;
    //ñ@ê¸ÇÃéÊìæ
    float3 normals[3] =
    {
        Vertices[indices[0]].normal,
        Vertices[indices[1]].normal,
        Vertices[indices[2]].normal,
    };

    float3 N = getNormal(normals, attr);

    float3 worldPos = hitWorldPosition();

    float3 L = normalize(g_sceneCB.lightPosition.xyz - worldPos);
    RayDesc shadowRay;
    shadowRay.Origin = hitWorldPosition();
    shadowRay.Direction = L;
    shadowRay.TMin = 0.01;
    shadowRay.TMax = 10000.0;
    ShadowPayload shadowPayload = { false };
    TraceRay(
        g_scene,
        RAY_FLAG_NONE,
        ~0,
        1,
        1,
        1,
        shadowRay,
        shadowPayload);

    float factor = shadowPayload.hit ? 0.1 : 1.0;

    float2 uvs[3] =
    {
        Vertices[indices[0]].uv,
        Vertices[indices[1]].uv,
        Vertices[indices[2]].uv,
    };

    float2 UV = getUV(uvs, attr);
    float3 texColor = l_texture.SampleLevel(samLinear, UV, 0.0).rgb;
    float4 color = float4(Lambert(N, L, texColor), 1);

    float3 V = normalize(g_sceneCB.cameraPosition.xyz - worldPos);
    color.rgb += Specular(N, L, float3(1, 1, 1));
    color.rgb += g_sceneCB.lightAmbient.rgb;
    color = saturate(color);
    color = color * factor;

    //payload.color = color;
    payload.color = float4(0, 0, 0, 1);

}
#endif //! SHADER_RAYTRACING_CLOSESTHIT_HLSL