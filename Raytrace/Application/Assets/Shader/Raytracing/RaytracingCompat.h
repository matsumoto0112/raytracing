#ifndef SHADER_RAYTRACING_RAYTRACINGCOMPAT_H
#define SHADER_RAYTRACING_RAYTRACINGCOMPAT_H

#ifdef HLSL
#include "RaytracingTypedef.h"
#else
#include "Framework/Math/Matrix4x4.h"
#include "Framework/Math/Vector2.h"
#include "Framework/Math/Vector3.h"
#include "Framework/Math/Vector4.h"
#include "Framework/Utility/Color4.h"
typedef UINT16 Index;
using Framework::Math::Vector2;
using Framework::Math::Vector3;
using Framework::Math::Vector4;
using Framework::Utility::Color4;
using Framework::Math::Matrix4x4;
using namespace DirectX;
#endif

/**
* @brief ���C���΂������Ɏg�p����y�C���[�h
*/
struct RayPayload {
    Color4 color;
    UINT recursion;
};

struct ShadowPayload {
    bool hit;
};

/**
* @brief ���_���
*/
struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
};

/**
* @brief �V�[�����
*/
struct SceneConstantBuffer {
    XMMATRIX projectionToWorld;
    Vector4 cameraPosition;
    Color4 lightAmbient;
    Color4 lightDiffuse;
    Vector4 lightPosition;
};

#ifdef HLSL
//closesthit�Ȃǂň����Ƃ��Ď󂯎��O�p�`�̏d�S
typedef BuiltInTriangleIntersectionAttributes MyAttr;
#endif 


#endif // !SHADER_RAYTRACING_RAYTRACINGCOMPAT_H

