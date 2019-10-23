#pragma once
#include <string>
#include <codecvt>
#include <Windows.h>

namespace Framework::Utility {
    /**
    * @brief std::string‚©‚çstd::wstring‚É•ÏŠ·‚·‚é
    */
    inline std::wstring toWString(const std::string& str) {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }
} //Framework::Utility 