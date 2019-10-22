#pragma once
#include <string>
#include <dxgi1_6.h>
#include "Framework/Device/ISystemEventNotify.h"
#include "Framework/Window/Procedure/IWindowProc.h"

namespace Framework::Window {
    /**
    * @class Window
    * @brief ウィンドウクラス
    */
    class Window {
    public:
        /**
        * @brief コンストラクタ
        * @param width ウィンドウ幅
        * @param height ウィンドウ高さ
        * @param title ウィンドウタイトル
        * @param hInstance インスタンスハンドル
        * @param nCmdShow ウィンドウの表示方法
        * @param systemNotify システム的な通知を送る対象
        */
        Window(UINT width, UINT height, const std::wstring& title,
            HINSTANCE hInstance, int nCmdShow, Device::ISystemEventNotify* systemNotify);
        /**
        * @brief デストラクタ
        */
        ~Window() { }
        void showWindow(int nCmdShow);
        void toggleFullScreenWindow(IDXGISwapChain* output = nullptr);
        void setWindowZOrderToTopMost(bool setToTopMost);
        HWND getHwnd() { return mHWnd; }
        bool isFullScreen() { return mFullScreenMode; }
        UINT getWidth() const { return mWindowRect.right - mWindowRect.left; }
        UINT getHeight() const { return mWindowRect.bottom - mWindowRect.top; }
    private:
        HWND mHWnd; //!< ウィンドウハンドル
        bool mFullScreenMode;
        static constexpr UINT mWindowStyle = WS_OVERLAPPEDWINDOW; //!< ウィンドウのスタイル
        RECT mWindowRect;
    };

} //Framework::Window
