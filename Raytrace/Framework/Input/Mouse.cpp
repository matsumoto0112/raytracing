#include "Mouse.h"
#include "Framework/Input/Keyboard.h"
#include "Framework/Window/Window.h"

namespace Framework::Input {
    //コンストラクタ
    Mouse::Mouse(HWND hWnd, Keyboard& keyboard)
        :mHWnd(hWnd), mKeyboard(keyboard) { }
    //デストラクタ
    Mouse::~Mouse() { }
    //更新
    void Mouse::update() {
        //前の座標を更新する
        mPrevPosition = mPosition;

        //スクリーン座標を取得する
        POINT pos;
        GetCursorPos(&pos);
        //クライアント座標に変換する
        ScreenToClient(mHWnd, &pos);
        mPosition.x = (float)pos.x;
        mPosition.y = (float)pos.y;
    }
    //マウスの今の座標を取得
    const Math::Vector2& Mouse::getMousePosition() const {
        return mPosition;
    }
    //マウスのボタンの押下状態を判定
    bool Mouse::getMouse(MouseButton button) const {
        return mKeyboard.getKey(static_cast<KeyCode::Enum>(button));
    }
    //マウスのボタンが押されたかどうか判定
    bool Mouse::getMouseDown(MouseButton button) const {
        return mKeyboard.getKeyDown(static_cast<KeyCode::Enum>(button));
    }
    //マウスのボタンが離されたかどうか判定
    bool Mouse::getMouseUp(MouseButton button) const {
        return mKeyboard.getKeyUp(static_cast<KeyCode::Enum>(button));
    }
    //マウスの前回からの移動量を取得
    Math::Vector2 Mouse::getMove() const {
        return mPosition - mPrevPosition;
    }
} //Framework::Input
