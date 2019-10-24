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

//closesthit�ň����Ƃ��Ď󂯎��O�p�`�̏d�S
typedef BuiltInTriangleIntersectionAttributes MyAttr;

static const float FOG_START = 0.1f;
static const float FOG_END = 100.0f;

//���C����
[shader("raygeneration")]
void MyRaygenShader() {
    Ray ray = generateCameraRay(DispatchRaysIndex().xy, g_sceneCB.cameraPosition.xyz, g_sceneCB.projectionToWorld);

    //���C���΂�
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    rayDesc.TMin = 0.001f;
    rayDesc.TMax = 10000.0;

    RayPayload payload = { float4(0,0,0,0),0 };
    TraceRay(g_scene, //AS
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES,//���C�̃t���O �o�b�N�J�����O��L����
        ~0, //�}�X�N�Ȃ� ���ׂĂɃq�b�g����悤��
        0, //�q�b�g�O���[�v�̃I�t�Z�b�g
        1, //�悭�킩��� 1���g���炵��
        0, //�~�X�V�F�[�_�[�̃C���f�b�N�X
        rayDesc, //���C
        payload); //�߂�l

    g_renderTarget[DispatchRaysIndex().xy] = payload.color;
}

inline float4 getFinalColor(float3 N, float3 L, float3 worldPos, float4 materialColor) {
    float dotNL = max(0.0, dot(N, L));
    float4 lambert = g_sceneCB.lightDiffuse * dotNL;
    float4 color = float4(0, 0, 0, 1);
    color.rgb += lambert.rgb;
    color.rgb += g_sceneCB.lightAmbient.rgb;
    color *= materialColor;

    float len = distance(g_sceneCB.cameraPosition.xyz, worldPos);
    float fog = saturate((FOG_END - len) / (FOG_END - FOG_START));
    color.rgb = color.rgb * fog + float3(1, 1, 1) * (1.0 - fog);
    color.a = 1.0;
    return color;
}

//�L���[�u�ɓ���������
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

    payload.color = getFinalColor(N, L, hitWorldPosition(), l_material.color);
}

//���ɓ���������
[shader("closesthit")]
void MyClosestHitShader_Plane(inout RayPayload payload, in MyAttr attr) {
    float3 worldPos = hitWorldPosition();

    RayDesc ray;
    ray.Origin = worldPos;
    ray.Direction = normalize(g_sceneCB.lightPosition.xyz - worldPos);
    ray.TMin = 0.01f;
    ray.TMax = 10000.0;
    ShadowPayload shadowPayload = { true };

    //�e���ł��邩�ǂ���
    TraceRay(g_scene,
        RAY_FLAG_NONE,
        ~0,
        1, //�q�b�g�O���[�v�̉e�p�V�F�[�_�[
        1,
        1, //�e�p�̃~�X�V�F�[�_�[
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
    float factor = shadowPayload.hit ? 0.1f : 1.0f;
    payload.color = getFinalColor(N, L, hitWorldPosition(), l_material.color) * factor;
}

[shader("closesthit")]
void MyClosestHitShader_Shadow(inout ShadowPayload payload, in MyAttr attr) {
    payload.hit = true;
}

[shader("miss")]
void MyMissShader(inout RayPayload payload) {
    float4 back = float4(1, 1, 1, 1);
    payload.color = back;
}

[shader("miss")]
void MyMissShader_Shadow(inout ShadowPayload payload) {
    payload.hit = false;
}

#endif //! INCLUDE_SHADER_RAYTRACING_HLSL