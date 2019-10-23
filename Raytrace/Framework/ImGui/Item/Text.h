#pragma once
#include "ImGui/Item/IItem.h"

namespace Framework {
namespace ImGUI {

/**
* @class Text
* @brief discription
*/
class Text :public IItem {
public:
    /**
    * @brief コンストラクタ
    * @param text テキスト
    */
    Text(const std::string& text);
    /**
    * @brief デストラクタ
    */
    ~Text();
    /**
    * @brief テキストを設定する
    */
    void setText(const std::string& text);
    /**
    * @brief テキストを取得する
    */
    const std::string& getText() const { return getName(); }
    /**
    * @brief 描画
    */
    virtual void draw() override;
};

} //ImGUI 
} //Framework
