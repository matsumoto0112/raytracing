#pragma once
#include "Framework/Window/Procedure/IWindowProc.h"

namespace Framework::Window {
    /**
    * @class DestroyProc
    * @brief �E�B���h�E�j��v���V�[�W��
    */
    class DestroyProc : public IWindowProc {
    public:
        /**
        * @brief �R���X�g���N�^
        */
        DestroyProc();
        /**
        * @brief �f�X�g���N�^
        */
        ~DestroyProc();
        /**
        * @brief �E�B���h�E�v���V�[�W������
        */
        virtual LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* isReturn) override;
    };

} //Framework::Window 