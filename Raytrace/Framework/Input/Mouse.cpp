#include "Mouse.h"

namespace Framework::Input {
    //�R���X�g���N�^
    Mouse::Mouse(HWND hWnd)
        :mHWnd(hWnd) {
    #define CHECK_MOUSE_BUTTON_PRESS(key) { \
        mCurrentMouseInfo[key] = GetKeyState(key) & 0x80; \
        } 

        //�ŏ��ɉ�����Ԃ𒲂ׁA�}�b�v���쐬���Ă���
        CHECK_MOUSE_BUTTON_PRESS(MouseButton::Left);
        CHECK_MOUSE_BUTTON_PRESS(MouseButton::Middle);
        CHECK_MOUSE_BUTTON_PRESS(MouseButton::Right);
    }
    //�f�X�g���N�^
    Mouse::~Mouse() { }
    //�X�V
    void Mouse::update() {
        //�O�̍��W���X�V����
        mPrevPosition = mPosition;

        //�X�N���[�����W���擾����
        POINT pos;
        GetCursorPos(&pos);
        //�N���C�A���g���W�ɕϊ�����
        ScreenToClient(mHWnd, &pos);
        mPosition.x = (float)pos.x;
        mPosition.y = (float)pos.y;

        std::copy(mCurrentMouseInfo.begin(), mCurrentMouseInfo.end(), mPrevMouseInfo.begin());
        CHECK_MOUSE_BUTTON_PRESS(MouseButton::Left);
        CHECK_MOUSE_BUTTON_PRESS(MouseButton::Middle);
        CHECK_MOUSE_BUTTON_PRESS(MouseButton::Right);
    }
    //�}�E�X�̍��̍��W���擾
    const Math::Vector2& Mouse::getMousePosition() const {
        return mPosition;
    }
    //�}�E�X�̃{�^���̉�����Ԃ𔻒�
    bool Mouse::getMouse(MouseButton button) const {
        return mCurrentMouseInfo.at(button);
    }
    //�}�E�X�̃{�^���������ꂽ���ǂ�������
    bool Mouse::getMouseDown(MouseButton button) const {
        return mCurrentMouseInfo.at(button) && mPrevMouseInfo.at(button);
    }
    //�}�E�X�̃{�^���������ꂽ���ǂ�������
    bool Mouse::getMouseUp(MouseButton button) const {
        return !mCurrentMouseInfo.at(button) && mPrevMouseInfo.at(button);
    }
    //�}�E�X�̑O�񂩂�̈ړ��ʂ��擾
    Math::Vector2 Mouse::getMove() const {
        return mPosition - mPrevPosition;
    }
    //�}�E�X�������Ă��邩�ǂ������肷��
    bool Mouse::isMouseVisible() const {
        return false;
    }
} //Framework::Input
