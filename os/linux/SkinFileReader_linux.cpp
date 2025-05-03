#include "../../src/SkinFileReader.h"
#include <cstdio>
#include <cstring>
#include <UIFileHelper.h>
#include <UIResourceMgr.h>

ByteArray SkinFileReader::ReadFile(const UIString &fileName) {
    UIString  fullFileName = fileName;
    if(!UIFileHelper::IsAbsolutePath(fileName)){
        fullFileName = UIResourceMgr::GetInstance().GetResourcePath() + UIFileHelper::UI_PATH_SEPARATOR + fileName;
    }
    ByteArray  result = {nullptr, 0};
    FILE *fp = fopen(fullFileName.GetData(),"rb");
    if(fp == nullptr){
        return result;
    }
    fseek(fp, 0, SEEK_END);
    result.m_bufferSize = ftell(fp);
    result.m_buffer = new unsigned char[result.m_bufferSize+2];
    memset(result.m_buffer, 0, result.m_bufferSize+2);
    fseek(fp, 0, SEEK_SET);
    fread(result.m_buffer, 1, result.m_bufferSize, fp);
    fclose(fp);
    return result;
}
