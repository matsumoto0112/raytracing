#include "Procedures.h"

namespace Framework::Window {
    //�����o�[�̏�����
    std::vector<std::unique_ptr<IWindowProc>> Procedures::mWindowProcs = std::vector<std::unique_ptr<IWindowProc>>();

    //���C���E�B���h�E�v���V�[�W��
    LRESULT Procedures::mainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        bool isReturn = false;
        //�擪���珇�Ƀv���V�[�W�������������܂Ń��[�v
        for (auto&& proc : mWindowProcs) {
            LRESULT res = proc->wndProc(hWnd, msg, wParam, lParam, &isReturn);
            if (isReturn)return res;
        }
        //��������Ȃ���Ί���̏������s��
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

} //Framework::Window 
