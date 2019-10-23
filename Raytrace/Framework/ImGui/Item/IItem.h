#pragma once
#include <string>
#include <functional>
#include "ImGui/ImGuiInclude.h"

namespace Framework::ImGUI {

    /**
    * @class IItem
    * @brief ImGUIのアイテムインターフェース
    */
    class IItem {
    public:
        /**
        * @brief コンストラクタ
        * @param name アイテム名
        */
        IItem(const std::string& name)
            :mName(name) { }
        /**
        * @brief デストラクタ
        */
        virtual ~IItem() = default;
        /**
        * @brief 描画
        */
        virtual void draw() = 0;
        /**
        * @brief 名前を設定する
        */
        void setName(const std::string& name) { mName = name; }
        /**
        * @brief 名前の取得
        */
        const std::string& getName() const { return mName; }
    protected:
        std::string mName; //!< アイテム名
    };

} //Framework::ImGUI
