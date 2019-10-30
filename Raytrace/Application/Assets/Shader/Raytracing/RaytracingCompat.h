#ifndef SHADER_RAYTRACING_RAYTRACINGCOMPAT_H
#define SHADER_RAYTRACING_RAYTRACINGCOMPAT_H

#ifdef HLSL
#include "RaytracingTypedef.h"
#else
typedef UINT Index;
using Framework::Math::Vector2;
using Framework::Math::Vector3;
using Framework::Math::Vector4;
using Framework::Utility::Color4;
#endif

/**
* @brief ƒŒƒC‚ð”ò‚Î‚µ‚½
*/
struct RayPayload {
    Color4 color;
    UINT recursion;
};

struct Vertex {
    Vector3 position;
};

struct SceneConstantBuffer {
    Matrix4x4 projectionToWorld;
    Vector3 cameraPosition;
};

#endif // !SHADER_RAYTRACING_RAYTRACINGCOMPAT_H

