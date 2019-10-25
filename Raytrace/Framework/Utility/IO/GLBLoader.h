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
        * @brief コンストラクタ
        * @param filepath ファイルへのパス
        */
        GLBLoader(const std::string& filepath);
        /**
        * @brief デストラクタ
        */
        ~GLBLoader();
    private:
        std::string& mFilePath;
        std::unique_ptr<GLBResourceReader> mResourceReader;
    };
} //Framework::Utility 