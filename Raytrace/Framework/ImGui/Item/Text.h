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
    * @brief �R���X�g���N�^
    * @param text �e�L�X�g
    */
    Text(const std::string& text);
    /**
    * @brief �f�X�g���N�^
    */
    ~Text();
    /**
    * @brief �e�L�X�g��ݒ肷��
    */
    void setText(const std::string& text);
    /**
    * @brief �e�L�X�g���擾����
    */
    const std::string& getText() const { return getName(); }
    /**
    * @brief �`��
    */
    virtual void draw() override;
};

} //ImGUI 
} //Framework
