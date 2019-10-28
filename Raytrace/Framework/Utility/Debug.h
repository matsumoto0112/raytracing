#pragma once
#include <d3d12.h>
#include "Framework/Utility/StringBuilder.h"

/**
* @def VisualStudioの出力にログ出力
* @param mes 表示するメッセージ
*/
#define MY_DEBUG_LOG(mes) Framework::Utility::Debug::debugLog(__FUNCTION__, __LINE__, mes)

/**
* @def アサーション
* @param cond 条件式
* @param mes condがfalseの時に表示するメッセージ
*/
#define MY_ASSERTION(cond,mes) Framework::Utility::Debug::assertion(__FUNCTION__, __LINE__, cond, mes)

/**
* @def エラーウィンドウの表示
* @param cond 条件式
* @param mes condがfalseの時に表示するメッセージ
* @details デバッグ時にはメッセージとともに発生した関数名、行番号が表示される
*/
#define MY_ERROR_WINDOW(cond,mes) Framework::Utility::Debug::errorWindow(__FUNCTION__, __LINE__, cond, mes)

/**
* @def 条件を満たしていたらVisualStudioの出力にログ出力
* @param cond 条件式
* @param mes 表示するメッセージ
*/
#define MY_DEBUG_LOG_IF(cond,mes) if((cond)) MY_DEBUG_LOG(mes)

#define MY_THROW_IF_FAILED(hr,mes) Framework::Utility::throwIfFailed(hr,mes)

#define MY_THROW_IF_FALSE(cond,mes) Framework::Utility::throwIfFalse(cond,mes)

namespace Framework::Utility {
    /**
    * @brief デバッグ用インターフェース
    */
    class Debug {
    public:
        /**
        * @brief アサーション
        * @param funcName 関数名
        * @param line 発生した行
        * @param condition 条件
        * @param message エラーメッセージ
        */
        static void assertion(const std::string& funcName, int line,
            bool condition, const std::wstring& message);
        /**
        * @brief ログ出力
        * @param funcName 関数名
        * @param line 発生した行
        * @param message 出力内容
        */
        static void debugLog(const std::string& funcName, int line, const std::wstring& message);
        /**
        * @brief エラーウィンドウの表示
        * @param funcName 関数名
        * @param line 発生した行
        * @param condition 条件式
        * @param message エラーメッセージ
        */
        static void errorWindow(const std::string& funcName, int line,
            bool condition, const std::wstring& message);
        /**
        * @brief エラーメッセージの作成
        * @param funcName 関数名
        * @param line 発生した行
        * @param message 実際のエラーメッセージ
        * @return [funcName line:message]の形式のメッセージを返す
        */
        static std::wstring errorMessage(const std::string& funcName, int line,
            const std::wstring& message);
    };

    /**
    * @brief HRESULT例外クラス
    */
    class HrException : public std::runtime_error {
        inline std::string HrToString(HRESULT hr) {
            char s_str[64] = {};
            sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
            return std::string(s_str);
        }
    public:
        HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) { }
        HRESULT Error() const { return m_hr; }
    private:
        const HRESULT m_hr;
    };

    /**
    * @brief 失敗していたら例外を投げる
    */
    inline void throwIfFailed(HRESULT hr) {
        if (FAILED(hr)) {
            throw HrException(hr);
        }
    }

    /**
    * @brief 失敗していたら例外を投げる
    */
    inline void throwIfFailed(HRESULT hr, const wchar_t* mes) {
        if (FAILED(hr)) {
            MY_DEBUG_LOG(mes);
            throw HrException(hr);
        }
    }

    /**
    * @brief 失敗していたら例外を投げる
    */
    inline void throwIfFalse(bool value) {
        throwIfFailed(value ? S_OK : E_FAIL);
    }

    /**
    * @brief 失敗していたら例外を投げる
    */
    inline void throwIfFalse(bool value, const wchar_t* mes) {
        throwIfFailed(value ? S_OK : E_FAIL, mes);
    }

} //Framework::Utility