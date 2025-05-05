#include "UIFileHelper.h"

UIString UIFileHelper::UI_PATH_SEPARATOR = UIString{"/"};

bool UIFileHelper::IsAbsolutePath(const UIString &fileName) {
    return (fileName.GetLength()>0 && fileName[0]=='/');
}
