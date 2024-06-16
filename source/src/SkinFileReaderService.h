#ifndef DIRECTUI_SKINFILEREADERSERVICE_H
#define DIRECTUI_SKINFILEREADERSERVICE_H
#include <cstdint>
#include <UIString.h>
#include <memory>

using namespace std;

typedef struct ByteArray_s
{
    unsigned char     *m_buffer;
    uint32_t          m_bufferSize;
}ByteArray;

class SkinFileReaderService
{
public:
    virtual ~SkinFileReaderService()=default;
    virtual ByteArray   ReadFile(const UIString &fileName) = 0;
};

class SkinFileReaderFactory
{
public:
    static shared_ptr<SkinFileReaderService> GetSkinFileReader();
};

#endif //DIRECTUI_SKINFILEREADERSERVICE_H