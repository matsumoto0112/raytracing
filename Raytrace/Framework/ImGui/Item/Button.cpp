#include "Button.h"

namespace Framework::ImGUI {
    //�R���X�g���N�^
    Button::Button(const std::string& text, CallBack callFunc)
        :CallBackItem(text) {
        setCallBack(callFunc);
    }
    //�f�X�g���N�^
    Button::~Button() { }
    //�`��
    void Button::draw() {
        if (ImGui::Button(mName.c_str())) {
            if (mCallBack) mCallBack();
        }
    }
} //Framework::ImGUI
