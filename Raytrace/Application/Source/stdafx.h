#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <Windows.h>
#include <iostream>
#include <string>
#include <assert.h>
#include <wrl.h>
#include <array>
#include <vector>
#include <unordered_map>

#include "ImGui/ImGuiInclude.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include "Framework/Libs/d3dx12.h"
#include <DirectXMath.h>

#include <fbxsdk.h>
