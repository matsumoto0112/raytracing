#include "Button.h"

namespace Framework::ImGUI {
    //コンストラクタ
    Button::Button(const std::string& text, CallBack callFunc)
        :CallBackItem(text) {
        setCallBack(callFunc);
    }
    //デストラクタ
    Button::~Button() { }
    //描画
    void Button::draw() {
        if (ImGui::Button(mName.c_str())) {
            if (mCallBack) mCallBack();
        }
    }
} //Framework::ImGUI
