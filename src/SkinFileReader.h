#ifndef DIRECTUI_SKINFILEREADER_H
#define DIRECTUI_SKINFILEREADER_H
#include "SkinFileReaderService.h"
#include <UIString.h>

class SkinFileReader : public SkinFileReaderService {
public:
    SkinFileReader();
    ~SkinFileReader() override;
    ByteArray   ReadFile(const UIString &fileName)override;
};

#endif //DIRECTUI_SKINFILEREADER_H
