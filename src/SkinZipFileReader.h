#ifndef DIRECTUI_SKINZIPFILEREADER_H
#define DIRECTUI_SKINZIPFILEREADER_H
#include <UIString.h>
#include "SkinFileReaderService.h"

class SkinZipFileReader : public SkinFileReaderService{
public:
    SkinZipFileReader();
    ~SkinZipFileReader()override;
    ByteArray   ReadFile(const UIString &fileName) override;
private:
    bool        DoOpenZipFile();
    static UIString    GetZipFileName();
    uint64_t    GetCurrentUnCompressSize()const;
private:
    void    *m_unzHandle;
};

#endif //DIRECTUI_SKINZIPFILEREADER_H
