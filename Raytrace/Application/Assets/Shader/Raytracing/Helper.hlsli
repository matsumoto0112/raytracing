#ifndef SHADER_RAYTRACING_HELPER_HLSLI
#define SHADER_RAYTRACING_HELPER_HLSLI

#define HLSL
#include "RaytracingCompat.h"

struct Ray {
    Vector3 origin;
    Vector3 direction;
};

/**
* @brief �J��������̃��C�𐶐�����
*/
inline Ray generateCameraRay(in uint2 index, in float3 cameraPosition, in float4x4 projectionToWorld, in float2 offset = float2(0.5, 0.5)) {
    float2 xy = index + offset;
    //�X�N���[�����W�ɕϊ�����
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;
    screenPos.y = -screenPos.y;

    //���[���h���W�ɕϊ�����
    float4 world = mul(float4(screenPos, 0, 1), projectionToWorld);
    world.xyz /= world.w;

    Ray ray;
    ray.origin = cameraPosition;
    ray.direction = normalize(world.xyz - ray.origin);
    return ray;
}

/**
* @brief �C���f�b�N�X��ǂݍ���
* @details �C���f�b�N�X��2�o�C�g�œo�^����Ă��邪�V�F�[�_�[���ł�4�o�C�g�P�ʂł����ǂݍ��߂Ȃ�����
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
* @brief �Փ˓_�̃��[���h���W���擾����
*/
inline float3 hitWorldPosition() {
    return WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
}

#endif //! SHADER_RAYTRACING_HELPER_HLSLI