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
        std::vector<std::vector<BYTE>> getImageDatas() const;
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
        std::vector<std::vector<Vector3>> getPositionsPerSubMeshes() const;
        std::vector<std::vector<Vector3>> getNormalsPerSubMeshes() const;
    private:
        std::unique_ptr<Microsoft::glTF::GLBResourceReader> mResourceReader;
        Microsoft::glTF::Document mDocument;
    };
} //Framework::Utility 