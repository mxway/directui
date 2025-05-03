#include <UIFileHelper.h>

UIString UIFileHelper::UI_PATH_SEPARATOR = UIString{"\\"};

bool UIFileHelper::IsAbsolutePath(const UIString &fileName) {
    if(fileName.GetLength()<2){
        return false;
    }
    if( (!(fileName[0]>='a' && fileName[0]<='z')) && (!(fileName[0]>='A' && fileName[0]<='Z')) ){
        return false;
    }
    return fileName[1] == ':';
}