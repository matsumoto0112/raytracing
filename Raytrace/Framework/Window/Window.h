#pragma once
#include <string>
#include <dxgi1_6.h>
#include "Framework/Device/ISystemEventNotify.h"
#include "Framework/Window/Procedure/IWindowProc.h"

namespace Framework::Window {
    /**
    * @class Window
    * @brief �E�B���h�E�N���X
    */
    class Window {
    public:
        /**
        * @brief �R���X�g���N�^
        * @param width �E�B���h�E��
        * @param height �E�B���h�E����
        * @param title �E�B���h�E�^�C�g��
        * @param hInstance �C���X�^���X�n���h��
        * @param nCmdShow �E�B���h�E�̕\�����@
        * @param systemNotify �V�X�e���I�Ȓʒm�𑗂�Ώ�
        */
        Window(UINT width, UINT height, const std::wstring& title,
            HINSTANCE hInstance, int nCmdShow, Device::ISystemEventNotify* systemNotify);
        /**
        * @brief �f�X�g���N�^
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
        HWND mHWnd; //!< �E�B���h�E�n���h��
        bool mFullScreenMode;
        static constexpr UINT mWindowStyle = WS_OVERLAPPEDWINDOW; //!< �E�B���h�E�̃X�^�C��
        RECT mWindowRect;
    };

} //Framework::Window
