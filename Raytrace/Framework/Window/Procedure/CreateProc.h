#pragma once
#include "Framework/Window/Procedure/IWindowProc.h"

namespace Framework::Window {
    /**
    * @class CreateProc
    * @brief �E�B���h�E�����v���V�[�W��
    */
    class CreateProc : public IWindowProc {
    public:
        /**
        * @brief �R���X�g���N�^
        */
        CreateProc();
        /**
        * @brief �f�X�g���N�^
        */
        ~CreateProc();
        /**
        * @brief �E�B���h�E�v���V�[�W������
        */
        virtual LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool * isReturn) override;
    private:


    };
} //Framework::Window 