#pragma once

namespace Framework::DX {
    interface IDeviceState {
        public:
        virtual bool isTearingSupported() const = 0;
    };
} //Framework::DX 