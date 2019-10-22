#ifndef INCLUDE_SHADER_RAYTRACING_HLSL
#define INCLUDE_SHADER_RAYTRACING_HLSL

#define HLSL
#include "RaytracingStructure.h"
#include "RaytracingHelper.hlsli"

RaytracingAccelerationStructure g_scene : register(t0, space0);
RWTexture2D<float4> g_renderTarget : register(u0);
ByteAddressBuffer Indices : register(t1, space0);
StructuredBuffer<Vertex> Vertices : register(t2, space0);

ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
typedef BuiltInTriangleIntersectionAttributes MyAttr;

float3 hitAttribute(float3 vertexAttribute[3], MyAttr attr) {
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

float3 calcLam(float3 color, float3 L, float3 N) {
    return color * max(0.0, dot(L, N)) * (1.0 / PI);
}

[shader("raygeneration")]
void MyRaygenShader() {
    Ray ray = generateCameraRay(DispatchRaysIndex().xy, g_sceneCB.cameraPosition.xyz, g_sceneCB.projectionToWorld);

    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    rayDesc.TMin = 0.001f;
    rayDesc.TMax = 10000.0;

    RayPayload payload = { float4(0,0,0,0) };
    TraceRay(g_scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, rayDesc, payload);

    g_renderTarget[DispatchRaysIndex().xy] = payload.color;
}

inline uint3 getIndex() {
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;

    return load3x16BitIndices(baseIndex, Indices);
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttr attr) {
    uint3 indices = getIndex();
    float3 normals[3] =
    {
        Vertices[indices[0]].normal,
        Vertices[indices[1]].normal,
        Vertices[indices[2]].normal,
    };

    float3 N = hitAttribute(normals, attr);
    float3 lightPosition = float3(0, 10, -10);
    float3 L = normalize(lightPosition - hitWorldPosition());

    float3 lightColor = float3(1, 0, 0);
    float4 color = float4(calcLam(lightColor, L, N), 1.0f);
    payload.color = color;
}

[shader("miss")]
void MyMissShader(inout RayPayload payload) {
    float4 back = float4(0, 0.8, 0.8, 1.0f);
    payload.color = back;
}

#endif //! INCLUDE_SHADER_RAYTRACING_HLSL