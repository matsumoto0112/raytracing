#pragma once
#include "Framework/Window/Procedure/IWindowProc.h"

namespace Framework::Window {
    /**
    * @class CreateProc
    * @brief ウィンドウ生成プロシージャ
    */
    class CreateProc : public IWindowProc {
    public:
        /**
        * @brief コンストラクタ
        */
        CreateProc();
        /**
        * @brief デストラクタ
        */
        ~CreateProc();
        /**
        * @brief ウィンドウプロシージャ処理
        */
        virtual LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool * isReturn) override;
    private:


    };
} //Framework::Window 