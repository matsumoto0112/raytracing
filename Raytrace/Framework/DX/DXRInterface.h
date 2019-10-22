#pragma once
#include "DX/DeviceResource.h"

namespace Framework::DX {
    /**
    * @class DXRInterface
    * @brief DXRインターフェース
    */
    class DXRInterface {
    public:
        /**
        * @brief コンストラクタ
        */
        DXRInterface(DeviceResource* deviceResource);
        /**
        * @brief デストラクタ
        */
        ~DXRInterface();

        void createStateObject(CD3DX12_STATE_OBJECT_DESC& desc);
        void clear();
        void recreate();
    public:
        ID3D12Device5* getDXRDevice() const { return mDXRDevice.Get(); }
        ID3D12GraphicsCommandList5* getCommandList() const { return mDXRCommandList.Get(); }
        ID3D12StateObject* getStateObject() const { return mDXRStateObject.Get(); }
    private:
        DeviceResource* mDeviceResource;
        ComPtr<ID3D12Device5> mDXRDevice;
        ComPtr<ID3D12GraphicsCommandList5> mDXRCommandList;
        ComPtr<ID3D12StateObject> mDXRStateObject;
    };
} //Framework::DX 