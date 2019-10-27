#pragma once
#include <string>
#include <Windows.h>
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/Deserialize.h>
#include "Framework/Math/Vector2.h"
#include "Framework/Math/Vector3.h"

namespace Framework::Utility {
    enum class AlphaMode {
        Opaque,
        Blend,
        Mask,
    };

    struct TextureData {
        std::vector<BYTE> data;
        UINT width;
        UINT height;
        UINT textureSizePerPixel;
    };

    struct Material {
        std::string name;
        int normalMapID;
        AlphaMode alphaMode;
    };

    /**
    * @class GLBLoader
    * @brief discription
    */
    class GLBLoader {
    public:
        /**
        * @brief コンストラクタ
        */
        GLBLoader(const std::string& filepath);
        /**
        * @brief デストラクタ
        */
        ~GLBLoader();
        /**
        * @brief 画像データを取得する
        */
        std::vector<TextureData> getImageDatas() const;
        /**
        * @brief マテリアルデータを取得する
        */
        std::vector<Material> getMaterialDatas() const;
        /**
        * @brief サブメッシュごとの頂点インデックスを取得する
        */
        std::vector<std::vector<uint16_t>> getIndicesPerSubMeshes() const;
        /**
        * @brief サブメッシュごとの頂点座標を取得する
        */
        std::vector<std::vector<Math::Vector3>> getPositionsPerSubMeshes() const;
        /**
        * @brief サブメッシュごとの法線を取得する
        */
        std::vector<std::vector<Math::Vector3>> getNormalsPerSubMeshes() const;
        /**
        * @brief サブメッシュごとのUV座標を取得する
        */
        std::vector<std::vector<Math::Vector2>> getUVsPerSubMeshes() const;
    private:
        std::unique_ptr<Microsoft::glTF::GLBResourceReader> mResourceReader;
        Microsoft::glTF::Document mDocument;
    };
} //Framework::Utility 