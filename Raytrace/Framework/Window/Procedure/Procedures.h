#pragma once
#include <Windows.h>
#include <vector>
#include <memory>
#include "Framework/Window/Procedure/IWindowProc.h"

namespace Framework::Window {
    /**
    * @class Procedures
    * @brief �v���V�[�W������
    */
    class Procedures {
    public:
        /**
        * @brief ���C���E�B���h�E�v���V�[�W��
        */
        static LRESULT CALLBACK mainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    public:
        static std::vector<std::unique_ptr<IWindowProc>> mWindowProcs; //!< �E�B���h�E�v���V�[�W�����X�g
    };
} //Framework::Window 