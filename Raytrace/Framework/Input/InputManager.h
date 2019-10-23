#pragma once
#include <memory>
#include "Input/Keyboard.h"
#include "Input/Mouse.h"

namespace Framework::Input {
    /**
    * @class InputManager
    * @brief ���͂��Ǘ�����
    */
    class InputManager {
    public:
        /**
        * @brief �R���X�g���N�^
        * @param hWnd �E�B���h�E�n���h��
        */
        InputManager(HWND hWnd);
        /**
        * @brief �f�X�g���N�^
        */
        ~InputManager();
        /**
        * @brief ���͏����X�V����
        */
        void update();
        /**
        * @brief �L�[�{�[�h���擾����
        */
        inline Keyboard* getKeyboard() const { return mKeyboard.get(); }
        /**
        * @brief �}�E�X���擾����
        */
        inline Mouse* getMouse() const { return mMouse.get(); }
    private:
        std::unique_ptr<Keyboard> mKeyboard; //!< �L�[�{�[�h
        std::unique_ptr<Mouse> mMouse; //!< �}�E�X
    };
} //Framework::Input 