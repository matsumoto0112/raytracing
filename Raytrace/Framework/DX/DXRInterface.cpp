#include "DXRInterface.h"
#include "DX/DXHelper.h"

namespace Framework::DX {
    //コンストラクタ
    DXRInterface::DXRInterface(DeviceResource* deviceResource)
        :mDeviceResource(deviceResource) {
        recreate();
    }

    //デストラクタ
    DXRInterface::~DXRInterface() { 
        clear();
    }

    void DXRInterface::createStateObject(CD3DX12_STATE_OBJECT_DESC& desc) {
        throwIfFailed(mDXRDevice->CreateStateObject(desc, IID_PPV_ARGS(&mDXRStateObject)), L"StateObject作成失敗");
    }

    void DXRInterface::clear() {
        mDXRDevice.Reset();
        mDXRCommandList.Reset();
        mDXRStateObject.Reset();
    }

    void DXRInterface::recreate() {
        ID3D12Device* device = mDeviceResource->getDevice();
        ID3D12CommandList* list = mDeviceResource->getCommandList();

        throwIfFailed(device->QueryInterface(IID_PPV_ARGS(&mDXRDevice)), L"Couldn't get DirectX Raytracing interface for the device.\n");
        throwIfFailed(list->QueryInterface(IID_PPV_ARGS(&mDXRCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");
    }

} //Framework::DX 