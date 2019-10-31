#ifndef SHADER_RAYTRACING_CLOSESTHIT_PLANE_HLSL
#define SHADER_RAYTRACING_CLOSESTHIT_PLANE_HLSL

#define HLSL
#include "Global.hlsli"
#include "RaytracingCompat.h"
#include "Local.h"
#include "Helper.hlsli"

typedef  HitGroupParams::LocalRootSignatureParams::Constant::MaterialConstantBuffer Material;

ConstantBuffer<Material> l_material : register(b1);

Texture2D<float4> l_texture : register(t3);

inline float3 getNormal(float3 normals[3], in MyAttr attr) {
    return normals[0] +
        attr.barycentrics.x * (normals[1] - normals[0]) +
        attr.barycentrics.y * (normals[2] - normals[0]);
}

inline float2 getUV(float2 uvs[3], in MyAttr attr) {
    return uvs[0] +
        attr.barycentrics.x * (uvs[1] - uvs[0]) +
        attr.barycentrics.y * (uvs[2] - uvs[0]);
}

inline float3 Lambert(in float3 N, float3 L, float3 diffuse) {
    const float dotNL = max(0.0, dot(N, L));
    return diffuse * dotNL;
}

inline float3 Specular(in float3 N, in float3 L, in float3 V, in float3 specular) {
    L = normalize(L);
    N = normalize(N);
    V = normalize(V);
    const float dotNL = saturate(dot(reflect(L, V), N));
    return specular * pow(dotNL, 1000.0);
}

[shader("closesthit")]
void ClosestHit_Plane(inout RayPayload payload, in MyAttr attr) {
    // Get the base index of the triangle's first 16 bit index.
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
    float4 color = float4(Lambert(N, L, g_sceneCB.lightDiffuse.rgb), 1);

    float3 V = normalize(g_sceneCB.cameraPosition.xyz - worldPos);
    color.rgb += Specular(N, L, V, float3(1, 1, 1));
    color = color * factor;

    g_renderTarget[DispatchRaysIndex().xy] = color;
}
#endif //! SHADER_RAYTRACING_CLOSESTHIT_PLANE_HLSL