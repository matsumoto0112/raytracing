#include "Texture2D.h"
#include "Utility/Debug.h"

namespace Framework::DX {
    //コンストラクタ
    Texture2D::Texture2D(ID3D12Device* device, DescriptorTable* descriptor, const TextureData& textureData) {
        //テクスチャリソースを作成する
        CD3DX12_RESOURCE_DESC texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, textureData.width, textureData.height, 1);

        D3D12_HEAP_PROPERTIES heapProp = {};
        heapProp.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_CUSTOM;
        heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
        heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_L0;
        heapProp.VisibleNodeMask = 1;
        heapProp.CreationNodeMask = 1;

        MY_THROW_IF_FAILED(device->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&mTextureResource.resource)), L"");

        //テクスチャデータを書き込む
        D3D12_BOX box = { 0,0,0,textureData.width ,textureData.height,1 };
        UINT row = textureData.width * textureData.textureSizePerPixel;
        UINT slice = row * textureData.height;
        MY_THROW_IF_FAILED(mTextureResource.resource->WriteToSubresource(
            0,
            &box,
            textureData.data.data(), row, slice),
            L"");

        //シェーダーリソースビューを作成する
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = texDesc.Format;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.PlaneSlice = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        UINT index = descriptor->allocateWithGPU(&mTextureResource.cpuHandle, &mTextureResource.gpuHandle);
        device->CreateShaderResourceView(mTextureResource.resource.Get(), &srvDesc, mTextureResource.cpuHandle);
    }
    //デストラクタ
    Texture2D::~Texture2D() { }

} //Framework::DX 
