#pragma once
#include <Windows.h>

namespace Framework::Window {
        /**
        * @interface IWindowProc
        * @brief ウィンドウプロシージャインターフェース
        */
    __interface IWindowProc {
        /**
        * @brief ウィンドウプロシージャ処理
        */
        virtual LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* isReturn) = 0;
    };
} //Framework::Window