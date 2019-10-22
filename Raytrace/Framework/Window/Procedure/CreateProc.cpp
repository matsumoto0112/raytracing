#include "CreateProc.h"

namespace Framework::Window {
    //�R���X�g���N�^
    CreateProc::CreateProc() { }

    //�f�X�g���N�^
    CreateProc::~CreateProc() { }

    //�v���V�[�W������
    LRESULT CreateProc::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* isReturn) {
        if (msg == WM_CREATE) {
            *isReturn = true;
            //�V�[������ۑ����Ă������߂̏���
            LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
        }
        return 0L;
    }

} //Framework::Window 
