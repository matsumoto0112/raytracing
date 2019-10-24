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
ConstantBuffer<PowerConstantBuffer> l_power : register(b2);

typedef BuiltInTriangleIntersectionAttributes MyAttr;

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

//�L���[�u�ɓ���������
[shader("closesthit")]
void MyClosestHitShader_Cube(inout RayPayload payload, in MyAttr attr) {
    float4 color = float4(l_material.color);
    payload.color = color;
}

//�O�p�`�ɓ���������
[shader("closesthit")]
void MyClosestHitShader_Triangle(inout RayPayload payload, in MyAttr attr) {
    float4 color = float4(l_material.color);
    payload.color = color;
}

//���ɓ���������
[shader("closesthit")]
void MyClosestHitShader_Plane(inout RayPayload payload, in MyAttr attr) {
    float hitT = RayTCurrent();
    float3 worldRayDir = WorldRayDirection();
    float3 worldRayOrigin = WorldRayOrigin();

    float3 worldPos = worldRayOrigin + worldRayDir * hitT;

    RayDesc ray;
    ray.Origin = worldPos;
    ray.Direction = normalize(float3(-1, 1, 1));
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

    float factor = shadowPayload.hit ? 0.1 : 1.0;
    //float factor = 0.1f;
    float4 color = float4(l_material.color) * factor;


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