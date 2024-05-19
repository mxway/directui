#include "UIComputerExamine.h"
#include <UIDlgBuilder.h>

UIComputerExamine::UIComputerExamine() {
    UIDlgBuilder    builder;
    UIContainer *uiComputerExamine = dynamic_cast<UIContainer*>(
            builder.Create(UIString{u8"ComputerExamine.xml"}));
    if(uiComputerExamine){
        this->Add(uiComputerExamine);
    }else{
        this->RemoveAll();
    }
}
