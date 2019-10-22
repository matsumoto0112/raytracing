#include "SysKeyDownProc.h"
#include "Utility/Debug.h"

namespace Framework::Window {
    //コンストラクタ
    SysKeyDownProc::SysKeyDownProc(Device::ISystemEventNotify* notify)
        :mSystemEventNotify(notify) { }

        //デストラクタ
    SysKeyDownProc::~SysKeyDownProc() { }

    //プロシージャ処理
    LRESULT SysKeyDownProc::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* isReturn) {
        if (msg == WM_SYSKEYDOWN) {
            //ALT + Enterが押された
            if ((wParam == VK_RETURN) && (lParam & (1 << 29))) {
                *isReturn = true;
                mSystemEventNotify->pushAltEnter();

            }
        }
        return 0L;
    }

} //Framework::Window 
