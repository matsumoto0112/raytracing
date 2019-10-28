#pragma once
#include <d3d12.h>
#include "Utility/Typedef.h"

namespace Framework::DX {

    namespace HeapType {
        enum Enum {
            CBV_SRV_UAV = 0,
            Sampler,
            RTV,
            DSV,
        };
    } //HeapType 

    namespace HeapFlag {
        enum Enum {
            None = 0,
            ShaderVisible,
        };
    } //HeapFlag 

    /**
    * @class DescriptorTable
    * @brief discription
    */
    class DescriptorTable {
    public:
        /**
        * @brief コンストラクタ
        */
        DescriptorTable(ID3D12Device* device, HeapType::Enum heapType, HeapFlag::Enum heapFlag, UINT descriptorNum);
        /**
        * @brief デストラクタ
        */
        ~DescriptorTable();

        void create(ID3D12Device* device, HeapType::Enum heapType, HeapFlag::Enum heapFlag, UINT descriptorNum);
        void reset();

        ID3D12DescriptorHeap* getHeap() const { return mHeap.Get(); }

        UINT allocate(D3D12_CPU_DESCRIPTOR_HANDLE* handle);
        UINT allocateWithGPU(D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle);
    private:
        ComPtr<ID3D12DescriptorHeap> mHeap;
        UINT mDescriptorSize;
        UINT mAllocatedNum;
    };
} //Framework::DX 