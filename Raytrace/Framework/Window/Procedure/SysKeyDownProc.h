#pragma once
#include <dxgi1_6.h>
#include <functional>
#include "Framework/Window/Procedure/IWindowProc.h"
#include "Device/ISystemEventNotify.h"

namespace Framework::Window {
    /**
    * @class SysKeyDown
    * @brief システムキーが押された時のプロシージャ
    * @details ALT + Enter キーが押されたときにフルスクリーンにする処理をする
    */
    class SysKeyDownProc : public IWindowProc {
    public:
        /**
        * @brief コンストラクタ
        */
        SysKeyDownProc(Device::ISystemEventNotify* notify);
        /**
        * @brief デストラクタ
        */
        ~SysKeyDownProc();
        /**
        * @brief ウィンドウプロシージャ処理
        */
        virtual LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool * isReturn) override;
    private:
        Device::ISystemEventNotify* mSystemEventNotify;
    };

} //Framework::Window 