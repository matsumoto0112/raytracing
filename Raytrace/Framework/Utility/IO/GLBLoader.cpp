#include "GLBLoader.h"
#include <fstream>
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/Deserialize.h>

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
    auto streamReader = std::make_unique<StreamReader>(path);
    auto glbStream = streamReader->GetInputStream(filepath);
    auto glbResourceReader = std::make_unique<GLBResourceReader>(std::move(streamReader), std::move(glbStream));
    auto manifest = glbResourceReader->GetJson();
    auto document = Deserialize(manifest);
}

} //Framework::Utility 
