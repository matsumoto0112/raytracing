#pragma once
#include <vector>
#include <d3d12.h>
#include "DX/DescriptorTable.h"
#include "DX/DXHelper.h"

namespace Framework::DX {
    /**
    * @brief テクスチャデータ
    */
    struct TextureData {
        std::vector<BYTE> data; //!< テクスチャの中身
        UINT width; //!< テクスチャの幅
        UINT height; //!< テクスチャの高さ
        UINT textureSizePerPixel; //!< ピクセル単位のバイトの大きさ
    };

    /**
    * @class Texture2D
    * @brief テクスチャ2D
    */
    class Texture2D {
    public:
        /**
        * @brief コンストラクタ
        * @device デバイス
        * @param descriptor このテクスチャを管理するディスクリプタヒープ
        * @param textureData 使用するテクスチャデータ
        */
        Texture2D(ID3D12Device* device, DescriptorTable* descriptor, const TextureData& textureData);
        /**
        * @brief デストラクタ
        */
        ~Texture2D();
        /**
        * @brief CPUハンドルを取得する
        */
        const D3D12_CPU_DESCRIPTOR_HANDLE& getCPUHandle() const { return mTextureResource.cpuHandle; }
        /**
        * @brief GPUハンドルを取得する
        */
        const D3D12_GPU_DESCRIPTOR_HANDLE& getGPUHandle() const { return mTextureResource.gpuHandle; }
    private:
        D3DBuffer mTextureResource; //!< テクスチャリソース
    };

} //Framework::DX 