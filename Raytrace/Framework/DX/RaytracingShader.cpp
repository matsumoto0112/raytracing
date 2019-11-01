#include "RaytracingShader.h"
#include <algorithm>
#include "DX/RaytracingHelper.h"
#include "Utility/Debug.h"
#include "Math/MathUtility.h"
#include "Utility/GPUTimer.h"

namespace Framework::DX {

    RaytracingShader::RaytracingShader(DXRInterface* dxrInterface)
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
        if (!mProps)
            MY_THROW_IF_FAILED(mDXRInterface->getStateObject()->QueryInterface(IID_PPV_ARGS(&mProps)), L"QueryInterface失敗");
        UINT shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        ID3D12Device* device = mDXRInterface->getDXRDevice();
        {
            UINT numShaderRecords = 1;
            UINT shaderRecordSize = shaderIDSize;
            void* constants = nullptr;
            UINT constantsSize = 0;
            if (rayGenShader.localRootSignature.use) {
                shaderRecordSize += rayGenShader.localRootSignature.localConstantsSize;
                constants = rayGenShader.localRootSignature.localConstants;
                constantsSize = rayGenShader.localRootSignature.localConstantsSize;
            }

            void* rayGenShaderID = mProps->GetShaderIdentifier(rayGenShader.name.c_str());
            ShaderTable table(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
            table.push_back(ShaderRecord(rayGenShaderID, shaderIDSize, constants, constantsSize));
            mRayGenShaderTable = table.getResource();
        }
    }

    void RaytracingShader::missShader(const MissShaderList& missShader) {
        if (!mProps)
            MY_THROW_IF_FAILED(mDXRInterface->getStateObject()->QueryInterface(IID_PPV_ARGS(&mProps)), L"QueryInterface失敗");
        UINT shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        ID3D12Device* device = mDXRInterface->getDXRDevice();

        UINT numShaderRecords = (UINT)missShader.size();
        UINT shaderRecordSize = shaderIDSize;
        //シェーダーレコードの大きさは一定なので最大値に合わせる必要がある
        for (UINT i = 0; i < numShaderRecords; i++) {
            if (missShader[i].localRootSignature.use) {
                shaderRecordSize = Math::MathUtil::mymax(shaderIDSize + missShader[i].localRootSignature.localConstantsSize, shaderRecordSize);
            }
        }

        ShaderTable table(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");
        for (UINT i = 0; i < numShaderRecords; i++) {
            const MissShader& miss = missShader[i];
            void* id = mProps->GetShaderIdentifier(miss.name.c_str());
            void* constants = nullptr;
            UINT constantsSize = 0;
            if (miss.localRootSignature.use) {
                constants = miss.localRootSignature.localConstants;
                constantsSize = miss.localRootSignature.localConstantsSize;
            }
            table.push_back(ShaderRecord(id, shaderIDSize, constants, constantsSize));
        }
        mMissShaderStrideInBytes = table.getShaderRecordSize();
        mMissShaderTable = table.getResource();
        mMissShaderTable->SetName(L"MissShaderTable");
    }

    void RaytracingShader::hitGroup(const HitGroupShaderList& hitGroupList, const HitGroupShaderIndex& indices) {
        if (!mProps)
            MY_THROW_IF_FAILED(mDXRInterface->getStateObject()->QueryInterface(IID_PPV_ARGS(&mProps)), L"QueryInterface失敗");
        UINT shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        ID3D12Device* device = mDXRInterface->getDXRDevice();

        UINT numShaderRecords = (UINT)indices.size();
        UINT shaderRecordSize = shaderIDSize;
        for (auto&& hit : hitGroupList) {
            if (hit.localRootSignature.use)
                shaderRecordSize = Math::MathUtil::mymax(shaderIDSize + hit.localRootSignature.localConstantsSize, shaderRecordSize);
        }

        ShaderTable table(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");
        for (UINT i = 0; i < numShaderRecords; i++) {
            const HitGroup& hit = hitGroupList[indices[i]];
            void* id = mProps->GetShaderIdentifier(hit.name.c_str());
            void* constants = nullptr;
            UINT constantsSize = 0;
            if (hit.localRootSignature.use) {
                constants = hit.localRootSignature.localConstants;
                constantsSize = hit.localRootSignature.localConstantsSize;
            }
            table.push_back(ShaderRecord(id, shaderIDSize, constants, constantsSize));
        }
        mHitGroupShaderStrideInBytes = table.getShaderRecordSize();
        mHitGroupShaderTable = table.getResource();
        mHitGroupShaderTable->SetName(L"HitGroupShaderTable");
    }

    void RaytracingShader::doRaytracing(UINT width, UINT height, Utility::GPUTimer* gpuTimer) {
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
        if (gpuTimer) {
            gpuTimer->start(commandList);
        }
        commandList->DispatchRays(&desc);
        if (gpuTimer) {
            gpuTimer->stop(commandList);
        }
    }

    void RaytracingShader::printOut() {
        printStateObjectDesc(mRaytracingPipeline);
    }

} //Framework::DX 
