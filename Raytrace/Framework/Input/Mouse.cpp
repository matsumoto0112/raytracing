#include "Mouse.h"
#include "Framework/Input/Keyboard.h"
#include "Framework/Window/Window.h"

namespace Framework::Input {
    //�R���X�g���N�^
    Mouse::Mouse(HWND hWnd, Keyboard& keyboard)
        :mHWnd(hWnd), mKeyboard(keyboard) { }
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
    }
    //�}�E�X�̍��̍��W���擾
    const Math::Vector2& Mouse::getMousePosition() const {
        return mPosition;
    }
    //�}�E�X�̃{�^���̉�����Ԃ𔻒�
    bool Mouse::getMouse(MouseButton button) const {
        return mKeyboard.getKey(static_cast<KeyCode::Enum>(button));
    }
    //�}�E�X�̃{�^���������ꂽ���ǂ�������
    bool Mouse::getMouseDown(MouseButton button) const {
        return mKeyboard.getKeyDown(static_cast<KeyCode::Enum>(button));
    }
    //�}�E�X�̃{�^���������ꂽ���ǂ�������
    bool Mouse::getMouseUp(MouseButton button) const {
        return mKeyboard.getKeyUp(static_cast<KeyCode::Enum>(button));
    }
    //�}�E�X�̑O�񂩂�̈ړ��ʂ��擾
    Math::Vector2 Mouse::getMove() const {
        return mPosition - mPrevPosition;
    }
} //Framework::Input
