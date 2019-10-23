#pragma once

#include <functional>
#include "ImGui/Item/CallBackItem.h"

namespace Framework::ImGUI {
    /**
    * @class IntField
    * @brief 整数値入力エリア
    */
    class IntField :public CallBackItem<int> {
    public:
        /**
        * @brief コンストラクタ
        * @param text テキスト
        * @param defaultValue デフォルト値
        */
        IntField(const std::string& text, int defaultValue);
        /**
        * @brief デストラクタ
        */
        ~IntField();
        /**
        * @brief 描画
        */
        virtual void draw() override;
    private:
        int mValue;
        int mMinValue;
        int mMaxValue;
    };
} //Framework::ImGUI