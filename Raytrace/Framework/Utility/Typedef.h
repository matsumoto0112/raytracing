#pragma once

#include <wrl/client.h>

//共通で使うものを定義しておく

template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;