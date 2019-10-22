#include "Game.h"
#include <Windows.h>
#include <shellapi.h>
#include "Framework/Utility/Debug.h"
#include "Framework/Window/Procedure/Procedures.h"
#include "Framework/Window/Procedure/DestroyProc.h"
#include "Framework/Window/Procedure/CreateProc.h"
#include "Window/Procedure/SysKeyDownProc.h"
#include "Window/Procedure/ImGuiProc.h"
#include "Window/Procedure/PaintProc.h"

namespace Framework {
    //コンストラクタ
    Game::Game(UINT width, UINT height, const std::wstring& title)
        :mWidth(width), mHeight(height), mTitle(title) { }
    //デストラクタ
    Game::~Game() { }
    //実行
    int Game::run(HINSTANCE hInstance, int nCmdShow) {

        try {
            Window::Procedures::mWindowProcs.emplace_back(std::make_unique<Window::CreateProc>());
            Window::Procedures::mWindowProcs.emplace_back(std::make_unique<Window::DestroyProc>());
            Window::Procedures::mWindowProcs.emplace_back(std::make_unique<Window::ImGuiProc>());
            Window::Procedures::mWindowProcs.emplace_back(std::make_unique<Window::SysKeyDownProc>(this));
            Window::Procedures::mWindowProcs.emplace_back(std::make_unique<Window::PaintProc>());

            //ウィンドウ生成

            mWindow = std::make_unique<Window::Window>(mWidth, mHeight, mTitle,
                hInstance, nCmdShow, this);

            onInit();
            mWindow->showWindow(nCmdShow);

            //メインループ
            MSG msg = {};
            while (msg.message != WM_QUIT) {
                if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }

            //ループ終了時に終了処理をする
            onDestroy();

            return static_cast<char>(msg.lParam);
        }
        catch (const std::exception& e) {
            OutputDebugString(L"ERROR:\n");
            OutputDebugStringA(e.what());
            onDestroy();
            return EXIT_FAILURE;
        }
    }

    //初期化
    void Game::onInit() {
        mDeviceResource = std::make_unique<DX::DeviceResource>(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
            DXGI_FORMAT::DXGI_FORMAT_UNKNOWN,
            FRAME_COUNT,
            D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0,
            DX::DeviceResource::REQUIER_TEARING_SUPPORT,
            UINT_MAX);

        mDeviceResource->registerDeviceNotiry(this);
        mDeviceResource->setWindow(mWindow.get());
        mDeviceResource->initializeDXGIAdapter();

        mDeviceResource->createDeviceResource();
        mDeviceResource->createWindowSizeDependentResource();


        MY_DEBUG_LOG(L"init");
    }
    //更新処理
    void Game::onUpdate() {
        //MY_DEBUG_LOG(L"update");
    }

    void Game::onRender() {
        if (!mDeviceResource->isWindowVisible())return;
        //mDeviceResource->prepare(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT);

        //mDeviceResource->present(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
        ////MY_DEBUG_LOG(L"render");
    }

    //終了処理
    void Game::onDestroy() {
        mDeviceResource->waitForGPU();
        MY_DEBUG_LOG(L"destroy");
    }

    void Game::pushAltEnter() {
        if (mDeviceResource->isTearingSupported()) {
            mWindow->toggleFullScreenWindow(mDeviceResource->getSwapChain());
        }
    }

} //Framework 