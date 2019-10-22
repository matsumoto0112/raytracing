#pragma once

#include <string>
#include <sstream>

namespace Framework::Utility {
    /**
    * @class StringBuilder
    * @brief 文字列連結クラス
    */
    class StringBuilder {
    public:
        /**
        * @brief コンストラクタ
        */
        StringBuilder() { wss.clear(); }
        template<class T>
        StringBuilder(const T& src) {
            wss.clear();
            wss << src;
        }
        /**
        * @brief デストラクタ
        */
        ~StringBuilder() = default;
        /**
        * @brief 文字列の結合
        */
        StringBuilder& operator<<(const StringBuilder& src) {
            wss << src.wss.str();
            return *this;
        }
        /**
        * @brief 文字列の結合
        */
        template <class T>
        StringBuilder& operator<<(const T& src) {
            wss << src;
            return *this;
        }
        StringBuilder operator+(const StringBuilder& other) {
            return StringBuilder(wss.str() + other.wss.str());
        }
        std::wstring getStr() const {
            return wss.str();
        }
        operator const wchar_t*() {
            return wss.str().c_str();
        }
        operator std::wstring() {
            return wss.str();
        }
    private:
        std::wstringstream wss;
    };
} //Framework::Utility 