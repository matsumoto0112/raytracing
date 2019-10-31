#pragma once
#include <string>
#include <vector>
#include <d3d12.h>
#include "DX/RootSignature.h"
#include "Utility/Typedef.h"
#include "DX/DXRInterface.h"

namespace Framework::DX {
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
        bool use;
        void* localConstants;
        UINT localConstantsSize;
        RootSignature* rootSignature;
        LocalRootSignature()
            :use(false), localConstants(nullptr), localConstantsSize(0), rootSignature(nullptr) { }
        template <class T>
        LocalRootSignature(T* constants, RootSignature* rootSignature)
            : use(true), localConstants(constants), localConstantsSize(sizeof(T)), rootSignature(rootSignature) { }
        LocalRootSignature(void* constants, UINT constantsSize, RootSignature* rootSignature)
            :use(true), localConstants(constants), localConstantsSize(constantsSize), rootSignature(rootSignature) { }
    };

    struct RayGenShader {
        std::wstring name;
        LocalRootSignature localRootSignature;

        RayGenShader(const std::wstring& name, LocalRootSignature rootSignature = {})
            :name(name), localRootSignature(rootSignature) { }
    };

    struct MissShader {
        std::wstring name;
        LocalRootSignature localRootSignature;
        MissShader()
            :name(), localRootSignature({}) { }

        MissShader(const std::wstring& name, LocalRootSignature rootSignature = {})
            :name(name), localRootSignature(rootSignature) { }
    };

    struct HitGroup {
        std::wstring name;
        std::wstring closestHit;
        std::wstring anyHit;
        std::wstring intersection;
        LocalRootSignature localRootSignature;
        HitGroupType type;
        HitGroup()
            :name(), closestHit(), anyHit(), intersection(), localRootSignature({}), type(HitGroupType::Triangle) { }

        HitGroup(const std::wstring& name, HitGroupType type)
            :name(name), closestHit(), anyHit(), intersection(), localRootSignature({}), type(type) { }
    };
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

        void printOut();
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