#include "DeviceResource.h"
#include "DX/DXHelper.h"
#include "Math/MathUtility.h"
#include "Utility/Debug.h"

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

namespace {
    inline DXGI_FORMAT NoSRGB(DXGI_FORMAT fmt) {
        switch (fmt) {
        case DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   return DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
        case DXGI_FORMAT::DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:   return DXGI_FORMAT::DXGI_FORMAT_B8G8R8X8_UNORM;
        default:                                return fmt;
        }
    }

    inline bool operator==(const RECT& r1, const RECT& r2) {
        return (r1.left == r2.left
            || r1.top == r2.top
            || r1.right == r2.right
            || r1.bottom == r2.bottom);
    }
}

namespace Framework::DX {

    DeviceResource::DeviceResource(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, UINT backBufferCount, D3D_FEATURE_LEVEL minFeatureLevel, UINT flags, UINT adapterIDoverride)
        :mAdapterIDoverride(0),
        mBackBufferIndex(0),
        mAdapter(nullptr),
        mAdapterID(UINT_MAX),
        mAdapterDescription(),
        mFenceValue{},
        mRTVDesctiptorSize(0),
        mScreenViewport{},
        mScissorRect{},
        mBackBufferFormat(backBufferFormat),
        mDepthBufferFormat(depthBufferFormat),
        mBackBufferCount(backBufferCount),
        mMinFeatureLevel(minFeatureLevel),
        mWindow(nullptr),
        mFeatureLevel(D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0),
        mIsWindowVisible(true),
        mOptions(flags),
        mDeviceNotify(nullptr) {
        MY_ASSERTION(backBufferCount <= MAX_BACK_BUFFER_COUNT, L"backBufferCountが大きすぎます");
        MY_ASSERTION(minFeatureLevel >= D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0, L"minFeatureLevelが低すぎます");
        if (mOptions & REQUIER_TEARING_SUPPORT) {
            mOptions |= ALLOW_TEARING;
        }
    }

    DeviceResource::~DeviceResource() {
        waitForGPU();
    }

    void DeviceResource::initializeDXGIAdapter() {
        bool debugDXGI = false;

    #ifdef _DEBUG
        ComPtr<ID3D12Debug> debug;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) {
            debug->EnableDebugLayer();
        }
        else {
            MY_DEBUG_LOG(L"WARNING: Direct3D Debug Device is not available\n");
        }
        ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue)))) {
            debugDXGI = true;
            Utility::throwIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&mFactory)));
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY::DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY::DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
        }
    #endif

        if (!debugDXGI) {
            Utility::throwIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mFactory)));
        }

        if (mOptions & (ALLOW_TEARING | REQUIER_TEARING_SUPPORT)) {
            BOOL allowTearing = FALSE;

            ComPtr<IDXGIFactory5> factory5;
            HRESULT hr = mFactory.As(&factory5);
            if (SUCCEEDED(hr)) {
                hr = factory5->CheckFeatureSupport(DXGI_FEATURE::DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
            }

            if (FAILED(hr) || !allowTearing) {
                MY_DEBUG_LOG(L"WARNING: Variable refresh rate displays are not supported.\n");
                if (mOptions & REQUIER_TEARING_SUPPORT) {
                   Utility::throwIfFalse(false, L"Error: Sample must be run on an OS with tearing support.\n");
                }
                mOptions &= ~ALLOW_TEARING;
            }
        }

        initializeAdapter(&mAdapter);
    }

    void DeviceResource::createDeviceResource() {
        Utility::throwIfFailed(D3D12CreateDevice(mAdapter.Get(), mMinFeatureLevel, IID_PPV_ARGS(&mDevice)));

    #ifndef NDEBUG
        ComPtr<ID3D12InfoQueue> infoQueue;
        if (SUCCEEDED(mDevice.As(&infoQueue))) {
        #ifdef _DEBUG
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_ERROR, true);
        #endif
            D3D12_MESSAGE_ID hide[] =
            {
                D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
            };
            D3D12_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = ARRAYSIZE(hide);
            filter.DenyList.pIDList = hide;
            infoQueue->AddStorageFilterEntries(&filter);
        }
    #endif // !NDEBUG
        static constexpr D3D_FEATURE_LEVEL FEATURE_LEVELS[] =
        {
            D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0,
        };

        D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels =
        {
            ARRAYSIZE(FEATURE_LEVELS),FEATURE_LEVELS,D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0
        };

        HRESULT hr = mDevice->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));
        if (SUCCEEDED(hr)) {
            mFeatureLevel = featLevels.MaxSupportedFeatureLevel;
        }
        else {
            mFeatureLevel = mMinFeatureLevel;
        }

        //コマンドキューの作成
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;

        Utility::throwIfFailed(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

        //RTV・DSVのヒープ作成
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = mBackBufferCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        Utility::throwIfFailed(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRTVHeap)));
        mRTVDesctiptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        if (mDepthBufferFormat != DXGI_FORMAT::DXGI_FORMAT_UNKNOWN) {
            D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
            dsvHeapDesc.NumDescriptors = 1;
            dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

            Utility::throwIfFailed(mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mDSVHeap)));
        }

        //バックバッファの数分アロケータ作成
        for (UINT n = 0; n < mBackBufferCount; n++) {
            Utility::throwIfFailed(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocators[n])));
        }

        //コマンドリスト作成
        Utility::throwIfFailed(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&mCommandList)));
        Utility::throwIfFailed(mCommandList->Close());

        //GPUの処理用フェンス作成
        Utility::throwIfFailed(mDevice->CreateFence(mFenceValue[mBackBufferIndex], D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
        mFenceValue[mBackBufferIndex]++;

        mFenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));
        if (!mFenceEvent.IsValid()) {
            Utility::throwIfFailed(E_FAIL, L"CreateEvent failed.\n");
        }
    }

    void DeviceResource::createWindowSizeDependentResource() {
        if (!mWindow) {
            Utility::throwIfFailed(E_HANDLE, L"ウィンドウハンドルがセットされていません\n");
        }

        waitForGPU();

        //リソースの開放
        for (UINT n = 0; n < mBackBufferCount; n++) {
            mRenderTargets[n].Reset();
            mFenceValue[n] = mFenceValue[mBackBufferIndex];
        }

        UINT backBufferWidth = Math::MathUtil::mymax(mOutputSize.right - mOutputSize.left, 1L);
        UINT backBufferHeight = Math::MathUtil::mymax(mOutputSize.bottom - mOutputSize.top, 1L);
        DXGI_FORMAT backBufferFormat = NoSRGB(mBackBufferFormat);

        if (mSwapChain) {
            HRESULT hr = mSwapChain->ResizeBuffers(
                mBackBufferCount,
                backBufferWidth, backBufferHeight,
                backBufferFormat,
                (mOptions & ALLOW_TEARING) ? DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);

            if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
            #ifdef _DEBUG
                wchar_t buff[64] = {};
                swprintf_s(buff, L"デバイスロストが発生しました:0x%08X\n", (hr == DXGI_ERROR_DEVICE_REMOVED) ? mDevice->GetDeviceRemovedReason() : hr);
                MY_DEBUG_LOG(buff);
            #endif
                //デバイスロストの通知
                handleDeviceLost();
                return;
            }
            else {
                Utility::throwIfFailed(hr);
            }
        }
        else {
            //スワップチェイン作成
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.Width = backBufferWidth;
            swapChainDesc.Height = backBufferHeight;
            swapChainDesc.Format = backBufferFormat;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = mBackBufferCount;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_IGNORE;
            swapChainDesc.Flags = (mOptions & ALLOW_TEARING) ? DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { 0 };
            fsSwapChainDesc.Windowed = TRUE;

            ComPtr<IDXGISwapChain1> swapChain;
            bool prevIsFullScreen = mWindow->isFullScreen();
            if (prevIsFullScreen) {
                mWindow->setWindowZOrderToTopMost(false);
            }

            Utility::throwIfFailed(mFactory->CreateSwapChainForHwnd(mCommandQueue.Get(), mWindow->getHwnd(), &swapChainDesc, &fsSwapChainDesc, nullptr, &swapChain));
            if (prevIsFullScreen) {
                mWindow->setWindowZOrderToTopMost(true);
            }

            Utility::throwIfFailed(swapChain.As(&mSwapChain));

            if (isTearingSupported()) {
                mFactory->MakeWindowAssociation(mWindow->getHwnd(), DXGI_MWA_NO_ALT_ENTER);
            }
        }

        for (UINT n = 0; n < mBackBufferCount; n++) {
            Utility::throwIfFailed(mSwapChain->GetBuffer(n, IID_PPV_ARGS(&mRenderTargets[n])));
            std::wstring name = (Utility::StringBuilder(L"Render target ") << n).getStr();
            mRenderTargets[n]->SetName(name.c_str());

            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = mBackBufferFormat;
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;

            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRTVHeap->GetCPUDescriptorHandleForHeapStart(), n, mRTVDesctiptorSize);
            mDevice->CreateRenderTargetView(mRenderTargets[n].Get(), &rtvDesc, rtvHandle);
        }

        mBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

        if (mDepthBufferFormat != DXGI_FORMAT::DXGI_FORMAT_UNKNOWN) {
            CD3DX12_HEAP_PROPERTIES dsvProp(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT);
            D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
                mDepthBufferFormat,
                backBufferWidth,
                backBufferHeight,
                1,
                1);

            depthStencilDesc.Flags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

            D3D12_CLEAR_VALUE depthClearValue = {};
            depthClearValue.Format = mDepthBufferFormat;
            depthClearValue.DepthStencil.Depth = 1.0f;
            depthClearValue.DepthStencil.Stencil = 0;

            Utility::throwIfFailed(mDevice->CreateCommittedResource(
                &dsvProp,
                D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
                &depthStencilDesc,
                D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &depthClearValue,
                IID_PPV_ARGS(&mDepthStencil)));

            mDepthStencil->SetName(L"Depth stencil");

            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = mDepthBufferFormat;
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;

            mDevice->CreateDepthStencilView(mDepthStencil.Get(), &dsvDesc, mDSVHeap->GetCPUDescriptorHandleForHeapStart());
        }

        mScreenViewport.TopLeftX = mScreenViewport.TopLeftY = 0.0f;
        mScreenViewport.Width = static_cast<float>(backBufferWidth);
        mScreenViewport.Height = static_cast<float>(backBufferHeight);
        mScreenViewport.MinDepth = D3D12_MIN_DEPTH;
        mScreenViewport.MaxDepth = D3D12_MAX_DEPTH;

        mScissorRect.left = mScissorRect.top = 0;
        mScissorRect.right = backBufferWidth;
        mScissorRect.bottom = backBufferHeight;
    }

    void DeviceResource::setWindow(Window::Window* window) {
        mWindow = window;

        mOutputSize.left = mOutputSize.top;
        mOutputSize.right = window->getWidth();
        mOutputSize.bottom = window->getHeight();
    }

    bool DeviceResource::windowSizeChange(UINT width, UINT height, bool minimized) {
        mIsWindowVisible = !minimized;
        if (minimized || width == 0 || height == 0) {
            return false;
        }

        RECT newRect{ 0,0,static_cast<LONG>(width),static_cast<LONG>(height) };
        if (newRect == mOutputSize) {
            return false;
        }

        mOutputSize = newRect;
        createWindowSizeDependentResource();
        return true;
    }

    void DeviceResource::handleDeviceLost() {
        if (mDeviceNotify) {
            mDeviceNotify->onDeviceLost();
        }

        for (UINT n = 0; n < mBackBufferCount; n++) {
            mCommandAllocators[n].Reset();
            mRenderTargets[n].Reset();
        }

        mDepthStencil.Reset();
        mCommandQueue.Reset();
        mCommandList.Reset();
        mFence.Reset();
        mRTVHeap.Reset();
        mDSVHeap.Reset();
        mSwapChain.Reset();
        mDevice.Reset();
        mFactory.Reset();
        mAdapter.Reset();

    #ifdef _DEBUG
        {

            ComPtr<IDXGIDebug> dxgiDebug;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)))) {
                dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
            }
        }
    #endif

        initializeDXGIAdapter();
        createDeviceResource();
        createWindowSizeDependentResource();
        if (mDeviceNotify) {
            mDeviceNotify->onDeviceRestored();
        }
    }

    void DeviceResource::registerDeviceNotiry(IDeviceNotify* notify) {
        mDeviceNotify = notify;

        __if_exists(DXGIDeclareAdapterRemovalSupport) {
            if (notify) {
                if (FAILED(DXGIDeclareAdapterRemovalSupport())) {
                    OutputDebugString(L"Warning: application failed to declare adapter removal support.\n");
                }
            }
        }
    }

    void DeviceResource::prepare(D3D12_RESOURCE_STATES beforeState) {
        Utility::throwIfFailed(mCommandAllocators[mBackBufferIndex]->Reset());
        Utility::throwIfFailed(mCommandList->Reset(mCommandAllocators[mBackBufferIndex].Get(), nullptr));

        if (beforeState != D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET) {
            D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mBackBufferIndex].Get(), beforeState, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &barrier);
        }
        mCommandList->OMSetRenderTargets(1, &getRenderTargetView(), FALSE, nullptr);
        float color[4] = { 0.0f,0.0f,0.0f,0.0f };
        mCommandList->ClearRenderTargetView(getRenderTargetView(), color, 1, &mScissorRect);
    }

    void DeviceResource::present(D3D12_RESOURCE_STATES beforeState) {
        if (beforeState != D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT) {
            D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mBackBufferIndex].Get(),
                beforeState, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT);
            mCommandList->ResourceBarrier(1, &barrier);
        }

        executeCommandList();

        HRESULT hr;
        if (mOptions & ALLOW_TEARING) {
            hr = mSwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
        }
        else {
            hr = mSwapChain->Present(1, 0);
        }
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        #ifdef _DEBUG
            wchar_t buff[64] = {};
            swprintf_s(buff, L"プレゼント時にデバイスロスト発生: 0x%08X\n", (hr == DXGI_ERROR_DEVICE_REMOVED) ? mDevice->GetDeviceRemovedReason() : hr);
            MY_DEBUG_LOG(buff);
        #endif
            handleDeviceLost();
        }
        else {
            Utility::throwIfFailed(hr);
            moveToNextFrame();
        }

        Utility::throwIfFailed(mCommandAllocators[mBackBufferIndex]->Reset());
        Utility::throwIfFailed(mCommandList->Reset(mCommandAllocators[mBackBufferIndex].Get(), nullptr));
    }

    void DeviceResource::executeCommandList() {
        Utility::throwIfFailed(mCommandList->Close());
        ID3D12CommandList* list[] = { mCommandList.Get() };
        mCommandQueue->ExecuteCommandLists(ARRAYSIZE(list), list);
    }

    void DeviceResource::waitForGPU() {
        if (mCommandQueue && mFence && mFenceEvent.IsValid()) {
            UINT64 value = mFenceValue[mBackBufferIndex];
            if (SUCCEEDED(mCommandQueue->Signal(mFence.Get(), value))) {
                if (SUCCEEDED(mFence->SetEventOnCompletion(value, mFenceEvent.Get()))) {
                    WaitForSingleObjectEx(mFenceEvent.Get(), INFINITE, FALSE);
                    mFenceValue[mBackBufferIndex]++;
                }
            }
        }
    }

    void DeviceResource::moveToNextFrame() {
        const UINT64 currentFenceValue = mFenceValue[mBackBufferIndex];
        Utility::throwIfFailed(mCommandQueue->Signal(mFence.Get(), currentFenceValue));

        mBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
        if (mFence->GetCompletedValue() < mFenceValue[mBackBufferIndex]) {
            Utility::throwIfFailed(mFence->SetEventOnCompletion(mFenceValue[mBackBufferIndex], mFenceEvent.Get()));
            WaitForSingleObjectEx(mFenceEvent.Get(), INFINITE, FALSE);
        }
        mFenceValue[mBackBufferIndex] = currentFenceValue + 1;
    }

    void DeviceResource::initializeAdapter(IDXGIAdapter1** ppAdapter) {
        *ppAdapter = nullptr;

        ComPtr<IDXGIAdapter1> adapter;
        for (UINT adapterID = 0; mFactory->EnumAdapters1(adapterID, &adapter) != DXGI_ERROR_NOT_FOUND; ++adapterID) {
            if (mAdapterIDoverride != UINT_MAX && adapterID != mAdapterIDoverride) {
                continue;
            }

            DXGI_ADAPTER_DESC1 desc;
            Utility::throwIfFailed(adapter->GetDesc1(&desc));

            if (desc.Flags & DXGI_ADAPTER_FLAG::DXGI_ADAPTER_FLAG_SOFTWARE) {
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), mMinFeatureLevel, __uuidof(ID3D12Device), nullptr))) {
                mAdapterID = adapterID;
                mAdapterDescription = desc.Description;
            #ifdef _DEBUG
                wchar_t buff[256] = {};
                swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterID, desc.VendorId, desc.DeviceId, desc.Description);
                MY_DEBUG_LOG(buff);
            #endif
                break;
            }
        }

    #ifndef NDEBUG
        if (!adapter && mAdapterIDoverride == UINT_MAX) {
            if (FAILED(mFactory->EnumWarpAdapter(IID_PPV_ARGS(&adapter)))) {
                throw std::exception("WARP12 not available. Enable the 'Graphics Tools' optional feature");
            }

            MY_DEBUG_LOG(L"Direct3D Adapter - WRAP12\n");
        }
    #endif

        if (!adapter) {
            if (mAdapterIDoverride != UINT_MAX) {
                throw std::exception("Unavailable adapter requested.");
            }
            else {
                throw std::exception("Unavailable adapter.");
            }
        }

        *ppAdapter = adapter.Detach();
    }

} //Framework::DX 
