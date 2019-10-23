#pragma once
#include <functional>
#include "ImGui/Item/IItem.h"

namespace Framework::ImGUI {
    /**
    * @class CallBackItem
    * @brief �R�[���o�b�N�̂���A�C�e�����N���X
    */
    template <class T>
    class CallBackItem : public IItem {
    protected:
        using CallBack = std::function<void(T)>;
    public:
        /**
        * @brief �R���X�g���N�^
        * @param label ���x����
        */
        CallBackItem(const std::string& label)
            :IItem(label), mCallBack(nullptr) { }
        /**
        * @brief �f�X�g���N�^
        */
        virtual ~CallBackItem() { }
        /**
        * @brief �R�[���o�b�N�̓o�^
        */
        void setCallBack(CallBack callBack) { mCallBack = callBack; }
    protected:
        CallBack mCallBack; //!< �R�[���o�b�N�֐�
    };
} //Framework::ImGUI 