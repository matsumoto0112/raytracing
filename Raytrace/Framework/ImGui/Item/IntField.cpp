#include "IntField.h"

namespace Framework::ImGUI {
    //コンストラクタ
    IntField::IntField(const std::string& text, int defaultValue)
        :CallBackItem(text), mValue(defaultValue), mMinValue(0), mMaxValue(1) { }
    //デストラクタ
    IntField::~IntField() { }
    //描画
    void IntField::draw() {
        if (ImGui::SliderInt(mName.c_str(), &mValue, mMinValue, mMaxValue) && mCallBack) {
            if (mCallBack) mCallBack(mValue);
        }
    }
} //Framework::ImGUI
