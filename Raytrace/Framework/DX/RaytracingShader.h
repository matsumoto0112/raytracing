#pragma once
#include <string>
#include <vector>
#include <d3d12.h>
#include "DX/RootSignature.h"
#include "Utility/Typedef.h"
#include "DX/DXRInterface.h"

namespace Framework::DX {
    //struct LocalRootSignature {
    //    bool useLocal;
    //    UINT LocalRootSignatureDataSize;
    //    void* LocalRootSignatureData;
    //    RootSignature* localRootSignature;
    //    LocalRootSignature(UINT size, void* data, RootSignature* localRootSignature)
    //        :LocalRootSignatureDataSize(size), LocalRootSignatureData(data), localRootSignature(localRootSignature), useLocal(true) { }
    //    LocalRootSignature()
    //        :LocalRootSignatureDataSize(0), LocalRootSignatureData(nullptr), localRootSignature(nullptr), useLocal(false) { }
    //};


    //struct RayGenShaderData {
    //    std::wstring name;
    //    LocalRootSignature local;

    //    RayGenShaderData(const std::wstring& name, LocalRootSignature local = {})
    //        :name(name), local(local) { }
    //};

    //struct MissShader {
    //    std::wstring name;
    //    LocalRootSignature local;

    //    MissShader(const std::wstring& name, LocalRootSignature local = {})
    //        :name(name), local(local) { }
    //};


    //struct HitGroupShader {
    //    std::wstring name;
    //    LocalRootSignature local;
    //    std::wstring closestHit;
    //    std::wstring anyHit;
    //    std::wstring intersection;
    //    HitGroupType type;

    //    HitGroupShader(const std::wstring& name, HitGroupType type = HitGroupType::Triangle)
    //        :name(name), local(), closestHit(), anyHit(), intersection(), type(type) { }
    //};

    //struct HitGroupShaderTableOrder {
    //    std::vector<std::reference_wrapper<const std::wstring>> order;
    //    HitGroupShaderTableOrder(const std::initializer_list<std::reference_wrapper<const std::wstring>>& names = {})
    //        :order(names) { }
    //};

    enum class HitGroupType {
        Triangle = 0,
        Procedural,
    };


    struct ShaderFile {
        void* shaderFile;
        UINT shaderFileSize;
        std::vector<std::wstring> entryPoints;
    };

    struct ShaderConfig {
        UINT maxPayloadByteSize;
        UINT maxAttributeByteSize;
        UINT maxRecursionNum;

        ShaderConfig(UINT maxPayloadByteSize, UINT maxAttributeByteSize, UINT maxRecursionNum)
            :maxPayloadByteSize(maxPayloadByteSize), maxAttributeByteSize(maxAttributeByteSize), maxRecursionNum(maxRecursionNum) { }
    };

    struct LocalRootSignature {
        void* localConstants;
        UINT localConstantsSize;
        RootSignature* rootSignature;
        LocalRootSignature()
            :localConstants(nullptr), localConstantsSize(0), rootSignature(nullptr) { }
        template <class T>
        LocalRootSignature(T* constants, RootSignature* rootSignature)
            : localConstants(constants), localConstantsSize(sizeof(T)), rootSignature(rootSignature) { }
        LocalRootSignature(void* constants, UINT constantsSize, RootSignature* rootSignature)
            :localConstants(constants), localConstantsSize(constantsSize), rootSignature(rootSignature) { }
    };

    struct RayGenShader {
        std::wstring name;
        LocalRootSignature* localRootSignature;

        RayGenShader(const std::wstring& name, LocalRootSignature* rootSignature = nullptr)
            :name(name), localRootSignature(rootSignature) { }
    };

    struct MissShader {
        std::wstring name;
        LocalRootSignature* localRootSignature;
        MissShader()
            :name(), localRootSignature(nullptr) { }

        MissShader(const std::wstring& name, LocalRootSignature* rootSignature = nullptr)
            :name(name), localRootSignature(rootSignature) { }
    };

    struct HitGroup {
        std::wstring name;
        std::wstring closestHit;
        std::wstring anyHit;
        std::wstring intersection;
        LocalRootSignature* localRootSignature;
        HitGroupType type;
        HitGroup()
            :name(), closestHit(), anyHit(), intersection(), localRootSignature(), type(HitGroupType::Triangle) { }

        HitGroup(const std::wstring& name, HitGroupType type)
            :name(name), closestHit(), anyHit(), intersection(), localRootSignature(nullptr), type(type) { }
    };

        //struct RaytracingShaderData {
        //    void* shaderFile;
        //    UINT shaderFileSize;
        //    RayGenShaderData rayGenShader; //!< レイ生成関数
        //    std::vector<MissShader> missShaders;
        //    std::vector<HitGroupShader> hitGroups;
        //    HitGroupShaderTableOrder hitGroupOrder;
        //    ShaderConfig shaderConfig;
        //    RootSignature* globalRootSignature;
        //    UINT maxRecursionNum;

        //    RaytracingShaderData(void* shaderFile, UINT shaderSize,
        //        const RayGenShaderData& rayGenShader, RootSignature* global,
        //        ShaderConfig config, UINT maxRecursionNum) :shaderFile(shaderFile), shaderFileSize(shaderSize),
        //        rayGenShader(rayGenShader), missShaders(), hitGroups(), hitGroupOrder(), shaderConfig(config),
        //        globalRootSignature(global), maxRecursionNum(maxRecursionNum) { }
        //};

        /**
        * @class RaytracingShader
        * @brief レイトレーシングシェーダー構築
        */
    class RaytracingShader {
        using MissShaderList = std::vector<MissShader>;
        using HitGroupShaderList = std::vector<HitGroup>;
        using HitGroupShaderIndex = std::vector<UINT>;
    public:
        ///**
        //* @brief コンストラクタ
        //*/
        //RaytracingShader(DXRInterface* dxrInterface, const RaytracingShaderData& shaderData);
        RaytracingShader(DXRInterface* dxrInterface);
        /**
        * @brief デストラクタ
        */
        ~RaytracingShader();
        void setGlobalRootSignature(RootSignature* globalRootSignature);

        void setLocalRootSignature(const std::wstring& name, RootSignature* localRootSignature);

        void loadShaderFiles(const ShaderFile& shaderFile);

        void bindHitGroup(const HitGroup& hitGroup);

        void setConfig(const ShaderConfig& config);

        void buildPipeline();

        void rayGenerationShader(const RayGenShader& rayGenShader);
        void missShader(const MissShaderList& missShader);
        void hitGroup(const HitGroupShaderList& hitGroupList, const HitGroupShaderIndex& indices);

        void doRaytracing(UINT width, UINT height);
    private:
        DXRInterface* mDXRInterface;
        CD3DX12_STATE_OBJECT_DESC mRaytracingPipeline;

        ComPtr<ID3D12Resource> mRayGenShaderTable;
        ComPtr<ID3D12Resource> mMissShaderTable;
        UINT mMissShaderStrideInBytes;
        ComPtr<ID3D12Resource> mHitGroupShaderTable;
        UINT mHitGroupShaderStrideInBytes;
    };
} //Framework::DX 