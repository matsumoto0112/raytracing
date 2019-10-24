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

//HLSLファイルのほうではTypedefで下の型を処理する
#ifdef HLSL
#include "RaytracingTypedef.h"
#include "RaytracingDefine.h"
#else
//cppファイルの方ならDirectXMathを利用する
using namespace DirectX;
//インデックスは16bitにする
typedef UINT16 Index;
#endif

/**
} @brief シーン全体の情報用コンスタントバッファ
*/
struct SceneConstantBuffer {
    XMMATRIX projectionToWorld;
    XMVECTOR cameraPosition;
    XMVECTOR lightPosition;
    XMVECTOR lightAmbient;
    XMVECTOR lightDiffuse;
};

/**
* @brief 頂点情報
*/
struct Vertex {
    XMFLOAT3 position;
    XMFLOAT3 normal;
};


/**
* @brief レイの当たった時につかうデータ
*/
struct RayPayload {
    XMFLOAT4 color;
    UINT recursion;
};

/**
* @brief 影用
*/
struct ShadowPayload {
    bool hit;
};


struct MaterialConstantBuffer {
    XMFLOAT4 color;
};
struct PowerConstantBuffer {
    float power;
};

/**
* @brief ローカルのルートシグネチャパラメータ
* @details シェーダーファイルの一部でのみ使用するパラメータ
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
            Power,
            Count
        };
        struct RootArgument {
            MaterialConstantBuffer material;
            PowerConstantBuffer power;
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