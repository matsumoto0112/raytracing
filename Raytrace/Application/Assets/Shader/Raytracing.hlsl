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
static const float A = 0.2f;

//テクスチャのサンプラー
SamplerState samLinear : register(s0);
//使用するテクスチャ
Texture2D tex : register(t3);


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

//ランバート反射の色を求める
inline float3 lambertColor(float3 N, float3 L, float3 diffuse) {
    float dotNL = max(0.0, dot(N, L));
    return diffuse.rgb * dotNL * (1.0 / PI);
}

inline float3 specular(float3 N, float3 L, float3 spec) {
    float dotNL = max(0.0, dot(N, L));
    return spec.rgb *pow(dotNL, 10);
}

//フォグを適用する
inline float4 applyFog(in float3 worldPos, in float4 color) {
    float len = distance(g_sceneCB.cameraPosition.xyz, worldPos);
    float fog = saturate((g_sceneCB.fogEnd - len) / (g_sceneCB.fogEnd - g_sceneCB.fogStart));
    color.rgb = color.rgb * fog + float3(1, 1, 1) * (1.0 - fog);
    color.a = 1.0;
    return color;
}

//影に覆われているか判定する
inline bool castShadow(Ray ray) {
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    rayDesc.TMin = 0.01f;
    rayDesc.TMax = 10000.0;
    ShadowPayload shadowPayload = { true };

    //影ができるかどうか
    TraceRay(g_scene,
        RAY_FLAG_NONE,
        ~0,
        1, //ヒットグループの影用シェーダー
        1,
        1, //影用のミスシェーダー
        rayDesc,
        shadowPayload);
    return shadowPayload.hit;
}

//頂点インデックスを取得する
inline uint3 getIndices(uint indexOffset) {
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride + indexOffset;
    return load3x16BitIndices(baseIndex, Indices);
}

//衝突した三角形の法線を取得する
inline float3 getNormal(in MyAttr attr, uint vertexOffset, uint indexOffset) {
    const uint3 indices = getIndices(indexOffset) + vertexOffset;
    float3 normals[3] =
    {
        Vertices[indices.x].normal,
        Vertices[indices.y].normal,
        Vertices[indices.z].normal,
    };

    return getHitAttribute(normals, attr);
}

inline float2 getUV(in MyAttr attr, uint vertexOffset, uint indexOffset) {
    const uint3 indices = getIndices(indexOffset) + vertexOffset;
    float2 uvs[3] =
    {
        Vertices[indices.x].uv,
        Vertices[indices.y].uv,
        Vertices[indices.z].uv,
    };
    return getUVHitAttribute(uvs, attr);
}

//キューブに当たった時
[shader("closesthit")]
void MyClosestHitShader_Cube(inout RayPayload payload, in MyAttr attr) {
    //再帰回数を制限する
    if (payload.recursion > 0) {
        payload.color = float4(0, 0, 0, 0);
        return;
    }

    float3 worldPos = hitWorldPosition();

    //影用のレイキャスト
    RayDesc ray;
    ray.Origin = worldPos;
    Ray shadowRay = { worldPos, normalize(g_sceneCB.lightPosition.xyz - worldPos) };
    bool shadow = castShadow(shadowRay);

    float3 N = getNormal(attr, l_material.vertexOffset, l_material.indexOffset);
    float3 L = normalize(g_sceneCB.lightPosition.xyz - worldPos);

    float4 color = float4(0, 0, 0, 0);
    float2 uv = getUV(attr, l_material.vertexOffset, l_material.indexOffset);
    float4 texColor = tex.SampleLevel(samLinear, uv, 0.0);
    //texColor = float4(1, 1, 1, 1);
    //ランバート
    color.rgb += lambertColor(N, L, g_sceneCB.lightDiffuse.rgb * texColor.rgb);

    color.rgb += specular(N, L, float3(1, 1, 1));

    //アンビエント
    color.rgb += g_sceneCB.lightAmbient.rgb;
    ////フォグの適用
    color = applyFog(hitWorldPosition(), color);

    //影に覆われていたら黒くする
    float factor = shadow ? 0.1f : 1.0f;
    //float factor = 1.0f;
    payload.color = color * factor;
}

//床に当たった時
[shader("closesthit")]
void MyClosestHitShader_Plane(inout RayPayload payload, in MyAttr attr) {
    //再帰回数を制限する
    if (payload.recursion > 0) {
        payload.color = float4(0, 0, 0, 0);
        return;
    }

    float3 worldPos = hitWorldPosition();

    //影用のレイキャスト
    float3 L = normalize(g_sceneCB.lightPosition.xyz - worldPos);
    Ray shadowRay = { worldPos, L };
    bool shadow = castShadow(shadowRay);

    float3 N = getNormal(attr, l_material.vertexOffset, l_material.indexOffset);

    float4 color = float4(0, 0, 0, 0);
    float2 uv = getUV(attr, l_material.vertexOffset, l_material.indexOffset);
    float4 texColor = tex.SampleLevel(samLinear, uv, 0.0);

     //ランバート
    color.rgb += lambertColor(N, L, g_sceneCB.lightDiffuse.rgb * texColor.rgb);
    //アンビエント
    color.rgb += g_sceneCB.lightAmbient.rgb;
    ////フォグの適用
    color = applyFog(hitWorldPosition(), color);

    //影に覆われていたら黒くする
    float factor = shadow ? 0.1f : 1.0f;
    payload.color = color * factor;
}

//影用のレイ
[shader("closesthit")]
void MyClosestHitShader_Shadow(inout ShadowPayload payload, in MyAttr attr) {
    payload.hit = true;
}

//何もヒットしなかったときの処理
[shader("miss")]
void MyMissShader(inout RayPayload payload) {
    float4 back = float4(144.0 / 255.0, 215.0 / 255.0, 236.0 / 255.0, 1);
    payload.color = back;
}

//影用のレイの何もヒットしなかったときの処理
[shader("miss")]
void MyMissShader_Shadow(inout ShadowPayload payload) {
    payload.hit = false;
}

#endif //! INCLUDE_SHADER_RAYTRACING_HLSL