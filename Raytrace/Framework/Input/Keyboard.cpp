#include "Keyboard.h"
#include "Framework/Input/KeyCode.h"
#include "Framework/Utility/Debug.h"

namespace Framework::Input {
    //�R���X�g���N�^
    Keyboard::Keyboard(HWND hWnd)
        :mResult(), mCurrentKeys(), mPrevKeys() { }
    //�f�X�g���N�^
    Keyboard::~Keyboard() { }
    //�X�V
    void Keyboard::update() {
        std::copy(mCurrentKeys.begin(), mCurrentKeys.end(), mPrevKeys.begin());
        GetKeyboardState(mCurrentKeys.data());
    }
    //�L�[�̉������̎擾
    bool Keyboard::getKey(KeyCode key) const {
        return checkKeyDown(mCurrentKeys, key);
    }
    //�L�[�̉������u�Ԃ��ǂ������擾
    bool Keyboard::getKeyDown(KeyCode key) const {
        bool prev = checkKeyDown(mPrevKeys, key);
        bool cur = checkKeyDown(mCurrentKeys, key);
        return !prev && cur;
    }
    //�L�[�̗������u�Ԃ��ǂ������擾
    bool Keyboard::getKeyUp(KeyCode key) const {
        bool prev = checkKeyDown(mPrevKeys, key);
        bool cur = checkKeyDown(mCurrentKeys, key);
        return prev && !cur;
    }
    //�L�[��������Ă��邩�ǂ�������
    bool Keyboard::checkKeyDown(const KeyInfo& keys, KeyCode key) const {
        return (keys[key] & 0x80) != 0;
    }

} //Framework::Input
