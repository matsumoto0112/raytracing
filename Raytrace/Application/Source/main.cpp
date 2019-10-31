#include "stdafx.h"
#include <iomanip>
#include <locale>
#include "DX/DXRInterface.h"
#include "DX/RaytracingHelper.h"
#include "Framework/Game.h"
#include "Framework/DX/ConstantBuffer.h"
#include "Framework/DX/DXHelper.h"
#include "Framework/Utility/Debug.h"
#include "Framework/ImGui/ImGuiManager.h"
#include "Utility/GPUTimer.h"
#include "Utility/StringUtil.h"
#include "ImGui/Item/Window.h"
#include "ImGui/Item/Text.h"
#include "ImGui/Item/FloatField.h"
#include "Utility/Time.h"
#include "Utility/IO/TextureLoader.h"
#include "Util/Path.h"
#include "Utility/IO/GLBLoader.h"
#include "Math/MathUtility.h"
#include "Utility/StringUtil.h"
#include "Math/Quaternion.h"
#include "DX/RootSignature.h"
#include "DX/DescriptorTable.h"
#include "DX/Texture2D.h"
#include "DX/RaytracingShader.h"
#include "DX/AccelerationStructure.h"
#include "../Assets/Shader/Raytracing/RaytracingCompat.h"

#ifdef _DEBUG
//#include "Temp/bin/x64/Debug/Application/CompiledShaders/Raytracing.hlsl.h"
#include "Temp/bin/x64/Debug/Application/CompiledShaders/RayGenShader.hlsl.h"
#include "Temp/bin/x64/Debug/Application/CompiledShaders/ClosestHit.hlsl.h"
#include "Temp/bin/x64/Debug/Application/CompiledShaders/MissShader.hlsl.h"
#else
//#include "Temp/bin/x64/Release/Application/CompiledShaders/Raytracing.hlsl.h"
#include "Temp/bin/x64/Release/Application/CompiledShaders/RayGenShader.hlsl.h"
#include "Temp/bin/x64/Release/Application/CompiledShaders/ClosestHit.hlsl.h"
#include "Temp/bin/x64/Release/Application/CompiledShaders/MissShader.hlsl.h"

#endif

using namespace DirectX;

using namespace Framework::DX;

/**
* @brief グローバルのリソースパラメータ
* @details シェーダーファイル全体で共有するパラメータ
*/
namespace GlobalRootSignatureParameter {
    enum MyEnum {
        RenderTarget = 0,
        AccelerationStructureSlot,
        VertexBuffers,
        ConstantBuffer,
        Count
    };
} //GlobalRootSignatureParameter 

namespace HitGroupParams {
    namespace LocalRootSignatureParams {
        namespace Type {
            enum MyEnum {
                Sphere,
                Plane,

                Count
            };
        } //Type 
        namespace Constant {
            enum MyEnum {
                Material,
                Count
            };
            struct MaterialConstantBuffer {
                Color4 color;
            }; //MaterialConstantBuffer 
        } //Constant 
    } //LocalRootSignatureParameter 
} //HitGroup 

namespace GeometryType {
    enum MyEnum {
        Cube,
        Plane,
        Count
    };
}

static constexpr UINT CUBE_COUNT = 1;
static constexpr UINT PLANE_COUNT = 1;
static constexpr UINT TLAS_NUM = CUBE_COUNT + PLANE_COUNT;
static const std::wstring MODEL_NAME = L"sphere.glb";
static const std::wstring MODEL_PLANE_NAME = L"checker.glb";

/**
* @class MainApp
* @brief discription
*/
class MainApp : public Framework::Game {
public:
    /**
    * @brief コンストラクタ
    */
    MainApp(UINT width, UINT height, const std::wstring& title)
        :Game(width, height, title),
        mRotation(0.0f),
        mImGUIWindow(std::make_unique<Framework::ImGUI::Window>("GPU")) {
        mGPUInfoText = std::make_shared<Framework::ImGUI::Text>("");
        mImGUIWindow->addItem(mGPUInfoText);
        updateForSizeChange(width, height);
    }
    /**
    * @brief デストラクタ
    */
    ~MainApp() {

    }
    virtual void onInit() override {
        Game::onInit();

        mDXRInterface = std::make_unique<Framework::DX::DXRInterface>(mDeviceResource.get());

        //シーン情報を先に作っておく
        initializeScene();
        //リソースが作られた後にデバイスを作成する
        createDeviceDependentResources();
        createWindowSizeDependentResources();

        Framework::ImGuiManager::getInstance()->init(mWindow->getHwnd(), mDeviceResource->getDevice(), mDeviceResource->getBackBufferFormat());
    }
    virtual void onUpdate() override {
        //更新開始時に準備をかける
        Game::onUpdate();
        mTimer.update();

        calcFrameStatus();
        updateTLAS();

    }

    virtual void onRender() override {
        Game::onRender();
        mDeviceResource->prepare();
        Framework::ImGuiManager::getInstance()->beginFrame();

        ID3D12Device* device = mDeviceResource->getDevice();
        ID3D12GraphicsCommandList* list = mDeviceResource->getCommandList();

        mGPUTimer.beginFrame();


        doRaytracing();
        copyOutput();

#ifdef _DEBUG
        mImGUIWindow->draw();
        mCameraParameterWindow->draw();
#endif
        Framework::ImGuiManager::getInstance()->endFrame(mDeviceResource->getCommandList());

        mGPUTimer.endFrame(list);
        //終了時にコマンドリストを初期化したい
        mDeviceResource->present(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);

    }
    virtual void onDeviceLost() override {
        releaseWindowSizeDependentResources();
        releaseDeviceDependentResources();
    }
    virtual void onDeviceRestored() override {
        createDeviceDependentResources();
        createWindowSizeDependentResources();
    }
private:
    static constexpr UINT FRAME_COUNT = 3;
    static constexpr UINT NUM_BLAS = 1; //ボトムレベルAS使用数は1つ

    //DXRオブジェクト
    std::unique_ptr<Framework::DX::DXRInterface> mDXRInterface;

    std::unique_ptr<Framework::DX::RootSignature> mGlobalRootSignature; //!< グローバルルートシグネチャ
    std::unique_ptr<Framework::DX::RootSignature> mLocalRootSignatures[HitGroupParams::LocalRootSignatureParams::Type::Count]; //!< ローカルルートシグネチャ

    ConstantBuffer<SceneConstantBuffer> mSceneCB;
    Vector4 mCameraPosition;
    XMFLOAT3 mCameraRotation;
    Vector4 mLightPosition;

    float mRotation;

    std::array<XMFLOAT3, CUBE_COUNT> mCubePositions;

    //レイトレーシング出力先
    D3DBuffer mRaytracingOutput;
    //シェーダーテーブル

    static const std::wstring RAY_GEN_SHADER_NAME;
    static const std::wstring CLOSEST_HIT_NAME;
    static const std::wstring MISS_SHADER;
    static const std::wstring HIT_GROUP_SPHERE_NAME;
    static const std::wstring HIT_GROUP_PLANE_NAME;

    ComPtr<ID3D12Resource> mMissShaderTable;
    UINT mMissShaderStrideInBytes;
    ComPtr<ID3D12Resource> mHitGroupShaderTable;
    UINT mHitGroupShaderStrideInBytes;
    ComPtr<ID3D12Resource> mRayGenShaderTable;
    Framework::Utility::GPUTimer mGPUTimer;

    std::unique_ptr<Framework::ImGUI::Window> mImGUIWindow;
    std::shared_ptr<Framework::ImGUI::Text> mGPUInfoText;
    Framework::Utility::Time mTimer;
    std::shared_ptr<Framework::ImGUI::Window> mCameraParameterWindow;

    std::array<D3DBuffer, GeometryType::Count> mGeometryIndexBuffers;
    std::array<D3DBuffer, GeometryType::Count> mGeometryVertexBuffers;
    std::array<UINT, GeometryType::Count> mIndexOffsets;
    std::array<UINT, GeometryType::Count> mVertexOffsets;
    D3DBuffer mResourceIndexBuffer;
    D3DBuffer mResourceVertexBuffer;
    std::vector<Vertex> mResourceVertices;
    std::vector<Index> mResourceIndices;

    std::unique_ptr<DescriptorTable> mDescriptoaTable;

    std::vector<std::unique_ptr<Framework::DX::Texture2D>> mTextures;

    std::unique_ptr<RaytracingShader> mRaytracingShader;
    std::unique_ptr<AccelerationStructure> mAccelerationStructure;

    /**
    * @brief カメラ行列の更新
    */
    void updateCameraMatrices();
    /**
    * @brief シーンの初期化
    */
    void initializeScene();
    /**
    * @brief デバイスの再生成
    */
    void recreateD3D();
    /**
    * @brief レイトレーシング実行
    */
    void doRaytracing();
    /**
    * @brief コンスタントバッファの作成
    */
    void createConstantBuffers();
    /**
    * @brief デバイスに基づくリソースの生成
    */
    void createDeviceDependentResources();
    /**
    * @brief ウィンドウサイズに基づくリソースの作成
    */
    void createWindowSizeDependentResources();
    /**
    * @brief デバイスに基づくリソースの解放
    */
    void releaseDeviceDependentResources();
    /**
    * @brief ウィンドウサイズに基づくリソースの解放
    */
    void releaseWindowSizeDependentResources();
    /**
    * @brief レイトレーシング用インターフェース作成
    */
    void createRaytracinginterfaces();
    /**
    * @brief ルートシグネチャの作成
    */
    void createRootSignatures();

    /**
    * @brief レイトレーシングパイプラインオブジェクト作成
    */
    void createRaytracingPipelineStateObject();
    /**
    * @brief いろんな副次的なリソース作成
    */
    void createAuxillaryDeviceResources();
    /**
    * @brief ディスクリプタヒープの作成
    */
    void createDescriptorHeap();
    /**
    * @brief レイトレーシングの出力先を作成
    */
    void createRaytracingOutputResource();
    /**
    * @brief ASの作成
    */
    void buildAccelerationStructures();
    /**
    * @brief ウィンドウサイズ更新
    */
    void updateForSizeChange(UINT clientWidth, UINT clientHeight);
    /**
    * @brief レイトレーシングの出力をバックバッファにコピーする
    */
    void copyOutput();
    /**
    * @brief 更新処理
    */
    void calcFrameStatus();
    /**
    * @brief バッファのシェーダーリソースビューを作成する
    */
    UINT createBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize);

    ComPtr<ID3D12Resource> createBuffer(uint64_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps);

    void updateTLAS();
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    setlocale(LC_ALL, "");
    MainApp app(800, 600, L"Game");
    return app.run(hInstance, nCmdShow);
}

//const std::wstring MainApp::RAY_GEN_SHADER_NAME = L"MyRaygenShader";
//const std::wstring MainApp::CLOSEST_HIT_SHADER_CUBE_NAME = L"MyClosestHitShader_Cube";
//const std::wstring MainApp::CLOSEST_HIT_SHADER_PLANE_NAME = L"MyClosestHitShader_Plane";
//const std::wstring MainApp::CLOSEST_HIT_SHADER_SHADOW_NAME = L"MyClosestHitShader_Shadow";
//const std::wstring MainApp::MISS_SHADER_NAME = L"MyMissShader";
//const std::wstring MainApp::MISS_SHADER_SHADOW_NAME = L"MyMissShader_Shadow";
//
//const std::wstring MainApp::HIT_GROUP_CUBE_NAME = L"MyHitGroup_Cube";
//const std::wstring MainApp::HIT_GROUP_PLANE_NAME = L"MyHitGroup_Plane";
//const std::wstring MainApp::HIT_GROUP_SHADER_NAME = L"MyHitGroup_Shadow";

const std::wstring MainApp::RAY_GEN_SHADER_NAME = L"MyRayGenShader";
const std::wstring MainApp::MISS_SHADER = L"MissShader";
const std::wstring MainApp::CLOSEST_HIT_NAME = L"ClosestHit";
const std::wstring MainApp::HIT_GROUP_SPHERE_NAME = L"HitGroup_Sphere";
const std::wstring MainApp::HIT_GROUP_PLANE_NAME = L"HitGroup_Plane";

void MainApp::updateCameraMatrices() {
    mSceneCB->cameraPosition = mCameraPosition;


    XMMATRIX rot = XMMatrixRotationRollPitchYaw(mCameraRotation.x, mCameraRotation.y, mCameraRotation.z);
    XMMATRIX trans = XMMatrixTranslation(mCameraPosition.x, mCameraPosition.y, mCameraPosition.z);
    XMMATRIX view = XMMatrixInverse(nullptr, rot * trans);

    float fovY = 45.0f;
    XMVECTOR mEye = { 0,0,-10,1 };
    XMVECTOR mAt = { 0,0,0,1 };
    XMVECTOR mUp = { 0,1,0,1 };
    //XMMATRIX view = XMMatrixLookAtLH(mEye, mAt, mUp);
    const float aspect = static_cast<float>(mWidth) / static_cast<float>(mHeight);
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovY), aspect, 0.1f, 125.0f);
    XMMATRIX vp = view * proj;

    mSceneCB->projectionToWorld = XMMatrixInverse(nullptr, vp);

    //mSceneCB->lightPosition = mLightPosition;
}

void MainApp::initializeScene() {
    //mCameraPosition = { 0,3.0f,-30.0f };
    //mCameraRotation = { 0,0,0 };
    mCameraPosition = Vector4(0, 70, -100, 1);
    //mCameraRotation = 
    mCameraRotation = { 0.61f,0,0 };
    mLightPosition = { 20,40,-70 };

    mCameraParameterWindow = std::make_unique<Framework::ImGUI::Window>("Camera");
#define PARAMETER_CHANGE_SLIDER(name,type,min,max){\
    auto field = std::make_shared<Framework::ImGUI::FloatField>(name, type); \
        field->setCallBack([&](float val) {type = val; }); \
        field->setMinValue(min); \
        field->setMaxValue(max); \
        mCameraParameterWindow->addItem(field); \
    }

    mCameraParameterWindow->addItem(std::make_shared<Framework::ImGUI::Text>("Position"));

    float range = 100.0f;
    float anbleRange = (Framework::Math::PI2);
    PARAMETER_CHANGE_SLIDER("X", mCameraPosition.x, -range, range);
    PARAMETER_CHANGE_SLIDER("Y", mCameraPosition.y, -range, range);
    PARAMETER_CHANGE_SLIDER("Z", mCameraPosition.z, -range, range);
    mCameraParameterWindow->addItem(std::make_shared<Framework::ImGUI::Text>("Rotation"));
    PARAMETER_CHANGE_SLIDER("RX", mCameraRotation.x, -anbleRange, anbleRange);
    PARAMETER_CHANGE_SLIDER("RY", mCameraRotation.y, -anbleRange, anbleRange);
    PARAMETER_CHANGE_SLIDER("RZ", mCameraRotation.z, -anbleRange, anbleRange);
    mCameraParameterWindow->addItem(std::make_shared<Framework::ImGUI::Text>("Light"));
    PARAMETER_CHANGE_SLIDER("LX", mLightPosition.x, -range, range);
    PARAMETER_CHANGE_SLIDER("LY", mLightPosition.y, -range, range);
    PARAMETER_CHANGE_SLIDER("LZ", mLightPosition.z, -range, range);

    //mSceneCB->lightAmbient = Color4(0.3f, 0.3f, 0.3f, 1.0f);
    //mSceneCB->lightDiffuse = Color4(0, 0, 1, 1.0f);
    //mSceneCB->fogStart = 500.0f;
    //mSceneCB->fogEnd = 1000.0f;
    int w = (int)Framework::Math::MathUtil::sqrt(CUBE_COUNT) + 1;
    int h = (int)Framework::Math::MathUtil::sqrt(CUBE_COUNT) + 1;
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            if (y * w + x >= CUBE_COUNT)break;
            float xpos = (x - w / 2) * 2.5f;
            float zpos = (y - h / 2) * 2.5f;
            mCubePositions[y * w + x] = { xpos,3.0f,zpos };
        }
    }

    Framework::Utility::TextureLoader loader;
    UINT width, height;
    std::vector<BYTE> texture = loader.load(Path::getInstance()->texture() + L"texture.png", &width, &height);


    updateCameraMatrices();
}

void MainApp::recreateD3D() {
    try {
        mDeviceResource->waitForGPU();
    }
    catch (const Framework::Utility::HrException&) {

    }
    mDeviceResource->handleDeviceLost();
}

void MainApp::doRaytracing() {
    ID3D12GraphicsCommandList* list = mDeviceResource->getCommandList();
    UINT frameCount = mDeviceResource->getCurrentFrameIndex();

    mGlobalRootSignature->setComputeRootSignature(list);
    mSceneCB.copyStagingToGPU(frameCount);
    list->SetComputeRootConstantBufferView(GlobalRootSignatureParameter::ConstantBuffer, mSceneCB.gpuVirtualAddress(frameCount));

    D3D12_DISPATCH_RAYS_DESC desc = {};
    ID3D12DescriptorHeap* heaps[] = { mDescriptoaTable->getHeap() };
    list->SetDescriptorHeaps(_countof(heaps), heaps);
    list->SetComputeRootDescriptorTable(GlobalRootSignatureParameter::RenderTarget, mRaytracingOutput.gpuHandle);
    mAccelerationStructure->setDescriptorTable(list, GlobalRootSignatureParameter::AccelerationStructureSlot);
    list->SetComputeRootDescriptorTable(GlobalRootSignatureParameter::VertexBuffers, mResourceIndexBuffer.gpuHandle);
    mRaytracingShader->doRaytracing(mWidth, mHeight);
}

void MainApp::createConstantBuffers() {
    ID3D12Device* device = mDeviceResource->getDevice();
    UINT frameCount = mDeviceResource->getBackBufferCount();
    mSceneCB.create(device, frameCount, L"SceneConstant");
}

void MainApp::createDeviceDependentResources() {
    //補助リソースを先に生成
    createAuxillaryDeviceResources();
    //ヒープ作成
    createDescriptorHeap();
    //テクスチャの作成 
    {
        {
            mTextures.resize(2);
            Framework::Utility::GLBLoader glbLoader(
                Framework::Utility::toString(Path::getInstance()->model() + MODEL_NAME));
            Framework::Utility::TextureData texRowData = glbLoader.getImageDatas()[0];
            ID3D12Device* device = mDeviceResource->getDevice();
            mTextures[0] = std::make_unique<Framework::DX::Texture2D>(device, mDescriptoaTable.get(), texRowData);
        }
        {
            Framework::Utility::GLBLoader glbLoader(
                Framework::Utility::toString(Path::getInstance()->model() + MODEL_PLANE_NAME));
            Framework::Utility::TextureData texRowData = glbLoader.getImageDatas()[0];
            ID3D12Device* device = mDeviceResource->getDevice();
            mTextures[1] = std::make_unique<Framework::DX::Texture2D>(device, mDescriptoaTable.get(), texRowData);
        }
    }

    //レイトレース用インターフェース作成
    createRaytracinginterfaces();
    //AS作成
    buildAccelerationStructures();
    //ルートシグネチャを作成する
    createRootSignatures();
    //レイトレーシングに必要なパイプラインオブジェクトを生成する
    createRaytracingPipelineStateObject();
    //コンスタントバッファ作成
    createConstantBuffers();
    //レイトレーシング出力先を作成
    createRaytracingOutputResource();

}

void MainApp::createWindowSizeDependentResources() {
    createRaytracingOutputResource();
    updateCameraMatrices();
}

void MainApp::releaseDeviceDependentResources() {
    mGPUTimer.releaseDevice();
    mGlobalRootSignature->reset();
    mDXRInterface->clear();
    mDescriptoaTable->reset();

    for (auto&& index : mGeometryIndexBuffers) {
        index.resource.Reset();
    }
    for (auto&& vertex : mGeometryVertexBuffers) {
        vertex.resource.Reset();
    }

    for (UINT i = 0; i < GeometryType::Count; i++) {
        //mBottomLevelAS[i].Reset();
    }
    //mTopLevelAS.Reset();

}

void MainApp::releaseWindowSizeDependentResources() {
    mRaytracingOutput.resource.Reset();
}

void MainApp::createRaytracinginterfaces() {
    mDXRInterface->recreate();
}

void MainApp::createRootSignatures() {
    ID3D12Device* device = mDeviceResource->getDevice();

    //まずはグローバルルートシグネチャを作成する
    {
        using namespace Framework::DX;
        std::vector<DescriptorRange> ranges(2);
        ranges[0].init(DescriptorRangeType::UAV, 1, 0);
        ranges[1].init(DescriptorRangeType::SRV, 2, 1);

        std::vector<RootParameterDesc> params(GlobalRootSignatureParameter::Count);
        params[GlobalRootSignatureParameter::RenderTarget].initAsDescriptor();
        params[GlobalRootSignatureParameter::AccelerationStructureSlot].init(RootParameterType::SRV, 0);
        params[GlobalRootSignatureParameter::VertexBuffers].initAsDescriptor();
        params[GlobalRootSignatureParameter::ConstantBuffer].init(RootParameterType::CBV, 0);

        std::vector<StaticSampler> sampler(1);
        sampler[0].filter = D3D12_FILTER::D3D12_FILTER_MIN_MAG_MIP_LINEAR;

        RootSignatureDesc desc(RootSignatureFlag::GlobalRootSignature, &ranges, nullptr, &params, &sampler, L"GlobalRootSignature");
        mGlobalRootSignature = std::make_unique<RootSignature>(device, desc);

    }
    //ローカルルートシグネチャを作成する
    {
        {

            std::vector<RootParameterDesc> params(1);
            params[0].init(RootParameterType::Constants, 1);
            std::vector<ConstantsDesc> constants(1);
            constants[0].bufferSize = sizeof(HitGroupParams::LocalRootSignatureParams::Constant::MaterialConstantBuffer);
            RootSignatureDesc desc(RootSignatureFlag::LocalRootSignature, nullptr, &constants, &params, nullptr);
            mLocalRootSignatures[HitGroupParams::LocalRootSignatureParams::Type::Sphere] = std::make_unique<RootSignature>(device, desc);
        }
        {
            std::vector<RootParameterDesc> params(1);
            params[0].init(RootParameterType::Constants, 1);
            std::vector<ConstantsDesc> constants(1);
            constants[0].bufferSize = sizeof(HitGroupParams::LocalRootSignatureParams::Constant::MaterialConstantBuffer);
            RootSignatureDesc desc(RootSignatureFlag::LocalRootSignature, nullptr, &constants, &params, nullptr);
            mLocalRootSignatures[HitGroupParams::LocalRootSignatureParams::Type::Plane] = std::make_unique<RootSignature>(device, desc);
        }
        //AABB ローカルルートシグネチャ
        //{
        //    using namespace Framework::DX;
        //    std::vector<DescriptorRange> range(1);
        //    range[0].init(DescriptorRangeType::SRV, 1, 3);

        //    std::vector<RootParameterDesc> params(LocalRootSignatureParams::AABB::Count);
        //    params[LocalRootSignatureParams::AABB::Material].init(RootParameterType::Constants, 1);
        //    params[LocalRootSignatureParams::AABB::Texture].initAsDescriptor();

        //    std::vector<ConstantsDesc> constants(1);
        //    constants[0].bufferSize = sizeof(MaterialConstantBuffer);

        //    RootSignatureDesc desc(RootSignatureFlag::LocalRootSignature, &range, &constants, &params, nullptr, L"AABBLocalRootSignature");
        //    mLocalRootSignatures[LocalRootSignatureParams::Type::AABB] = std::make_unique<RootSignature>(device, desc);
        //}
        ////Plane
        //{
        //    using namespace Framework::DX;
        //    std::vector<DescriptorRange> range(1);
        //    range[0].init(DescriptorRangeType::SRV, 1, 3);

        //    std::vector<RootParameterDesc> params(LocalRootSignatureParams::Plane::Count);
        //    params[LocalRootSignatureParams::Plane::Material].init(RootParameterType::Constants, 1);
        //    params[LocalRootSignatureParams::Plane::Texture].initAsDescriptor();

        //    std::vector<ConstantsDesc> constants(1);
        //    constants[0].bufferSize = sizeof(MaterialConstantBuffer);

        //    RootSignatureDesc desc(RootSignatureFlag::LocalRootSignature, &range, &constants, &params, nullptr, L"PlaneLocalRootSignature");
        //    mLocalRootSignatures[LocalRootSignatureParams::Type::Plane] = std::make_unique<RootSignature>(device, desc);
        //}
    }
}

void MainApp::createRaytracingPipelineStateObject() {
    using namespace Framework::DX;
    //シェーダー作成
    mRaytracingShader = std::make_unique<RaytracingShader>(mDXRInterface.get());
    //グローバルルートシグネチャのセット
    mRaytracingShader->setGlobalRootSignature(mGlobalRootSignature.get());

    //シェーダーの設定
    ShaderConfig config(Framework::Math::MathUtil::mymax<UINT>({ sizeof(RayPayload) }), 2 * sizeof(float), 15);
    mRaytracingShader->setConfig(config);

    //シェーダーファイルの読み込み
    {
        ShaderFile file;
        file.shaderFile = (void*)g_pRayGenShader;
        file.shaderFileSize = _countof(g_pRayGenShader);
        file.entryPoints = { RAY_GEN_SHADER_NAME };
        mRaytracingShader->loadShaderFiles(file);

        file.shaderFile = (void*)g_pClosestHit;
        file.shaderFileSize = _countof(g_pClosestHit);
        file.entryPoints = { CLOSEST_HIT_NAME };
        mRaytracingShader->loadShaderFiles(file);

        file.shaderFile = (void*)g_pMissShader;
        file.shaderFileSize = _countof(g_pMissShader);
        file.entryPoints = { MISS_SHADER };
        mRaytracingShader->loadShaderFiles(file);
    }
    {
        mRaytracingShader->setLocalRootSignature(HIT_GROUP_SPHERE_NAME, mLocalRootSignatures[0].get());
        mRaytracingShader->setLocalRootSignature(HIT_GROUP_PLANE_NAME, mLocalRootSignatures[1].get());
    }
    {
        HitGroup hit(HIT_GROUP_SPHERE_NAME, HitGroupType::Triangle);
        hit.closestHit = CLOSEST_HIT_NAME;
        mRaytracingShader->bindHitGroup(hit);

        hit.name = HIT_GROUP_PLANE_NAME;
        hit.closestHit = CLOSEST_HIT_NAME;
        mRaytracingShader->bindHitGroup(hit);
    }
    mRaytracingShader->buildPipeline();
    {

        std::vector<HitGroup> hitGroups(2);
        {
            LocalRootSignature local;
            struct RootArgument {
                HitGroupParams::LocalRootSignatureParams::Constant::MaterialConstantBuffer cb;
            } rootArguments;
            rootArguments.cb.color = Color4(1, 1, 0, 1);

            local.localConstants = &rootArguments;
            local.localConstantsSize = sizeof(RootArgument);
            local.rootSignature = mLocalRootSignatures[0].get();

            hitGroups[0] = HitGroup(HIT_GROUP_SPHERE_NAME, HitGroupType::Triangle);
            hitGroups[0].closestHit = CLOSEST_HIT_NAME;
            hitGroups[0].localRootSignature = &local;
        }
        {
            LocalRootSignature local;
            struct RootArgument {
                HitGroupParams::LocalRootSignatureParams::Constant::MaterialConstantBuffer cb;
            } rootArguments;
            rootArguments.cb.color = Color4(0, 0, 1, 1);

            local.localConstants = &rootArguments;
            local.localConstantsSize = sizeof(RootArgument);
            local.rootSignature = mLocalRootSignatures[1].get();

            hitGroups[1] = HitGroup(HIT_GROUP_PLANE_NAME, HitGroupType::Triangle);
            hitGroups[1].closestHit = CLOSEST_HIT_NAME;
            hitGroups[1].localRootSignature = &local;
        }


        std::vector<UINT> indices = { 0 ,1 };
        mRaytracingShader->hitGroup(hitGroups, indices);
    }

    RayGenShader rayGenShader(RAY_GEN_SHADER_NAME);
    mRaytracingShader->rayGenerationShader(RAY_GEN_SHADER_NAME);

    std::vector<MissShader> missShaders(1);
    missShaders[0] = MissShader(MISS_SHADER);
    mRaytracingShader->missShader(missShaders);
}

void MainApp::createAuxillaryDeviceResources() {
    ID3D12Device* device = mDeviceResource->getDevice();
    ID3D12CommandQueue* queue = mDeviceResource->getCommandQueue();
    UINT frameCount = mDeviceResource->getBackBufferCount();

    mGPUTimer.storeDevice(device, queue, frameCount);
}

void MainApp::createDescriptorHeap() {
    ID3D12Device* device = mDeviceResource->getDevice();
    using namespace Framework::DX;
    mDescriptoaTable = std::make_unique<DescriptorTable>(
        device, HeapType::CBV_SRV_UAV, HeapFlag::ShaderVisible, 10000);
}

void MainApp::createRaytracingOutputResource() {
    ID3D12Device* device = mDeviceResource->getDevice();
    DXGI_FORMAT format = mDeviceResource->getBackBufferFormat();
    CD3DX12_RESOURCE_DESC uavResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        format, mWidth, mHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT);
    Framework::Utility::throwIfFailed(device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
        &uavResourceDesc,
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(&mRaytracingOutput.resource)));

    mDescriptoaTable->allocateWithGPU(&mRaytracingOutput.cpuHandle, &mRaytracingOutput.gpuHandle);
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;
    device->CreateUnorderedAccessView(mRaytracingOutput.resource.Get(), nullptr, &uavDesc, mRaytracingOutput.cpuHandle);
}

//ASを作成する
void MainApp::buildAccelerationStructures() {
    mDeviceResource->getCommandList()->Reset(mDeviceResource->getCommandAllocator(), nullptr);
    mAccelerationStructure = std::make_unique<AccelerationStructure>();

    mIndexOffsets[GeometryType::Cube] = 0;
    mVertexOffsets[GeometryType::Cube] = 0;

    AccelerationStructureBuffers blasBuffers[GeometryType::Count];

    ID3D12Device5* device = mDXRInterface->getDXRDevice();
    UINT planeID;
    {
        Framework::Utility::GLBLoader glbLoader(
            Framework::Utility::toString(Path::getInstance()->model() + MODEL_NAME));
        auto positions = glbLoader.getPositionsPerSubMeshes()[0];
        auto uvs = glbLoader.getUVsPerSubMeshes()[0];
        auto normals = glbLoader.getNormalsPerSubMeshes()[0];
        std::vector<Index> indices = glbLoader.getIndicesPerSubMeshes()[0];
        std::vector<Vertex> vertices(positions.size());
        for (size_t i = 0; i < vertices.size(); i++) {
            vertices[i].position = positions[i];
            //vertices[i].uv = uvs[i];
            //vertices[i].normal = normals[i];
        }

        mAccelerationStructure->addBLASBuffer(device, vertices, indices);
        mAccelerationStructure->buildBLAS(device, mDXRInterface->getCommandList());

        mResourceIndices.insert(mResourceIndices.end(), indices.begin(), indices.end());
        mResourceVertices.insert(mResourceVertices.end(), vertices.begin(), vertices.end());
        mIndexOffsets[GeometryType::Cube + 1] = (UINT)indices.size() * sizeof(Index);
        mVertexOffsets[GeometryType::Cube + 1] = (INT)vertices.size();
    }

    UINT sphereID;
    {
        Framework::Utility::GLBLoader glbLoader(
            Framework::Utility::toString(Path::getInstance()->model() + MODEL_PLANE_NAME));
        auto positions = glbLoader.getPositionsPerSubMeshes()[0];
        auto uvs = glbLoader.getUVsPerSubMeshes()[0];
        auto normals = glbLoader.getNormalsPerSubMeshes()[0];
        std::vector<Index> indices = glbLoader.getIndicesPerSubMeshes()[0];
        std::vector<Vertex> vertices(positions.size());
        for (size_t i = 0; i < vertices.size(); i++) {
            vertices[i].position = positions[i];
            //vertices[i].uv = uvs[i];
            //vertices[i].normal = normals[i];
        }

        mAccelerationStructure->addBLASBuffer(device, vertices, indices);

        mAccelerationStructure->buildBLAS(device, mDXRInterface->getCommandList());
        mResourceIndices.insert(mResourceIndices.end(), indices.begin(), indices.end());
        mResourceVertices.insert(mResourceVertices.end(), vertices.begin(), vertices.end());
    }

    mDeviceResource->executeCommandList();
    mDeviceResource->waitForGPU();
    mDeviceResource->getCommandList()->Reset(mDeviceResource->getCommandAllocator(), nullptr);

    allocateUploadBuffer(device, mResourceIndices.data(), mResourceIndices.size() * sizeof(Index), &mResourceIndexBuffer.resource);
    allocateUploadBuffer(device, mResourceVertices.data(), mResourceVertices.size() * sizeof(Vertex), &mResourceVertexBuffer.resource);

    createBufferSRV(&mResourceIndexBuffer, static_cast<UINT>(mResourceIndices.size()) * sizeof(Index) / 4, 0);
    createBufferSRV(&mResourceVertexBuffer, static_cast<UINT>(mResourceVertices.size()), sizeof(Vertex));
}

void MainApp::updateForSizeChange(UINT clientWidth, UINT clientHeight) { }

void MainApp::copyOutput() {
    ID3D12GraphicsCommandList* list = mDeviceResource->getCommandList();
    ID3D12Resource* renderTarget = mDeviceResource->getRenderTarget();

    D3D12_RESOURCE_BARRIER preBarrier[2];
    preBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget,
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
    preBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(mRaytracingOutput.resource.Get(),
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE);

    list->ResourceBarrier(ARRAYSIZE(preBarrier), preBarrier);

    list->CopyResource(renderTarget, mRaytracingOutput.resource.Get());

    D3D12_RESOURCE_BARRIER postBarrier[2];
    postBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget,
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
    postBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(mRaytracingOutput.resource.Get(),
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    list->ResourceBarrier(ARRAYSIZE(postBarrier), postBarrier);
}

void MainApp::calcFrameStatus() {
    updateCameraMatrices();

    std::stringstream ss;
    float time = mGPUTimer.getElapsedTime();
    ss << std::setprecision(5) << std::fixed << "Dispatch Rays" << time << "ms\n"
        << NumMRaysPerSecond(mWidth, mHeight, time) << "Rays/s\n" <<
        "FPS:" << mTimer.getFPS() << "\n" <<
        "DeltaTime:" << mTimer.getDeltaTime() << "s\n";

    mGPUInfoText->setText(ss.str());

    //SetWindowText(mWindow->getHwnd(), Framework::Utility::StringBuilder(L"FPS:") << mTimer.getFPS());

    //mRotation += 0.05;
    //const float RANGE = 100.0f;
    //const float x = Framework::Math::MathUtil::sin(mRotation) * RANGE;
    //const float z = Framework::Math::MathUtil::cos(mRotation) * RANGE;
    //mLightPosition.x = x;
    //mLightPosition.z = z;
}

UINT MainApp::createBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize) {
    ID3D12Device* device = mDeviceResource->getDevice();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements = numElements;
    if (elementSize == 0) {
        srvDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAGS::D3D12_BUFFER_SRV_FLAG_RAW;
        srvDesc.Buffer.StructureByteStride = 0;
    }
    else {
        srvDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAGS::D3D12_BUFFER_SRV_FLAG_NONE;
        srvDesc.Buffer.StructureByteStride = elementSize;
    }

    mDescriptoaTable->allocateWithGPU(&buffer->cpuHandle, &buffer->gpuHandle);
    device->CreateShaderResourceView(buffer->resource.Get(), &srvDesc, buffer->cpuHandle);
    return 0;
}

ComPtr<ID3D12Resource> MainApp::createBuffer(uint64_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES & heapProps) {
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size, flags);
    ComPtr<ID3D12Resource> buffer;
    ID3D12Device* device = mDeviceResource->getDevice();
    Framework::Utility::throwIfFailed(device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        initState,
        nullptr,
        IID_PPV_ARGS(&buffer)));

    return buffer;
}

void MainApp::updateTLAS() {
    mAccelerationStructure->tlasConfig(mDXRInterface->getDXRDevice(), TLAS_NUM);
    {
        for (int i = 0; i < CUBE_COUNT; i++) {
            XMMATRIX tr = XMMatrixTranslation(mCubePositions[i].x, mCubePositions[i].y, mCubePositions[i].z);
            mAccelerationStructure->addTLASBuffer(0, 0, 0, tr);
        }
        XMMATRIX tr = XMMatrixScaling(50, 1, 50);
        mAccelerationStructure->addTLASBuffer(1, 1, 1, tr);
    }
    mAccelerationStructure->buildTLAS(mDXRInterface->getCommandList());

    ////ASの構築はGPUでやっているらしいので終了まで待つ必要がある
    mDeviceResource->executeCommandList();
    mDeviceResource->waitForGPU();
}
