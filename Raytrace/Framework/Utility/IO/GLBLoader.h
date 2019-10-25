#pragma once
#include <string>
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/Deserialize.h>

namespace Framework::Utility {
    /**
* @class GLBLoader
* @brief discription
*/
    class GLBLoader {
    public:
        /**
        * @brief �R���X�g���N�^
        * @param filepath �t�@�C���ւ̃p�X
        */
        GLBLoader(const std::string& filepath);
        /**
        * @brief �f�X�g���N�^
        */
        ~GLBLoader();
    private:
        std::string& mFilePath;
        std::unique_ptr<GLBResourceReader> mResourceReader;
    };
} //Framework::Utility 