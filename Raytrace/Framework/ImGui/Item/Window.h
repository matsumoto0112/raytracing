#pragma once
#include "ImGui/Item/IItem.h"

namespace Framework::ImGUI {
    /**
    * @class Window
    * @brief ImGUI�E�B���h�E�N���X
    */
    class Window {
    public:
        /**
        * @brief �R���X�g���N�^
        * @param name �E�B���h�E��
        */
        Window(const std::string& name);
        /**
        * @brief �f�X�g���N�^
        */
        ~Window();
        /**
        * @brief �A�C�e����ǉ�����
        * @param �ǉ�����A�C�e��
        */
        void addItem(std::shared_ptr<IItem> item);
        /**
        * @brief �`��
        */
        void draw();
    private:
        std::string mName;
        std::vector<std::shared_ptr<IItem>> mItems;
    };
} //Framework::ImGUI 