#pragma once

#include "ImGui/Include/ImGuiInclude.h"
#include <d3d12.h>
#include "Framework/Utility/Singleton.h"
#include "Framework/Utility/Typedef.h"

namespace Framework {
    /**
    * @class ImGui
    * @brief ImGui管理クラス
    */
    class ImGuiManager : public Utility::Singleton<ImGuiManager> {
    public:
        /**
        * @brief コンストラクタ
        */
        ImGuiManager();
        /**
        * @brief デストラクタ
        */
        ~ImGuiManager();
        /**
        @brief ImGuiの初期化
        */
        void init(HWND hWnd, ID3D12Device* device, DXGI_FORMAT format);
        /**
        * @brief フレーム開始時に呼ぶ
        */
        void beginFrame();
        /**
        * @brief フレーム終了時に呼ぶ
        */
        void endFrame(ID3D12GraphicsCommandList* commandList);
    private:
        ComPtr<ID3D12DescriptorHeap> mHeap; //!< ImGui用ヒープ
    };
} //Framework 