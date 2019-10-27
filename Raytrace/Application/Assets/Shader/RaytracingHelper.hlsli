/**
* @brief レイトレーシングHlsl用ヘルパー関数、構造体まとめ
*/

#ifndef INCLUDE_SHADER_RAYTRACINGHELPER_HLSLI
#define INCLUDE_SHADER_RAYTRACINGHELPER_HLSLI

struct Ray {
    float3 origin;
    float3 direction;
};

static uint3 load3x16BitIndices(uint offsetBytes, ByteAddressBuffer Indices) {
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

inline Ray generateCameraRay(uint2 index, in float3 cameraPosition, in float4x4 projectionToWorld) {
    float2 xy = index + 0.5f; // ピクセルの中心にずらす
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    screenPos.y = -screenPos.y;

    float4 world = mul(float4(screenPos, 0, 1), projectionToWorld);
    world.xyz /= world.w;

    Ray ray;
    ray.origin = cameraPosition;
    ray.direction = normalize(world.xyz - ray.origin);
    return ray;
}

inline float3 hitWorldPosition() {
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

inline float3 rotVectorByQuat(float3 v, float4 q) {
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

inline float3 getHitAttribute(in float3 normal[3], in BuiltInTriangleIntersectionAttributes attr) {
    return normal[0] +
        (attr.barycentrics.x * (normal[1] - normal[0])) +
        (attr.barycentrics.y * (normal[2] - normal[0]));
}

inline float2 getUVHitAttribute(in float2 uv[3], in BuiltInTriangleIntersectionAttributes attr) {
    return uv[0] +
        (attr.barycentrics.x * (uv[1] - uv[0])) +
        (attr.barycentrics.y * (uv[2] - uv[0]));
}

#endif //! INCLUDE_SHADER_RAYTRACINGHELPER_HLSLI
