#include "DescriptorTable.h"
#include "Utility/Debug.h"
#include "Libs/d3dx12.h"

namespace Framework::DX {

    DescriptorTable::DescriptorTable(ID3D12Device* device, HeapType::Enum heapType, HeapFlag::Enum heapFlag, UINT descriptorNum)
        :mHeap(nullptr), mDescriptorSize(0), mAllocatedNum(0) {
        create(device, heapType, heapFlag, descriptorNum);
    }

    DescriptorTable::~DescriptorTable() { }

    void DescriptorTable::create(ID3D12Device* device, HeapType::Enum heapType, HeapFlag::Enum heapFlag, UINT descriptorNum) {
        constexpr auto getHeapType = [](HeapType::Enum type) {
            static constexpr D3D12_DESCRIPTOR_HEAP_TYPE heapTypes[] =
            {
                D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
                D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            };
            return heapTypes[type];
        };

        constexpr auto getHeapFlag = [](HeapFlag::Enum flag) {
            static constexpr D3D12_DESCRIPTOR_HEAP_FLAGS heapFlags[] =
            {
                D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
            };
            return heapFlags[flag];
        };

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = descriptorNum;
        heapDesc.Type = getHeapType(heapType);
        heapDesc.Flags = getHeapFlag(heapFlag);
        heapDesc.NodeMask = 0;
        device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mHeap));
        mDescriptorSize = device->GetDescriptorHandleIncrementSize(heapDesc.Type);
    }

    void DescriptorTable::reset() {
        mHeap.Reset();
        mDescriptorSize = 0;
        mAllocatedNum = 0;
    }

    UINT DescriptorTable::allocate(D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle) {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = mHeap->GetCPUDescriptorHandleForHeapStart();
        MY_THROW_IF_FALSE(mAllocatedNum < mHeap->GetDesc().NumDescriptors, L"Š„‚è“–‚Ä‚½ƒq[ƒv”‚ð’´‚¦‚Ä‚¢‚Ü‚·");
        UINT index = mAllocatedNum++;
        *cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(handle, index, mDescriptorSize);
        return index;
    }

    UINT DescriptorTable::allocateWithGPU(D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle) {
        UINT index = allocate(cpuHandle);
        *gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeap->GetGPUDescriptorHandleForHeapStart(), index, mDescriptorSize);
        return index;
    }

} //Framework::DX 
