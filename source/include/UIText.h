#ifndef DIRECTUI_UITEXT_H
#define DIRECTUI_UITEXT_H
#include <UILabel.h>

class UIText : public UILabel
{
public:
    UIText();
    ~UIText();

    uint32_t GetControlFlags() const override;

    UIString GetClass() const override;

    LPVOID GetInterface(const UIString &name) override;

    UIString *GetLinkContent(int iIndex);

    void DoEvent(TEventUI &event) override;

    void PaintText(HANDLE_DC hDC) override;

protected:
    enum { MAX_LINK = 8};
    int         m_nLinks;
    RECT        m_rcLinks[MAX_LINK];
    UIString    m_sLinks[MAX_LINK];
    int         m_nHoverLink;
};

#endif //DIRECTUI_UITEXT_H