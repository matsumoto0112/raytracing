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
        * @brief �R���X�g���N�^
        */
        GLBLoader(const std::string& filepath);
        /**
        * @brief �f�X�g���N�^
        */
        ~GLBLoader();
        /**
        * @brief �摜�f�[�^���擾����
        */
        std::vector<TextureData> getImageDatas() const;
        /**
        * @brief �}�e���A���f�[�^���擾����
        */
        std::vector<Material> getMaterialDatas() const;
        /**
        * @brief �T�u���b�V�����Ƃ̒��_�C���f�b�N�X���擾����
        */
        std::vector<std::vector<uint16_t>> getIndicesPerSubMeshes() const;
        /**
        * @brief �T�u���b�V�����Ƃ̒��_���W���擾����
        */
        std::vector<std::vector<Math::Vector3>> getPositionsPerSubMeshes() const;
        /**
        * @brief �T�u���b�V�����Ƃ̖@�����擾����
        */
        std::vector<std::vector<Math::Vector3>> getNormalsPerSubMeshes() const;
        /**
        * @brief �T�u���b�V�����Ƃ�UV���W���擾����
        */
        std::vector<std::vector<Math::Vector2>> getUVsPerSubMeshes() const;
    private:
        std::unique_ptr<Microsoft::glTF::GLBResourceReader> mResourceReader;
        Microsoft::glTF::Document mDocument;
    };
} //Framework::Utility 