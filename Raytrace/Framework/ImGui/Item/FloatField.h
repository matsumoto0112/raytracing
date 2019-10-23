#pragma once
#include "ImGui/Item/CallBackItem.h"

namespace Framework::ImGUI {
    /**
    * @class FloatField
    * @brief ���������_���̓G���A
    */
    class FloatField : public CallBackItem<float> {
    public:
        /**
        * @brief �R���X�g���N�^
        * @param text �e�L�X�g
        * @param defaultValue �f�t�H���g�l
        */
        FloatField(const std::string& text, float defaultValue);
        /**
        * @brief �f�X�g���N�^
        */
        ~FloatField();
        /**
        * @brief �`��
        */
        virtual void draw() override;
        float getValue() const { return mValue; }
        float getMinValue() const { return mMinValue; }
        float getMaxValue() const { return mMaxValue; }
        void setMinValue(float value) { mMinValue = value; }
        void setMaxValue(float value) { mMaxValue = value; }
    private:
        float mValue; //!< ���݂̒l
        float mMinValue; //!< �ŏ��l
        float mMaxValue; //!< �ő�l
    };
} //Framework ::ImGUI