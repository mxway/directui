#ifndef DIRECTUI_UIMICROBLOG_H
#define DIRECTUI_UIMICROBLOG_H
#include <UIList.h>
#include <UIPaintManager.h>

class UIMicroBlog : public UIList
{
public:
    UIMicroBlog(UIPaintManager& paint_manager);
    ~UIMicroBlog();

private:
    UIPaintManager& paint_manager_;
};

#endif //DIRECTUI_UIMICROBLOG_H
