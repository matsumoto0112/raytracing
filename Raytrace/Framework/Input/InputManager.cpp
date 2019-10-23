#include "InputManager.h"

namespace Framework::Input {
    //コンストラクタ
    InputManager::InputManager(HWND hWnd)
        :mKeyboard(std::make_unique<Keyboard>(hWnd)),
        mMouse(std::make_unique<Mouse>(hWnd)) { }
    //デストラクタ
    InputManager::~InputManager() { }
    //入力情報の更新
    void InputManager::update() {
        mKeyboard->update();
        mMouse->update();
    }

} //Framework::Input 
