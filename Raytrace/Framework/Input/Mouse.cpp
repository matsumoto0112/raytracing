#include "Mouse.h"

namespace Framework::Input {
    //コンストラクタ
    Mouse::Mouse(HWND hWnd)
        :mHWnd(hWnd) {
    #define CHECK_MOUSE_BUTTON_PRESS(key) { \
        mCurrentMouseInfo[key] = GetKeyState(key) & 0x80; \
        } 

        //最初に押下状態を調べ、マップを作成しておく
        CHECK_MOUSE_BUTTON_PRESS(MouseButton::Left);
        CHECK_MOUSE_BUTTON_PRESS(MouseButton::Middle);
        CHECK_MOUSE_BUTTON_PRESS(MouseButton::Right);
    }
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

        std::copy(mCurrentMouseInfo.begin(), mCurrentMouseInfo.end(), mPrevMouseInfo.begin());
        CHECK_MOUSE_BUTTON_PRESS(MouseButton::Left);
        CHECK_MOUSE_BUTTON_PRESS(MouseButton::Middle);
        CHECK_MOUSE_BUTTON_PRESS(MouseButton::Right);
    }
    //マウスの今の座標を取得
    const Math::Vector2& Mouse::getMousePosition() const {
        return mPosition;
    }
    //マウスのボタンの押下状態を判定
    bool Mouse::getMouse(MouseButton button) const {
        return mCurrentMouseInfo.at(button);
    }
    //マウスのボタンが押されたかどうか判定
    bool Mouse::getMouseDown(MouseButton button) const {
        return mCurrentMouseInfo.at(button) && mPrevMouseInfo.at(button);
    }
    //マウスのボタンが離されたかどうか判定
    bool Mouse::getMouseUp(MouseButton button) const {
        return !mCurrentMouseInfo.at(button) && mPrevMouseInfo.at(button);
    }
    //マウスの前回からの移動量を取得
    Math::Vector2 Mouse::getMove() const {
        return mPosition - mPrevPosition;
    }
    //マウスが見えているかどうか判定する
    bool Mouse::isMouseVisible() const {
        return false;
    }
} //Framework::Input
