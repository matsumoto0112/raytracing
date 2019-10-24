#ifndef INCLUDE_SHADER_RAYTRACING_HLSL
#define INCLUDE_SHADER_RAYTRACING_HLSL

#define HLSL
#include "RaytracingStructure.h"
#include "RaytracingHelper.hlsli"

RaytracingAccelerationStructure g_scene : register(t0, space0);
ByteAddressBuffer Indices : register(t1, space0);
StructuredBuffer<Vertex> Vertices : register(t2, space0);

RWTexture2D<float4> g_renderTarget : register(u0);

ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
ConstantBuffer<MaterialConstantBuffer> l_material : register(b1);

//closesthitで引数として受け取る三角形の重心
typedef BuiltInTriangleIntersectionAttributes MyAttr;

//レイ生成
[shader("raygeneration")]
void MyRaygenShader() {
    Ray ray = generateCameraRay(DispatchRaysIndex().xy, g_sceneCB.cameraPosition.xyz, g_sceneCB.projectionToWorld);

    //レイを飛ばす
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    rayDesc.TMin = 0.001f;
    rayDesc.TMax = 10000.0;

    RayPayload payload = { float4(0,0,0,0),0 };
    TraceRay(g_scene, //AS
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES,//レイのフラグ バックカリングを有効に
        ~0, //マスクなし すべてにヒットするように
        0, //ヒットグループのオフセット
        1, //よくわからん 1を使うらしい
        0, //ミスシェーダーのインデックス
        rayDesc, //レイ
        payload); //戻り値

    g_renderTarget[DispatchRaysIndex().xy] = payload.color;
}

//キューブに当たった時
[shader("closesthit")]
void MyClosestHitShader_Cube(inout RayPayload payload, in MyAttr attr) {
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;

    const uint3 indices = load3x16BitIndices(baseIndex, Indices);
    float3 normals[3] =
    {
        Vertices[indices[0]].normal,
        Vertices[indices[1]].normal,
        Vertices[indices[2]].normal,
    };
    float3 N = getHitAttribute(normals, attr);
    float3 L = normalize(g_sceneCB.lightPosition.xyz - hitWorldPosition());
    float dotNL = max(0.0, dot(N, L));

    float4 lambert = g_sceneCB.lightDiffuse * dotNL;
    float4 color = float4(0, 0, 0, 1);
    color.rgb += lambert.rgb;
    color.rgb += g_sceneCB.lightAmbient.rgb;
    color.rgb *= l_material.color.rgb;
    color.a = 1.0;

    payload.color = color;
}

//床に当たった時
[shader("closesthit")]
void MyClosestHitShader_Plane(inout RayPayload payload, in MyAttr attr) {
    float3 worldPos = hitWorldPosition();

    RayDesc ray;
    ray.Origin = worldPos;
    ray.Direction = normalize(g_sceneCB.lightPosition.xyz - worldPos);
    ray.TMin = 0.01f;
    ray.TMax = 10000.0;
    ShadowPayload shadowPayload = { true };

    //影ができるかどうか
    TraceRay(g_scene,
        RAY_FLAG_NONE,
        ~0,
        1, //ヒットグループの影用シェーダー
        1,
        1, //影用のミスシェーダー
        ray,
        shadowPayload);

    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;
    const uint3 indices = load3x16BitIndices(baseIndex, Indices);
    float3 normals[3] =
    {
        Vertices[indices[0]].normal,
        Vertices[indices[1]].normal,
        Vertices[indices[2]].normal,
    };
    float3 N = getHitAttribute(normals, attr);
    float3 L = normalize(g_sceneCB.lightPosition.xyz - hitWorldPosition());
    float dotNL = max(0.0, dot(N, L));

    float4 lambert = g_sceneCB.lightDiffuse * dotNL;

    float factor = shadowPayload.hit ? 0.1 : 1.0;
    float4 color = lambert;
    color.rgb += g_sceneCB.lightAmbient.rgb;
    color.rgb *= factor;
    color.a = 1.0;

    payload.color = color;
}

[shader("closesthit")]
void MyClosestHitShader_Shadow(inout ShadowPayload payload, in MyAttr attr) {
    payload.hit = true;
}

[shader("miss")]
void MyMissShader(inout RayPayload payload) {
    float4 back = float4(0, 0.8, 0.8, 1.0f);
    payload.color = back;
}

[shader("miss")]
void MyMissShader_Shadow(inout ShadowPayload payload) {
    payload.hit = false;
}

#endif //! INCLUDE_SHADER_RAYTRACING_HLSL