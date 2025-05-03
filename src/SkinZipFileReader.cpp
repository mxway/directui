#include "SkinZipFileReader.h"
#include <UIResourceMgr.h>
#include <unzip.h>
#include <UIFileHelper.h>
#ifdef WIN32
#include "../os/win32/EncodingTransform.h"
#include <iowin32.h>
#endif

SkinZipFileReader::SkinZipFileReader()
    :m_unzHandle{nullptr}
{

}

SkinZipFileReader::~SkinZipFileReader() {
    if(m_unzHandle != nullptr){
        unzClose(m_unzHandle);
    }
}

ByteArray SkinZipFileReader::ReadFile(const UIString &fileName) {
    ByteArray result = {nullptr, 0};

    if(!this->DoOpenZipFile()){
        return result;
    }
    if(unzLocateFile(m_unzHandle,fileName.GetData(), 1)!=UNZ_OK){
        return result;
    }
    if(unzOpenCurrentFile(m_unzHandle) != UNZ_OK){
        return result;
    }
    result.m_bufferSize = this->GetCurrentUnCompressSize();
    result.m_buffer = new unsigned char [result.m_bufferSize+2];
    unzReadCurrentFile(m_unzHandle,result.m_buffer, result.m_bufferSize);
    return result;
}

bool SkinZipFileReader::DoOpenZipFile() {
    if(m_unzHandle != nullptr){
        return true;
    }
    UIString zipFile = GetZipFileName();
    zlib_filefunc64_def     ffunc;
#ifdef WIN32
    fill_win32_filefunc64W(&ffunc);
    wchar_t *unicodeFileName = Utf8ToUcs2(zipFile.GetData());
    m_unzHandle = unzOpen2_64(unicodeFileName, &ffunc);
    delete []unicodeFileName;
#else
    fill_fopen64_filefunc(&ffunc);
    m_unzHandle = unzOpen2_64(zipFile.GetData(),&ffunc);
#endif
    return m_unzHandle != nullptr;
}

UIString SkinZipFileReader::GetZipFileName() {
    UIString    zipFile = UIResourceMgr::GetInstance().GetResourceZip();
    if(UIFileHelper::IsAbsolutePath(zipFile)){
        return zipFile;
    }
    return UIResourceMgr::GetInstance().GetCurrentPath() + UIFileHelper::UI_PATH_SEPARATOR + zipFile;
}

uint64_t SkinZipFileReader::GetCurrentUnCompressSize() const {
    unz_file_info64     file_info{0};
    char                filename_inzip[256] = {0};
    int ret = unzGetCurrentFileInfo64(m_unzHandle, &file_info, filename_inzip,sizeof(filename_inzip)-2,
                            nullptr, 0, nullptr, 0);
    return ret==UNZ_OK?file_info.uncompressed_size:0;
}
