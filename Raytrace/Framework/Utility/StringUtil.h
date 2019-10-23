#pragma once
#include <string>
#include <codecvt>
#include <Windows.h>

namespace Framework::Utility {
    /**
    * @brief std::string‚©‚çstd::wstring‚É•ÏŠ·‚·‚é
    */
    inline std::wstring toWString(const std::string& str) {
        const int len = ::MultiByteToWideChar(932, 0, str.c_str(), -1, nullptr, 0);
        std::wstring res(len * 2 + 2, L'\0');
        if (!::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &res[0], len)) {
            throw std::runtime_error("function toWString ERROR");
        }

        const std::size_t real_len = std::wcslen(res.c_str());
        res.resize(real_len);
        res.shrink_to_fit();
        return res;
    }

    /**
    * @brief std::wstring‚©‚çstd::string‚É•ÏŠ·‚·‚é
    */
    inline std::string toString(const std::wstring& str) {
        const int len = ::WideCharToMultiByte(932, 0, str.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string res(len * 2, '\0');
        if (!::WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, &res[0], len, nullptr, nullptr)) {
            throw std::runtime_error("function toString ERROR");
        }
        const std::size_t real_len = std::strlen(res.c_str());
        res.resize(real_len);
        res.shrink_to_fit();
        return res;
    }
} //Framework::Utility 