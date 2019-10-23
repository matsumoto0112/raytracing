#include "InputManager.h"

namespace Framework::Input {
    //�R���X�g���N�^
    InputManager::InputManager(HWND hWnd)
        :mKeyboard(std::make_unique<Keyboard>(hWnd)),
        mMouse(std::make_unique<Mouse>(hWnd)) { }
    //�f�X�g���N�^
    InputManager::~InputManager() { }
    //���͏��̍X�V
    void InputManager::update() {
        mKeyboard->update();
        mMouse->update();
    }

} //Framework::Input 
