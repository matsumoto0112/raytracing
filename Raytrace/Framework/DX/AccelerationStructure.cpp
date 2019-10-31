#include "AccelerationStructure.h"
#include "Utility/Debug.h"
namespace {
    inline ComPtr<ID3D12Resource> createBuffer(ID3D12Device5* device, UINT size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps) {
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
}

namespace Framework::DX {
    AccelerationStructure::AccelerationStructure() { }
    AccelerationStructure::~AccelerationStructure() { }

    void AccelerationStructure::addBLASBuffer(ID3D12Device5 * device, const VertexList& vertices, const IndexList& indices) {
        Buffer buffer;
        const UINT indexSize = UINT(indices.size()) * sizeof(indices[0]);
        const UINT vertexSize = UINT(vertices.size()) * sizeof(vertices[0]);
        allocateUploadBuffer(device, (void*)indices.data(), indexSize, &buffer.indexBuffer.resource);
        allocateUploadBuffer(device, (void*)vertices.data(), vertexSize, &buffer.vertexBuffer.resource);
        mBuffers.emplace_back(buffer);
    }

    AccelerationStructure::ID AccelerationStructure::buildBLAS(ID3D12Device5* device, ID3D12GraphicsCommandList5* commandList) {

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
        inputs.NumDescs = UINT(size);
        inputs.pGeometryDescs = geomertyDescs.data();
        inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild = {};
        device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);

        AccelerationStructureBuffers buffers;

        buffers.scratch = createBuffer(
            device, (UINT)prebuild.ScratchDataSizeInBytes,
            D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON,
            CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT));
        buffers.scratch->SetName(L"Scratch");

        buffers.accelerationStructure = createBuffer(
            device, (UINT)prebuild.ResultDataMaxSizeInBytes,
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

        UINT id = (UINT)mBottomLevelASs.size();
        mBottomLevelASs.emplace_back(buffers);

        mBufferStacks.insert(mBufferStacks.end(), mBuffers.begin(), mBuffers.end());
        mBuffers.clear();
        return id;
    }

    void AccelerationStructure::tlasConfig(ID3D12Device5* device, UINT tlasNum) {
        mInputs = {};
        mInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT::D3D12_ELEMENTS_LAYOUT_ARRAY;
        mInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
        mInputs.NumDescs = tlasNum;
        mInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO pre;
        device->GetRaytracingAccelerationStructurePrebuildInfo(&mInputs, &pre);

        mTLASTmpBuffer.scratch.Reset();
        mTLASTmpBuffer.accelerationStructure.Reset();
        mTLASTmpBuffer.instanceDesc.Reset();

        mTLASTmpBuffer.scratch = createBuffer(device, (UINT)pre.ScratchDataSizeInBytes,
            D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT));
        mTLASTmpBuffer.accelerationStructure = createBuffer(device, (UINT)pre.ResultDataMaxSizeInBytes,
            D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
            CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT));
        mTLASSize = (UINT)pre.ResultDataMaxSizeInBytes;


        mTLASTmpBuffer.instanceDesc = createBuffer(device, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * tlasNum,
            D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
            CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD));

        MY_THROW_IF_FAILED(mTLASTmpBuffer.instanceDesc->Map(0, nullptr, (void**)&mMappedInstanceDescs), L"Map failed");
        ZeroMemory(mMappedInstanceDescs, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * tlasNum);
        mTLASIndex = 0;
    }

    void AccelerationStructure::addTLASBuffer(UINT id, UINT instanceID, UINT hitGroupIndex, const XMMATRIX& transform) {
        mMappedInstanceDescs[mTLASIndex].InstanceID = instanceID;
        mMappedInstanceDescs[mTLASIndex].InstanceContributionToHitGroupIndex = hitGroupIndex;
        mMappedInstanceDescs[mTLASIndex].Flags = D3D12_RAYTRACING_INSTANCE_FLAGS::D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
        XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(mMappedInstanceDescs[mTLASIndex].Transform), transform);
        mMappedInstanceDescs[mTLASIndex].AccelerationStructure = mBottomLevelASs[id].accelerationStructure->GetGPUVirtualAddress();
        mMappedInstanceDescs[mTLASIndex].InstanceMask = 0xff;

        mTLASIndex++;
    }
    void AccelerationStructure::buildTLAS(ID3D12GraphicsCommandList5* commandList) {
        mTLASTmpBuffer.instanceDesc->Unmap(0, nullptr);

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
        asDesc.Inputs = mInputs;
        asDesc.Inputs.InstanceDescs = mTLASTmpBuffer.instanceDesc->GetGPUVirtualAddress();
        asDesc.DestAccelerationStructureData = mTLASTmpBuffer.accelerationStructure->GetGPUVirtualAddress();
        asDesc.ScratchAccelerationStructureData = mTLASTmpBuffer.scratch->GetGPUVirtualAddress();

        commandList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = mTLASTmpBuffer.accelerationStructure.Get();
        commandList->ResourceBarrier(1, &barrier);
    }

    void AccelerationStructure::setDescriptorTable(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex) {
        mTopLevelAS = mTLASTmpBuffer.accelerationStructure;
        commandList->SetComputeRootShaderResourceView(rootParameterIndex, mTopLevelAS->GetGPUVirtualAddress());
    }
} //Framework::DX 
