#ifndef SHADER_RAYTRACING_RAYTRACINGTYPEDEF_H
#define SHADER_RAYTRACING_RAYTRACINGTYPEDEF_H

/**
* @brief HLSL��Cpp�̂ق��ŋ��L���Đ錾���邽�߂̒�`
*/

#ifdef HLSL
typedef float2 Vector2;
typedef float3 Vector3;
typedef float4 Vector4;
typedef float4 Color4;
typedef float4x4 Matrix4x4;
typedef float4x4 XMMATRIX;
typedef uint UINT;
#endif

#endif // !SHADER_RAYTRACING_RAYTRACINGTYPEDEF_H
