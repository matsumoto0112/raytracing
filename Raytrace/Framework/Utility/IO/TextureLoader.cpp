#include "TextureLoader.h"
#include <vector>
#include <atlstr.h>
#include "Framework/Utility/Debug.h"

namespace Framework {
namespace Utility {

TextureLoader::TextureLoader() {
    //�C���[�W�t�@�N�g���쐬
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory,
        nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&mFactory));
    MY_ASSERTION(SUCCEEDED(hr), L"�C���[�W�t�@�N�g���쐬���s");

    //�t�H�[�}�b�g�ϊ���쐬
    hr = mFactory->CreateFormatConverter(&mConverter);
    MY_ASSERTION(SUCCEEDED(hr), L"FormatConverter�쐬���s");
}

TextureLoader::~TextureLoader() { }

std::vector<BYTE> TextureLoader::load(const std::wstring& filepath, UINT* width, UINT* height) {
    //�p�X����f�R�[�_�[���쐬
    HRESULT hr = mFactory->CreateDecoderFromFilename(
        filepath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &mDecoder);
    MY_ASSERTION(SUCCEEDED(hr), L"�f�R�[�_�[�쐬���s\n�t�@�C������" + filepath);

    //�t���[���̎擾
    hr = mDecoder->GetFrame(0, &mFrame);
    MY_ASSERTION(SUCCEEDED(hr), L"GetFrame���s");

    UINT w, h;
    mFrame->GetSize(&w, &h);
    //�s�N�Z���`�����擾
    WICPixelFormatGUID pixelFormat;
    hr = mFrame->GetPixelFormat(&pixelFormat);
    MY_ASSERTION(SUCCEEDED(hr), L"GetPixelFrame���s");

    int stride = w * 4;
    int bufferSize = stride * h;
    std::vector<BYTE> buffer(bufferSize);
    //�s�N�Z���`����32bitRGBA�łȂ�������ϊ�����
    if (pixelFormat != GUID_WICPixelFormat32bppRGBA) {
        hr = mFactory->CreateFormatConverter(&mConverter);
        MY_ASSERTION(SUCCEEDED(hr), L"FormatConverter�쐬���s");

        hr = mConverter->Initialize(mFrame, GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeErrorDiffusion, nullptr, 0, WICBitmapPaletteTypeCustom);
        MY_ASSERTION(SUCCEEDED(hr), L"Converter�̏��������s");

        hr = mConverter->CopyPixels(0, stride, bufferSize, buffer.data());
        MY_ASSERTION(SUCCEEDED(hr), L"�s�N�Z���f�[�^�̃R�s�[���s");
    }
    else {
        hr = mFrame->CopyPixels(nullptr, stride, bufferSize, buffer.data());
        MY_ASSERTION(SUCCEEDED(hr), L"�s�N�Z���f�[�^�̃R�s�[���s");
    }

    *width = w;
    *height = h;
    return buffer;
}
} //Utility 
} //Framework 