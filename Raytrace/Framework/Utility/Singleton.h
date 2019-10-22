#pragma once

namespace Framework {
    namespace Utility {
        template <class T>
        class Singleton {
        public:
            static inline T* getInstance() {
                static T instance;
                return &instance;
            }
        protected:
            Singleton() { };
            virtual  ~Singleton() = default;
        private:
            void operator=(const Singleton& other) = delete;
            Singleton(const Singleton& other) = delete;
        };
    } //Utility 
} //Framework 