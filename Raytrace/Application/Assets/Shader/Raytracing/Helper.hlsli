#ifndef SHADER_RAYTRACING_HELPER_HLSLI
#define SHADER_RAYTRACING_HELPER_HLSLI

#define HLSL
#include "RaytracingCompat.h"

struct Ray {
    Vector3 origin;
    Vector3 direction;
};

/**
* @brief カメラからのレイを生成する
*/
inline Ray generateCameraRay(in uint2 index, in float3 cameraPosition, in float4x4 projectionToWorld, in float2 offset = float2(0.5, 0.5)) {
    float2 xy = index + offset;
    //スクリーン座標に変換する
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;
    screenPos.y = -screenPos.y;

    //ワールド座標に変換する
    float4 world = mul(float4(screenPos, 0, 1), projectionToWorld);
    world.xyz /= world.w;

    Ray ray;
    ray.origin = cameraPosition;
    ray.direction = normalize(world.xyz - ray.origin);
    return ray;
}

/**
* @brief インデックスを読み込む
* @details インデックスは2バイトで登録されているがシェーダー内では4バイト単位でしか読み込めないため
*/
static inline uint3 loadIndices(uint offsetBytes, ByteAddressBuffer Indices) {
    uint3 indices;

    const uint alignedOffset = offsetBytes & ~3;
    const uint2 four16BitIndices = Indices.Load2(alignedOffset);

    if (alignedOffset == offsetBytes) {
        indices.x = four16BitIndices.x & 0xffff;
        indices.y = (four16BitIndices.x >> 16) & 0xffff;
        indices.z = four16BitIndices.y & 0xffff;
    }
    else {
        indices.x = (four16BitIndices.x >> 16) & 0xffff;
        indices.y = four16BitIndices.y & 0xffff;
        indices.z = (four16BitIndices.y >> 16) & 0xffff;
    }
    return indices;
}

/**
* @brief 衝突点のワールド座標を取得する
*/
inline float3 hitWorldPosition() {
    return WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
}

/**
* @brief 衝突点の法線を取得する
*/
inline float3 getNormal(float3 normals[3], in MyAttr attr) {
    return normals[0] +
        attr.barycentrics.x * (normals[1] - normals[0]) +
        attr.barycentrics.y * (normals[2] - normals[0]);
}

/**
* @brief 衝突点のUVを取得する
*/
inline float2 getUV(float2 uvs[3], in MyAttr attr) {
    return uvs[0] +
        attr.barycentrics.x * (uvs[1] - uvs[0]) +
        attr.barycentrics.y * (uvs[2] - uvs[0]);
}

/**
* @brief ランバードを計算する
*/
inline float3 Lambert(in float3 N, float3 L, float3 diffuse) {
    const float dotNL = max(0.0, dot(N, L));
    return diffuse * dotNL;
}

/**
* @brief スペキュラを計算する
*/
inline float3 Specular(in float3 N, in float3 L, in float3 specular) {
    L = normalize(L);
    N = normalize(N);
    const float dotNL = saturate(dot(L, N));
    return specular * pow(dotNL, 30);
}


#endif //! SHADER_RAYTRACING_HELPER_HLSLI