#pragma once
#include <iomanip>
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
        * @brief ƒRƒ“ƒXƒgƒ‰ƒNƒ^
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
            Utility::throwIfFailed(mShaderRecords.size() < mShaderRecords.capacity());
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
        Utility::throwIfFailed(device->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            initialResourceState,
            nullptr,
            IID_PPV_ARGS(resource)));
        if (resourceName) {
            MY_THROW_IF_FAILED((*resource)->SetName(resourceName), L"SetNameŽ¸”s");
        }
    }

    inline void allocateUploadBuffer(ID3D12Device* device, void* data, UINT64 dataSize, ID3D12Resource** resource, const wchar_t* resourceName = nullptr) {
        CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize);
        Utility::throwIfFailed(device->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(resource)));

        if (resourceName) {
            MY_THROW_IF_FAILED((*resource)->SetName(resourceName), L"SetNameŽ¸”s");
        }

        void* mapped;
        MY_THROW_IF_FAILED((*resource)->Map(0, nullptr, &mapped), L"MapŽ¸”s");
        memcpy(mapped, data, dataSize);
        (*resource)->Unmap(0, nullptr);
    }

    inline float NumMRaysPerSecond(UINT width, UINT height, float dispatchRaysTimeMs) {
        float resolutionMRays = static_cast<float>(width * height);
        float raytracingTimeInSeconds = 0.001f * dispatchRaysTimeMs;
        return resolutionMRays / (raytracingTimeInSeconds * static_cast<float>(1e6));
    }

    // Pretty-print a state object tree.
    inline void printStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc) {
        std::wstringstream wstr;
        wstr << L"\n";
        wstr << L"--------------------------------------------------------------------\n";
        wstr << L"| D3D12 State Object 0x" << static_cast<const void*>(desc) << L": ";
        if (desc->Type == D3D12_STATE_OBJECT_TYPE_COLLECTION) wstr << L"Collection\n";
        if (desc->Type == D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE) wstr << L"Raytracing Pipeline\n";

        auto ExportTree = [](UINT depth, UINT numExports, const D3D12_EXPORT_DESC* exports) {
            std::wostringstream woss;
            for (UINT i = 0; i < numExports; i++) {
                woss << L"|";
                if (depth > 0) {
                    for (UINT j = 0; j < 2 * depth - 1; j++) woss << L" ";
                }
                woss << L" [" << i << L"]: ";
                if (exports[i].ExportToRename) woss << exports[i].ExportToRename << L" --> ";
                woss << exports[i].Name << L"\n";
            }
            return woss.str();
        };

        for (UINT i = 0; i < desc->NumSubobjects; i++) {
            wstr << L"| [" << i << L"]: ";
            switch (desc->pSubobjects[i].Type) {
                case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE:
                    wstr << L"Global Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
                    break;
                case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
                    wstr << L"Local Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
                    break;
                case D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK:
                    wstr << L"Node Mask: 0x" << std::hex << std::setfill(L'0') << std::setw(8) << *static_cast<const UINT*>(desc->pSubobjects[i].pDesc) << std::setw(0) << std::dec << L"\n";
                    break;
                case D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY:
                {
                    wstr << L"DXIL Library 0x";
                    auto lib = static_cast<const D3D12_DXIL_LIBRARY_DESC*>(desc->pSubobjects[i].pDesc);
                    wstr << lib->DXILLibrary.pShaderBytecode << L", " << lib->DXILLibrary.BytecodeLength << L" bytes\n";
                    wstr << ExportTree(1, lib->NumExports, lib->pExports);
                    break;
                }
                case D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION:
                {
                    wstr << L"Existing Library 0x";
                    auto collection = static_cast<const D3D12_EXISTING_COLLECTION_DESC*>(desc->pSubobjects[i].pDesc);
                    wstr << collection->pExistingCollection << L"\n";
                    wstr << ExportTree(1, collection->NumExports, collection->pExports);
                    break;
                }
                case D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
                {
                    wstr << L"Subobject to Exports Association (Subobject [";
                    auto association = static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
                    UINT index = static_cast<UINT>(association->pSubobjectToAssociate - desc->pSubobjects);
                    wstr << index << L"])\n";
                    for (UINT j = 0; j < association->NumExports; j++) {
                        wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
                    }
                    break;
                }
                case D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
                {
                    wstr << L"DXIL Subobjects to Exports Association (";
                    auto association = static_cast<const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
                    wstr << association->SubobjectToAssociate << L")\n";
                    for (UINT j = 0; j < association->NumExports; j++) {
                        wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
                    }
                    break;
                }
                case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
                {
                    wstr << L"Raytracing Shader Config\n";
                    auto config = static_cast<const D3D12_RAYTRACING_SHADER_CONFIG*>(desc->pSubobjects[i].pDesc);
                    wstr << L"|  [0]: Max Payload Size: " << config->MaxPayloadSizeInBytes << L" bytes\n";
                    wstr << L"|  [1]: Max Attribute Size: " << config->MaxAttributeSizeInBytes << L" bytes\n";
                    break;
                }
                case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
                {
                    wstr << L"Raytracing Pipeline Config\n";
                    auto config = static_cast<const D3D12_RAYTRACING_PIPELINE_CONFIG*>(desc->pSubobjects[i].pDesc);
                    wstr << L"|  [0]: Max Recursion Depth: " << config->MaxTraceRecursionDepth << L"\n";
                    break;
                }
                case D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP:
                {
                    wstr << L"Hit Group (";
                    auto hitGroup = static_cast<const D3D12_HIT_GROUP_DESC*>(desc->pSubobjects[i].pDesc);
                    wstr << (hitGroup->HitGroupExport ? hitGroup->HitGroupExport : L"[none]") << L")\n";
                    wstr << L"|  [0]: Any Hit Import: " << (hitGroup->AnyHitShaderImport ? hitGroup->AnyHitShaderImport : L"[none]") << L"\n";
                    wstr << L"|  [1]: Closest Hit Import: " << (hitGroup->ClosestHitShaderImport ? hitGroup->ClosestHitShaderImport : L"[none]") << L"\n";
                    wstr << L"|  [2]: Intersection Import: " << (hitGroup->IntersectionShaderImport ? hitGroup->IntersectionShaderImport : L"[none]") << L"\n";
                    break;
                }
            }
            wstr << L"|--------------------------------------------------------------------\n";
        }
        wstr << L"\n";
        OutputDebugStringW(wstr.str().c_str());
    }


} //Framework::DX 