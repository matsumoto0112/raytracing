#pragma once

#include <wincodec.h>
#include <memory>
#include <vector>
#include <string>

namespace Framework::Utility {
/**
* @class TextureLoader
* @brief テクスチャ読み込み機
*/
class TextureLoader {
public:
    /**
    * @brief コンストラクタ
    */
    TextureLoader();
    /**
    * @brief デストラクタ
    */
    ~TextureLoader();
    /**
    * @brief テクスチャ読み込み
    * @param filepath ファイルへのパス
    * @param[out] width テクスチャの幅
    * @param[out] height テクスチャの高さ
    */
    std::vector<BYTE> load(const std::wstring& filepath, _Out_ UINT* width, _Out_ UINT* height);
private:
    IWICBitmapDecoder* mDecoder; //!< デコーダー
    IWICBitmapFrameDecode* mFrame; //!< フレームデコード
    IWICFormatConverter* mConverter; //!< フォーマット変換器
    IWICImagingFactory* mFactory; //!< イメージ生成器
};

} //Framework::Utility
