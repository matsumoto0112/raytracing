#include "CreateProc.h"

namespace Framework::Window {
    //コンストラクタ
    CreateProc::CreateProc() { }

    //デストラクタ
    CreateProc::~CreateProc() { }

    //プロシージャ処理
    LRESULT CreateProc::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* isReturn) {
        if (msg == WM_CREATE) {
            *isReturn = true;
            //シーン情報を保存しておくための処理
            LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
        }
        return 0L;
    }

} //Framework::Window 
