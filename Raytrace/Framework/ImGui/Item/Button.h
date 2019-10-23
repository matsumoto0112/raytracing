#pragma once
#include <functional>
#include "ImGui/Item/CallBackItem.h"

namespace Framework::ImGUI {

    /**
    * @class Button
    * @brief ImGUIボタンクラス
    */
    class Button :public CallBackItem<void> {
    public:
        /**
        * @brief コンストラクタ
        * @param text テキスト
        * @param callFunc ボタンが押されたときに呼ぶ関数
        */
        Button(const std::string& text, CallBack callFunc = nullptr);
        /**
        * @brief デストラクタ
        */
        ~Button();
        /**
        * @brief 描画
        */
        virtual void draw() override;
    };
} //Framework::ImGUI
