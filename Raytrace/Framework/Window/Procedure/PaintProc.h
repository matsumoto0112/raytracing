#pragma once
#include "Framework/Window/Procedure/IWindowProc.h"

namespace Framework::Window {
    /**
    * @class PaintProc
    * @brief �E�B���h�E�����v���V�[�W��
    */
    class PaintProc : public IWindowProc {
    public:
        /**
        * @brief �R���X�g���N�^
        */
        PaintProc();
        /**
        * @brief �f�X�g���N�^
        */
        ~PaintProc();
        /**
        * @brief �E�B���h�E�v���V�[�W������
        */
        virtual LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool * isReturn) override;
    };
} //Framework::Window 