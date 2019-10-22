#pragma once
#include "Framework/Window/Procedure/IWindowProc.h"

namespace Framework::Window {
    /**
    * @class ImGuiProc
    * @brief ImGui�̃C�x���g�����p�v���V�[�W��
    */
    class ImGuiProc : public IWindowProc {
    public:
        /**
        * @brief �R���X�g���N�^
        */
        ImGuiProc();
        /**
        * @brief �f�X�g���N�^
        */
        ~ImGuiProc();
        /**
        * @brief �E�B���h�E�v���V�[�W������
        */
        virtual LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool * isReturn) override;
    };
} //Framework::Window 