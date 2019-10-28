#include "RaytracingShader.h"
#include <algorithm>
#include "DX/RaytracingHelper.h"
#include "Utility/Debug.h"
#include "Math/MathUtility.h"

namespace {
#pragma optimize("",off)
    template <class T>
    inline auto createLocalRootSignatureIfExist(CD3DX12_STATE_OBJECT_DESC* pipeline, const T& ShaderData) {
        //OutputDebugStringA(ShaderData.local ? "OK\n" : "NO\n");
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

    RaytracingShader::RaytracingShader(DXRInterface* dxrInterface, const RaytracingShaderData& shaderData) {
        auto settingDxilLibrary = [&](CD3DX12_STATE_OBJECT_DESC* pipeline) {
            //シェーダーのエントリポイントの設定を行う
            auto lib = pipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
            D3D12_SHADER_BYTECODE libDxil = CD3DX12_SHADER_BYTECODE(shaderData.shaderFile, shaderData.shaderFileSize);
            lib->SetDXILLibrary(&libDxil);

            //実際に使用するシェーダーの名前を定義する
            lib->DefineExport(shaderData.rayGenShader.name.c_str());
            for (auto&& miss : shaderData.missShaders) {
                lib->DefineExport(miss.name.c_str());
            }
            for (auto&& hitGroup : shaderData.hitGroups) {
                if (!hitGroup.closestHit.empty())lib->DefineExport(hitGroup.closestHit.c_str());
                if (!hitGroup.anyHit.empty())lib->DefineExport(hitGroup.anyHit.c_str());
                if (!hitGroup.intersection.empty())lib->DefineExport(hitGroup.intersection.c_str());
            }
        };

        auto getHitGroupType = [](HitGroupType type) {
            switch (type) {
                case Framework::DX::HitGroupType::Triangle:
                    return D3D12_HIT_GROUP_TYPE::D3D12_HIT_GROUP_TYPE_TRIANGLES;
                default:
                    return D3D12_HIT_GROUP_TYPE::D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;
            }
        };

        auto bindHitGroup = [&](CD3DX12_STATE_OBJECT_DESC* pipeline, const HitGroupShader& hitGroupShader) {
            auto hitGroup = pipeline->CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
            if (!hitGroupShader.closestHit.empty())hitGroup->SetClosestHitShaderImport(hitGroupShader.closestHit.c_str());
            if (!hitGroupShader.anyHit.empty())hitGroup->SetAnyHitShaderImport(hitGroupShader.anyHit.c_str());
            if (!hitGroupShader.intersection.empty())hitGroup->SetIntersectionShaderImport(hitGroupShader.intersection.c_str());
            //HitGroupをエクスポートする
            hitGroup->SetHitGroupExport(hitGroupShader.name.c_str());

            hitGroup->SetHitGroupType(getHitGroupType(hitGroupShader.type));
        };

        auto setConfig = [](CD3DX12_STATE_OBJECT_DESC* pipeline, const ShaderConfig& shaderConfig) {
            //シェーダー設定
            //サイズは使用するオブジェクトの最大値を利用する
            auto* config = pipeline->CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
            UINT payloadSize = shaderConfig.maxPayloadByteSize;
            UINT attrSize = shaderConfig.maxAttributeByteSize;
            config->Config(payloadSize, attrSize);

        };


        //RTPSOの作成
        CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE::D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

        //最初にDXILライブラリの設定を行う
        settingDxilLibrary(&raytracingPipeline);

        //HitGroupをまとめる
        for (auto&& hitGroup : shaderData.hitGroups) {
            bindHitGroup(&raytracingPipeline, hitGroup);
        }

        setConfig(&raytracingPipeline, shaderData.shaderConfig);

        createLocalRootSignatureIfExist(&raytracingPipeline, shaderData.rayGenShader);
        for (auto&& miss : shaderData.missShaders) {
            createLocalRootSignatureIfExist(&raytracingPipeline, miss);
        }
        for (auto&& hitGroup : shaderData.hitGroups) {
            createLocalRootSignatureIfExist(&raytracingPipeline, hitGroup);
        }

        MY_ASSERTION(shaderData.globalRootSignature, L"グローバルルートシグネチャが設定されていません");
        //グローバルルートシグネチャを設定する
        auto* global = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
        shaderData.globalRootSignature->setGlobalRootSignature(global);

        auto* pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
        pipelineConfig->Config(shaderData.maxRecursionNum);

        dxrInterface->createStateObject(raytracingPipeline);

        ComPtr<ID3D12StateObjectProperties> props;
        dxrInterface->getStateObject()->QueryInterface(IID_PPV_ARGS(&props));
        UINT shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        ID3D12Device* device = dxrInterface->getDXRDevice();
        {
            UINT numShaderRecords = 1;
            UINT shaderRecordSize = shaderIDSize;
            if (shaderData.rayGenShader.local.useLocal)
                shaderRecordSize += shaderData.rayGenShader.local.LocalRootSignatureDataSize;

            void* rayGenShaderID = props->GetShaderIdentifier(shaderData.rayGenShader.name.c_str());
            ShaderTable table(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
            table.push_back(ShaderRecord(rayGenShaderID, shaderIDSize, shaderData.rayGenShader.local.LocalRootSignatureData, shaderData.rayGenShader.local.LocalRootSignatureDataSize));
            mRayGenShaderTable = table.getResource();
        }
        {
            UINT numShaderRecords = shaderData.missShaders.size();
            //シェーダーレコードの大きさは一定なので最大値に合わせる必要がある
            UINT shaderRecordSize = shaderIDSize;
            for (UINT i = 0; i < numShaderRecords; i++) {
                shaderRecordSize = Math::MathUtil::mymax(shaderIDSize + shaderData.missShaders[i].local.LocalRootSignatureDataSize, shaderRecordSize);
            }
            ShaderTable table(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");
            for (UINT i = 0; i < numShaderRecords; i++) {
                const MissShader& miss = shaderData.missShaders[i];
                void* id = props->GetShaderIdentifier(miss.name.c_str());
                table.push_back(ShaderRecord(id, shaderIDSize, miss.local.LocalRootSignatureData, miss.local.LocalRootSignatureDataSize));
            }
            mMissShaderStrideInBytes = table.getShaderRecordSize();
            mMissShaderTable = table.getResource();
        }

        {
            UINT numShaderRecords = shaderData.hitGroupOrder.order.size();
            UINT shaderRecordSize = shaderIDSize;
            for (auto&& hit : shaderData.hitGroups) {
                shaderRecordSize = Math::MathUtil::mymax(shaderIDSize + hit.local.LocalRootSignatureDataSize, shaderRecordSize);
            }
            auto getHitGroupByName = [](const std::wstring& name, const std::vector<HitGroupShader>& hitGroups) {
                return *std::find_if(hitGroups.begin(), hitGroups.end(),
                    [&](auto& hit) {return hit.name == name; });
            };

            ShaderTable table(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");
            for (UINT i = 0; i < numShaderRecords; i++) {
                const HitGroupShader& hit = getHitGroupByName(shaderData.hitGroupOrder.order[i], shaderData.hitGroups);
                void* id = props->GetShaderIdentifier(hit.name.c_str());
                table.push_back(ShaderRecord(id, shaderIDSize, hit.local.LocalRootSignatureData, hit.local.LocalRootSignatureDataSize));
            }
            mHitGroupShaderStrideInBytes = table.getShaderRecordSize();
            mHitGroupShaderTable = table.getResource();
        }
    }
    RaytracingShader::~RaytracingShader() { }

    void RaytracingShader::doRaytracing(ID3D12GraphicsCommandList5* list, ID3D12StateObject* state, UINT width, UINT height) {
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

        list->SetPipelineState1(state);

        list->DispatchRays(&desc);
    }

} //Framework::DX 
