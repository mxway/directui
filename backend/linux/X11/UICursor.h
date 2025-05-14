#ifndef UICURSOR_H
#define UICURSOR_H
#include <UIPaintManager.h>
#include <map>

using namespace std;

class UICursor {
public:
    ~UICursor();
    UICursor(const UICursor &)=delete;
    UICursor &operator=(const UICursor &)=delete;
    static UICursor &GetInstance();
    void   LoadCursor(UIPaintManager *manager,int cursor);
private:
    UICursor();
private:
    map<int,Cursor> m_cursorMapping;
};



#endif //UICURSOR_H
