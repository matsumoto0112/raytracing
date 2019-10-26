//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#ifndef RAYTRACINGHLSLCOMPAT_H
#define RAYTRACINGHLSLCOMPAT_H

//HLSL�t�@�C���̂ق��ł�Typedef�ŉ��̌^����������
#ifdef HLSL
#include "RaytracingTypedef.h"
#include "RaytracingDefine.h"
#else
//cpp�t�@�C���̕��Ȃ�DirectXMath�𗘗p����
using namespace DirectX;
//�C���f�b�N�X��16bit�ɂ���
typedef UINT16 Index;
#endif

/**
} @brief �V�[���S�̂̏��p�R���X�^���g�o�b�t�@
*/
struct SceneConstantBuffer {
    XMMATRIX projectionToWorld;
    XMVECTOR cameraPosition;
    XMVECTOR lightPosition;
    XMVECTOR lightAmbient;
    XMVECTOR lightDiffuse;
    float fogStart;
    float fogEnd;
};

/**
* @brief ���_���
*/
struct Vertex {
    XMFLOAT3 position;
    XMFLOAT3 normal;
};

struct TextureVertex {
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT2 uv;
};


/**
* @brief ���C�̓����������ɂ����f�[�^
*/
struct RayPayload {
    XMFLOAT4 color;
    UINT recursion;
};

/**
* @brief �e�p
*/
struct ShadowPayload {
    bool hit;
};


struct MaterialConstantBuffer {
    XMFLOAT4 color;
    UINT indexOffset;
    UINT vertexOffset;
};

/**
* @brief ���[�J���̃��[�g�V�O�l�`���p�����[�^
* @details �V�F�[�_�[�t�@�C���̈ꕔ�ł̂ݎg�p����p�����[�^
*/
namespace LocalRootSignatureParams {
    namespace Type {
        enum MyEnum {
            AABB,
            Triangle,
            Plane,
            Count
        };
    } //Type 
    namespace AABB {
        enum MyEnum {
            Material = 0,
            Count
        };
        struct RootArgument {
            MaterialConstantBuffer material;
        };
    } //AABB
    namespace Plane {
        enum MyEnum {
            Material = 0,
            Count
        };
        struct RootArgument {
            MaterialConstantBuffer material;
        };
    } //Plane 
} //LocalRootSignatureParams



#endif // RAYTRACINGHLSLCOMPAT_H