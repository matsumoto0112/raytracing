#ifndef SHADER_RAYTRACING_CLOSESTHIT_HLSL
#define SHADER_RAYTRACING_CLOSESTHIT_HLSL

#define HLSL
#include "Global.hlsli"
#include "RaytracingCompat.h"
#include "Local.h"
#include "Helper.hlsli"

typedef  HitGroupParams::LocalRootSignatureParams::Constant::MaterialConstantBuffer Material;

ConstantBuffer<Material> l_material : register(b1);

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

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in MyAttr attr) {
    // Get the base index of the triangle's first 16 bit index.
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;

    uint3 indices = loadIndices(baseIndex, Indices);
    float3 normals[3] =
    {
        Vertices[indices[0]].normal,
        Vertices[indices[1]].normal,
        Vertices[indices[2]].normal,
    };

    float2 uvs[3] =
    {
        Vertices[indices[0]].uv,
        Vertices[indices[1]].uv,
        Vertices[indices[2]].uv,
    };

    float3 N = getNormal(normals, attr);
    N = N * 0.5 + 0.5;

    float2 UV = getUV(uvs, attr);

    float4 color = float4(UV, 0.0f, 1.0f);
    g_renderTarget[DispatchRaysIndex().xy] = color;
    //g_renderTarget[DispatchRaysIndex().xy] = g_sceneCB.lightAmbient;
    //payload.color = float4(1, 0, 0, 1);
}
#endif //! SHADER_RAYTRACING_CLOSESTHIT_HLSL