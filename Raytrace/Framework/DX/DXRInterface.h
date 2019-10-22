#pragma once
#include <d3d12.h>
#include "DX/DeviceResource.h"
#include "Libs/d3dx12.h"

namespace Framework::DX {
    struct DXRInfo {

    };

    /**
    * @class DXRInterface
    * @brief DXR用インターフェース
    */
    class DXRInterface {
    public:
        /**
        * @brief コンストラクタ
        */
        DXRInterface(DeviceResource* deviceResource, UINT width, UINT height);
        /**
        * @brief デストラクタ
        */
        ~DXRInterface();
        void createGlobalRootSignature(const CD3DX12_ROOT_SIGNATURE_DESC& desc);
        void createPipeline(CD3DX12_STATE_OBJECT_DESC& desc);

        void doRaytracing();
        void copyOutput();
        void buildShaderTable();
        void buildAS();
        void buildBLAS();
        void buildTLAS();
    //private:
        UINT allocateDescriptorHeap(D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, UINT descriptorIndexToUse);
    //private:
        DeviceResource* mDeviceResource;
        ComPtr<ID3D12Device5> mDXRDevice;
        ComPtr<ID3D12GraphicsCommandList5> mDXRCommandList;
        ComPtr<ID3D12StateObject> mDXRStateObject;
        ComPtr<ID3D12RootSignature> mGlobalRootSignature;
        //ディスクリプタヒープ
        ComPtr<ID3D12DescriptorHeap> mDescriptorHeap;
        UINT mDescriptorAllocated;
        UINT mDescriptorSize;
        ComPtr<ID3D12Resource> mBottomLevelAS;
        ComPtr<ID3D12Resource> mTopLevelAS;

        ComPtr<ID3D12Resource> mMissShaderTable;
        ComPtr<ID3D12Resource> mHitGroupShaderTable;
        ComPtr<ID3D12Resource> mRayGenShaderTable;

        UINT mWidth;
        UINT mHeight;

    };
} //Framework::DX 