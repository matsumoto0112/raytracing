#include "DXRInterface.h"
#include "DX/DXHelper.h"

namespace Framework::DX {

    DXRInterface::DXRInterface(DeviceResource* deviceResource, UINT width, UINT height)
        :mDeviceResource(deviceResource), mWidth(width), mHeight(height) {
        ID3D12Device* device = mDeviceResource->getDevice();
        ID3D12CommandList* list = mDeviceResource->getCommandList();

        throwIfFailed(device->QueryInterface(IID_PPV_ARGS(&mDXRDevice)), L"Couldn't get DirectX Raytracing interface for the device.\n");
        throwIfFailed(list->QueryInterface(IID_PPV_ARGS(&mDXRCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = 3; //1つはレンダーターゲット、２つは頂点バッファ
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heapDesc.NodeMask = 0;
        device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mDescriptorHeap));
        mDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        DXGI_FORMAT format = mDeviceResource->getBackBufferFormat();
        CD3DX12_RESOURCE_DESC uavResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, mWidth, mHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

        CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT);
        throwIfFailed(device->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
            &uavResourceDesc,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            nullptr,
            IID_PPV_ARGS(&mRaytracingOutput)));

        D3D12_CPU_DESCRIPTOR_HANDLE uavHandle;
        mRaytracingOutputResourceUAVDescriptorHeapIndex = allocateDescriptor(&uavHandle, mRaytracingOutputResourceUAVDescriptorHeapIndex);

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;
        device->CreateUnorderedAccessView(mRaytracingOutput.Get(), nullptr, &uavDesc, uavHandle);
        mRaytracingOutputResourceUAVGpuDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(mDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), mRaytracingOutputResourceUAVDescriptorHeapIndex, mDescriptorSize);

    }

    DXRInterface::~DXRInterface() { }

    void DXRInterface::createGlobalRootSignature(const CD3DX12_ROOT_SIGNATURE_DESC& desc) {
        ID3D12Device* device = mDeviceResource->getDevice();

        ComPtr<ID3DBlob> blob, error;
        throwIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), L"ルートシグネチャ作成失敗");
        throwIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&mGlobalRootSignature)));
        mGlobalRootSignature->SetName(L"GlobalRootSignature");
    }

    void DXRInterface::createPipeline(CD3DX12_STATE_OBJECT_DESC& desc) {
        throwIfFailed(mDXRDevice->CreateStateObject(desc, IID_PPV_ARGS(&mDXRStateObject)), L"StateObject作成失敗");
    }

    void DXRInterface::doRaytracing() {
        ID3D12GraphicsCommandList* list = mDeviceResource->getCommandList();
        UINT frameCount = mDeviceResource->getCurrentFrameIndex();
        auto dispatchRays = [&](ID3D12GraphicsCommandList5* list, ID3D12StateObject* state, D3D12_DISPATCH_RAYS_DESC* desc) {
            desc->HitGroupTable.StartAddress = mHitGroupShaderTable->GetGPUVirtualAddress();
            desc->HitGroupTable.SizeInBytes = mHitGroupShaderTable->GetDesc().Width;
            desc->HitGroupTable.StrideInBytes = desc->HitGroupTable.SizeInBytes;

            desc->MissShaderTable.StartAddress = mMissShaderTable->GetGPUVirtualAddress();
            desc->MissShaderTable.SizeInBytes = mMissShaderTable->GetDesc().Width;
            desc->MissShaderTable.StrideInBytes = desc->MissShaderTable.SizeInBytes;

            desc->RayGenerationShaderRecord.StartAddress = mRayGenShaderTable->GetGPUVirtualAddress();
            desc->RayGenerationShaderRecord.SizeInBytes = mRayGenShaderTable->GetDesc().Width;

            desc->Width = mWidth;
            desc->Height = mHeight;
            desc->Depth = 1;

            list->SetPipelineState1(state);
            list->DispatchRays(desc);
        };

        list->SetComputeRootSignature(mRaytracingGlobalRootSignature.Get());
        mSceneCB.copyStagingToGPU(frameCount);
        list->SetComputeRootConstantBufferView(GlobalRootSignatureParameter::ConstantBuffer, mSceneCB.gpuVirtualAddress(frameCount));

        D3D12_DISPATCH_RAYS_DESC desc = {};
        list->SetDescriptorHeaps(1, mDescriptorHeap.GetAddressOf());
        list->SetComputeRootDescriptorTable(GlobalRootSignatureParameter::RenderTarget, mRaytracingOutputResourceUAVGpuDescriptor);
        list->SetComputeRootShaderResourceView(GlobalRootSignatureParameter::AccelerationStructureSlot, mTopLevelAS->GetGPUVirtualAddress());
        list->SetComputeRootDescriptorTable(GlobalRootSignatureParameter::VertexBuffers, mIndexBuffer.gpuHandle);
        dispatchRays(mDXRCommandList.Get(), mDXRStateObject.Get(), &desc);
    }

    void DXRInterface::copyOutput() {
        ID3D12GraphicsCommandList* list = mDeviceResource->getCommandList();
        ID3D12Resource* renderTarget = mDeviceResource->getRenderTarget();

        D3D12_RESOURCE_BARRIER preBarrier[2];
        preBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
        preBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(mRaytracingOutput.Get(),
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE);

        list->ResourceBarrier(ARRAYSIZE(preBarrier), preBarrier);

        list->CopyResource(renderTarget, mRaytracingOutput.Get());

        D3D12_RESOURCE_BARRIER postBarrier[2];
        postBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT);
        postBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(mRaytracingOutput.Get(),
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        list->ResourceBarrier(ARRAYSIZE(postBarrier), postBarrier);
    }

    void DXRInterface::buildShaderTable() {
        ID3D12Device* device = mDeviceResource->getDevice();

        void* rayGenShaderID;
        void* missShaderID;
        void* hitGroupShaderID;
        UINT shaderIDSize;

        ComPtr<ID3D12StateObjectProperties> props;
        throwIfFailed(mDXRStateObject.As(&props));
        rayGenShaderID = props->GetShaderIdentifier(RAY_GEN_SHADER_NAME);
        missShaderID = props->GetShaderIdentifier(MISS_SHADER_NAME);
        hitGroupShaderID = props->GetShaderIdentifier(HIT_GROUP_NAME);
        shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

        {
            UINT numShaderRecords = 1;
            UINT shaderRecordSize = shaderIDSize;
            ShaderTable table(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
            table.push_back(ShaderRecord(rayGenShaderID, shaderIDSize));
            mRayGenShaderTable = table.getResource();
        }

        {
            UINT numShaderRecords = 1;
            UINT shaderRecordSize = shaderIDSize;
            ShaderTable table(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");
            table.push_back(ShaderRecord(missShaderID, shaderIDSize));
            mMissShaderTable = table.getResource();
        }

        {
            UINT numShaderRecords = 1;
            UINT shaderRecordSize = shaderIDSize;
            ShaderTable table(device, numShaderRecords, shaderRecordSize, L"HitGroupTable");
            table.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize));
            mHitGroupShaderTable = table.getResource();
        }
    }

    void DXRInterface::buildAS() {
        auto device = mDeviceResource->getDevice();
        auto commandList = mDeviceResource->getCommandList();
        auto commandQueue = mDeviceResource->getCommandQueue();
        auto commandAllocator = mDeviceResource->getCommandAllocator();
        // Reset the command list for the acceleration structure construction.
        commandList->Reset(commandAllocator, nullptr);

        D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
        geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        geometryDesc.Triangles.IndexBuffer = mIndexBuffer.resource->GetGPUVirtualAddress();
        geometryDesc.Triangles.IndexCount = static_cast<UINT>(mIndexBuffer.resource->GetDesc().Width) / sizeof(Index);
        geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
        geometryDesc.Triangles.Transform3x4 = 0;
        geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        geometryDesc.Triangles.VertexCount = static_cast<UINT>(mVertexBuffer.resource->GetDesc().Width) / sizeof(Vertex);
        geometryDesc.Triangles.VertexBuffer.StartAddress = mVertexBuffer.resource->GetGPUVirtualAddress();
        geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);

        // Mark the geometry as opaque. 
        // PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
        // Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
        geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

        // Get required sizes for an acceleration structure.
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
        topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        topLevelInputs.Flags = buildFlags;
        topLevelInputs.NumDescs = 1;
        topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
        mDXRDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = topLevelInputs;
        bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        bottomLevelInputs.pGeometryDescs = &geometryDesc;
        mDXRDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);

        ComPtr<ID3D12Resource> scratchResource;
        allocateUAVBuffer(device, std::max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes), &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

        // Allocate resources for acceleration structures.
        // Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
        // Default heap is OK since the application doesn稚 need CPU read/write access to them. 
        // The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
        // and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
        //  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
        //  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
        {
            D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

            allocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &mBottomLevelAS, initialResourceState, L"BottomLevelAccelerationStructure");
            allocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &mTopLevelAS, initialResourceState, L"TopLevelAccelerationStructure");
        }

        // Create an instance desc for the bottom-level acceleration structure.
        ComPtr<ID3D12Resource> instanceDescs;
        D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
        instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
        instanceDesc.InstanceMask = 1;
        instanceDesc.AccelerationStructure = mBottomLevelAS->GetGPUVirtualAddress();
        allocateUploadBuffer(device, &instanceDesc, sizeof(instanceDesc), &instanceDescs, L"InstanceDescs");

        // Bottom Level Acceleration Structure desc
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
        {
            bottomLevelBuildDesc.Inputs = bottomLevelInputs;
            bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
            bottomLevelBuildDesc.DestAccelerationStructureData = mBottomLevelAS->GetGPUVirtualAddress();
        }

        // Top Level Acceleration Structure desc
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
        {
            topLevelInputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
            topLevelBuildDesc.Inputs = topLevelInputs;
            topLevelBuildDesc.DestAccelerationStructureData = mTopLevelAS->GetGPUVirtualAddress();
            topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
        }

        auto BuildAccelerationStructure = [&](auto* raytracingCommandList) {
            raytracingCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(mBottomLevelAS.Get()));
            raytracingCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
        };

        // Build acceleration structure.
        BuildAccelerationStructure(mDXRCommandList.Get());

        // Kick off acceleration structure construction.
        mDeviceResource->executeCommandList();

        // Wait for GPU to finish as the locally created temporary GPU resources will get released once we go out of scope.
        mDeviceResource->waitForGPU();
    }

    void DXRInterface::buildBLAS() {

    }

    void DXRInterface::buildTLAS() { }

    UINT DXRInterface::allocateDescriptorHeap(D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, UINT descriptorIndexToUse) {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

        if (descriptorIndexToUse >= mDescriptorHeap->GetDesc().NumDescriptors) {
            throwIfFalse(mDescriptorAllocated < mDescriptorHeap->GetDesc().NumDescriptors, L"Ran out of descriptors on the heap!");
            descriptorIndexToUse = mDescriptorAllocated++;
        }
        *cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(handle, descriptorIndexToUse, mDescriptorSize);
        return descriptorIndexToUse;
    }

}

} //Framework::DX 
