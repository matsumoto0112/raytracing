#pragma once
#include <Windows.h>
#include <unordered_map>
#include "Framework/Math/Vector2.h"

namespace Framework::Input {
    namespace MouseButton {
        /**
        * @enum Enum
        * @brief �}�E�X�̃{�^����`
        */
        enum Enum {
            Left = VK_LBUTTON,
            Right = VK_RBUTTON,
            Middle = VK_MBUTTON,
        };
    }; //MouseButton

    /**
    * @class Mouse
    * @brief �}�E�X���Ǘ��N���X
    */
    class Mouse {
        using MouseButton = MouseButton::Enum;
        using MouseInfo = std::unordered_map<MouseButton, bool>;
    public:
        /**
        * @brief �R���X�g���N�^
        * @param hWnd �E�B���h�E�n���h��
        */
        Mouse(HWND hWnd);
        /**
        * @brief �f�X�g���N�^
        */
        ~Mouse();
        /**
        * @brief �X�V
        */
        void update();
        /**
        * @brief �}�E�X�̍��W���擾
        * @return �N���C�A���g�̈�ł̃}�E�X���W��Ԃ�
        */
        const Math::Vector2& getMousePosition() const;
        /**
        * @brief �}�E�X�̃{�^���������Ă��邩
        * @param button �{�^���̎��
        * @return �����Ă����ԂȂ�true��Ԃ�
        */
        bool getMouse(MouseButton button) const;
        /**
        * @brief �}�E�X�̃{�^������������
        * @param button �{�^���̎��
        * @return �������^�C�~���O�Ȃ�true��Ԃ�
        */
        bool getMouseDown(MouseButton button) const;
        /**
        * @brief �}�E�X�̃{�^���𗣂�����
        * @param button �{�^���̎��
        * @return �������^�C�~���O�Ȃ�true��Ԃ�
        */
        bool getMouseUp(MouseButton button) const;
        /**
        * @brief �}�E�X�̈ړ��ʂ��擾
        */
        Math::Vector2 getMove() const;
        /**
        * @brief �}�E�X���o�����Ă��邩�ǂ������肷��
        */
        bool isMouseVisible() const;
    private:
        HWND mHWnd; //!< �E�B���h�E�n���h��
        Math::Vector2 mPosition; //!< ���̃}�E�X�̍��W
        Math::Vector2 mPrevPosition; //!< �O�̃}�E�X�̍��W
        MouseInfo mPrevMouseInfo; //!< �O�t���[���̃}�E�X�̃{�^���̏��
        MouseInfo mCurrentMouseInfo; //!< ���t���[���̃}�E�X�̃{�^���̏��
    };
} //Framework::Input
