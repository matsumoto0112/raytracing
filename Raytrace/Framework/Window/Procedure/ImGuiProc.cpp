#include "ImGuiProc.h"
#include "ImGui/ImGuiInclude.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Framework::Window {
    //コンストラクタ
    ImGuiProc::ImGuiProc() { }
    //デストラクタ
    ImGuiProc::~ImGuiProc() { }

    //プロシージャ処理
    LRESULT ImGuiProc::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* isReturn) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
            *isReturn = true;
        }
        return 0L;
    }

} //Framework::WIndow 
