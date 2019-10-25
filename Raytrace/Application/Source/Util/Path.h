#pragma once

#include <string>
#include "Framework/Utility/Singleton.h"

/**
* @class Path
* @brief 各種ファイルへのパスを定義
*/
class Path : public Framework::Utility::Singleton<Path> {
public:
    /**
    * @brief 実行ファイルのフォルダパスを取得する
    */
    const std::wstring& exe() const { return mExe; }
    /**
    * @brief リソースフォルダまでのパス
    */
    const std::wstring& resource() const { return mResource; }
    /**
    * @brief テクスチャフォルダまでのパス
    */
    const std::wstring& texture() const { return mTexture; }
    /**
    * @brief シェーダーファイルへのパス
    */
    const std::wstring& shader() const { return mShader; }
    /**
    * @brief モデルのあるファイルへのパス
    */
    const std::wstring& model() const { return mModel; }
private:
    std::wstring mExe; //!< 実行ファイルへのパス
    std::wstring mResource; //!< リソースフォルダへのパス
    std::wstring mTexture; //!< テクスチャのルートフォルダへのパス
    std::wstring mShader; //!< シェーダファイルへのパス
    std::wstring mModel; //!< モデルデータファイルへのパス
protected:
    Path();
    ~Path();
};
