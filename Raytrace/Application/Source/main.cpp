#include "stdafx.h"
#include "Framework/Game.h"
#include "Framework/Utility/Debug.h"
#include <locale>
#include "DX/RaytracingHelper.h"
#include "Framework/DX/DXHelper.h"
#include "Framework/DX/ConstantBuffer.h"
#include "../Assets/Shader/RaytracingStructure.h"
#include "FBXLoader.h"
#include <iomanip>

#ifdef _DEBUG
#include "Temp/bin/x64/Debug/Application/CompiledShaders/Raytracing.hlsl.h"
#else
#include "Temp/bin/x64/Release/Application/CompiledShaders/Raytracing.hlsl.h"
#endif

using namespace DirectX;

using namespace Framework::DX;

/**
* @brief グローバルのリソースパラメータ
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

/**
* @brief ローカルのルートシグネチャパラメータ
*/
namespace LocalRootSignatureParams {
    enum MyEnum {
        Instance = 0,
        Count
    };
} //LocalRootSignatureParams

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
        mRaytracingOutputResourceUAVDescriptorHeapIndex(UINT_MAX),
        mDescriptorSize(0) {
        updateForSizeChange(width, height);
    }
    /**
    * @brief デストラクタ
    */
    ~MainApp() {

    }
    virtual void onInit() override {
        Game::onInit();

        //シーン情報を先に作っておく
        initializeScene();
        //リソースが作られた後にデバイスを作成する
        createDeviceDependentResources();
        createWindowSizeDependentResources();
    }
    virtual void onUpdate() override {
        Game::onUpdate();

        calcFrameStatus();
    }

    virtual void onRender() override {
        Game::onRender();

        ID3D12Device* device = mDeviceResource->getDevice();
        ID3D12GraphicsCommandList* list = mDeviceResource->getCommandList();

        mDeviceResource->prepare();

        doRaytracing();
        copyOutput();

        mDeviceResource->present(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT);

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
    ComPtr<ID3D12Device5> mDXRDevice;
    ComPtr<ID3D12GraphicsCommandList5> mDXRCommandList;
    ComPtr<ID3D12StateObject> mDXRStateObject;

    ComPtr<ID3D12RootSignature> mRaytracingGlobalRootSignature; //!< グローバルルートシグネチャ
    ComPtr<ID3D12RootSignature> mRaytracingLocalRootSignature; //!< ローカルルートシグネチャ

    //ディスクリプタヒープ
    ComPtr<ID3D12DescriptorHeap> mDescriptorHeap;
    UINT mDescriptorAllocated;
    UINT mDescriptorSize;

    ConstantBuffer<SceneConstantBuffer> mSceneCB;
    ConstantBuffer<Instance> mInstanceCB;
    XMVECTOR mEye, mAt, mUp;

    D3DBuffer mIndexBuffer;
    D3DBuffer mVertexBuffer;

    //AS
    ComPtr<ID3D12Resource> mBottomLevelAS;
    ComPtr<ID3D12Resource> mTopLevelAS;

    //レイトレーシング出力先
    ComPtr<ID3D12Resource> mRaytracingOutput;
    D3D12_GPU_DESCRIPTOR_HANDLE mRaytracingOutputResourceUAVGpuDescriptor;
    UINT mRaytracingOutputResourceUAVDescriptorHeapIndex;

    //シェーダーテーブル
    static const wchar_t* HIT_GROUP_NAME;
    static const wchar_t* RAY_GEN_SHADER_NAME;
    static const wchar_t* CLOSEST_HIT_SHADER_NAME;
    static const wchar_t* MISS_SHADER_NAME;

    ComPtr<ID3D12Resource> mMissShaderTable;
    ComPtr<ID3D12Resource> mHitGroupShaderTable;
    ComPtr<ID3D12Resource> mRayGenShaderTable;

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
    * @brief ルートシグネチャを作成する
    */
    void serializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);
    /**
    * @brief ルートシグネチャの作成
    */
    void createRootSignatures();
    /**
    * @brief DXILライブラリのサブオブジェクト作成
    */
    void createDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* pipeline);
    /**
    * @brief ヒットグループのサブオブジェクト作成
    */
    void createHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* pipeline);
    /**
    * @brief ローカルルートシグネチャのサブオブジェクト作成
    */
    void createLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* pipeline);
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
    * @brief ジオメトリの作成
    */
    void buildGeometry();
    /**
    * @brief ASの作成
    */
    void buildAccelerationStructures();
    /**
    * @brief シェーダーテーブル作成
    */
    void buildShaderTables();
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
    * @brief ディスクリプタヒープのアロケート処理
    */
    UINT allocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, UINT descriptorIndexToUse = UINT_MAX);
    /**
    * @brief バッファのシェーダーリソースビューを作成する
    */
    UINT createBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize);
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    setlocale(LC_ALL, "");
    MainApp app(1280, 720, L"Game");
    return app.run(hInstance, nCmdShow);
}

//これらの名前はシェーダーファイルの関数名と一致させる必要がある
//この名前をもとにエントリポイントを探すため
const wchar_t* MainApp::RAY_GEN_SHADER_NAME = L"MyRaygenShader";
const wchar_t* MainApp::CLOSEST_HIT_SHADER_NAME = L"MyClosestHitShader";
const wchar_t* MainApp::MISS_SHADER_NAME = L"MyMissShader";

//HitGroupは名前は何でもよい
const wchar_t* MainApp::HIT_GROUP_NAME = L"MyHitGroup";

static float X = 0.0f;
void MainApp::updateCameraMatrices() {

    mEye = { 0.0f,X,-10.0f,1.0f };
    mAt = { 0.0f,X,0.0f,1.0f };
    mUp = { 0.0f,1.0f,0.0f,1.0f };
    //X += 0.0001f;

    mSceneCB->cameraPosition = mEye;

    float fovY = 45.0f;
    XMMATRIX view = XMMatrixLookAtLH(mEye, mAt, mUp);
    const float aspect = static_cast<float>(mWidth) / static_cast<float>(mHeight);
    //const float aspect = static_cast<float>(mHeight) / static_cast<float>(mWidth);
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovY), aspect, 0.1f, 125.0f);
    XMMATRIX vp = view * proj;
    mSceneCB->projectionToWorld = XMMatrixInverse(nullptr, vp);
}

void MainApp::initializeScene() {
    updateCameraMatrices();
}

void MainApp::recreateD3D() {
    try {
        mDeviceResource->waitForGPU();
    }
    catch (const HrException&) {

    }
    mDeviceResource->handleDeviceLost();
}

void MainApp::doRaytracing() {
    ID3D12GraphicsCommandList* list = mDeviceResource->getCommandList();
    UINT frameCount = mDeviceResource->getCurrentFrameIndex();
    auto dispatchRays = [&](ID3D12GraphicsCommandList5* list, ID3D12StateObject* state, D3D12_DISPATCH_RAYS_DESC* desc) {
        desc->HitGroupTable.StartAddress = mHitGroupShaderTable->GetGPUVirtualAddress();
        desc->HitGroupTable.SizeInBytes = mHitGroupShaderTable->GetDesc().Width;
        desc->HitGroupTable.StrideInBytes = desc->HitGroupTable.SizeInBytes;

        desc->MissShaderTable.StartAddress = mMissShaderTable->GetGPUVirtualAddress();
        desc->MissShaderTable.SizeInBytes = mMissShaderTable->GetDesc().Width;
        desc->MissShaderTable.StrideInBytes = desc->MissShaderTable.SizeInBytes;

        desc->RayGenerationShaderRecord.StartAddress = mRayGenShaderTable->GetGPUVirtualAddress();
        desc->RayGenerationShaderRecord.SizeInBytes = mRayGenShaderTable->GetDesc().Width;

        desc->Width = mWidth;
        desc->Height = mHeight;
        desc->Depth = 1;

        list->SetPipelineState1(state);
        list->DispatchRays(desc);
    };

    list->SetComputeRootSignature(mRaytracingGlobalRootSignature.Get());
    mSceneCB.copyStagingToGPU(frameCount);
    list->SetComputeRootConstantBufferView(GlobalRootSignatureParameter::ConstantBuffer, mSceneCB.gpuVirtualAddress(frameCount));

    D3D12_DISPATCH_RAYS_DESC desc = {};
    list->SetDescriptorHeaps(1, mDescriptorHeap.GetAddressOf());
    list->SetComputeRootDescriptorTable(GlobalRootSignatureParameter::RenderTarget, mRaytracingOutputResourceUAVGpuDescriptor);
    list->SetComputeRootShaderResourceView(GlobalRootSignatureParameter::AccelerationStructureSlot, mTopLevelAS->GetGPUVirtualAddress());
    list->SetComputeRootDescriptorTable(GlobalRootSignatureParameter::VertexBuffers, mIndexBuffer.gpuHandle);
    dispatchRays(mDXRCommandList.Get(), mDXRStateObject.Get(), &desc);
}

void MainApp::createConstantBuffers() {
    ID3D12Device* device = mDeviceResource->getDevice();
    UINT frameCount = mDeviceResource->getBackBufferCount();
    mSceneCB.create(device, frameCount, L"SceneConstant");
}

void MainApp::createDeviceDependentResources() {
    //補助リソースを先に生成
    //createAuxillaryDeviceResources();
    //レイトレース用インターフェース作成
    createRaytracinginterfaces();
    ////ルートシグネチャを作成する
    createRootSignatures();
    ////レイトレーシングに必要なパイプラインオブジェクトを生成する
    createRaytracingPipelineStateObject();
    //ヒープ作成
    createDescriptorHeap();
    //ジオメトリ作成
    buildGeometry();
    //AS作成
    buildAccelerationStructures();
    //コンスタントバッファ作成
    createConstantBuffers();
    //シェーダーテーブル作成
    buildShaderTables();
    //レイトレーシング出力先を作成
    createRaytracingOutputResource();
}

void MainApp::createWindowSizeDependentResources() {
    createRaytracingOutputResource();
    buildShaderTables();
    updateCameraMatrices();
}

void MainApp::releaseDeviceDependentResources() {
    mRaytracingGlobalRootSignature.Reset();
    mDXRDevice.Reset();
    mDXRCommandList.Reset();
    mDXRStateObject.Reset();

    mDescriptorHeap.Reset();
    mDescriptorAllocated = 0;
    mRaytracingOutputResourceUAVDescriptorHeapIndex = UINT_MAX;
    mIndexBuffer.resource.Reset();
    mVertexBuffer.resource.Reset();

    mBottomLevelAS.Reset();
    mTopLevelAS.Reset();
}

void MainApp::releaseWindowSizeDependentResources() {
    mRaytracingOutput.Reset();
    mRayGenShaderTable.Reset();
    mHitGroupShaderTable.Reset();
    mMissShaderTable.Reset();
}

void MainApp::createRaytracinginterfaces() {
    ID3D12Device* device = mDeviceResource->getDevice();
    ID3D12CommandList* list = mDeviceResource->getCommandList();

    throwIfFailed(device->QueryInterface(IID_PPV_ARGS(&mDXRDevice)), L"Couldn't get DirectX Raytracing interface for the device.\n");
    throwIfFailed(list->QueryInterface(IID_PPV_ARGS(&mDXRCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");
}

void MainApp::serializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig) {
    ID3D12Device* device = mDeviceResource->getDevice();

    ComPtr<ID3DBlob> blob, error;
    throwIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), L"ルートシグネチャ作成失敗");
    throwIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig))));
}

void MainApp::createRootSignatures() {
    ID3D12Device* device = mDeviceResource->getDevice();

    //まずはグローバルルートシグネチャを作成する
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[2];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); //レンダーターゲット
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1); //頂点バッファ

        CD3DX12_ROOT_PARAMETER param[GlobalRootSignatureParameter::Count];
        param[GlobalRootSignatureParameter::RenderTarget].InitAsDescriptorTable(1, &ranges[0]);
        param[GlobalRootSignatureParameter::AccelerationStructureSlot].InitAsShaderResourceView(0);
        param[GlobalRootSignatureParameter::VertexBuffers].InitAsDescriptorTable(1, &ranges[1]);
        param[GlobalRootSignatureParameter::ConstantBuffer].InitAsConstantBufferView(0);

        CD3DX12_ROOT_SIGNATURE_DESC desc(ARRAYSIZE(param), param);
        serializeAndCreateRaytracingRootSignature(desc, &mRaytracingGlobalRootSignature);
        mRaytracingGlobalRootSignature->SetName(L"GlobalRootSignature");
    }
    //ローカルルートシグネチャを作成する
    {
#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)

        CD3DX12_ROOT_PARAMETER rootParameters[LocalRootSignatureParams::Count];
        rootParameters[LocalRootSignatureParams::Instance].InitAsConstants(SizeOfInUint32(mInstanceCB), 1, 0);
        CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
        serializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &mRaytracingLocalRootSignature);
        mRaytracingLocalRootSignature->SetName(L"LocalRootSignature");
    }
}

void MainApp::createDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* pipeline) {
    //シェーダーのエントリポイントの設定を行う
    auto lib = pipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libDxil = CD3DX12_SHADER_BYTECODE((void*)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
    lib->SetDXILLibrary(&libDxil);

    ////実際に使用するシェーダーの名前を定義する
    ////この定義された名前のエントリポイントをシェーダーコードの中から探すため、一致させる必要がある
    lib->DefineExport(RAY_GEN_SHADER_NAME);
    lib->DefineExport(CLOSEST_HIT_SHADER_NAME);
    lib->DefineExport(MISS_SHADER_NAME);
}

void MainApp::createHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* pipeline) {
    auto hitGroup = pipeline->CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    //今回はClosest Hitのみ使う
    hitGroup->SetClosestHitShaderImport(CLOSEST_HIT_SHADER_NAME);
    //HitGroupをエクスポートする
    hitGroup->SetHitGroupExport(HIT_GROUP_NAME);
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE::D3D12_HIT_GROUP_TYPE_TRIANGLES);
}

void MainApp::createLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* pipeline) {
    auto local = pipeline->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
    local->SetRootSignature(mRaytracingLocalRootSignature.Get());

    auto asso = pipeline->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
    asso->SetSubobjectToAssociate(*local);
    asso->AddExport(HIT_GROUP_NAME);
}

void MainApp::createRaytracingPipelineStateObject() {
    //RTPSOの作成
    CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE::D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

    //最初にDXILライブラリの設定を行う
    //Dxilライブラリ
    createDxilLibrarySubobject(&raytracingPipeline);

    //ヒットグループ系のサブオブジェクト
    //Intersection,Closest Hit,AnyHitは普通ひとまとめになっている
    //それをまとめる処理
    createHitGroupSubobjects(&raytracingPipeline);

    //シェーダー設定
    //サイズは使用するオブジェクトの最大値を利用する
    auto* config = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = sizeof(float) * 4; //レイが当たった時の情報を格納するpayload
    UINT attrSize = 2 * sizeof(float); //三角形の重心情報を利用する
    config->Config(payloadSize, attrSize);

    createLocalRootSignatureSubobjects(&raytracingPipeline);

    //グローバルルートシグネチャを設定する
    auto* global = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    global->SetRootSignature(mRaytracingGlobalRootSignature.Get());

    auto* pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    UINT maxDepth = 1;
    pipelineConfig->Config(maxDepth);

    throwIfFailed(mDXRDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&mDXRStateObject)), L"StateObject作成失敗");
}

void MainApp::createAuxillaryDeviceResources() {
    //タイマー作ったりなんかする
}

void MainApp::createDescriptorHeap() {
    ID3D12Device* device = mDeviceResource->getDevice();

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 3; //1つはレンダーターゲット、２つは頂点バッファ
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mDescriptorHeap));
    mDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void MainApp::createRaytracingOutputResource() {
    ID3D12Device* device = mDeviceResource->getDevice();
    DXGI_FORMAT format = mDeviceResource->getBackBufferFormat();
    CD3DX12_RESOURCE_DESC uavResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, mWidth, mHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT);
    throwIfFailed(device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
        &uavResourceDesc,
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(&mRaytracingOutput)));

    D3D12_CPU_DESCRIPTOR_HANDLE uavHandle;
    mRaytracingOutputResourceUAVDescriptorHeapIndex = allocateDescriptor(&uavHandle, mRaytracingOutputResourceUAVDescriptorHeapIndex);

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;
    device->CreateUnorderedAccessView(mRaytracingOutput.Get(), nullptr, &uavDesc, uavHandle);
    mRaytracingOutputResourceUAVGpuDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(mDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), mRaytracingOutputResourceUAVDescriptorHeapIndex, mDescriptorSize);
}

void MainApp::buildGeometry() {

    Framework::Utility::FBXLoader loader("p.fbx");
    std::vector<UINT> indices32;
    std::vector<Framework::Math::Vector4> pos;
    loader.getPosition(&pos, &indices32);
    std::vector<Framework::Math::Vector3> normal = loader.getNormal();
    std::vector<Index> indices(indices32.size());
    for (UINT i = 0; i < indices32.size(); i++) {
        indices[i] = static_cast<Index>(indices32[i]);
    }
    std::vector<Vertex> vertices(pos.size());
    float scale = 0.5f;
    for (UINT i = 0; i < pos.size(); i++) {
        vertices[i].position = { pos[i].x * scale,pos[i].y * scale,pos[i].z  * scale};
        vertices[i].normal = { normal[i].x,normal[i].y,normal[i].z };
    }

    //std::vector<Index> indices =
    //{
    //    3,1,0,
    //    2,1,3,

    //    6,4,5,
    //    7,4,6,

    //    11,9,8,
    //    10,9,11,

    //    14,12,13,
    //    15,12,14,

    //    19,17,16,
    //    18,17,19,

    //    22,20,21,
    //    23,20,22
    //};

    //std::vector<Vertex> vertices =
    //{
    //    { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    //{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    //{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    //{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

    //{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
    //{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
    //{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
    //{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },

    //{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
    //{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
    //{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
    //{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

    //{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
    //{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
    //{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
    //{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

    //{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
    //{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
    //{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
    //{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

    //{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    //{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    //{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    //{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    //};

    //Index indices[] =
    //{
    //    0,1,2
    //};
    //Vertex vertices[] =
    //{
    //    {XMFLOAT3(0,3,0),XMFLOAT3(0,0,-1)},
    //    {XMFLOAT3(1,-0,0),XMFLOAT3(0,0,-1)},
    //    {XMFLOAT3(-1,0,0),XMFLOAT3(0,0,-1)},
    //};

    ID3D12Device* device = mDeviceResource->getDevice();
    allocateUploadBuffer(device, indices.data(), indices.size() * sizeof(indices[0]), &mIndexBuffer.resource);
    allocateUploadBuffer(device, vertices.data(), vertices.size() * sizeof(vertices[0]), &mVertexBuffer.resource);

    //UINT IB = createBufferSRV(&mIndexBuffer, ARRAYSIZE(indices), sizeof(Index));
    UINT IB = createBufferSRV(&mIndexBuffer, indices.size() * sizeof(indices[0]) / 4, 0);
    UINT VB = createBufferSRV(&mVertexBuffer, vertices.size(), sizeof(vertices[0]));
}

void MainApp::buildAccelerationStructures() {
    auto device = mDeviceResource->getDevice();
    auto commandList = mDeviceResource->getCommandList();
    auto commandQueue = mDeviceResource->getCommandQueue();
    auto commandAllocator = mDeviceResource->getCommandAllocator();

    // Reset the command list for the acceleration structure construction.
    commandList->Reset(commandAllocator, nullptr);

    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geometryDesc.Triangles.IndexBuffer = mIndexBuffer.resource->GetGPUVirtualAddress();
    geometryDesc.Triangles.IndexCount = static_cast<UINT>(mIndexBuffer.resource->GetDesc().Width) / sizeof(Index);
    geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
    geometryDesc.Triangles.Transform3x4 = 0;
    geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    geometryDesc.Triangles.VertexCount = static_cast<UINT>(mVertexBuffer.resource->GetDesc().Width) / sizeof(Vertex);
    geometryDesc.Triangles.VertexBuffer.StartAddress = mVertexBuffer.resource->GetGPUVirtualAddress();
    geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);

    // Mark the geometry as opaque. 
    // PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
    // Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    // Get required sizes for an acceleration structure.
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
    topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    topLevelInputs.Flags = buildFlags;
    topLevelInputs.NumDescs = 1;
    topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
    mDXRDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = topLevelInputs;
    bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    bottomLevelInputs.pGeometryDescs = &geometryDesc;
    mDXRDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);

    ComPtr<ID3D12Resource> scratchResource;
    allocateUAVBuffer(device, std::max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes), &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

    // Allocate resources for acceleration structures.
    // Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
    // Default heap is OK since the application doesn稚 need CPU read/write access to them. 
    // The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
    // and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
    //  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
    //  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
    {
        D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

        allocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &mBottomLevelAS, initialResourceState, L"BottomLevelAccelerationStructure");
        allocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &mTopLevelAS, initialResourceState, L"TopLevelAccelerationStructure");
    }

    // Create an instance desc for the bottom-level acceleration structure.
    ComPtr<ID3D12Resource> instanceDescs;
    D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
    XMMATRIX trans = XMMatrixRotationRollPitchYaw(0, 0, XMConvertToRadians(45.0f)) * XMMatrixTranslation(5.0f, 0.0f, 0.0f);
    XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(instanceDesc.Transform), trans);
    instanceDesc.InstanceMask = 1;
    instanceDesc.AccelerationStructure = mBottomLevelAS->GetGPUVirtualAddress();
    allocateUploadBuffer(device, &instanceDesc, sizeof(instanceDesc), &instanceDescs, L"InstanceDescs");

    // Bottom Level Acceleration Structure desc
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
    {
        bottomLevelBuildDesc.Inputs = bottomLevelInputs;
        bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
        bottomLevelBuildDesc.DestAccelerationStructureData = mBottomLevelAS->GetGPUVirtualAddress();
    }

    // Top Level Acceleration Structure desc
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
    {
        topLevelInputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
        topLevelBuildDesc.Inputs = topLevelInputs;
        topLevelBuildDesc.DestAccelerationStructureData = mTopLevelAS->GetGPUVirtualAddress();
        topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
    }

    auto BuildAccelerationStructure = [&](auto* raytracingCommandList) {
        raytracingCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(mBottomLevelAS.Get()));
        raytracingCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
    };

    // Build acceleration structure.
    BuildAccelerationStructure(mDXRCommandList.Get());

    // Kick off acceleration structure construction.
    mDeviceResource->executeCommandList();

    // Wait for GPU to finish as the locally created temporary GPU resources will get released once we go out of scope.
    mDeviceResource->waitForGPU();
}

void MainApp::buildShaderTables() {
    ID3D12Device* device = mDeviceResource->getDevice();

    void* rayGenShaderID;
    void* missShaderID;
    void* hitGroupShaderID;
    UINT shaderIDSize;

    ComPtr<ID3D12StateObjectProperties> props;
    throwIfFailed(mDXRStateObject.As(&props));
    rayGenShaderID = props->GetShaderIdentifier(RAY_GEN_SHADER_NAME);
    missShaderID = props->GetShaderIdentifier(MISS_SHADER_NAME);
    hitGroupShaderID = props->GetShaderIdentifier(HIT_GROUP_NAME);
    shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

    {
        struct RootArguments {
            Instance cb;
        } rootArguments;
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIDSize + sizeof(RootArguments);
        ShaderTable table(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
        table.push_back(ShaderRecord(rayGenShaderID, shaderIDSize, &rootArguments, sizeof(rootArguments)));
        mRayGenShaderTable = table.getResource();
    }

    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIDSize;
        ShaderTable table(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");
        table.push_back(ShaderRecord(missShaderID, shaderIDSize));
        mMissShaderTable = table.getResource();
    }

    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIDSize;
        ShaderTable table(device, numShaderRecords, shaderRecordSize, L"HitGroupTable");
        table.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize));
        mHitGroupShaderTable = table.getResource();
    }
}

void MainApp::updateForSizeChange(UINT clientWidth, UINT clientHeight) {
    //float border = 0.1f;
    //float aspect = static_cast<float>(mWidth) / static_cast<float>(mHeight);
    //mRaygenCB.stencil =
    //{
    //    -1 + border / aspect, -1 + border,
    //    1 - border / aspect, 1.0f - border
    //};
    //mRaygenCB.viewport = { -1.0f, -1.0f, 1.0f, 1.0f };

}

void MainApp::copyOutput() {
    ID3D12GraphicsCommandList* list = mDeviceResource->getCommandList();
    ID3D12Resource* renderTarget = mDeviceResource->getRenderTarget();

    D3D12_RESOURCE_BARRIER preBarrier[2];
    preBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget,
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
    preBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(mRaytracingOutput.Get(),
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE);

    list->ResourceBarrier(ARRAYSIZE(preBarrier), preBarrier);

    list->CopyResource(renderTarget, mRaytracingOutput.Get());

    D3D12_RESOURCE_BARRIER postBarrier[2];
    postBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget,
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT);
    postBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(mRaytracingOutput.Get(),
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    list->ResourceBarrier(ARRAYSIZE(postBarrier), postBarrier);
}

//static float XXX = 0.0f;
void MainApp::calcFrameStatus() {
    //XXX += 0.0001f;
    //mEye = { XXX,0,-10,1 };
    updateCameraMatrices();
}

UINT MainApp::allocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, UINT descriptorIndexToUse) {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

    if (descriptorIndexToUse >= mDescriptorHeap->GetDesc().NumDescriptors) {
        throwIfFalse(mDescriptorAllocated < mDescriptorHeap->GetDesc().NumDescriptors, L"Ran out of descriptors on the heap!");
        descriptorIndexToUse = mDescriptorAllocated++;
    }
    *cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(handle, descriptorIndexToUse, mDescriptorSize);
    return descriptorIndexToUse;
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

    UINT index = allocateDescriptor(&buffer->cpuHandle);
    device->CreateShaderResourceView(buffer->resource.Get(), &srvDesc, buffer->cpuHandle);
    buffer->gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), index, mDescriptorSize);
    return index;
}
