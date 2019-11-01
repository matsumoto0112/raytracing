#ifndef SHADER_RAYTRACING_CLOSESTHIT_LOCAL_HLSLI
#define SHADER_RAYTRACING_CLOSESTHIT_LOCAL_HLSLI


#include "RaytracingTypedef.h"
#ifdef HLSL
#include "Helper.hlsli"
#endif

namespace HitGroupParams {
    namespace LocalRootSignatureParams {
        namespace Constant {
            struct MaterialConstantBuffer {

                Color4 color;
                UINT indexOffset;
                UINT vertexOffset;
#ifndef HLSL
                D3D12_GPU_DESCRIPTOR_HANDLE texture;
#endif
            };
        } //Constant 
    } //LocalRootSignatureParameter 
} //HitGroup 

#ifdef HLSL
typedef  HitGroupParams::LocalRootSignatureParams::Constant::MaterialConstantBuffer Material;
//マテリアル情報
ConstantBuffer<Material> l_material : register(b1);
//テクスチャ
Texture2D<float4> l_texture : register(t3);

static inline uint3 getIndices() {
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride + l_material.indexOffset;

    return loadIndices(baseIndex, Indices) + l_material.vertexOffset;
}

static inline float3 getNormal(in uint3 indices, in MyAttr attr) {
    //法線の取得
    float3 normals[3] =
    {
        Vertices[indices[0]].normal,
        Vertices[indices[1]].normal,
        Vertices[indices[2]].normal,
    };

    return getNormal(normals, attr);
}

static inline float2 getUV(in uint3 indices, in MyAttr attr) {
    float2 uvs[3] =
    {
        Vertices[indices[0]].uv,
        Vertices[indices[1]].uv,
        Vertices[indices[2]].uv,
    };

    return getUV(uvs, attr);
}
#endif

#endif //! SHADER_RAYTRACING_CLOSESTHIT_LOCAL_HLSLI