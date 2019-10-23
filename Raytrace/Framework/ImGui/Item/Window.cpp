#include "Window.h"

namespace Framework::ImGUI {
    //コンストラクタ
    Window::Window(const std::string& name)
        :mName(name), mItems() { }
    //デストラクタ
    Window::~Window() {
        mItems.clear();
    }
    //アイテムの追加
    void Window::addItem(std::shared_ptr<IItem> item) {
        mItems.emplace_back(item);
    }
    //描画
    void Framework::ImGUI::Window::draw() {
        ImGui::Begin(mName.c_str());
        for (auto&& item : mItems) {
            item->draw();
        }
        ImGui::End();
    }

} //Framework::ImGUI 