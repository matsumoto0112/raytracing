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
#include "Framework/Math/Vector2.h"
#include "Framework/Math/Vector3.h"
#include "Framework/Math/Vector4.h"
#include "Framework/Utility/Color4.h"
//cppファイルの方ならDirectXMathを利用する
using namespace DirectX;
using Framework::Math::Vector2;
using Framework::Math::Vector3;
using Framework::Math::Vector4;
using Framework::Utility::Color4;
//インデックスは16bitにする
typedef UINT16 Index;
#endif

/**
} @brief シーン全体の情報用コンスタントバッファ
*/
struct SceneConstantBuffer {
    XMMATRIX projectionToWorld;
    Color4 lightAmbient;
    Color4 lightDiffuse;
    Vector4 lightPosition;
    Vector4 cameraPosition;
    float fogStart;
    float fogEnd;
};

/**
* @brief 頂点情報
*/
struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
};

/**
* @brief レイの当たった時につかうデータ
*/
struct RayPayload {
    Color4 color;
    UINT recursion;
};

/**
* @brief 影用
*/
struct ShadowPayload {
    bool hit;
};


struct MaterialConstantBuffer {
    Color4 color;
    UINT indexOffset;
    UINT vertexOffset;
};

/**
* @brief ローカルのルートシグネチャパラメータ
* @details シェーダーファイルの一部でのみ使用するパラメータ
*/
namespace LocalRootSignatureParams {
    namespace Type {
        enum MyEnum {
            AABB,
            Plane,
            Count
        };
    } //Type 
    namespace AABB {
        enum MyEnum {
            Material = 0,
            Texture,
            Count
        };
        struct RootArgument {
            MaterialConstantBuffer material;
        };
    } //AABB
    namespace Plane {
        enum MyEnum {
            Material = 0,
            Texture,
            Count
        };
        struct RootArgument {
            MaterialConstantBuffer material;
        };
    } //Plane 
} //LocalRootSignatureParams



#endif // RAYTRACINGHLSLCOMPAT_H