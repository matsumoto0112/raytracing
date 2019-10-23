#include "Keyboard.h"
#include "Framework/Input/KeyCode.h"
#include "Framework/Utility/Debug.h"

namespace Framework::Input {
    //コンストラクタ
    Keyboard::Keyboard(HWND hWnd)
        :mResult(), mCurrentKeys(), mPrevKeys() { }
    //デストラクタ
    Keyboard::~Keyboard() { }
    //更新
    void Keyboard::update() {
        //前フレームのキーの情報をコピーする
        std::copy(mCurrentKeys.begin(), mCurrentKeys.end(), mPrevKeys.begin());
        //現在のキーの押下状態を取得する
        GetKeyboardState(mCurrentKeys.data());
    }
    //キーの押下情報の取得
    bool Keyboard::getKey(KeyCode key) const {
        return checkKeyDown(mCurrentKeys, key);
    }
    //キーの押した瞬間かどうかを取得
    bool Keyboard::getKeyDown(KeyCode key) const {
        bool prev = checkKeyDown(mPrevKeys, key);
        bool cur = checkKeyDown(mCurrentKeys, key);
        return !prev && cur;
    }
    //キーの離した瞬間かどうかを取得
    bool Keyboard::getKeyUp(KeyCode key) const {
        bool prev = checkKeyDown(mPrevKeys, key);
        bool cur = checkKeyDown(mCurrentKeys, key);
        return prev && !cur;
    }
    //キーが押されているかどうか判定
    bool Keyboard::checkKeyDown(const KeyInfo& keys, KeyCode key) const {
        return (keys[key] & 0x80) != 0;
    }

} //Framework::Input
