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
#include "FBXLoader.h"
#include "../Assets/Shader/RaytracingStructure.h"
#include "Utility/GPUTimer.h"
#include "Utility/StringUtil.h"
#include "ImGui/Item/Window.h"
#include "ImGui/Item/Text.h"
#include "ImGui/Item/FloatField.h"
#include "Utility/Time.h"

#ifdef _DEBUG
#include "Temp/bin/x64/Debug/Application/CompiledShaders/Raytracing.hlsl.h"
#else
#include "Temp/bin/x64/Release/Application/CompiledShaders/Raytracing.hlsl.h"
#endif

using namespace DirectX;

using namespace Framework::DX;

/**
* @brief �O���[�o���̃��\�[�X�p�����[�^
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
* @brief ���[�J���̃��[�g�V�O�l�`���p�����[�^
*/
namespace LocalRootSignatureParams {
    enum MyEnum {
        Instance = 0,
        Count
    };
} //LocalRootSignatureParams

namespace GeometryType {
    enum MyEnum {
        Cube,
        Triangle,
        Count
    };
}

static constexpr UINT TRIANGLE_COUNT = 9;
static constexpr UINT CUBE_COUNT = 1;

/**
* @class MainApp
* @brief discription
*/
class MainApp : public Framework::Game {
public:
    /**
    * @brief �R���X�g���N�^
    */
    MainApp(UINT width, UINT height, const std::wstring& title)
        :Game(width, height, title),
        mRaytracingOutputResourceUAVDescriptorHeapIndex(UINT_MAX),
        mDescriptorSize(0),
        mImGUIWindow(std::make_unique<Framework::ImGUI::Window>("GPU")) {
        mGPUInfoText = std::make_shared<Framework::ImGUI::Text>("");
        mImGUIWindow->addItem(mGPUInfoText);
        updateForSizeChange(width, height);
    }
    /**
    * @brief �f�X�g���N�^
    */
    ~MainApp() {

    }
    virtual void onInit() override {
        Game::onInit();

        mDXRInterface = std::make_unique<Framework::DX::DXRInterface>(mDeviceResource.get());

        //�V�[�������ɍ���Ă���
        initializeScene();
        //���\�[�X�����ꂽ��Ƀf�o�C�X���쐬����
        createDeviceDependentResources();
        createWindowSizeDependentResources();

        Framework::ImGuiManager::getInstance()->init(mWindow->getHwnd(), mDeviceResource->getDevice(), mDeviceResource->getBackBufferFormat());
    }
    virtual void onUpdate() override {
        Game::onUpdate();
        mTimer.update();
        Framework::ImGuiManager::getInstance()->beginFrame();

        calcFrameStatus();
    }

    virtual void onRender() override {
        Game::onRender();

        ID3D12Device* device = mDeviceResource->getDevice();
        ID3D12GraphicsCommandList* list = mDeviceResource->getCommandList();

        mDeviceResource->prepare();
        mGPUTimer.beginFrame();

        doRaytracing();
        copyOutput();

#ifdef _DEBUG
        mImGUIWindow->draw();
        mCameraParameterWindow->draw();
#endif
        Framework::ImGuiManager::getInstance()->endFrame(mDeviceResource->getCommandList());

        mGPUTimer.endFrame(list);
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
    static constexpr UINT NUM_BLAS = 1; //�{�g�����x��AS�g�p����1��

    //DXR�I�u�W�F�N�g
    std::unique_ptr<Framework::DX::DXRInterface> mDXRInterface;

    ComPtr<ID3D12RootSignature> mRaytracingGlobalRootSignature; //!< �O���[�o�����[�g�V�O�l�`��
    ComPtr<ID3D12RootSignature> mRaytracingLocalRootSignature; //!< ���[�J�����[�g�V�O�l�`��

    //�f�B�X�N���v�^�q�[�v
    ComPtr<ID3D12DescriptorHeap> mDescriptorHeap;
    UINT mDescriptorAllocated;
    UINT mDescriptorSize;

    ConstantBuffer<SceneConstantBuffer> mSceneCB;
    ConstantBuffer<Instance> mInstanceCB;
    //XMVECTOR mEye, mAt, mUp;
    XMFLOAT3 mCameraPosition;
    XMFLOAT3 mCameraRotation;

    //D3DBuffer mIndexBuffer;
    //D3DBuffer mVertexBuffer;



    //AS
    ComPtr<ID3D12Resource> mBottomLevelAS[GeometryType::Count];
    ComPtr<ID3D12Resource> mTopLevelAS;
    uint64_t mTLASSize;

    //���C�g���[�V���O�o�͐�
    ComPtr<ID3D12Resource> mRaytracingOutput;
    D3D12_GPU_DESCRIPTOR_HANDLE mRaytracingOutputResourceUAVGpuDescriptor;
    UINT mRaytracingOutputResourceUAVDescriptorHeapIndex;

    //�V�F�[�_�[�e�[�u��
    static const wchar_t* HIT_GROUP_NAME;
    static const wchar_t* RAY_GEN_SHADER_NAME;
    static const wchar_t* CLOSEST_HIT_SHADER_NAME;
    static const wchar_t* MISS_SHADER_NAME;

    ComPtr<ID3D12Resource> mMissShaderTable;
    ComPtr<ID3D12Resource> mHitGroupShaderTable;
    ComPtr<ID3D12Resource> mRayGenShaderTable;
    Framework::Utility::GPUTimer mGPUTimer;

    std::unique_ptr<Framework::ImGUI::Window> mImGUIWindow;
    std::shared_ptr<Framework::ImGUI::Text> mGPUInfoText;
    Framework::Utility::Time mTimer;
    std::shared_ptr<Framework::ImGUI::Window> mCameraParameterWindow;

    std::array<D3DBuffer, GeometryType::Count> mGeometryIndexBuffers;
    std::array<D3DBuffer, GeometryType::Count> mGeometryVertexBuffers;


     /**
     * @brief �J�����s��̍X�V
     */
    void updateCameraMatrices();
    /**
    * @brief �V�[���̏�����
    */
    void initializeScene();
    /**
    * @brief �f�o�C�X�̍Đ���
    */
    void recreateD3D();
    /**
    * @brief ���C�g���[�V���O���s
    */
    void doRaytracing();
    /**
    * @brief �R���X�^���g�o�b�t�@�̍쐬
    */
    void createConstantBuffers();
    /**
    * @brief �f�o�C�X�Ɋ�Â����\�[�X�̐���
    */
    void createDeviceDependentResources();
    /**
    * @brief �E�B���h�E�T�C�Y�Ɋ�Â����\�[�X�̍쐬
    */
    void createWindowSizeDependentResources();
    /**
    * @brief �f�o�C�X�Ɋ�Â����\�[�X�̉��
    */
    void releaseDeviceDependentResources();
    /**
    * @brief �E�B���h�E�T�C�Y�Ɋ�Â����\�[�X�̉��
    */
    void releaseWindowSizeDependentResources();
    /**
    * @brief ���C�g���[�V���O�p�C���^�[�t�F�[�X�쐬
    */
    void createRaytracinginterfaces();
    /**
    * @brief ���[�g�V�O�l�`�����쐬����
    */
    void serializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);
    /**
    * @brief ���[�g�V�O�l�`���̍쐬
    */
    void createRootSignatures();
    /**
    * @brief DXIL���C�u�����̃T�u�I�u�W�F�N�g�쐬
    */
    void createDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* pipeline);
    /**
    * @brief �q�b�g�O���[�v�̃T�u�I�u�W�F�N�g�쐬
    */
    void createHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* pipeline);
    /**
    * @brief ���[�J�����[�g�V�O�l�`���̃T�u�I�u�W�F�N�g�쐬
    */
    void createLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* pipeline);
    /**
    * @brief ���C�g���[�V���O�p�C�v���C���I�u�W�F�N�g�쐬
    */
    void createRaytracingPipelineStateObject();
    /**
    * @brief �����ȕ����I�ȃ��\�[�X�쐬
    */
    void createAuxillaryDeviceResources();
    /**
    * @brief �f�B�X�N���v�^�q�[�v�̍쐬
    */
    void createDescriptorHeap();
    /**
    * @brief ���C�g���[�V���O�̏o�͐���쐬
    */
    void createRaytracingOutputResource();
    /**
    * @brief �W�I���g���̍쐬
    */
    void buildCubeGeometry(D3DBuffer* indexBuffer, D3DBuffer* vertexBuffer);
    /**
    * @brief �O�p�`�̃W�I���g������
    */
    void buildTriangleGeometry(D3DBuffer* indexBuffer, D3DBuffer* vertexBuffer);
    /**
    * @brief �{�g�����x����AS���\�z����
    */
    Framework::DX::AccelerationStructureBuffers buildBLAS(std::vector<D3DBuffer> indexBuffers, std::vector<D3DBuffer> vertexBuffers);
    Framework::DX::AccelerationStructureBuffers buildTLAS(ComPtr<ID3D12Resource> bottomLevelAS[GeometryType::Count], uint64_t tlasSize);

    /**
    * @brief AS�̍쐬
    */
    void buildAccelerationStructures();
    /**
    * @brief �V�F�[�_�[�e�[�u���쐬
    */
    void buildShaderTables();
    /**
    * @brief �E�B���h�E�T�C�Y�X�V
    */
    void updateForSizeChange(UINT clientWidth, UINT clientHeight);
    /**
    * @brief ���C�g���[�V���O�̏o�͂��o�b�N�o�b�t�@�ɃR�s�[����
    */
    void copyOutput();
    /**
    * @brief �X�V����
    */
    void calcFrameStatus();
    /**
    * @brief �f�B�X�N���v�^�q�[�v�̃A���P�[�g����
    */
    UINT allocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, UINT descriptorIndexToUse = UINT_MAX);
    /**
    * @brief �o�b�t�@�̃V�F�[�_�[���\�[�X�r���[���쐬����
    */
    UINT createBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize);

    ComPtr<ID3D12Resource> createBuffer(uint64_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps);
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    setlocale(LC_ALL, "");
    MainApp app(1280, 720, L"Game");
    return app.run(hInstance, nCmdShow);
}

//�����̖��O�̓V�F�[�_�[�t�@�C���̊֐����ƈ�v������K�v������
//���̖��O�����ƂɃG���g���|�C���g��T������
const wchar_t* MainApp::RAY_GEN_SHADER_NAME = L"MyRaygenShader";
const wchar_t* MainApp::CLOSEST_HIT_SHADER_NAME = L"MyClosestHitShader";
const wchar_t* MainApp::MISS_SHADER_NAME = L"MyMissShader";

//HitGroup�͖��O�͉��ł��悢
const wchar_t* MainApp::HIT_GROUP_NAME = L"MyHitGroup";

void MainApp::updateCameraMatrices() {
    mSceneCB->cameraPosition = XMLoadFloat3(&mCameraPosition);
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
}

void MainApp::initializeScene() {
    //mEye = { 0.0f,10.0f,-10.0f,1.0f };
    //mAt = { 0.0f,0.0f,0.0f,1.0f };
    //mUp = { 0.0f,1.0f,0.0f,1.0f };
    mCameraPosition = { 0,3.0f,-10.0f };
    mCameraRotation = { 0,0,0 };

    mCameraParameterWindow = std::make_unique<Framework::ImGUI::Window>("Camera");
#define PARAMETER_CHANGE_SLIDER(name,type,min,max){\
    auto field = std::make_shared<Framework::ImGUI::FloatField>(name, type); \
    field->setCallBack([&](float val) {type = val; }); \
    field->setMinValue(min); \
    field->setMaxValue(max); \
    mCameraParameterWindow->addItem(field); \
    }

    mCameraParameterWindow->addItem(std::make_shared<Framework::ImGUI::Text>("Position"));
    PARAMETER_CHANGE_SLIDER("X", mCameraPosition.x, -30.0f, 30.0f);
    PARAMETER_CHANGE_SLIDER("Y", mCameraPosition.y, -30.0f, 30.0f);
    PARAMETER_CHANGE_SLIDER("Z", mCameraPosition.z, -30.0f, 30.0f);
    mCameraParameterWindow->addItem(std::make_shared<Framework::ImGUI::Text>("Rotation"));
    PARAMETER_CHANGE_SLIDER("RX", mCameraRotation.x, 0.0f, 2 * 3.14);
    PARAMETER_CHANGE_SLIDER("RY", mCameraRotation.y, 0.0f, 2 * 3.14);
    PARAMETER_CHANGE_SLIDER("RZ", mCameraRotation.z, 0.0f, 2 * 3.14);

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

        mGPUTimer.start(list);
        list->DispatchRays(desc);
        mGPUTimer.stop(list);
    };

    list->SetComputeRootSignature(mRaytracingGlobalRootSignature.Get());
    mSceneCB.copyStagingToGPU(frameCount);
    list->SetComputeRootConstantBufferView(GlobalRootSignatureParameter::ConstantBuffer, mSceneCB.gpuVirtualAddress(frameCount));

    D3D12_DISPATCH_RAYS_DESC desc = {};
    list->SetDescriptorHeaps(1, mDescriptorHeap.GetAddressOf());
    list->SetComputeRootDescriptorTable(GlobalRootSignatureParameter::RenderTarget, mRaytracingOutputResourceUAVGpuDescriptor);
    list->SetComputeRootShaderResourceView(GlobalRootSignatureParameter::AccelerationStructureSlot, mTopLevelAS->GetGPUVirtualAddress());
    list->SetComputeRootDescriptorTable(GlobalRootSignatureParameter::VertexBuffers, mGeometryIndexBuffers[0].gpuHandle);
    //list->SetComputeRootDescriptorTable(GlobalRootSignatureParameter::VertexBuffers, mIndexBuffer.gpuHandle);
    dispatchRays(mDXRInterface->getCommandList(), mDXRInterface->getStateObject(), &desc);
}

void MainApp::createConstantBuffers() {
    ID3D12Device* device = mDeviceResource->getDevice();
    UINT frameCount = mDeviceResource->getBackBufferCount();
    mSceneCB.create(device, frameCount, L"SceneConstant");
}

void MainApp::createDeviceDependentResources() {
    //�⏕���\�[�X���ɐ���
    createAuxillaryDeviceResources();
    //���C�g���[�X�p�C���^�[�t�F�[�X�쐬
    createRaytracinginterfaces();
    ////���[�g�V�O�l�`�����쐬����
    createRootSignatures();
    ////���C�g���[�V���O�ɕK�v�ȃp�C�v���C���I�u�W�F�N�g�𐶐�����
    createRaytracingPipelineStateObject();
    //�q�[�v�쐬
    createDescriptorHeap();
    ////AS�쐬
    buildAccelerationStructures();
    //�R���X�^���g�o�b�t�@�쐬
    createConstantBuffers();
    //�V�F�[�_�[�e�[�u���쐬
    buildShaderTables();
    //���C�g���[�V���O�o�͐���쐬
    createRaytracingOutputResource();
}

void MainApp::createWindowSizeDependentResources() {
    createRaytracingOutputResource();
    buildShaderTables();
    updateCameraMatrices();
}

void MainApp::releaseDeviceDependentResources() {
    mGPUTimer.releaseDevice();
    mRaytracingGlobalRootSignature.Reset();
    mDXRInterface->clear();

    mDescriptorHeap.Reset();
    mDescriptorAllocated = 0;
    mRaytracingOutputResourceUAVDescriptorHeapIndex = UINT_MAX;
    //mIndexBuffer.resource.Reset();
    //mVertexBuffer.resource.Reset();

    //mBottomLevelAS.Reset();
    //mTopLevelAS.Reset();
}

void MainApp::releaseWindowSizeDependentResources() {
    mRaytracingOutput.Reset();
    mRayGenShaderTable.Reset();
    mHitGroupShaderTable.Reset();
    mMissShaderTable.Reset();
}

void MainApp::createRaytracinginterfaces() {
    mDXRInterface->recreate();
}

void MainApp::serializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig) {
    ID3D12Device* device = mDeviceResource->getDevice();

    ComPtr<ID3DBlob> blob, error;
    throwIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), L"���[�g�V�O�l�`���쐬���s");
    throwIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig))));
}

void MainApp::createRootSignatures() {
    ID3D12Device* device = mDeviceResource->getDevice();

    //�܂��̓O���[�o�����[�g�V�O�l�`�����쐬����
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[2];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); //�����_�[�^�[�Q�b�g
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1); //���_�o�b�t�@

        CD3DX12_ROOT_PARAMETER param[GlobalRootSignatureParameter::Count];
        param[GlobalRootSignatureParameter::RenderTarget].InitAsDescriptorTable(1, &ranges[0]);
        param[GlobalRootSignatureParameter::AccelerationStructureSlot].InitAsShaderResourceView(0);
        param[GlobalRootSignatureParameter::VertexBuffers].InitAsDescriptorTable(1, &ranges[1]);
        param[GlobalRootSignatureParameter::ConstantBuffer].InitAsConstantBufferView(0);

        CD3DX12_ROOT_SIGNATURE_DESC desc(ARRAYSIZE(param), param);
        serializeAndCreateRaytracingRootSignature(desc, &mRaytracingGlobalRootSignature);
        mRaytracingGlobalRootSignature->SetName(L"GlobalRootSignature");
    }
    //���[�J�����[�g�V�O�l�`�����쐬����
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
    //�V�F�[�_�[�̃G���g���|�C���g�̐ݒ���s��
    auto lib = pipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libDxil = CD3DX12_SHADER_BYTECODE((void*)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
    lib->SetDXILLibrary(&libDxil);

    ////���ۂɎg�p����V�F�[�_�[�̖��O���`����
    ////���̒�`���ꂽ���O�̃G���g���|�C���g���V�F�[�_�[�R�[�h�̒�����T�����߁A��v������K�v������
    lib->DefineExport(RAY_GEN_SHADER_NAME);
    lib->DefineExport(CLOSEST_HIT_SHADER_NAME);
    lib->DefineExport(MISS_SHADER_NAME);
}

void MainApp::createHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* pipeline) {
    auto hitGroup = pipeline->CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    //�����Closest Hit�̂ݎg��
    hitGroup->SetClosestHitShaderImport(CLOSEST_HIT_SHADER_NAME);
    //HitGroup���G�N�X�|�[�g����
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
    //RTPSO�̍쐬
    CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE::D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

    //�ŏ���DXIL���C�u�����̐ݒ���s��
    //Dxil���C�u����
    createDxilLibrarySubobject(&raytracingPipeline);

    //�q�b�g�O���[�v�n�̃T�u�I�u�W�F�N�g
    //Intersection,Closest Hit,AnyHit�͕��ʂЂƂ܂Ƃ߂ɂȂ��Ă���
    //������܂Ƃ߂鏈��
    createHitGroupSubobjects(&raytracingPipeline);

    //�V�F�[�_�[�ݒ�
    //�T�C�Y�͎g�p����I�u�W�F�N�g�̍ő�l�𗘗p����
    auto* config = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = sizeof(float) * 4; //���C�������������̏����i�[����payload
    UINT attrSize = 2 * sizeof(float); //�O�p�`�̏d�S���𗘗p����
    config->Config(payloadSize, attrSize);

    createLocalRootSignatureSubobjects(&raytracingPipeline);

    //�O���[�o�����[�g�V�O�l�`����ݒ肷��
    auto* global = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    global->SetRootSignature(mRaytracingGlobalRootSignature.Get());

    auto* pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    UINT maxDepth = 1;
    pipelineConfig->Config(maxDepth);

    mDXRInterface->createStateObject(raytracingPipeline);
    //throwIfFailed(mDXRDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&mDXRStateObject)), L"StateObject�쐬���s");
}

void MainApp::createAuxillaryDeviceResources() {
    ID3D12Device* device = mDeviceResource->getDevice();
    ID3D12CommandQueue* queue = mDeviceResource->getCommandQueue();
    UINT frameCount = mDeviceResource->getBackBufferCount();

    mGPUTimer.storeDevice(device, queue, frameCount);
}

void MainApp::createDescriptorHeap() {
    ID3D12Device* device = mDeviceResource->getDevice();

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 3; //1�̓����_�[�^�[�Q�b�g�A�Q�͒��_�o�b�t�@
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mDescriptorHeap));
    mDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void MainApp::createRaytracingOutputResource() {
    ID3D12Device* device = mDeviceResource->getDevice();
    DXGI_FORMAT format = mDeviceResource->getBackBufferFormat();
    CD3DX12_RESOURCE_DESC uavResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        format, mWidth, mHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

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

//�L���[�u�̃W�I���g���𐶐�����
void MainApp::buildCubeGeometry(D3DBuffer* indexBuffer, D3DBuffer* vertexBuffer) {
    std::vector<Index> indices =
    {
        3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
    };

    std::vector<Vertex> vertices =
    {
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
    { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },

    { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

    { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
    { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
    { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

    { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    };

    ID3D12Device* device = mDeviceResource->getDevice();
    allocateUploadBuffer(device, indices.data(), indices.size() * sizeof(indices[0]), &indexBuffer->resource);
    allocateUploadBuffer(device, vertices.data(), vertices.size() * sizeof(vertices[0]), &vertexBuffer->resource);

    createBufferSRV(indexBuffer, static_cast<UINT>(indices.size()) * sizeof(indices[0]) / 4, 0);
    createBufferSRV(vertexBuffer, static_cast<UINT>(vertices.size()), sizeof(vertices[0]));
}

//�O�p�`�̃W�I���g�����쐬����
void MainApp::buildTriangleGeometry(D3DBuffer* indexBuffer, D3DBuffer* vertexBuffer) {
    Index indices[] = { 0,1,2 };
    Vertex vertices[] = {
        {XMFLOAT3(0,1,0),XMFLOAT3(0,0,-1) },
        {XMFLOAT3(1,0,0),XMFLOAT3(0,0,-1) },
        {XMFLOAT3(-1,0,0),XMFLOAT3(0,0,-1) },
    };

    ID3D12Device* device = mDeviceResource->getDevice();
    allocateUploadBuffer(device, indices, _countof(indices) * sizeof(indices[0]), &indexBuffer->resource);
    allocateUploadBuffer(device, vertices, _countof(vertices) * sizeof(vertices[0]), &vertexBuffer->resource);
}

//�{�g�����x��AS���\�z����
Framework::DX::AccelerationStructureBuffers MainApp::buildBLAS(
    std::vector<D3DBuffer> indexBuffers, std::vector<D3DBuffer> vertexBuffers) {
    //�W�I���g����������
    const UINT size = indexBuffers.size();
    std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs(size);

    for (int i = 0; i < size; i++) {
        D3D12_RAYTRACING_GEOMETRY_DESC& desc = geometryDescs[i];
        desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE::D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        desc.Triangles.IndexBuffer = indexBuffers[i].resource->GetGPUVirtualAddress();
        //�C���f�b�N�X�̐��̓C���f�b�N�X�̃o�C�g�T�C�Y�Ɉˑ����Ă��邽��16�r�b�g����ύX�����ꍇ���s��
        desc.Triangles.IndexCount = indexBuffers[i].resource->GetDesc().Width / sizeof(Index);
        desc.Triangles.IndexFormat = DXGI_FORMAT::DXGI_FORMAT_R16_UINT;
        desc.Triangles.VertexBuffer.StartAddress = vertexBuffers[i].resource->GetGPUVirtualAddress();
        desc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
        //���_���������s��
        desc.Triangles.VertexCount = vertexBuffers[i].resource->GetDesc().Width / sizeof(Vertex);
        desc.Triangles.VertexFormat = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
        desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAGS::D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT::D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
    inputs.NumDescs = size;
    inputs.pGeometryDescs = geometryDescs.data();
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO pre = {};
    mDXRInterface->getDXRDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &pre);

    Framework::DX::AccelerationStructureBuffers buffers;
    buffers.scratch = createBuffer(pre.ScratchDataSizeInBytes,
        D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT));
    buffers.accelerationStructure = createBuffer(pre.ResultDataMaxSizeInBytes,
        D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT));

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
    asDesc.Inputs = inputs;
    asDesc.DestAccelerationStructureData = buffers.accelerationStructure->GetGPUVirtualAddress();
    asDesc.ScratchAccelerationStructureData = buffers.scratch->GetGPUVirtualAddress();

    auto list = mDXRInterface->getCommandList();
    list->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource = buffers.accelerationStructure.Get();
    list->ResourceBarrier(1, &barrier);

    return buffers;
}

Framework::DX::AccelerationStructureBuffers MainApp::buildTLAS(ComPtr<ID3D12Resource> bottomLevelAS[GeometryType::Count], uint64_t tlasSize) {
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT::D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
    inputs.NumDescs = TRIANGLE_COUNT + CUBE_COUNT;
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO pre;
    auto device = mDXRInterface->getDXRDevice();
    device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &pre);

    AccelerationStructureBuffers buffers;
    buffers.scratch = createBuffer(pre.ScratchDataSizeInBytes,
        D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT));
    buffers.accelerationStructure = createBuffer(pre.ResultDataMaxSizeInBytes,
        D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT));
    tlasSize = pre.ResultDataMaxSizeInBytes;

    buffers.instanceDesc = createBuffer(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * (TRIANGLE_COUNT + CUBE_COUNT),
        D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
        CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD));
    D3D12_RAYTRACING_INSTANCE_DESC* instanceDescs;
    buffers.instanceDesc->Map(0, nullptr, (void**)&instanceDescs);
    ZeroMemory(instanceDescs, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * (TRIANGLE_COUNT + CUBE_COUNT));

    XMMATRIX transform[TRIANGLE_COUNT + CUBE_COUNT];
    //�L���[�u�̃g�����X�t�H�[��
    transform[0] = XMMatrixIdentity();
    for (int i = 0; i < TRIANGLE_COUNT; i++) {
        transform[i + 1] = XMMatrixTranslation((i / 3) * 3, (i % 3) * 3, 0);
    }

    instanceDescs[0].InstanceID = 0;
    instanceDescs[0].InstanceContributionToHitGroupIndex = 0;
    instanceDescs[0].Flags = D3D12_RAYTRACING_INSTANCE_FLAGS::D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
    XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(instanceDescs[0].Transform), transform[0]);
    instanceDescs[0].AccelerationStructure = bottomLevelAS[0]->GetGPUVirtualAddress();
    instanceDescs[0].InstanceMask = 0xff;

    for (int i = 1; i <= TRIANGLE_COUNT; i++) {
        instanceDescs[i].InstanceID = i;
        instanceDescs[i].InstanceContributionToHitGroupIndex = 0;
        instanceDescs[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAGS::D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
        XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(instanceDescs[i].Transform), transform[i]);
        instanceDescs[i].AccelerationStructure = bottomLevelAS[1]->GetGPUVirtualAddress();
        instanceDescs[i].InstanceMask = 0xff;
    }

    buffers.instanceDesc->Unmap(0, nullptr);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
    asDesc.Inputs = inputs;
    asDesc.Inputs.InstanceDescs = buffers.instanceDesc->GetGPUVirtualAddress();
    asDesc.DestAccelerationStructureData = buffers.accelerationStructure->GetGPUVirtualAddress();
    asDesc.ScratchAccelerationStructureData = buffers.scratch->GetGPUVirtualAddress();

    mDXRInterface->getCommandList()->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource = buffers.accelerationStructure.Get();
    mDXRInterface->getCommandList()->ResourceBarrier(1, &barrier);

    return buffers;
}

//AS���쐬����
void MainApp::buildAccelerationStructures() {
    mDeviceResource->getCommandList()->Reset(mDeviceResource->getCommandAllocator(), nullptr);

    buildCubeGeometry(&mGeometryIndexBuffers[GeometryType::Cube], &mGeometryVertexBuffers[GeometryType::Cube]);
    buildTriangleGeometry(&mGeometryIndexBuffers[GeometryType::Triangle], &mGeometryVertexBuffers[GeometryType::Triangle]);

    AccelerationStructureBuffers blasBuffers[2];
    blasBuffers[GeometryType::Cube] =
        buildBLAS({ mGeometryIndexBuffers[GeometryType::Cube] }, { mGeometryVertexBuffers[GeometryType::Cube] });
    mBottomLevelAS[GeometryType::Cube] = blasBuffers[GeometryType::Cube].accelerationStructure;

    blasBuffers[GeometryType::Triangle] =
        buildBLAS({ mGeometryIndexBuffers[GeometryType::Triangle] }, { mGeometryVertexBuffers[GeometryType::Triangle] });
    mBottomLevelAS[GeometryType::Triangle] = blasBuffers[GeometryType::Triangle].accelerationStructure;

    AccelerationStructureBuffers tlasBuffer = buildTLAS(mBottomLevelAS, mTLASSize);

    mDeviceResource->executeCommandList();
    mDeviceResource->waitForGPU();

    mTopLevelAS = tlasBuffer.accelerationStructure;
}

void MainApp::buildShaderTables() {
    ID3D12Device* device = mDeviceResource->getDevice();

    void* rayGenShaderID;
    void* missShaderID;
    void* hitGroupShaderID;
    UINT shaderIDSize;

    ComPtr<ID3D12StateObjectProperties> props;
    mDXRInterface->getStateObject()->QueryInterface(IID_PPV_ARGS(&props));
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

void MainApp::updateForSizeChange(UINT clientWidth, UINT clientHeight) { }

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
        D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
    postBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(mRaytracingOutput.Get(),
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

ComPtr<ID3D12Resource> MainApp::createBuffer(uint64_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES & heapProps) {
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size, flags);
    ComPtr<ID3D12Resource> buffer;
    ID3D12Device* device = mDeviceResource->getDevice();
    throwIfFailed(device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        initState,
        nullptr,
        IID_PPV_ARGS(&buffer)));

    return buffer;
}
