#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include "DX/DXHelper.h"
#include "Utility/Typedef.h"

namespace Framework::DX {
    struct AccelerationStructureBuffers {
        ComPtr<ID3D12Resource> scratch;
        ComPtr<ID3D12Resource> accelerationStructure;
        ComPtr<ID3D12Resource> instanceDesc;
        UINT64 resultDataMaxSizeInBytes;
    };

    /**
    * @class ShaderRecord
    * @brief discription
    */
    class ShaderRecord {
    public:
        /**
        * @brief コンストラクタ
        */
        ShaderRecord(void* shaderIdentifier, UINT shaderIdentifierSize)
            :shaderIdentifier(shaderIdentifier, shaderIdentifierSize) { }
        ShaderRecord(void* shaderIdentifier, UINT shaderIdentifierSize, void* localRootArguments, UINT localRootArgumentsSize)
            :shaderIdentifier(shaderIdentifier, shaderIdentifierSize), localRootArguments(localRootArguments, localRootArgumentsSize) { }
        void copyTo(void* dest) const {
            uint8_t*  byteDest = static_cast<uint8_t*>(dest);
            memcpy(byteDest, shaderIdentifier.ptr, shaderIdentifier.size);
            if (localRootArguments.ptr) {
                memcpy(byteDest + shaderIdentifier.size, localRootArguments.ptr, localRootArguments.size);
            }
        }
        struct PointerWithSize {
            void* ptr;
            UINT size;

            PointerWithSize() :ptr(nullptr), size(0) { }
            PointerWithSize(void* ptr, UINT size) :ptr(ptr), size(size) { }
        };
        PointerWithSize shaderIdentifier;
        PointerWithSize localRootArguments;
    };

    class ShaderTable : public GPUUploadBuffer {
        uint8_t* mMappedShaderRecords;
        UINT mShaderRecordSize;

        std::wstring mName;
        std::vector<ShaderRecord> mShaderRecords;

    public:
        ShaderTable(ID3D12Device* device, UINT numShaderRecords, UINT shaderRecordSize, LPCWSTR resourceName = nullptr)
            :mName(resourceName) {
            mShaderRecordSize = align(shaderRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
            mShaderRecords.reserve(numShaderRecords);
            UINT bufferSize = numShaderRecords * mShaderRecordSize;
            allocate(device, bufferSize, resourceName);
            mMappedShaderRecords = mapCPUWriteOnly();
        }

        void push_back(const ShaderRecord& record) {
            throwIfFailed(mShaderRecords.size() < mShaderRecords.capacity());
            mShaderRecords.push_back(record);
            record.copyTo(mMappedShaderRecords);
            mMappedShaderRecords += mShaderRecordSize;
        }

        UINT getShaderRecordSize() { return mShaderRecordSize; }
    };

    inline void allocateUAVBuffer(ID3D12Device* device, UINT64 bufferSize, ID3D12Resource** resource,
        D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON, const wchar_t* resourceName = nullptr) {
        CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        throwIfFailed(device->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            initialResourceState,
            nullptr,
            IID_PPV_ARGS(resource)));
        if (resourceName) {
            (*resource)->SetName(resourceName);
        }
    }

    inline void allocateUploadBuffer(ID3D12Device* device, void* data, UINT64 dataSize, ID3D12Resource** resource, const wchar_t* resourceName = nullptr) {
        CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize);
        throwIfFailed(device->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(resource)));

        if (resourceName) {
            (*resource)->SetName(resourceName);
        }

        void* mapped;
        (*resource)->Map(0, nullptr, &mapped);
        memcpy(mapped, data, dataSize);
        (*resource)->Unmap(0, nullptr);
    }

    inline float NumMRaysPerSecond(UINT width, UINT height, float dispatchRaysTimeMs)
    {
        float resolutionMRays = static_cast<float>(width * height);
        float raytracingTimeInSeconds = 0.001f * dispatchRaysTimeMs;
        return resolutionMRays / (raytracingTimeInSeconds * static_cast<float>(1e6));
    }

} //Framework::DX 