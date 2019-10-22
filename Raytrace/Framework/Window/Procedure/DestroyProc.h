#pragma once
#include "Framework/Window/Procedure/IWindowProc.h"

namespace Framework::Window {
    /**
    * @class DestroyProc
    * @brief ウィンドウ破壊プロシージャ
    */
    class DestroyProc : public IWindowProc {
    public:
        /**
        * @brief コンストラクタ
        */
        DestroyProc();
        /**
        * @brief デストラクタ
        */
        ~DestroyProc();
        /**
        * @brief ウィンドウプロシージャ処理
        */
        virtual LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* isReturn) override;
    };

} //Framework::Window 