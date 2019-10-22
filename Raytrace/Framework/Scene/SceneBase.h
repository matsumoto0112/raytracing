#pragma once

namespace Framework {
    namespace Scene {
        /**
        * @class SceneBase
        * @brief discription
        */
        class SceneBase {
        public:
            /**
            * @brief �f�X�g���N�^
            */
            virtual ~SceneBase() = default;

            virtual void update() = 0;
            virtual void draw() = 0;
        private:

        };

    } //Scene 
} //Framework 