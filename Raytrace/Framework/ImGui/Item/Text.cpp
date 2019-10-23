#include "Text.h"

namespace Framework::ImGUI {
Text::Text(const std::string& text)
    :IItem(text) {}

Text::~Text() {}

void Text::setText(const std::string& text) {
    mName = text;
}

void Text::draw() {
    ImGui::Text(mName.c_str());
}

} //Framework::ImGUI
