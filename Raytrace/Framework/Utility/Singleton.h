#pragma once

namespace Framework::Utility {
    /**
    * @brief �V���O���g�����N���X
    */
    template <class T>
    class Singleton {
    public:
        /**
        * @brief �C���X�^���X�̎擾
        */
        static inline T* getInstance() {
            static T instance;
            return &instance;
        }
    protected:
        /**
        * @brief �R���X�g���N�^
        */
        Singleton() { };
        /**
        * @brief �f�X�g���N�^
        */
        virtual  ~Singleton() = default;
    private:
        //�R�s�[�֎~
        void operator=(const Singleton& other) = delete;
        Singleton(const Singleton& other) = delete;
    };
} //Framework::Utility