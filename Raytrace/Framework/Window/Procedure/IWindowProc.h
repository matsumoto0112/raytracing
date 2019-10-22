#pragma once
#include <Windows.h>

namespace Framework::Window {
        /**
        * @interface IWindowProc
        * @brief �E�B���h�E�v���V�[�W���C���^�[�t�F�[�X
        */
    __interface IWindowProc {
        /**
        * @brief �E�B���h�E�v���V�[�W������
        */
        virtual LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* isReturn) = 0;
    };
} //Framework::Window