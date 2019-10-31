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
    * @brief AS�̊Ǘ�
    */
    class AccelerationStructure {
        using VertexList = std::vector<Vertex>;
        using IndexList = std::vector<UINT16>;
        using ID = UINT;
    public:
        /**
        * @brief �R���X�g���N�^
        */
        AccelerationStructure();
        /**
        * @brief �f�X�g���N�^
        */
        ~AccelerationStructure();
        /**
        * @brief Bottom-Level-AS�̃o�b�t�@���쐬����
        */
        void addBLASBuffer(ID3D12Device5* device, const VertexList& vertices, const IndexList& indices);
        /**
        * @brief Bottom-Level-AS���\�z����
        */
        ID buildBLAS(ID3D12Device5* device, ID3D12GraphicsCommandList5* commandList);
        /**
        * @brief Top-Level-AS�̐ݒ������
        * @param device DXR�f�o�C�X
        * @param tlasNum Top-Level-AS�̐����w�肷��
        */
        void tlasConfig(ID3D12Device5* device, UINT tlasNum);
        /**
        * @brief Top-Level-AS�̃o�b�t�@���쐬����
        */
        void addTLASBuffer(UINT id, UINT instanceID, UINT hitGroupIndex, const XMMATRIX& transform);
        /**
        * @brief Top-Level-AS���\�z����
        */
        void buildTLAS(ID3D12GraphicsCommandList5* commandList);
        /**
        * @brief AS�̃f�B�X�N���v�^�q�[�v���Z�b�g����
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