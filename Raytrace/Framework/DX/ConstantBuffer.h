#pragma once
#include "DX/DXHelper.h"

namespace Framework::DX {

/**
* @class ConstantBuffer
* @brief コンスタントバッファ
*/
    template <class T>
    class ConstantBuffer : public GPUUploadBuffer {
    public:
        /**
        * @brief コンストラクタ
        */
        ConstantBuffer()
            :mMappedData(nullptr), mAlignedInstanceSize(0), mNumInstances(0) { }
        /**
        * @brief デストラクタ
        */
        ~ConstantBuffer() { }

        void create(ID3D12Device* device, UINT numInstances = 1, LPCWSTR resourceName = nullptr) {
            mNumInstances = numInstances;
            mAlignedInstanceSize = calculateConstantBufferByteSize(sizeof(T));
            UINT bufferSize = numInstances * mAlignedInstanceSize;
            allocate(device, bufferSize, resourceName);
            mMappedData = mapCPUWriteOnly();
        }

        void copyStagingToGPU(UINT instanceIndex = 0) {
            memcpy(mMappedData + instanceIndex * mAlignedInstanceSize, &staging, sizeof(T));
        }
        UINT numInstances() { return mNumInstances; }
        D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress(UINT instanceIndex = 0) {
            return mResource->GetGPUVirtualAddress() + instanceIndex * mAlignedInstanceSize;
        }
    public:
        T staging;
        T* operator->() { return &staging; }
    private:
        uint8_t* mMappedData;
        UINT mAlignedInstanceSize;
        UINT mNumInstances;
    };

    template <class T>
    class StructuredBuffer : public GPUUploadBuffer {
    public:
        static_assert(sizeof(T) % 16 == 0, L"Align structure buffers on 16 byte boundary for performance reasons.");
        StructuredBuffer() : mMappedBuffers(nullptr), mNumInstances(0) { }
        void create(ID3D12Device* device, UINT numElements, UINT numInstances = 1, LPCWSTR resourceName = nullptr) {
            mStagings.resize(numElements);
            UINT bufferSize = numInstances * numElements * sizeof(T);
            allocate(device, bufferSize, resourceName);
            mMappedBuffers = reinterpret_cast<T*>(mapCPUWriteOnly());
        }
        void copyStagingToGPU(UINT instanceIndex = 0) {
            memcpy(mMappedBuffers + instanceIndex * numElementPerInstance(), &mStagings[0], instanceSize());
        }

        T& operator[](UINT elementIndex) { return mStagings[elementIndex]; }
        size_t numElementPerInstance() { return mStagings.size(); }
        UINT numInstances() { return mStagings.size(); }
        size_t instanceSize() { return numElementPerInstance() * sizeof(T); }
        D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress(UINT instanceIndex = 0) {
            return mResource->GetGPUVirtualAddress() + instanceIndex * instanceSize();
        }
    private:
        T* mMappedBuffers;
        std::vector<T> mStagings;
        UINT mNumInstances;
    };
} //Framework::DX 