#include "RaytracingShader.h"
#include <algorithm>
#include "DX/RaytracingHelper.h"
#include "Utility/Debug.h"
#include "Math/MathUtility.h"

namespace {
#pragma optimize("",off)
    template <class T>
    inline auto createLocalRootSignatureIfExist(CD3DX12_STATE_OBJECT_DESC* pipeline, const T& ShaderData) {
        if (!ShaderData.local.useLocal) return;
        auto subobject = pipeline->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        ShaderData.local.localRootSignature->setLocalRootSignature(subobject);

        auto asso = pipeline->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        asso->SetSubobjectToAssociate(*subobject);
        asso->AddExport(ShaderData.name.c_str());
    };
#pragma optimize("",on)
}

namespace Framework::DX {

#pragma region IGNORE
    //RaytracingShader::RaytracingShader(DXRInterface* dxrInterface, const RaytracingShaderData& shaderData) {
    //    //auto settingDxilLibrary = [&](CD3DX12_STATE_OBJECT_DESC* pipeline) {
    //    //    //シェーダーのエントリポイントの設定を行う
    //    //    auto lib = pipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    //    //    D3D12_SHADER_BYTECODE libDxil = CD3DX12_SHADER_BYTECODE(shaderData.shaderFile, shaderData.shaderFileSize);
    //    //    lib->SetDXILLibrary(&libDxil);

    //    //    //実際に使用するシェーダーの名前を定義する
    //    //    lib->DefineExport(shaderData.rayGenShader.name.c_str());
    //    //    for (auto&& miss : shaderData.missShaders) {
    //    //        lib->DefineExport(miss.name.c_str());
    //    //    }
    //    //    for (auto&& hitGroup : shaderData.hitGroups) {
    //    //        if (!hitGroup.closestHit.empty())lib->DefineExport(hitGroup.closestHit.c_str());
    //    //        if (!hitGroup.anyHit.empty())lib->DefineExport(hitGroup.anyHit.c_str());
    //    //        if (!hitGroup.intersection.empty())lib->DefineExport(hitGroup.intersection.c_str());
    //    //    }
    //    //};

    //    //auto getHitGroupType = [](HitGroupType type) {
    //    //    switch (type) {
    //    //        case Framework::DX::HitGroupType::Triangle:
    //    //            return D3D12_HIT_GROUP_TYPE::D3D12_HIT_GROUP_TYPE_TRIANGLES;
    //    //        default:
    //    //            return D3D12_HIT_GROUP_TYPE::D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;
    //    //    }
    //    //};

    //    //auto bindHitGroup = [&](CD3DX12_STATE_OBJECT_DESC* pipeline, const HitGroupShader& hitGroupShader) {
    //    //    auto hitGroup = pipeline->CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    //    //    if (!hitGroupShader.closestHit.empty())hitGroup->SetClosestHitShaderImport(hitGroupShader.closestHit.c_str());
    //    //    if (!hitGroupShader.anyHit.empty())hitGroup->SetAnyHitShaderImport(hitGroupShader.anyHit.c_str());
    //    //    if (!hitGroupShader.intersection.empty())hitGroup->SetIntersectionShaderImport(hitGroupShader.intersection.c_str());
    //    //    //HitGroupをエクスポートする
    //    //    hitGroup->SetHitGroupExport(hitGroupShader.name.c_str());

    //    //    hitGroup->SetHitGroupType(getHitGroupType(hitGroupShader.type));
    //    //};

    //    //auto setConfig = [](CD3DX12_STATE_OBJECT_DESC* pipeline, const ShaderConfig& shaderConfig) {
    //    //    //シェーダー設定
    //    //    //サイズは使用するオブジェクトの最大値を利用する
    //    //    auto* config = pipeline->CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    //    //    UINT payloadSize = shaderConfig.maxPayloadByteSize;
    //    //    UINT attrSize = shaderConfig.maxAttributeByteSize;
    //    //    config->Config(payloadSize, attrSize);

    //    //};


    //    ////RTPSOの作成
    //    //CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE::D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };
    //    //auto* pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    //    //pipelineConfig->Config(shaderData.maxRecursionNum);

    //    //MY_ASSERTION(shaderData.globalRootSignature, L"グローバルルートシグネチャが設定されていません");
    //    ////グローバルルートシグネチャを設定する
    //    //auto* global = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    //    //shaderData.globalRootSignature->setGlobalRootSignature(global);

    //    //settingDxilLibrary(&raytracingPipeline);

    //    ////HitGroupをまとめる
    //    //for (auto&& hitGroup : shaderData.hitGroups) {
    //    //    bindHitGroup(&raytracingPipeline, hitGroup);
    //    //}

    //    //setConfig(&raytracingPipeline, shaderData.shaderConfig);

    //    //createLocalRootSignatureIfExist(&raytracingPipeline, shaderData.rayGenShader);
    //    //for (auto&& miss : shaderData.missShaders) {
    //    //    createLocalRootSignatureIfExist(&raytracingPipeline, miss);
    //    //}
    //    //for (auto&& hitGroup : shaderData.hitGroups) {
    //    //    createLocalRootSignatureIfExist(&raytracingPipeline, hitGroup);
    //    //}


    //    //dxrInterface->createStateObject(raytracingPipeline);

    //    //ComPtr<ID3D12StateObjectProperties> props;
    //    //dxrInterface->getStateObject()->QueryInterface(IID_PPV_ARGS(&props));
    //    //UINT shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    //    //ID3D12Device* device = dxrInterface->getDXRDevice();
    //    //{
    //    //    UINT numShaderRecords = 1;
    //    //    UINT shaderRecordSize = shaderIDSize;
    //    //    if (shaderData.rayGenShader.local.useLocal)
    //    //        shaderRecordSize += shaderData.rayGenShader.local.LocalRootSignatureDataSize;

    //    //    void* rayGenShaderID = props->GetShaderIdentifier(shaderData.rayGenShader.name.c_str());
    //    //    ShaderTable table(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
    //    //    table.push_back(ShaderRecord(rayGenShaderID, shaderIDSize, shaderData.rayGenShader.local.LocalRootSignatureData, shaderData.rayGenShader.local.LocalRootSignatureDataSize));
    //    //    mRayGenShaderTable = table.getResource();
    //    //}
    //    //{
    //    //    UINT numShaderRecords = shaderData.missShaders.size();
    //    //    //シェーダーレコードの大きさは一定なので最大値に合わせる必要がある
    //    //    UINT shaderRecordSize = shaderIDSize;
    //    //    for (UINT i = 0; i < numShaderRecords; i++) {
    //    //        shaderRecordSize = Math::MathUtil::mymax(shaderIDSize + shaderData.missShaders[i].local.LocalRootSignatureDataSize, shaderRecordSize);
    //    //    }
    //    //    ShaderTable table(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");
    //    //    for (UINT i = 0; i < numShaderRecords; i++) {
    //    //        const MissShader& miss = shaderData.missShaders[i];
    //    //        void* id = props->GetShaderIdentifier(miss.name.c_str());
    //    //        table.push_back(ShaderRecord(id, shaderIDSize, miss.local.LocalRootSignatureData, miss.local.LocalRootSignatureDataSize));
    //    //    }
    //    //    mMissShaderStrideInBytes = table.getShaderRecordSize();
    //    //    mMissShaderTable = table.getResource();
    //    //}

    //    //{
    //    //    UINT numShaderRecords = shaderData.hitGroupOrder.order.size();
    //    //    UINT shaderRecordSize = shaderIDSize;
    //    //    for (auto&& hit : shaderData.hitGroups) {
    //    //        shaderRecordSize = Math::MathUtil::mymax(shaderIDSize + hit.local.LocalRootSignatureDataSize, shaderRecordSize);
    //    //    }
    //    //    auto getHitGroupByName = [](const std::wstring& name, const std::vector<HitGroupShader>& hitGroups) {
    //    //        return *std::find_if(hitGroups.begin(), hitGroups.end(),
    //    //            [&](auto& hit) {return hit.name == name; });
    //    //    };

    //    //    ShaderTable table(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");
    //    //    for (UINT i = 0; i < numShaderRecords; i++) {
    //    //        const HitGroupShader& hit = getHitGroupByName(shaderData.hitGroupOrder.order[i], shaderData.hitGroups);
    //    //        void* id = props->GetShaderIdentifier(hit.name.c_str());
    //    //        table.push_back(ShaderRecord(id, shaderIDSize, hit.local.LocalRootSignatureData, hit.local.LocalRootSignatureDataSize));
    //    //    }
    //    //    mHitGroupShaderStrideInBytes = table.getShaderRecordSize();
    //    //    mHitGroupShaderTable = table.getResource();
    //    //}
    //}
#pragma endregion
    RaytracingShader::RaytracingShader(DXRInterface * dxrInterface)
        :mDXRInterface(dxrInterface), mRaytracingPipeline({ D3D12_STATE_OBJECT_TYPE::D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE }) {

    }
    RaytracingShader::~RaytracingShader() { }

    void RaytracingShader::setGlobalRootSignature(RootSignature* globalRootSignature) {
        auto* global = mRaytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
        globalRootSignature->setGlobalRootSignature(global);
    }

    void RaytracingShader::setLocalRootSignature(const std::wstring& name, RootSignature* local) {
        auto subobject = mRaytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        local->setLocalRootSignature(subobject);

        auto asso = mRaytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        asso->SetSubobjectToAssociate(*subobject);
        asso->AddExport(name.c_str());
    }

    void RaytracingShader::loadShaderFiles(const ShaderFile& shaderFile) {
        //シェーダーのエントリポイントの設定を行う
        auto lib = mRaytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
        D3D12_SHADER_BYTECODE libDxil = CD3DX12_SHADER_BYTECODE(shaderFile.shaderFile, shaderFile.shaderFileSize);
        lib->SetDXILLibrary(&libDxil);

        for (auto&& name : shaderFile.entryPoints) {
            lib->DefineExport(name.c_str());
        }
    }

    void RaytracingShader::bindHitGroup(const HitGroup& hitGroup) {
        auto getHitGroupType = [](HitGroupType type) {
            switch (type) {
                case Framework::DX::HitGroupType::Triangle:
                    return D3D12_HIT_GROUP_TYPE::D3D12_HIT_GROUP_TYPE_TRIANGLES;
                default:
                    return D3D12_HIT_GROUP_TYPE::D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;
            }
        };

        auto hitGroupObj = mRaytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
        if (!hitGroup.closestHit.empty())hitGroupObj->SetClosestHitShaderImport(hitGroup.closestHit.c_str());
        if (!hitGroup.anyHit.empty())hitGroupObj->SetAnyHitShaderImport(hitGroup.anyHit.c_str());
        if (!hitGroup.intersection.empty())hitGroupObj->SetIntersectionShaderImport(hitGroup.intersection.c_str());
        //HitGroupをエクスポートする
        hitGroupObj->SetHitGroupExport(hitGroup.name.c_str());

        hitGroupObj->SetHitGroupType(getHitGroupType(hitGroup.type));
    }

    void RaytracingShader::setConfig(const ShaderConfig& config) {
        //シェーダー設定
        //サイズは使用するオブジェクトの最大値を利用する
        auto* configObj = mRaytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
        UINT payloadSize = config.maxPayloadByteSize;
        UINT attrSize = config.maxAttributeByteSize;
        configObj->Config(payloadSize, attrSize);

        auto* pipelineConfig = mRaytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
        pipelineConfig->Config(config.maxRecursionNum);

    }

    void RaytracingShader::buildPipeline() {
        mDXRInterface->createStateObject(mRaytracingPipeline);
    }

    void RaytracingShader::rayGenerationShader(const RayGenShader& rayGenShader) {
        ComPtr<ID3D12StateObjectProperties> props;
        mDXRInterface->getStateObject()->QueryInterface(IID_PPV_ARGS(&props));
        UINT shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        ID3D12Device* device = mDXRInterface->getDXRDevice();
        {
            UINT numShaderRecords = 1;
            UINT shaderRecordSize = shaderIDSize;
            void* constants = nullptr;
            UINT constantsSize = 0;
            if (rayGenShader.localRootSignature) {
                shaderRecordSize += rayGenShader.localRootSignature->localConstantsSize;
                constants = rayGenShader.localRootSignature->localConstants;
                constantsSize = rayGenShader.localRootSignature->localConstantsSize;
            }

            void* rayGenShaderID = props->GetShaderIdentifier(rayGenShader.name.c_str());
            ShaderTable table(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
            table.push_back(ShaderRecord(rayGenShaderID, shaderIDSize, constants, constantsSize));
            mRayGenShaderTable = table.getResource();
        }
    }

    void RaytracingShader::missShader(const MissShaderList& missShader) {
        ComPtr<ID3D12StateObjectProperties> props;
        mDXRInterface->getStateObject()->QueryInterface(IID_PPV_ARGS(&props));
        UINT shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        ID3D12Device* device = mDXRInterface->getDXRDevice();

        UINT numShaderRecords = missShader.size();
        UINT shaderRecordSize = shaderIDSize;
        //シェーダーレコードの大きさは一定なので最大値に合わせる必要がある
        for (UINT i = 0; i < numShaderRecords; i++) {
            if (missShader[i].localRootSignature) {
                shaderRecordSize = Math::MathUtil::mymax(shaderIDSize + missShader[i].localRootSignature->localConstantsSize, shaderRecordSize);
            }
        }

        ShaderTable table(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");
        for (UINT i = 0; i < numShaderRecords; i++) {
            const MissShader& miss = missShader[i];
            void* id = props->GetShaderIdentifier(miss.name.c_str());
            void* constants = nullptr;
            UINT constantsSize = 0;
            if (miss.localRootSignature) {
                constants = miss.localRootSignature->localConstants;
                constantsSize = miss.localRootSignature->localConstantsSize;
            }
            table.push_back(ShaderRecord(id, shaderIDSize, constants, constantsSize));
        }
        mMissShaderStrideInBytes = table.getShaderRecordSize();
        mMissShaderTable = table.getResource();

    }

    void RaytracingShader::hitGroup(const HitGroupShaderList& hitGroupList, const HitGroupShaderIndex& indices) {
        ComPtr<ID3D12StateObjectProperties> props;
        mDXRInterface->getStateObject()->QueryInterface(IID_PPV_ARGS(&props));
        UINT shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        ID3D12Device* device = mDXRInterface->getDXRDevice();

        UINT numShaderRecords = indices.size();
        UINT shaderRecordSize = shaderIDSize;
        for (auto&& hit : hitGroupList) {
            if (hit.localRootSignature)
                shaderRecordSize = Math::MathUtil::mymax(shaderIDSize + hit.localRootSignature->localConstantsSize, shaderRecordSize);
        }
        auto getHitGroupByName = [](const std::wstring& name, const HitGroupShaderList& hitGroups) {
            return *std::find_if(hitGroups.begin(), hitGroups.end(),
                [&](auto& hit) {return hit.name == name; });
        };

        ShaderTable table(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");
        for (UINT i = 0; i < numShaderRecords; i++) {
            const HitGroup& hit = hitGroupList[indices[i]];
            void* id = props->GetShaderIdentifier(hit.name.c_str());
            void* constants = nullptr;
            UINT constantsSize = 0;
            if (hit.localRootSignature) {
                constants = hit.localRootSignature->localConstants;
                constantsSize = hit.localRootSignature->localConstantsSize;
            }
            table.push_back(ShaderRecord(id, shaderIDSize, constants, constantsSize));
        }
        mHitGroupShaderStrideInBytes = table.getShaderRecordSize();
        mHitGroupShaderTable = table.getResource();
    }

    void RaytracingShader::doRaytracing(UINT width, UINT height) {
        ID3D12GraphicsCommandList5* commandList = mDXRInterface->getCommandList();
        ID3D12StateObject* state = mDXRInterface->getStateObject();

        D3D12_DISPATCH_RAYS_DESC desc = {};
        desc.HitGroupTable.StartAddress = mHitGroupShaderTable->GetGPUVirtualAddress();
        desc.HitGroupTable.SizeInBytes = mHitGroupShaderTable->GetDesc().Width;
        desc.HitGroupTable.StrideInBytes = mHitGroupShaderStrideInBytes;

        desc.MissShaderTable.StartAddress = mMissShaderTable->GetGPUVirtualAddress();
        desc.MissShaderTable.SizeInBytes = mMissShaderTable->GetDesc().Width;
        desc.MissShaderTable.StrideInBytes = mMissShaderStrideInBytes;

        desc.RayGenerationShaderRecord.StartAddress = mRayGenShaderTable->GetGPUVirtualAddress();
        desc.RayGenerationShaderRecord.SizeInBytes = mRayGenShaderTable->GetDesc().Width;

        desc.Width = width;
        desc.Height = height;
        desc.Depth = 1;

        commandList->SetPipelineState1(state);

        commandList->DispatchRays(&desc);
    }

} //Framework::DX 
