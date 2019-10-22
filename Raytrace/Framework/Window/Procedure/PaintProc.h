#pragma once
#include "Framework/Window/Procedure/IWindowProc.h"

namespace Framework::Window {
    /**
    * @class PaintProc
    * @brief ウィンドウ生成プロシージャ
    */
    class PaintProc : public IWindowProc {
    public:
        /**
        * @brief コンストラクタ
        */
        PaintProc();
        /**
        * @brief デストラクタ
        */
        ~PaintProc();
        /**
        * @brief ウィンドウプロシージャ処理
        */
        virtual LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool * isReturn) override;
    };
} //Framework::Window 