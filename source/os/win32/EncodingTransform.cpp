#include "EncodingTransform.h"
#include <windows.h>
#include <cstring>

wchar_t *Utf8ToUcs2(const char *str, int length) {
    if(str == nullptr){
        return nullptr;
    }
    int numberOfChar = ::MultiByteToWideChar(CP_UTF8,0,
                                             str, length,
                                             nullptr,0);
    if(numberOfChar == 0){
        return nullptr;
    }
    auto *result = new wchar_t[numberOfChar+1];
    memset(result, 0, sizeof(wchar_t )*(numberOfChar+1));
    ::MultiByteToWideChar(CP_UTF8, 0,
                          str, length, result, numberOfChar);
    return result;
}

char *Ucs2ToUtf8(const wchar_t *wideString, int length) {
    if(wideString == nullptr){
        return nullptr;
    }
    int numberOfChar = ::WideCharToMultiByte(CP_UTF8, 0,
                                             wideString, length,
                                             nullptr, 0,nullptr,nullptr);
    if(numberOfChar == 0){
        return nullptr;
    }
    auto *result = new char[numberOfChar+1];
    memset(result, 0, numberOfChar+1);
    ::WideCharToMultiByte(CP_UTF8, 0,
                          wideString, length,
                          result, numberOfChar,nullptr,nullptr);
    return result;
}
