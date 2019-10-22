#include "DestroyProc.h"

namespace Framework::Window {
    //コンストラクタ
    DestroyProc::DestroyProc() { }
    //デストラクタ
    DestroyProc::~DestroyProc() { }
    //ウィンドウプロシージャ
    LRESULT DestroyProc::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* isReturn) {
        if (msg == WM_DESTROY) {
            *isReturn = true;
            PostQuitMessage(0);
        }
        return 0L;
    }

} //Framework::Window 
