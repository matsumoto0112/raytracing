#include "Path.h"
#include <Windows.h>
#include <iostream>
#include "Framework/Utility/Debug.h"

namespace {
    constexpr int MAX_SIZE = 1024;
    //実行ファイルまでのパスを取得する
    std::wstring getExePath() {
        wchar_t path[MAX_SIZE];
        DWORD ret = GetModuleFileName(nullptr, path, sizeof(path));
        std::wstring res(path);
        return res;
    }
}

Path::Path() {
    mExe = getExePath();
    wchar_t dir[MAX_SIZE] = {};
    wchar_t drive[MAX_SIZE] = {};
    wchar_t name[MAX_SIZE] = {};
    wchar_t ext[MAX_SIZE] = {};

    _wsplitpath_s(&mExe[0],
        drive, sizeof(drive) / sizeof(drive[0]),
        dir, sizeof(dir) / sizeof(dir[0]),
        name, sizeof(name) / sizeof(name[0]),
        ext, sizeof(ext) / sizeof(ext[0]));
    std::wstring sDir(drive);
    sDir += dir;
    std::wstring::size_type pos = sDir.find_last_of(L"\\/");
    sDir = sDir.substr(0, pos);

    mShader = sDir + L"/cso/Resources/Shader/";
    mResource = sDir + L"/Resources/";
    mModel = mResource + L"Model/";
    mTexture = mResource + L"Textures/";
    mCompiledShader = sDir + L"/CompiledShader/";
}

Path::~Path() { }
