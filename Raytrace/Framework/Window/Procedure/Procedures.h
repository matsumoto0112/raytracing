#pragma once
#include <Windows.h>
#include <vector>
#include <memory>
#include "Framework/Window/Procedure/IWindowProc.h"

namespace Framework::Window {
    /**
    * @class Procedures
    * @brief プロシージャ処理
    */
    class Procedures {
    public:
        /**
        * @brief メインウィンドウプロシージャ
        */
        static LRESULT CALLBACK mainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    public:
        static std::vector<std::unique_ptr<IWindowProc>> mWindowProcs; //!< ウィンドウプロシージャリスト
    };
} //Framework::Window 