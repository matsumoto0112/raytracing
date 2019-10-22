#include "Procedures.h"

namespace Framework::Window {
    //メンバーの初期化
    std::vector<std::unique_ptr<IWindowProc>> Procedures::mWindowProcs = std::vector<std::unique_ptr<IWindowProc>>();

    //メインウィンドウプロシージャ
    LRESULT Procedures::mainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        bool isReturn = false;
        //先頭から順にプロシージャが処理されるまでループ
        for (auto&& proc : mWindowProcs) {
            LRESULT res = proc->wndProc(hWnd, msg, wParam, lParam, &isReturn);
            if (isReturn)return res;
        }
        //処理されなければ既定の処理を行う
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

} //Framework::Window 
