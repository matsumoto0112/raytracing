#include "DXRInterface.h"
#include "DX/DXHelper.h"

namespace Framework::DX {
    //�R���X�g���N�^
    DXRInterface::DXRInterface(DeviceResource* deviceResource)
        :mDeviceResource(deviceResource) {
        recreate();
    }

    //�f�X�g���N�^
    DXRInterface::~DXRInterface() {
        clear();
    }

    void DXRInterface::createStateObject(CD3DX12_STATE_OBJECT_DESC& desc) {
        Utility::throwIfFailed(mDXRDevice->CreateStateObject(desc, IID_PPV_ARGS(&mDXRStateObject)), L"StateObject�쐬���s");
        mDXRStateObject->SetName(L"DXRStateObject");
    }

    void DXRInterface::clear() {
        mDXRDevice.Reset();
        mDXRCommandList.Reset();
        mDXRStateObject.Reset();
    }

    void DXRInterface::recreate() {
        ID3D12Device* device = mDeviceResource->getDevice();
        ID3D12CommandList* list = mDeviceResource->getCommandList();

        Utility::throwIfFailed(device->QueryInterface(IID_PPV_ARGS(&mDXRDevice)), L"Couldn't get DirectX Raytracing interface for the device.\n");
        Utility::throwIfFailed(list->QueryInterface(IID_PPV_ARGS(&mDXRCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");
        mDXRDevice->SetName(L"DXRDevice");
        mDXRCommandList->SetName(L"DXRCommandList");
    }
} //Framework::DX 