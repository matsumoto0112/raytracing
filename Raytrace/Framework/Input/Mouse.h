#pragma once
#include <unordered_map>
#include "Framework/Input/KeyCode.h"
#include "Framework/Math/Vector2.h"

namespace Framework::Input {
    namespace MouseButton {
        /**
        * @enum Enum
        * @brief マウスのボタン定義
        */
        enum Enum {
            Left = KeyCode::LButton,
            Right = KeyCode::RButton,
            Middle = KeyCode::MButton,
        };
    }; //MouseButton

    /**
    * @class Mouse
    * @brief マウス情報管理クラス
    */
    class Mouse {
        using MouseButton = MouseButton::Enum;
    public:
        /**
        * @brief コンストラクタ
        * @param hWnd ウィンドウハンドル
        * @param keyboard キーボード
        * @details 入力の取得はキーボードと一緒に行うためキーボードが必要
        */
        Mouse(HWND hWnd, Keyboard& keyboard);
        /**
        * @brief デストラクタ
        */
        ~Mouse();
        /**
        * @brief 更新
        */
        void update();
        /**
        * @brief マウスの座標を取得
        * @return クライアント領域でのマウス座標を返す
        */
        const Math::Vector2& getMousePosition() const;
        /**
        * @brief マウスのボタンを押しているか
        * @param button ボタンの種類
        * @return 押している状態ならtrueを返す
        */
        bool getMouse(MouseButton button) const;
        /**
        * @brief マウスのボタンを押したか
        * @param button ボタンの種類
        * @return 押したタイミングならtrueを返す
        */
        bool getMouseDown(MouseButton button) const;
        /**
        * @brief マウスのボタンを離したか
        * @param button ボタンの種類
        * @return 離したタイミングならtrueを返す
        */
        bool getMouseUp(MouseButton button) const;
        /**
        * @brief マウスの移動量を取得
        */
        Math::Vector2 getMove() const;
    private:
        HWND mHWnd; //!< ウィンドウハンドル
        Keyboard& mKeyboard; //!< キーボード
        Math::Vector2 mPosition; //!< 今のマウスの座標
        Math::Vector2 mPrevPosition; //!< 前のマウスの座標
    };
} //Framework::Input
