#ifndef SHADER_RAYTRACING_GLOBAL_HLSLI
#define SHADER_RAYTRACING_GLOBAL_HLSLI

#define HLSL
#include "RaytracingCompat.h"

//Top-Level-AS構造体
RaytracingAccelerationStructure g_scene : register(t0, space0);

//インデックス配列
ByteAddressBuffer Indices : register(t1, space0);
//頂点配列
StructuredBuffer<Vertex> Vertices : register(t2, space0);
//レンダーターゲット
RWTexture2D<float4> g_renderTarget : register(u0);

//シーン情報
ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);

SamplerState samLinear : register(s0);

/**
* @brief 最大のレイの再帰回数
*/
#define MAX_RECURSION 3

#endif //! SHADER_RAYTRACING_GLOBAL_HLSLI