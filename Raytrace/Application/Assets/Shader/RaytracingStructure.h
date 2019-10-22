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
};

#endif // RAYTRACINGHLSLCOMPAT_H