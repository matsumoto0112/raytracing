#include "GLBLoader.h"
#include <fstream>
#include "Libs/stb/stb_image.h"

using namespace Microsoft::glTF;

namespace {
    /**
    * @class StreamReader
    * @brief discription
    */
    class StreamReader : public IStreamReader {
    public:
        /**
        * @brief コンストラクタ
        */
        StreamReader() { }
        /**
        * @brief デストラクタ
        */
        ~StreamReader() { }
        //データを取得する
        std::shared_ptr<std::istream> GetInputStream(const std::string& filepath) const override {
            auto stream = std::make_shared<std::ifstream>(filepath, std::ios::binary);
            return stream;
        }
    };
}

namespace Framework::Utility {
    GLBLoader::GLBLoader(const std::string& filepath) {
        auto streamReader = std::make_unique<StreamReader>();
        auto glbStream = streamReader->GetInputStream(filepath);
        mResourceReader = std::make_unique<GLBResourceReader>(std::move(streamReader), std::move(glbStream));
        auto manifest = mResourceReader->GetJson();
        mDocument = Deserialize(manifest);
    }

    GLBLoader::~GLBLoader() { }

    std::vector<TextureData> GLBLoader::getImageDatas() const {
        std::vector<TextureData> result;
        for (auto&& image : mDocument.images.Elements()) {
            TextureData tex;
            tex.textureSizePerPixel = 4;
            std::vector<BYTE> texRowData = mResourceReader->ReadBinaryData(mDocument, image);
            int width, height, bpp;

            BYTE* texByte = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(texRowData.data()), texRowData.size(), &width, &height, &bpp, 0);
            const int size = width * height * tex.textureSizePerPixel;
            //std::vector<BYTE> textureByte(size);
            //for (int i = 0; i < size; i++) {
            //    textureByte[i] = static_cast<BYTE>(pixels[i] * 255.0f);
            //}
            std::vector<BYTE> textureByte(texByte, texByte + size);

            tex.data = textureByte;
            tex.width = width;
            tex.height = height;
            result.emplace_back(tex);
        }
        return result;
    }

    std::vector<Material> GLBLoader::getMaterialDatas() const {
        std::vector<Material> result;
        for (auto&& mat : mDocument.materials.Elements()) {
            Material material;
            material.name = mat.name;
            //マップが存在しなければ-1にしておく
            material.normalMapID = (mat.normalTexture.textureId != "") ? std::stoi(mat.normalTexture.textureId) : -1;
            //gltfのアルファモードによる切り替え
            switch (mat.alphaMode) {
                case ALPHA_UNKNOWN:
                case ALPHA_OPAQUE:
                    material.alphaMode = AlphaMode::Opaque;
                    break;
                case ALPHA_BLEND:
                    material.alphaMode = AlphaMode::Blend;
                    break;
                default:
                    material.alphaMode = AlphaMode::Mask;
                    break;
            }
            result.emplace_back(material);
        }
        return result;
    }
    std::vector<std::vector<uint16_t>> GLBLoader::getIndicesPerSubMeshes() const {
        std::vector<std::vector<uint16_t>> result;
        for (auto&& mesh : mDocument.meshes.Elements()) {
            for (auto&& prim : mesh.primitives) {
                auto&& index_accesor = mDocument.accessors.Get(prim.indicesAccessorId);
                auto data_uint8 = mResourceReader->ReadBinaryData<uint16_t>(mDocument, index_accesor);
                std::vector<uint16_t> data(data_uint8.size());
                const size_t size = data.size() / 3;
                for (int i = 0; i < size; i++) {
                    data[i * 3 + 2] = static_cast<uint16_t>(data_uint8[i * 3 + 2]);
                    data[i * 3 + 1] = static_cast<uint16_t>(data_uint8[i * 3 + 1]);
                    data[i * 3 + 0] = static_cast<uint16_t>(data_uint8[i * 3 + 0]);
                }
                result.emplace_back(data);
            }
        }
        return result;
    }
    std::vector<std::vector<Math::Vector3>> GLBLoader::getPositionsPerSubMeshes() const {
        std::vector<std::vector<Math::Vector3>> result;
        for (auto&& mesh : mDocument.meshes.Elements()) {
            for (auto&& prim : mesh.primitives) {
                std::string accessorID;
                if (!prim.TryGetAttributeAccessorId(ACCESSOR_POSITION, accessorID))continue;
                auto&& accessor = mDocument.accessors.Get(accessorID);
                std::vector<float> data = mResourceReader->ReadBinaryData<float>(mDocument, accessor);
                const int elemCount = 3;
                const int vertexSize = static_cast<int>(data.size()) / elemCount;
                std::vector<Math::Vector3> positions(vertexSize);
                for (int i = 0; i < vertexSize; i++) {
                    positions[i] = Math::Vector3(data[i * elemCount], data[i * elemCount + 1], data[i * elemCount + 2]);
                }
                result.emplace_back(positions);
            }
        }
        return result;
    }
    std::vector<std::vector<Math::Vector3>> GLBLoader::getNormalsPerSubMeshes() const {
        std::vector<std::vector<Math::Vector3>> result;
        for (auto&& mesh : mDocument.meshes.Elements()) {
            for (auto&& prim : mesh.primitives) {
                std::string accessorID;
                if (!prim.TryGetAttributeAccessorId(ACCESSOR_NORMAL, accessorID))continue;
                auto&& accessor = mDocument.accessors.Get(accessorID);
                std::vector<float> data = mResourceReader->ReadBinaryData<float>(mDocument, accessor);
                const int elemCount = 3;
                const int vertexSize = static_cast<int>(data.size()) / elemCount;
                std::vector<Math::Vector3> normals(vertexSize);
                for (int i = 0; i < vertexSize; i++) {
                    normals[i] = Math::Vector3(data[i * elemCount], data[i * elemCount + 1], data[i * elemCount + 2]);
                }
                result.emplace_back(normals);
            }
        }
        return result;
    }
    std::vector<std::vector<Math::Vector2>> GLBLoader::getUVsPerSubMeshes() const {
        std::vector<std::vector<Math::Vector2>> result;
        for (auto&& mesh : mDocument.meshes.Elements()) {
            for (auto&& prim : mesh.primitives) {
                std::string accessorID;
                if (!prim.TryGetAttributeAccessorId(ACCESSOR_TEXCOORD_0, accessorID))continue;
                auto&& accessor = mDocument.accessors.Get(accessorID);
                std::vector<float> data = mResourceReader->ReadBinaryData<float>(mDocument, accessor);
                const int elemCount = 2;
                const int vertexSize = static_cast<int>(data.size()) / elemCount;
                std::vector<Math::Vector2> uvs(vertexSize);
                for (int i = 0; i < vertexSize; i++) {
                    uvs[i] = Math::Vector2(data[i * elemCount], data[i * elemCount + 1]);
                }
                result.emplace_back(uvs);
            }
        }
        return result;
    }
} //Framework::Utility 
