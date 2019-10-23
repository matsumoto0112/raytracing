#pragma once
#include "ImGui/Item/CallBackItem.h"

namespace Framework::ImGUI {
    /**
    * @class FloatField
    * @brief 浮動小数点入力エリア
    */
    class FloatField : public CallBackItem<float> {
    public:
        /**
        * @brief コンストラクタ
        * @param text テキスト
        * @param defaultValue デフォルト値
        */
        FloatField(const std::string& text, float defaultValue);
        /**
        * @brief デストラクタ
        */
        ~FloatField();
        /**
        * @brief 描画
        */
        virtual void draw() override;
        float getValue() const { return mValue; }
        float getMinValue() const { return mMinValue; }
        float getMaxValue() const { return mMaxValue; }
        void setMinValue(float value) { mMinValue = value; }
        void setMaxValue(float value) { mMaxValue = value; }
    private:
        float mValue; //!< 現在の値
        float mMinValue; //!< 最小値
        float mMaxValue; //!< 最大値
    };
} //Framework ::ImGUI