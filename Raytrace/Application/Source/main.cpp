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
#include "../Assets/Shader/Raytracing/Local.h"
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
namespace GeometryType {
    enum MyEnum {
        //Sphere,
        Plane,
        Cube,

        Count
    };
}

namespace HitGroupParams {
    namespace LocalRootSignatureParams {
        using Type = GeometryType::MyEnum;
        namespace Constant {
            enum MyEnum {
                Material,
                Count
            };
        } //Constant 
    } //LocalRootSignatureParameter 
} //HitGroup 


static constexpr UINT SPHERE_COUNT = 1;
static constexpr UINT PLANE_COUNT = 0;
static constexpr UINT CUBE_COUNT = 0;
static constexpr UINT TLAS_NUM = SPHERE_COUNT + PLANE_COUNT + CUBE_COUNT;

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

    static const std::vector<std::wstring> HIT_GROUP_NAMES;
    static const std::vector<std::wstring> MODEL_NAMES;

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

const std::wstring MainApp::RAY_GEN_SHADER_NAME = L"MyRayGenShader";
const std::wstring MainApp::MISS_SHADER = L"MissShader";
const std::wstring MainApp::CLOSEST_HIT_NAME = L"ClosestHit";
const std::vector<std::wstring> MainApp::HIT_GROUP_NAMES =
{
    //L"HitGroup_Sphere",
    L"HitGroup_Plane",
    L"HitGroup_Cube",
};
const std::vector<std::wstring> MainApp::MODEL_NAMES =
{
    //L"sphere.glb",
    L"checker.glb",
    L"cube.glb",
};

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

    mSceneCB->lightPosition = mLightPosition;
}

void MainApp::initializeScene() {
    mCameraPosition = Vector4(0, 70, -100, 1);
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

    mSceneCB->lightPosition = mLightPosition;
    mSceneCB->lightAmbient = Color4(0.3f, 0.3f, 0.3f, 1.0f);
    mSceneCB->lightDiffuse = Color4(0, 0, 1, 1.0f);
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
    mTextures.resize(MODEL_NAMES.size());
    for (int i = 0; i < MODEL_NAMES.size(); i++) {
        Framework::Utility::GLBLoader glbLoader(
            Framework::Utility::toString(Path::getInstance()->model() + MODEL_NAMES[i]));
        Framework::Utility::TextureData texRowData = glbLoader.getImageDatas()[0];
        ID3D12Device* device = mDeviceResource->getDevice();
        mTextures[i] = std::make_unique<Framework::DX::Texture2D>(device, mDescriptoaTable.get(), texRowData);
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
        std::vector<DescriptorRange> ranges(1);
        ranges[0].init(DescriptorRangeType::SRV, 1, 3);
        std::vector<RootParameterDesc> params(2);
        params[0].init(RootParameterType::Constants, 1);
        params[1].initAsDescriptor();
        std::vector<ConstantsDesc> constants(1);
        constants[0].bufferSize = sizeof(HitGroupParams::LocalRootSignatureParams::Constant::MaterialConstantBuffer);
        RootSignatureDesc desc(RootSignatureFlag::LocalRootSignature, &ranges, &constants, &params, nullptr);
        for (int i = 0; i < MODEL_NAMES.size(); i++) {
            mLocalRootSignatures[i] = std::make_unique<RootSignature>(device, desc);
        }
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
    for (int i = 0; i < HIT_GROUP_NAMES.size(); i++) {
        mRaytracingShader->setLocalRootSignature(HIT_GROUP_NAMES[i], mLocalRootSignatures[i].get());
    }
    for (int i = 0; i < HIT_GROUP_NAMES.size(); i++) {
        HitGroup hit(HIT_GROUP_NAMES[i], HitGroupType::Triangle);
        hit.closestHit = CLOSEST_HIT_NAME;
        mRaytracingShader->bindHitGroup(hit);
    }
    mRaytracingShader->buildPipeline();
    {

        std::vector<HitGroup> hitGroups(HIT_GROUP_NAMES.size());
        std::vector<UINT> indices;

        for (int i = 0; i < MODEL_NAMES.size(); i++) {
            LocalRootSignature local;
            struct RootArgument {
                HitGroupParams::LocalRootSignatureParams::Constant::MaterialConstantBuffer cb;
            } rootArguments;
            rootArguments.cb.vertexOffset = mVertexOffsets[i];
            rootArguments.cb.indexOffset = mIndexOffsets[i];
            rootArguments.cb.texture = mTextures[i]->getGPUHandle();

            local.use = true;
            local.localConstants = &rootArguments;
            local.localConstantsSize = sizeof(RootArgument);
            local.rootSignature = mLocalRootSignatures[i].get();

            hitGroups[i] = HitGroup(HIT_GROUP_NAMES[i], HitGroupType::Triangle);
            hitGroups[i].closestHit = CLOSEST_HIT_NAME;
            hitGroups[i].localRootSignature = local;

            indices.emplace_back(i);
        }

        mRaytracingShader->hitGroup(hitGroups, indices);
    }

    RayGenShader rayGenShader(RAY_GEN_SHADER_NAME);
    mRaytracingShader->rayGenerationShader(RAY_GEN_SHADER_NAME);

    std::vector<MissShader> missShaders(1);
    missShaders[0] = MissShader(MISS_SHADER);
    mRaytracingShader->missShader(missShaders);

    mRaytracingShader->printOut();
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

    int offset = 0;
    mIndexOffsets[offset] = 0;
    mVertexOffsets[offset] = 0;
    offset++;
    AccelerationStructureBuffers blasBuffers[GeometryType::Count];

    ID3D12Device5* device = mDXRInterface->getDXRDevice();
    for (int k = 0; k < MODEL_NAMES.size(); k++) {
        Framework::Utility::GLBLoader glbLoader(
            Framework::Utility::toString(Path::getInstance()->model() + MODEL_NAMES[k]));
        //サブメッシュの考慮をしない
        auto positions = glbLoader.getPositionsPerSubMeshes()[0];
        auto uvs = glbLoader.getUVsPerSubMeshes()[0];
        auto normals = glbLoader.getNormalsPerSubMeshes()[0];
        std::vector<Index> indices = glbLoader.getIndicesPerSubMeshes()[0];
        std::vector<Vertex> vertices(positions.size());
        for (size_t i = 0; i < vertices.size(); i++) {
            vertices[i].position = positions[i];
            vertices[i].uv = uvs[i];
            vertices[i].normal = normals[i];
        }

        mAccelerationStructure->addBLASBuffer(device, vertices, indices);
        mAccelerationStructure->buildBLAS(device, mDXRInterface->getCommandList());

        mResourceIndices.insert(mResourceIndices.end(), indices.begin(), indices.end());
        mResourceVertices.insert(mResourceVertices.end(), vertices.begin(), vertices.end());
        if (offset >= mIndexOffsets.size())break;
        mIndexOffsets[offset] = mIndexOffsets[offset - 1] + (UINT)indices.size() * sizeof(Index);
        mVertexOffsets[offset] = mVertexOffsets[offset - 1] + (INT)vertices.size();
        offset++;
    }

    mDeviceResource->executeCommandList();
    mDeviceResource->waitForGPU();
    mDeviceResource->getCommandList()->Reset(mDeviceResource->getCommandAllocator(), nullptr);

    allocateUploadBuffer(device, mResourceIndices.data(), mResourceIndices.size() * sizeof(Index), &mResourceIndexBuffer.resource);
    allocateUploadBuffer(device, mResourceVertices.data(), mResourceVertices.size() * sizeof(Vertex), &mResourceVertexBuffer.resource);

    createBufferSRV(&mResourceIndexBuffer, static_cast<UINT>(mResourceIndices.size()) * sizeof(Index) / 4, 0);
    createBufferSRV(&mResourceVertexBuffer, static_cast<UINT>(mResourceVertices.size()), sizeof(Vertex));
    mResourceIndexBuffer.resource->SetName(L"IndexBuffer");
    mResourceVertexBuffer.resource->SetName(L"VertexBuffer");
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
        mAccelerationStructure->addTLASBuffer(0, 0, 0, XMMatrixIdentity());
        //for (int i = 0; i < SPHERE_COUNT; i++) {
        //    XMMATRIX tr = XMMatrixTranslation(mCubePositions[i].x, mCubePositions[i].y, mCubePositions[i].z);
        //    mAccelerationStructure->addTLASBuffer(0, 0, 0, tr);
        //}
        //XMMATRIX tr = XMMatrixScaling(50, 1, 50);
        ////mAccelerationStructure->addTLASBuffer(1, 2, 2, tr);

        //tr = XMMatrixTranslation(5, 5, 0);
        ////mAccelerationStructure->addTLASBuffer(2, 1, 2, tr);
    }
    mAccelerationStructure->buildTLAS(mDXRInterface->getCommandList());

    ////ASの構築はGPUでやっているらしいので終了まで待つ必要がある
    mDeviceResource->executeCommandList();
    mDeviceResource->waitForGPU();
}
