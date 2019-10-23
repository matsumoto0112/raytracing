#pragma once
#include <functional>
#include "ImGui/Item/IItem.h"

namespace Framework::ImGUI {
    /**
    * @class CallBackItem
    * @brief コールバックのあるアイテム基底クラス
    */
    template <class T>
    class CallBackItem : public IItem {
    protected:
        using CallBack = std::function<void(T)>;
    public:
        /**
        * @brief コンストラクタ
        * @param label ラベル名
        */
        CallBackItem(const std::string& label)
            :IItem(label), mCallBack(nullptr) { }
        /**
        * @brief デストラクタ
        */
        virtual ~CallBackItem() { }
        /**
        * @brief コールバックの登録
        */
        void setCallBack(CallBack callBack) { mCallBack = callBack; }
    protected:
        CallBack mCallBack; //!< コールバック関数
    };
} //Framework::ImGUI 