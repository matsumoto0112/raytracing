#include "ImGuiProc.h"
#include "ImGui/ImGuiInclude.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Framework::Window {
    //�R���X�g���N�^
    ImGuiProc::ImGuiProc() { }
    //�f�X�g���N�^
    ImGuiProc::~ImGuiProc() { }

    //�v���V�[�W������
    LRESULT ImGuiProc::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* isReturn) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
            *isReturn = true;
        }
        return 0L;
    }

} //Framework::WIndow 
