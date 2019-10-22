#include "DestroyProc.h"

namespace Framework::Window {
    //�R���X�g���N�^
    DestroyProc::DestroyProc() { }
    //�f�X�g���N�^
    DestroyProc::~DestroyProc() { }
    //�E�B���h�E�v���V�[�W��
    LRESULT DestroyProc::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* isReturn) {
        if (msg == WM_DESTROY) {
            *isReturn = true;
            PostQuitMessage(0);
        }
        return 0L;
    }

} //Framework::Window 
