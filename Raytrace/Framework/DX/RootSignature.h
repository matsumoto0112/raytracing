#pragma once
#include <vector>
#include <d3d12.h>
#include "Libs/d3dx12.h"
#include "Utility/Typedef.h"

namespace Framework::DX {
    /**
    * @namespace ShaderVisibility
    * @brief シェーダーの可視性定義
    */
    namespace ShaderVisibility {
        /**
        * @enum Enum
        */
        enum Enum {
            ALL = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL,
            Vertex = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX,
            Hull = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_HULL,
            Domain = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_DOMAIN,
            Geometry = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_GEOMETRY,
            Pixel = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL
        };
    } //ShaderVisibility 

    /**
    * @enum RootParameterType
    * @brief ルートパラメータの種類
    */
    enum class RootParameterType {
        DescriptorTable,
        SRV,
        UAV,
        CBV,
        Constants,
    };

    /**
    * @namespace DescriptorRangeType
    * @brief ディスクリプタレンジの種類
    */
    namespace DescriptorRangeType {
        /**
        * @enum Enum
        */
        enum Enum {
            SRV,
            UAV,
            CBV,
            Sampler,
        };
    } //DescriptorRangeType 

    namespace RootSignatureFlag {
        enum Enum {
            GlobalRootSignature = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_NONE,
            LocalRootSignature = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE,
        };
    } //RootSignatureType 

    /**
    * @brief ディスクリプタレンジ
    */
    struct DescriptorRange {
        DescriptorRangeType::Enum rangeType; //!< レンジの種類
        UINT DescriptorNum; //!< 割り当てるディスクリプタヒープの数
        UINT shaderRegister; //!< シェーダーレジスターの開始番号

        /**
        * @brief コンストラクタ
        */
        DescriptorRange()
            :rangeType(DescriptorRangeType::SRV), DescriptorNum(1), shaderRegister(0) { }

        /**
        * @brief 初期化
        */
        void init(DescriptorRangeType::Enum rangeType, UINT num, UINT shaderRegister) {
            this->rangeType = rangeType;
            this->DescriptorNum = num;
            this->shaderRegister = shaderRegister;
        }
    };

    /**
    * @brief ルートパラメータ
    */
    struct RootParameterDesc {
        RootParameterType type; //!< パラメータの種類
        ShaderVisibility::Enum visibility; //!< シェーダーの可視性
        UINT shaderRegister; //!< シェーダーレジスター番号
        /**
        * @brief 初期化
        */
        void init(RootParameterType paramType, UINT shaderRegister = 0, ShaderVisibility::Enum visibility = ShaderVisibility::Enum::ALL) {
            this->type = paramType;
            this->shaderRegister = shaderRegister;
            this->visibility = visibility;
        }

        /**
        * @brief ディスクリプタヒープとして初期化
        */
        void initAsDescriptor() {
            this->type = RootParameterType::DescriptorTable;
            this->shaderRegister = 0;
            this->visibility = ShaderVisibility::Enum::ALL;
        }
    };

    /**
    * @brief コンスタントデスク
    */
    struct ConstantsDesc {
        UINT bufferSize;

        /**
        * @brief コンストラクタ
        */
        ConstantsDesc()
            :bufferSize(0) { }

        /**
        * @brief コンストラクタ
        * @tparam T 使用するコンスタントバッファの型
        */
        template <class T>
        ConstantsDesc() : bufferSize(sizeof(T)) { }
    };

    /**
    * @brief スタティックサンプラー
    */
    struct StaticSampler {
        D3D12_FILTER filter;
    };

    /**
    * @brief ルートシグネチャデスク
    */
    struct RootSignatureDesc {
        std::wstring name = L"";
        RootSignatureFlag::Enum flags = RootSignatureFlag::GlobalRootSignature;
        std::vector<DescriptorRange>* pRanges = nullptr;
        std::vector<ConstantsDesc>* pConstants = nullptr;
        std::vector<RootParameterDesc>* pParams = nullptr;
        std::vector<StaticSampler>* pStaticSamplers = nullptr;

        RootSignatureDesc(RootSignatureFlag::Enum flags, std::vector<DescriptorRange>* ranges,
            std::vector<ConstantsDesc>* constants, std::vector<RootParameterDesc>* params,
            std::vector<StaticSampler>* samplers, const std::wstring& name = L"")
            :name(name), flags(flags), pRanges(ranges), pConstants(constants), pParams(params), pStaticSamplers(samplers) { }
    };


    /**
    * @class RootSignature
    * @brief ルートシグネチャ
    */
    class RootSignature {
    public:
        /**
        * @brief コンストラクタ
        */
        RootSignature(ID3D12Device* device, const RootSignatureDesc& rootSignatureDesc);
        /**
        * @brief デストラクタ
        */
        ~RootSignature();
        /**
        * @brief ルートシグネチャを取得する
        */
        ID3D12RootSignature* getRootSignature() const { return mRootSignature.Get(); }
        /**
        * @brief ルートシグネチャを作成する
        */
        void create(ID3D12Device* device, const RootSignatureDesc& rootSignatureDesc);
        /**
        * @brief ルートシグネチャのリセット
        */
        void reset();
        /**
        * @brief ローカルルートシグネチャとして登録する
        */
        void setLocalRootSignature(CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* local);
        /**
        * @brief グローバルルートシグネチャとして登録する
        */
        void setGlobalRootSignature(CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT* global);
        /**
        * @brief コンピュートシェーダー用ルートシグネチャとしてセットする
        */
        void setComputeRootSignature(ID3D12GraphicsCommandList* commandList);
        /**
        * @brief グラフィックス用ルートシグネチャとしてセットする
        */
        void setGraphicsRootSignature(ID3D12GraphicsCommandList* commandList);
    private:
        ComPtr<ID3D12RootSignature> mRootSignature; //!< ルートシグネチャ
    };
} //Framework::DX 