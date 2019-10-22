#pragma once
#include <dxgi1_6.h>
#include <functional>
#include "Framework/Window/Procedure/IWindowProc.h"
#include "Device/ISystemEventNotify.h"

namespace Framework::Window {
    /**
    * @class SysKeyDown
    * @brief �V�X�e���L�[�������ꂽ���̃v���V�[�W��
    * @details ALT + Enter �L�[�������ꂽ�Ƃ��Ƀt���X�N���[���ɂ��鏈��������
    */
    class SysKeyDownProc : public IWindowProc {
    public:
        /**
        * @brief �R���X�g���N�^
        */
        SysKeyDownProc(Device::ISystemEventNotify* notify);
        /**
        * @brief �f�X�g���N�^
        */
        ~SysKeyDownProc();
        /**
        * @brief �E�B���h�E�v���V�[�W������
        */
        virtual LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool * isReturn) override;
    private:
        Device::ISystemEventNotify* mSystemEventNotify;
    };

} //Framework::Window 