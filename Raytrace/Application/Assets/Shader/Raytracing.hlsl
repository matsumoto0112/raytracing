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
static const float A = 0.2f;

//�e�N�X�`���̃T���v���[
SamplerState samLinear : register(s0);
//�g�p����e�N�X�`��
Texture2D tex : register(t3);


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

//�����o�[�g���˂̐F�����߂�
inline float3 lambertColor(float3 N, float3 L, float3 diffuse) {
    float dotNL = max(0.0, dot(N, L));
    return diffuse.rgb * dotNL * (1.0 / PI);
}

inline float3 specular(float3 N, float3 L, float3 spec) {
    float dotNL = max(0.0, dot(N, L));
    return spec.rgb *pow(dotNL, 10);
}

//�t�H�O��K�p����
inline float4 applyFog(in float3 worldPos, in float4 color) {
    float len = distance(g_sceneCB.cameraPosition.xyz, worldPos);
    float fog = saturate((g_sceneCB.fogEnd - len) / (g_sceneCB.fogEnd - g_sceneCB.fogStart));
    color.rgb = color.rgb * fog + float3(1, 1, 1) * (1.0 - fog);
    color.a = 1.0;
    return color;
}

//�e�ɕ����Ă��邩���肷��
inline bool castShadow(Ray ray) {
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    rayDesc.TMin = 0.01f;
    rayDesc.TMax = 10000.0;
    ShadowPayload shadowPayload = { true };

    //�e���ł��邩�ǂ���
    TraceRay(g_scene,
        RAY_FLAG_NONE,
        ~0,
        1, //�q�b�g�O���[�v�̉e�p�V�F�[�_�[
        1,
        1, //�e�p�̃~�X�V�F�[�_�[
        rayDesc,
        shadowPayload);
    return shadowPayload.hit;
}

//���_�C���f�b�N�X���擾����
inline uint3 getIndices(uint indexOffset) {
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride + indexOffset;
    return load3x16BitIndices(baseIndex, Indices);
}

//�Փ˂����O�p�`�̖@�����擾����
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

//�L���[�u�ɓ���������
[shader("closesthit")]
void MyClosestHitShader_Cube(inout RayPayload payload, in MyAttr attr) {
    //�ċA�񐔂𐧌�����
    if (payload.recursion > 0) {
        payload.color = float4(0, 0, 0, 0);
        return;
    }

    float3 worldPos = hitWorldPosition();

    //�e�p�̃��C�L���X�g
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
    //�����o�[�g
    color.rgb += lambertColor(N, L, g_sceneCB.lightDiffuse.rgb * texColor.rgb);

    color.rgb += specular(N, L, float3(1, 1, 1));

    //�A���r�G���g
    color.rgb += g_sceneCB.lightAmbient.rgb;
    ////�t�H�O�̓K�p
    color = applyFog(hitWorldPosition(), color);

    //�e�ɕ����Ă����獕������
    float factor = shadow ? 0.1f : 1.0f;
    //float factor = 1.0f;
    payload.color = color * factor;
}

//���ɓ���������
[shader("closesthit")]
void MyClosestHitShader_Plane(inout RayPayload payload, in MyAttr attr) {
    //�ċA�񐔂𐧌�����
    if (payload.recursion > 0) {
        payload.color = float4(0, 0, 0, 0);
        return;
    }

    float3 worldPos = hitWorldPosition();

    //�e�p�̃��C�L���X�g
    float3 L = normalize(g_sceneCB.lightPosition.xyz - worldPos);
    Ray shadowRay = { worldPos, L };
    bool shadow = castShadow(shadowRay);

    float3 N = getNormal(attr, l_material.vertexOffset, l_material.indexOffset);

    float4 color = float4(0, 0, 0, 0);
    float2 uv = getUV(attr, l_material.vertexOffset, l_material.indexOffset);
    float4 texColor = tex.SampleLevel(samLinear, uv, 0.0);

     //�����o�[�g
    color.rgb += lambertColor(N, L, g_sceneCB.lightDiffuse.rgb * texColor.rgb);
    //�A���r�G���g
    color.rgb += g_sceneCB.lightAmbient.rgb;
    ////�t�H�O�̓K�p
    color = applyFog(hitWorldPosition(), color);

    //�e�ɕ����Ă����獕������
    float factor = shadow ? 0.1f : 1.0f;
    payload.color = color * factor;
}

//�e�p�̃��C
[shader("closesthit")]
void MyClosestHitShader_Shadow(inout ShadowPayload payload, in MyAttr attr) {
    payload.hit = true;
}

//�����q�b�g���Ȃ������Ƃ��̏���
[shader("miss")]
void MyMissShader(inout RayPayload payload) {
    float4 back = float4(144.0 / 255.0, 215.0 / 255.0, 236.0 / 255.0, 1);
    payload.color = back;
}

//�e�p�̃��C�̉����q�b�g���Ȃ������Ƃ��̏���
[shader("miss")]
void MyMissShader_Shadow(inout ShadowPayload payload) {
    payload.hit = false;
}

#endif //! INCLUDE_SHADER_RAYTRACING_HLSL