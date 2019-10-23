#include "FloatField.h"

namespace Framework {
    namespace ImGUI {
        FloatField::FloatField(const std::string& text, float defaultValue)
            :CallBackItem(text), mValue(defaultValue), mMinValue(0.0f), mMaxValue(1.0f) { }

        FloatField::~FloatField() { }

        void FloatField::draw() {
            if (ImGui::SliderFloat(mName.c_str(), &mValue, mMinValue, mMaxValue)) {
                if (mCallBack) mCallBack(mValue);
            }
        }

    } //Imgui 
} //Framework 
