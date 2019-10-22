#pragma once
#include "Framework/Window/Procedure/IWindowProc.h"

namespace Framework::Window {
    /**
    * @class ImGuiProc
    * @brief ImGuiのイベント処理用プロシージャ
    */
    class ImGuiProc : public IWindowProc {
    public:
        /**
        * @brief コンストラクタ
        */
        ImGuiProc();
        /**
        * @brief デストラクタ
        */
        ~ImGuiProc();
        /**
        * @brief ウィンドウプロシージャ処理
        */
        virtual LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool * isReturn) override;
    };
} //Framework::Window 