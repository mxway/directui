#include "SkinFileReaderService.h"
#include <UIResourceMgr.h>
#include "SkinFileReader.h"
#include "SkinZipFileReader.h"

shared_ptr <SkinFileReaderService> SkinFileReaderFactory::GetSkinFileReader() {
    if(UIResourceMgr::GetInstance().GetResourceSkinType() == UIResourceMgr::ResourceSkinType_ZipFile){
        return make_shared<SkinZipFileReader>();
    }else{
        return make_shared<SkinFileReader>();
    }
}
