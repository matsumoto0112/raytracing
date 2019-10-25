#pragma once
#include <d3d12.h>
#include <stdexcept>
#include "Libs/d3dx12.h"
#include "Utility/Debug.h"
#include "Utility/Typedef.h"

namespace Framework::DX {
#ifdef _DEBUG
    inline void setName(ID3D12Object* object, LPCWSTR name) {
        object->SetName(name);
    }
    inline void setNameIndexed(ID3D12Object* object, LPCWSTR name, UINT index) {
        static constexpr int MAX_NAME_LENGTH = 50;
        WCHAR fullName[MAX_NAME_LENGTH];
        if (swprintf_s(fullName, L"%s[%u]", name, index) > 0) {
            object->SetName(fullName);
        }
    }
#else
    inline void setName(ID3D12Object*, LPCWSTR) { }
    inline void setNameIndexed(ID3D12Object*, LPCWSTR, UINT) { }
#endif

    inline UINT align(UINT size, UINT alignment) {
        return (size + (alignment - 1)) & ~(alignment - 1);
    }

    inline UINT calculateConstantBufferByteSize(UINT byteSize) {
        return align(byteSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    }

    /**
    * @class GPUUploadBuffer
    * @brief GPUにアップロードするバッファ
    */
    class GPUUploadBuffer {
    public:
        /**
        * @brief リソースの取得
        */
        ComPtr<ID3D12Resource> getResource() { return mResource; }
        /**
        * @brief リソースの開放
        */
        virtual void release() { mResource.Reset(); }
    protected:
        /**
        * @brief コンストラクタ
        */
        GPUUploadBuffer() { }
        /**
        * @brief デストラクタ
        */
        ~GPUUploadBuffer() {
            if (mResource) {
                mResource->Unmap(0, nullptr);
            }
        }

        /**
        * @brief リソースの割り当て
        */
        void allocate(ID3D12Device* device, UINT bufferSize, LPCWSTR resourceName = nullptr) {
            CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD);

            CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
            throwIfFailed(device->CreateCommittedResource(
                &heapProp,
                D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&mResource)));
            mResource->SetName(resourceName);
        }

        /**
        * @brief 書き込み可能なメモリを返す
        */
        uint8_t* mapCPUWriteOnly() {
            uint8_t* mapped;
            CD3DX12_RANGE range(0, 0);
            throwIfFailed(mResource->Map(0, &range, reinterpret_cast<void**>(&mapped)));
            return mapped;
        }
    protected:
        ComPtr<ID3D12Resource> mResource; //!< リソース
    };

    /**
    * @brief バッファのパッチ
    */
    struct D3DBuffer {
        ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    };

} //Framework::DX 