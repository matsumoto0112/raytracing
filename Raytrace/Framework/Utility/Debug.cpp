#include "Debug.h"
#include <crtdbg.h>
#include <Windows.h>
#include <sstream>
#include <codecvt>

namespace Framework::Utility {
    //エラーウィンドウ表示
    void errorWindow(bool condition, const std::wstring& message) {
        if (condition)return;
        int res = MessageBoxW(GetWindow(nullptr, 0), message.c_str(), L"エラー", IDOK);
        PostQuitMessage(0);
    }

    void Debug::assertion(const std::string& funcName, int line,
        bool condition, const std::wstring& message) {
        if (condition)return;
        _ASSERT_EXPR(condition, errorMessage(funcName, line, message).c_str());
    }

    void Debug::debugLog(const std::string& funcName, int line, const std::wstring& message) {
        _RPTW0(_CRT_WARN, errorMessage(funcName, line, message).c_str());
    }

    void Debug::errorWindow(const std::string& funcName, int line,
        bool condition, const std::wstring& message) {
        if (condition)return;
        std::wstring mes;

    #if _DEBUG //デバッグ時は詳細な情報を出力する
        mes = errorMessage(funcName, line, message);
    #else
        mes = message;
    #endif

        MessageBoxW(nullptr, mes.c_str(), L"エラー", MB_ICONINFORMATION);
        exit(-1);
    }

    std::wstring Debug::errorMessage(const std::string& funcName,
        int line, const std::wstring& message) {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &funcName[0], (int)funcName.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &funcName[0], (int)funcName.size(), &wstrTo[0], size_needed);
        return StringBuilder(wstrTo) << " " << line << ":" << message << "\n";
    }
} //Framework::Utility
