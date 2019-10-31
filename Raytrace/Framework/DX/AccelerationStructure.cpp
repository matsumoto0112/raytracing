#include "AccelerationStructure.h"
#include "Utility/Debug.h"

namespace Framework::DX {
    AccelerationStructure::AccelerationStructure() { }
    AccelerationStructure::~AccelerationStructure() { }
    //void AccelerationStructure::addBLASBuffer(ID3D12Device* device, VertexList vertices, IndexList indices) {
    //    Buffer buffer;
    //    allocateUploadBuffer(device, indices.data(), indices.size() * sizeof(Index), &buffer.indexBuffer.resource);
    //    allocateUploadBuffer(device, vertices.data(), vertices.size() * sizeof(Vertex2), &buffer.vertexBuffer.resource);
    //    mBuffers.emplace_back(buffer);
    //}

    void AccelerationStructure::addBLASBuffer(ID3D12Device * device, void * indices, size_t indexSize, void * vertices, size_t vertexSize) {
        Buffer buffer;
        allocateUploadBuffer(device, indices, indexSize, &buffer.indexBuffer.resource);
        allocateUploadBuffer(device, vertices, vertexSize, &buffer.vertexBuffer.resource);
        mBuffers.emplace_back(buffer);
    }

    AccelerationStructure::ID AccelerationStructure::buildBLAS(ID3D12Device5* device, ID3D12GraphicsCommandList5* commandList) {
        auto createBuffer = [](ID3D12Device* device, UINT size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps) {
            CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size, flags);
            ComPtr<ID3D12Resource> buffer;
            Framework::Utility::throwIfFailed(device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                initState,
                nullptr,
                IID_PPV_ARGS(&buffer)));

            return buffer;
        };

        const size_t size = mBuffers.size();

        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geomertyDescs(size);
        for (size_t i = 0; i < size; i++) {
            D3D12_RAYTRACING_GEOMETRY_DESC& desc = geomertyDescs[i];
            desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE::D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
            desc.Triangles.IndexBuffer = mBuffers[i].indexBuffer.resource->GetGPUVirtualAddress();
            desc.Triangles.IndexCount = static_cast<UINT>(mBuffers[i].indexBuffer.resource->GetDesc().Width / sizeof(Index));
            desc.Triangles.IndexFormat = DXGI_FORMAT::DXGI_FORMAT_R16_UINT;
            desc.Triangles.VertexBuffer.StartAddress = mBuffers[i].vertexBuffer.resource->GetGPUVirtualAddress();
            desc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
            desc.Triangles.VertexCount = static_cast<UINT>(mBuffers[i].vertexBuffer.resource->GetDesc().Width / sizeof(Vertex));
            desc.Triangles.VertexFormat = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;

            desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAGS::D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
        }

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
        inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT::D3D12_ELEMENTS_LAYOUT_ARRAY;
        inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
        inputs.NumDescs = size;
        inputs.pGeometryDescs = geomertyDescs.data();
        inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild = {};
        device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);

        AccelerationStructureBuffers buffers;

        buffers.scratch = createBuffer(
            device, prebuild.ScratchDataSizeInBytes,
            D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON,
            CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT));
        buffers.scratch->SetName(L"Scratch");

        buffers.accelerationStructure = createBuffer(
            device, prebuild.ResultDataMaxSizeInBytes,
            D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
            CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT));
        buffers.accelerationStructure->SetName(L"AccelerationStructure");

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
        asDesc.Inputs = inputs;
        asDesc.DestAccelerationStructureData = buffers.accelerationStructure->GetGPUVirtualAddress();
        asDesc.ScratchAccelerationStructureData = buffers.scratch->GetGPUVirtualAddress();

        commandList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = buffers.accelerationStructure.Get();
        commandList->ResourceBarrier(1, &barrier);

        UINT id = mBottomLevelASs.size();
        mBottomLevelASs.emplace_back(buffers);

        mBufferStacks.insert(mBufferStacks.end(), mBuffers.begin(), mBuffers.end());
        mBuffers.clear();
        return id;
    }
} //Framework::DX 
