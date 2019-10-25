#pragma once
#include <string>
#include <Windows.h>
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/Deserialize.h>

namespace Framework::Utility {
    enum class AlphaMode {
        Opaque,
        Blend,
        Mask,
    };

    struct Material {
        std::string name;
        int normalMapID;
        AlphaMode alphaMode;
    };

    struct Vector3 {
        float x;
        float y;
        float z;
        Vector3() { }
        Vector3(float x, float y, float z)
            :x(x), y(y), z(z) { }
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
        std::vector<std::vector<BYTE>> getImageDatas() const;
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
        std::vector<std::vector<Vector3>> getPositionsPerSubMeshes() const;
        std::vector<std::vector<Vector3>> getNormalsPerSubMeshes() const;
    private:
        std::unique_ptr<Microsoft::glTF::GLBResourceReader> mResourceReader;
        Microsoft::glTF::Document mDocument;
    };
} //Framework::Utility 