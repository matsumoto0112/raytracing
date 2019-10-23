#include "Window.h"

namespace Framework::ImGUI {
    //�R���X�g���N�^
    Window::Window(const std::string& name)
        :mName(name), mItems() { }
    //�f�X�g���N�^
    Window::~Window() {
        mItems.clear();
    }
    //�A�C�e���̒ǉ�
    void Window::addItem(std::shared_ptr<IItem> item) {
        mItems.emplace_back(item);
    }
    //�`��
    void Framework::ImGUI::Window::draw() {
        ImGui::Begin(mName.c_str());
        for (auto&& item : mItems) {
            item->draw();
        }
        ImGui::End();
    }

} //Framework::ImGUI 