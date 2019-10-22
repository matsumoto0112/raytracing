#include "SysKeyDownProc.h"
#include "Utility/Debug.h"

namespace Framework::Window {
    //�R���X�g���N�^
    SysKeyDownProc::SysKeyDownProc(Device::ISystemEventNotify* notify)
        :mSystemEventNotify(notify) { }

        //�f�X�g���N�^
    SysKeyDownProc::~SysKeyDownProc() { }

    //�v���V�[�W������
    LRESULT SysKeyDownProc::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* isReturn) {
        if (msg == WM_SYSKEYDOWN) {
            //ALT + Enter�������ꂽ
            if ((wParam == VK_RETURN) && (lParam & (1 << 29))) {
                *isReturn = true;
                mSystemEventNotify->pushAltEnter();

            }
        }
        return 0L;
    }

} //Framework::Window 
