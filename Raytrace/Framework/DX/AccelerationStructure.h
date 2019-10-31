#pragma once
#include <vector>
#include <d3d12.h>
#include "DX/DXHelper.h"
#include "DX/RaytracingHelper.h"
#include "Math/Vector3.h"
#include "Application/Assets/Shader/RaytracingStructure.h"

namespace Framework::DX {

    struct Buffer {
        D3DBuffer vertexBuffer;
        D3DBuffer indexBuffer;
    };

    /**
    * @class AccelerationStructure
    * @brief ASの管理
    */
    class AccelerationStructure {
        using VertexList = std::vector<Vertex>;
        using IndexList = std::vector<UINT16>;
        using ID = UINT;
    public:
        /**
        * @brief コンストラクタ
        */
        AccelerationStructure();
        /**
        * @brief デストラクタ
        */
        ~AccelerationStructure();
        /**
        * @brief Bottom-Level-ASのバッファを作成する
        */
        void addBLASBuffer(ID3D12Device5* device, const VertexList& vertices, const IndexList& indices);
        /**
        * @brief Bottom-Level-ASを構築する
        */
        ID buildBLAS(ID3D12Device5* device, ID3D12GraphicsCommandList5* commandList);
        /**
        * @brief Top-Level-ASの設定をする
        * @param device DXRデバイス
        * @param tlasNum Top-Level-ASの数を指定する
        */
        void tlasConfig(ID3D12Device5* device, UINT tlasNum);
        /**
        * @brief Top-Level-ASのバッファを作成する
        */
        void addTLASBuffer(UINT id, UINT instanceID, UINT hitGroupIndex, const XMMATRIX& transform);
        /**
        * @brief Top-Level-ASを構築する
        */
        void buildTLAS(ID3D12GraphicsCommandList5* commandList);
        /**
        * @brief ASのディスクリプタヒープをセットする
        */
        void setDescriptorTable(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex);
    private:
        std::vector<Buffer> mBuffers;
        std::vector<Buffer> mBufferStacks;
        std::vector<AccelerationStructureBuffers> mBottomLevelASs;
        UINT mTLASSize;
        UINT mTLASIndex;
        D3D12_RAYTRACING_INSTANCE_DESC* mMappedInstanceDescs;
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS mInputs;
        AccelerationStructureBuffers mTLASTmpBuffer;
        ComPtr<ID3D12Resource> mTopLevelAS;
    };
} //Framework::DX 