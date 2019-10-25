#pragma once

#include <string>
#include "Framework/Utility/Singleton.h"

/**
* @class Path
* @brief �e��t�@�C���ւ̃p�X���`
*/
class Path : public Framework::Utility::Singleton<Path> {
public:
    /**
    * @brief ���s�t�@�C���̃t�H���_�p�X���擾����
    */
    const std::wstring& exe() const { return mExe; }
    /**
    * @brief ���\�[�X�t�H���_�܂ł̃p�X
    */
    const std::wstring& resource() const { return mResource; }
    /**
    * @brief �e�N�X�`���t�H���_�܂ł̃p�X
    */
    const std::wstring& texture() const { return mTexture; }
    /**
    * @brief �V�F�[�_�[�t�@�C���ւ̃p�X
    */
    const std::wstring& shader() const { return mShader; }
    /**
    * @brief ���f���̂���t�@�C���ւ̃p�X
    */
    const std::wstring& model() const { return mModel; }
private:
    std::wstring mExe; //!< ���s�t�@�C���ւ̃p�X
    std::wstring mResource; //!< ���\�[�X�t�H���_�ւ̃p�X
    std::wstring mTexture; //!< �e�N�X�`���̃��[�g�t�H���_�ւ̃p�X
    std::wstring mShader; //!< �V�F�[�_�t�@�C���ւ̃p�X
    std::wstring mModel; //!< ���f���f�[�^�t�@�C���ւ̃p�X
protected:
    Path();
    ~Path();
};
