#pragma once
#include "ImGui/Item/IItem.h"

namespace Framework::ImGUI {
    /**
    * @class Window
    * @brief ImGUIウィンドウクラス
    */
    class Window {
    public:
        /**
        * @brief コンストラクタ
        * @param name ウィンドウ名
        */
        Window(const std::string& name);
        /**
        * @brief デストラクタ
        */
        ~Window();
        /**
        * @brief アイテムを追加する
        * @param 追加するアイテム
        */
        void addItem(std::shared_ptr<IItem> item);
        /**
        * @brief 描画
        */
        void draw();
    private:
        std::string mName;
        std::vector<std::shared_ptr<IItem>> mItems;
    };
} //Framework::ImGUI 