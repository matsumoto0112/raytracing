#pragma once
#include <functional>
#include "ImGui/Item/CallBackItem.h"

namespace Framework::ImGUI {

    /**
    * @class Button
    * @brief ImGUI�{�^���N���X
    */
    class Button :public CallBackItem<void> {
    public:
        /**
        * @brief �R���X�g���N�^
        * @param text �e�L�X�g
        * @param callFunc �{�^���������ꂽ�Ƃ��ɌĂԊ֐�
        */
        Button(const std::string& text, CallBack callFunc = nullptr);
        /**
        * @brief �f�X�g���N�^
        */
        ~Button();
        /**
        * @brief �`��
        */
        virtual void draw() override;
    };
} //Framework::ImGUI
