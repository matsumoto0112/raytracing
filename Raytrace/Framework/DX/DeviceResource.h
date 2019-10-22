#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <string>
#include <wrl.h>
#include "DX/IDeviceState.h"
#include "Libs/d3dx12.h"
#include "Utility/Typedef.h"
#include "Window/Window.h"

namespace Framework::DX {
    /**
    * @brief �f�o�C�X�̒ʒm��
    */
    interface IDeviceNotify {
        /**
        * @brief �f�o�C�X���X�g����
        */
        virtual void onDeviceLost() = 0;
        /**
        * @brief �f�o�C�X�̍ēo�^
        */
        virtual void onDeviceRestored() = 0;
    };

    /**
    * @class DeviceResource
    * @brief �f�o�C�X�̃��\�[�X�Ǘ�
    */
    class DeviceResource {
    public:
        /**
        * @brief �R���X�g���N�^
        * @param backBufferFormat �o�b�N�o�b�t�@�̃t�H�[�}�b�g
        * @param depthBufferFormat �[�x�E�X�e���V���o�b�t�@�̃t�H�[�}�b�g
        * @param backBufferCount �o�b�N�o�b�t�@�̖���
        * @param minFeatureLevel �v������f�o�C�X�̍ŏ��\�̓��x��
        * @param flags �X���b�v�`�F�C���̔\��
        */
        DeviceResource(DXGI_FORMAT backBufferFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
            DXGI_FORMAT depthBufferFormat = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT,
            UINT backBufferCount = 2,
            D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0,
            UINT flags = 0,
            UINT adapterIDoverride = UINT_MAX);
        /**
        * @brief �f�X�g���N�^
        */
        ~DeviceResource();
        /**
        * @brief �A�_�v�^�[�̏�����
        */
        void initializeDXGIAdapter();
        void setAdapterOverride(UINT adapterID) { mAdapterIDoverride = adapterID; }

        void createDeviceResource();
        void createWindowSizeDependentResource();
        void setWindow(Window::Window* window);
        bool windowSizeChange(UINT width, UINT height, bool minimized);
        void handleDeviceLost();
        void registerDeviceNotiry(IDeviceNotify* notify);
        void prepare(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT);
        void present(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
        void executeCommandList();
        void waitForGPU();
    public:
        RECT getOutputSize() const { return mOutputSize; }
        bool isWindowVisible() const { return mIsWindowVisible; }
        bool isTearingSupported() const { return mOptions & ALLOW_TEARING; }

        IDXGIAdapter1* getAdapter() const { return mAdapter.Get(); }
        ID3D12Device* getDevice() const { return mDevice.Get(); }
        IDXGIFactory4* getDXGIFactory() const { return mFactory.Get(); }
        IDXGISwapChain3* getSwapChain() const { return mSwapChain.Get(); }
        D3D_FEATURE_LEVEL getDeviceFeatureLevel() const { return mFeatureLevel; }
        ID3D12Resource* getRenderTarget() const { return mRenderTargets[mBackBufferIndex].Get(); }
        ID3D12Resource* getDepthStencil() const { return mDepthStencil.Get(); }
        ID3D12CommandQueue* getCommandQueue() const { return mCommandQueue.Get(); }
        ID3D12CommandAllocator* getCommandAllocator()const { return mCommandAllocators[mBackBufferIndex].Get(); }
        ID3D12GraphicsCommandList* getCommandList() const { return mCommandList.Get(); }
        DXGI_FORMAT getBackBufferFormat() const { return mBackBufferFormat; }
        DXGI_FORMAT getDepthBufferFormat() const { return mDepthBufferFormat; }
        D3D12_VIEWPORT getViewport() const { return mScreenViewport; }
        D3D12_RECT getScissorRect() const { return mScissorRect; }
        UINT getCurrentFrameIndex() const { return mBackBufferIndex; }
        UINT getPrevioutFrameIndex() const { return mBackBufferIndex == 0 ? mBackBufferCount - 1 : mBackBufferIndex - 1; }
        UINT getBackBufferCount() const { return mBackBufferCount; }
        UINT getDeviceOptions() const { return mOptions; }
        LPCWSTR getAdapterDescription() const { return mAdapterDescription.c_str(); }
        UINT getAdapterID() const { return mAdapterID; }

        CD3DX12_CPU_DESCRIPTOR_HANDLE getRenderTargetView() const {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(mRTVHeap->GetCPUDescriptorHandleForHeapStart(), mBackBufferIndex, mRTVDesctiptorSize);
        }

        CD3DX12_CPU_DESCRIPTOR_HANDLE getDepthStencilView() const {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(mDSVHeap->GetCPUDescriptorHandleForHeapStart());
        }
    private:
        /**
        * @brief �R���X�g���N�^
        */
        DeviceResource();
        void moveToNextFrame();
        void initializeAdapter(IDXGIAdapter1** ppAdapter);
    public:
        static constexpr UINT ALLOW_TEARING = 0x1;
        static constexpr UINT REQUIER_TEARING_SUPPORT = 0x2;

    private:
        static constexpr size_t MAX_BACK_BUFFER_COUNT = 3;
        UINT mAdapterIDoverride;
        UINT mBackBufferIndex;
        ComPtr<IDXGIAdapter1> mAdapter;
        UINT mAdapterID;
        std::wstring mAdapterDescription;

        ComPtr<ID3D12Device> mDevice;
        ComPtr<ID3D12CommandQueue> mCommandQueue;
        ComPtr<ID3D12GraphicsCommandList> mCommandList;
        ComPtr<ID3D12CommandAllocator> mCommandAllocators[MAX_BACK_BUFFER_COUNT];

        ComPtr<IDXGIFactory4> mFactory;
        ComPtr<IDXGISwapChain3> mSwapChain;
        ComPtr<ID3D12Resource> mRenderTargets[MAX_BACK_BUFFER_COUNT];
        ComPtr<ID3D12Resource> mDepthStencil;

        ComPtr<ID3D12Fence> mFence;
        UINT64 mFenceValue[MAX_BACK_BUFFER_COUNT];
        Microsoft::WRL::Wrappers::Event mFenceEvent;

        ComPtr<ID3D12DescriptorHeap> mRTVHeap;
        ComPtr<ID3D12DescriptorHeap> mDSVHeap;
        UINT mRTVDesctiptorSize;
        D3D12_VIEWPORT mScreenViewport;
        D3D12_RECT mScissorRect;

        DXGI_FORMAT mBackBufferFormat;
        DXGI_FORMAT mDepthBufferFormat;
        UINT mBackBufferCount;
        D3D_FEATURE_LEVEL mMinFeatureLevel;

        Window::Window* mWindow;
        D3D_FEATURE_LEVEL mFeatureLevel;
        RECT mOutputSize;
        bool mIsWindowVisible;

        UINT mOptions;

        IDeviceNotify* mDeviceNotify;
    };
} //Framework::DX 