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
        indices.y = (four16BitIndices.y >> 16) & 0xffff;
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



#endif //! INCLUDE_SHADER_RAYTRACINGHELPER_HLSLI
