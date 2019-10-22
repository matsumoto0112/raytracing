#pragma once

#include <string>
#include <sstream>

namespace Framework::Utility {
    /**
    * @class StringBuilder
    * @brief ������A���N���X
    */
    class StringBuilder {
    public:
        /**
        * @brief �R���X�g���N�^
        */
        StringBuilder() { wss.clear(); }
        template<class T>
        StringBuilder(const T& src) {
            wss.clear();
            wss << src;
        }
        /**
        * @brief �f�X�g���N�^
        */
        ~StringBuilder() = default;
        /**
        * @brief ������̌���
        */
        StringBuilder& operator<<(const StringBuilder& src) {
            wss << src.wss.str();
            return *this;
        }
        /**
        * @brief ������̌���
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