#ifndef DIRECTUI_UITILELAYOUT_H
#define DIRECTUI_UITILELAYOUT_H
#include <UIContainer.h>

class UITileLayout : public UIContainer
{
public:
    UITileLayout();

    UIString    GetClass() const override;

    LPVOID      GetInterface(const UIString &name) override ;

    void        SetPos(RECT rc, bool bNeedInvalidate) override ;

    int         GetFixedColumns()const;
    void        SetFixedColumns(int columns);
    int         GetChildVPadding()const;
    void        SetChildVPadding(int padding);

    SIZE        GetItemSize()const;
    void        SetItemSize(SIZE size);
    int         GetColumns()const;
    int         GetRows()const;

    void        SetAttribute(const char *pstrName, const char *pstrValue) override;
protected:
    SIZE        m_szItem;
    int         m_columns;
    int         m_rows;

    int         m_columnsFixed;
    int         m_childVPadding;
};

#endif //DIRECTUI_UITILELAYOUT_H