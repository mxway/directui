#include "../../src/SkinFileReader.h"
#include <windows.h>
#include "EncodingTransform.h"
#include <UIFileHelper.h>
#include <UIResourceMgr.h>

ByteArray SkinFileReader::ReadFile(const UIString &fileName) {
    UIString fullFileName = fileName;
    if(!UIFileHelper::IsAbsolutePath(fileName)){
        fullFileName = UIResourceMgr::GetInstance().GetResourcePath() + UIFileHelper::UI_PATH_SEPARATOR + fileName;
    }
    ByteArray result = {nullptr, 0};
    wchar_t *wideImageFile = Utf8ToUcs2(fullFileName.GetData(),-1);
    HANDLE hFile = ::CreateFileW(wideImageFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, \
					FILE_ATTRIBUTE_NORMAL, nullptr);
    delete []wideImageFile;
    if( hFile == INVALID_HANDLE_VALUE ) return result;
    DWORD dwSize = ::GetFileSize(hFile, nullptr);
    if (dwSize == 0)
    {
        ::CloseHandle(hFile);
        return result;
    }

    DWORD dwRead = 0;
    result.m_buffer = new BYTE[dwSize+2];
    ::ReadFile( hFile, result.m_buffer, dwSize, &dwRead, nullptr );
    ::CloseHandle( hFile );
    result.m_bufferSize = dwRead;
    return result;
}
