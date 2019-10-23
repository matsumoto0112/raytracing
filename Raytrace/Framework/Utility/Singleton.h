#pragma once

namespace Framework::Utility {
    /**
    * @brief シングルトン基底クラス
    */
    template <class T>
    class Singleton {
    public:
        /**
        * @brief インスタンスの取得
        */
        static inline T* getInstance() {
            static T instance;
            return &instance;
        }
    protected:
        /**
        * @brief コンストラクタ
        */
        Singleton() { };
        /**
        * @brief デストラクタ
        */
        virtual  ~Singleton() = default;
    private:
        //コピー禁止
        void operator=(const Singleton& other) = delete;
        Singleton(const Singleton& other) = delete;
    };
} //Framework::Utility