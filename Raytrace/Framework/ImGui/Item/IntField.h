#pragma once

#include <functional>
#include "ImGui/Item/CallBackItem.h"

namespace Framework::ImGUI {
    /**
    * @class IntField
    * @brief �����l���̓G���A
    */
    class IntField :public CallBackItem<int> {
    public:
        /**
        * @brief �R���X�g���N�^
        * @param text �e�L�X�g
        * @param defaultValue �f�t�H���g�l
        */
        IntField(const std::string& text, int defaultValue);
        /**
        * @brief �f�X�g���N�^
        */
        ~IntField();
        /**
        * @brief �`��
        */
        virtual void draw() override;
    private:
        int mValue;
        int mMinValue;
        int mMaxValue;
    };
} //Framework::ImGUI