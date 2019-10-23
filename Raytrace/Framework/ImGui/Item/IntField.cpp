#include "IntField.h"

namespace Framework::ImGUI {
    //�R���X�g���N�^
    IntField::IntField(const std::string& text, int defaultValue)
        :CallBackItem(text), mValue(defaultValue), mMinValue(0.0f), mMaxValue(1.0f) { }
    //�f�X�g���N�^
    IntField::~IntField() { }
    //�`��
    void IntField::draw() {
        if (ImGui::SliderInt(mName.c_str(), &mValue, mMinValue, mMaxValue) && mCallBack) {
            if (mCallBack) mCallBack(mValue);
        }
    }
} //Framework::ImGUI