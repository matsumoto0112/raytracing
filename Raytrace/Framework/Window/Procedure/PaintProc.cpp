#include "PaintProc.h"
#include "Device/ISystemEventNotify.h"

namespace Framework::Window {
    //コンストラクタ
    PaintProc::PaintProc() { }
    //デストラクタ
    PaintProc::~PaintProc() { }
    //プロシージャ処理
    LRESULT PaintProc::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool * isReturn) {
        if (msg == WM_PAINT) {
            *isReturn = true;
            //ウィンドウに登録したシステム通知クラスを取得する
            Device::ISystemEventNotify* notify = reinterpret_cast<Device::ISystemEventNotify*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
            if (notify) {
                notify->onUpdate();
                notify->onRender();
            }
        }
        return 0L;
    }
} //Framework::Window 
