#pragma once
#include <string>
#include <vector>
#include <d3d12.h>
#include "DX/RootSignature.h"
#include "Utility/Typedef.h"
#include "DX/DXRInterface.h"

namespace Framework::DX {
    struct LocalRootSignature {
        bool useLocal;
        UINT LocalRootSignatureDataSize;
        void* LocalRootSignatureData;
        RootSignature* localRootSignature;
        LocalRootSignature(UINT size, void* data, RootSignature* localRootSignature)
            :LocalRootSignatureDataSize(size), LocalRootSignatureData(data), localRootSignature(localRootSignature), useLocal(true) { }
        LocalRootSignature()
            :LocalRootSignatureDataSize(0), LocalRootSignatureData(nullptr), localRootSignature(nullptr), useLocal(false) { }
    };

    struct RayGenShaderData {
        std::wstring name;
        LocalRootSignature local;

        RayGenShaderData(const std::wstring& name, LocalRootSignature local = {})
            :name(name), local(local) { }
    };

    struct MissShader {
        std::wstring name;
        LocalRootSignature local;

        MissShader(const std::wstring& name, LocalRootSignature local = {})
            :name(name), local(local) { }
    };

    enum class HitGroupType {
        Triangle = 0,
        Procedural,
    };

    struct HitGroupShader {
        std::wstring name;
        LocalRootSignature local;
        std::wstring closestHit;
        std::wstring anyHit;
        std::wstring intersection;
        HitGroupType type;

        HitGroupShader(const std::wstring& name, HitGroupType type = HitGroupType::Triangle)
            :name(name), local(), closestHit(), anyHit(), intersection(), type(type) { }
    };

    struct HitGroupShaderTableOrder {
        std::vector<std::reference_wrapper<const std::wstring>> order;
        HitGroupShaderTableOrder(const std::initializer_list<std::reference_wrapper<const std::wstring>>& names = {})
            :order(names) { }
    };

    struct ShaderConfig {
        UINT maxPayloadByteSize;
        UINT maxAttributeByteSize;

        ShaderConfig(UINT maxPayloadByteSize, UINT maxAttributeByteSize)
            :maxPayloadByteSize(maxPayloadByteSize), maxAttributeByteSize(maxAttributeByteSize) { }
    };

    struct RaytracingShaderData {
        void* shaderFile;
        UINT shaderFileSize;
        RayGenShaderData rayGenShader; //!< レイ生成関数
        std::vector<MissShader> missShaders;
        std::vector<HitGroupShader> hitGroups;
        HitGroupShaderTableOrder hitGroupOrder;
        ShaderConfig shaderConfig;
        RootSignature* globalRootSignature;
        UINT maxRecursionNum;

        RaytracingShaderData(void* shaderFile, UINT shaderSize,
            const RayGenShaderData& rayGenShader, RootSignature* global,
            ShaderConfig config, UINT maxRecursionNum) :shaderFile(shaderFile), shaderFileSize(shaderSize),
            rayGenShader(rayGenShader), missShaders(), hitGroups(), hitGroupOrder(), shaderConfig(config),
            globalRootSignature(global), maxRecursionNum(maxRecursionNum) { }
    };

    /**
    * @class RaytracingShader
    * @brief レイトレーシングシェーダー構築
    */
    class RaytracingShader {
    public:
        /**
        * @brief コンストラクタ
        */
        RaytracingShader(DXRInterface* dxrInterface, const RaytracingShaderData& shaderData);
        /**
        * @brief デストラクタ
        */
        ~RaytracingShader();

        void doRaytracing(ID3D12GraphicsCommandList5* list, ID3D12StateObject* state, UINT width, UINT height);
    //private:
        ComPtr<ID3D12Resource> mRayGenShaderTable;
        ComPtr<ID3D12Resource> mMissShaderTable;
        UINT mMissShaderStrideInBytes;
        ComPtr<ID3D12Resource> mHitGroupShaderTable;
        UINT mHitGroupShaderStrideInBytes;
    };
} //Framework::DX 