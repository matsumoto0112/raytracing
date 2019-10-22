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
};

/**
* @brief ���_���
*/
struct Vertex {
    XMFLOAT3 position;
    XMFLOAT3 normal;
};


/**
* @brief ���C�̓����������ɂ����f�[�^
*/
struct RayPayload {
    XMFLOAT4 color;
};

#endif // RAYTRACINGHLSLCOMPAT_H