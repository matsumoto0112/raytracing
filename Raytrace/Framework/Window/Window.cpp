#include "Window.h"
#include "DX/DXHelper.h"
#include "Utility/Typedef.h"
#include "Window/Procedure/Procedures.h"

namespace Framework::Window {

    Window::Window(UINT width, UINT height, const std::wstring& title,
        HINSTANCE hInstance, int nCmdShow, Device::ISystemEventNotify* systemNotify) {
        //ウィンドウ生成
        WNDCLASSEX windowClass{ 0 };
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = Procedures::mainWndProc;
        windowClass.hInstance = hInstance;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.lpszClassName = L"hoge";
        RegisterClassEx(&windowClass);

        RECT windowRect = { 0,0,static_cast<LONG>(width),static_cast<LONG>(height) };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        mHWnd = CreateWindow(
            windowClass.lpszClassName,
            title.c_str(),
            mWindowStyle,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr,
            nullptr,
            hInstance,
            systemNotify);

        mWindowRect =
        {
            0,
            0,
            static_cast<LONG>(width),
            static_cast<LONG>(height)
        };
    }

    void Window::showWindow(int nCmdShow) {
        ShowWindow(mHWnd, nCmdShow);
    }

    void Window::toggleFullScreenWindow(IDXGISwapChain* swapChain) {
        if (mFullScreenMode) {
            SetWindowLong(mHWnd, GWL_STYLE, mWindowStyle);
            SetWindowPos(
                mHWnd,
                HWND_NOTOPMOST,
                mWindowRect.left,
                mWindowRect.top,
                mWindowRect.right - mWindowRect.left,
                mWindowRect.bottom - mWindowRect.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);
            ShowWindow(mHWnd, SW_NORMAL);
        }
        else {
            GetWindowRect(mHWnd, &mWindowRect);
            SetWindowLong(mHWnd, GWL_STYLE, mWindowStyle & WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME);
            RECT fullScreenWindowRect;
            try {
                if (swapChain) {
                    ComPtr<IDXGIOutput> output;
                    DX::throwIfFailed(swapChain->GetContainingOutput(&output));
                    DXGI_OUTPUT_DESC desc;
                    DX::throwIfFailed(output->GetDesc(&desc));
                    fullScreenWindowRect = desc.DesktopCoordinates;
                }
                else {
                    throw DX::HrException(S_FALSE);
                }
            }
            catch (const std::exception& e) {
                UNREFERENCED_PARAMETER(e);

                DEVMODE devMode = {};
                devMode.dmSize = sizeof(DEVMODE);
                EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

                fullScreenWindowRect = {
                    devMode.dmPosition.x,
                    devMode.dmPosition.y,
                    devMode.dmPosition.x + static_cast<LONG>(devMode.dmPelsWidth),
                    devMode.dmPosition.y + static_cast<LONG>(devMode.dmPelsHeight)
                };
            }

            SetWindowPos(
                mHWnd,
                HWND_TOPMOST,
                fullScreenWindowRect.left,
                fullScreenWindowRect.top,
                fullScreenWindowRect.right,
                fullScreenWindowRect.bottom,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ShowWindow(mHWnd, SW_MAXIMIZE);
        }

        mFullScreenMode = !mFullScreenMode;
    }

    void Window::setWindowZOrderToTopMost(bool setToTopMost) {
        RECT rect;
        GetWindowRect(mHWnd, &rect);

        SetWindowPos(
            mHWnd,
            (setToTopMost) ? HWND_TOPMOST : HWND_NOTOPMOST,
            rect.left,
            rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);
    }

} //Framework::Window 
