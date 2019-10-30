#ifndef SHADER_RAYTRACING_GLOBAL_HLSLI
#define SHADER_RAYTRACING_GLOBAL_HLSLI

#define HLSL
#include "RaytracingCompat.h"

//Top-Level-AS�\����
RaytracingAccelerationStructure g_scene : register(t0, space0);

//�C���f�b�N�X�z��
ByteAddressBuffer Indices : register(t1, space0);
//���_�z��
StructuredBuffer<Vertex> Vertices : register(t2, space0);
//�����_�[�^�[�Q�b�g
RWTexture2D<float4> g_renderTarget : register(u0);

//�V�[�����
ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);

//closesthit�Ȃǂň����Ƃ��Ď󂯎��O�p�`�̏d�S
typedef BuiltInTriangleIntersectionAttributes MyAttr;

#endif //! SHADER_RAYTRACING_GLOBAL_HLSLI