#pragma once
#include <string>
#include <functional>
#include "ImGui/ImGuiInclude.h"

namespace Framework::ImGUI {

    /**
    * @class IItem
    * @brief ImGUI�̃A�C�e���C���^�[�t�F�[�X
    */
    class IItem {
    public:
        /**
        * @brief �R���X�g���N�^
        * @param name �A�C�e����
        */
        IItem(const std::string& name)
            :mName(name) { }
        /**
        * @brief �f�X�g���N�^
        */
        virtual ~IItem() = default;
        /**
        * @brief �`��
        */
        virtual void draw() = 0;
        /**
        * @brief ���O��ݒ肷��
        */
        void setName(const std::string& name) { mName = name; }
        /**
        * @brief ���O�̎擾
        */
        const std::string& getName() const { return mName; }
    protected:
        std::string mName; //!< �A�C�e����
    };

} //Framework::ImGUI
