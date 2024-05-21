#ifndef DIRECTUI_UIEDIT_H
#define DIRECTUI_UIEDIT_H
#include <UILabel.h>
#include <memory>

using namespace std;

/**
 * 这里没有直接移植原有duilib代码edit控件的实现方式，在这里，直接使用自绘的方式实现edit控件。
 */

struct UIEditInternal;

class UIEdit : public UILabel
{
public:
    UIEdit();

    uint32_t        GetControlFlags() const override;

    UIString        GetClass() const override;

    LPVOID          GetInterface(const UIString &name) override;

    void            SetEnabled(bool enabled = true);
    void            SetMaxChar(uint32_t maxChar);
    uint32_t        GetMaxChar()const;
    void            SetReadOnly(bool readOnly);
    bool            IsReadOnly()const;
    void            SetPasswordMode(bool passwordMode);
    bool            IsPasswordMode()const;
    void            SetPasswordChar(char passwordChar);
    char            GetPasswordChar()const;
    bool            IsAutoSelAll()const;
    void            SetAutoSelAll(bool autoSelAll);
    void            SetNumberOnly(bool numberOnly);
    bool            IsNumberOnly()const;
    UIString        GetNormalImage()const;
    void            SetNormalImage(const UIString &normalImage);
    UIString        GetHotImage()const;
    void            SetHotImage(const UIString &hotImage);
    UIString        GetFocusedImage()const;
    void            SetFocusedImage(const UIString &focusedImage);
    UIString        GetDisabledImage()const;
    void            SetDisabledImage(const UIString &disabledImage);
    void            SetSel(long startChar, long endChar);
    void            SetSelAll();
    void            SetReplaceSel(const UIString &replace);

    void            SetVisible(bool visible=true)override;
    void            SetInternVisible(bool visible=true)override;
    void            DoEvent(TEventUI  &event)override;
    void            SetAttribute(const char *pstrName, const char *pstrValue)override;
    void            PaintStatusImage(HANDLE_DC hDC)override;
    void            PaintText(HANDLE_DC hDC)override;

    uint32_t        GetCharNumber()const;
private:
    void            InitInternal();
    void            DrawCaret();
protected:
    uint32_t        m_maxChar;
    bool            m_readOnly;
    bool            m_numberOnly;
    bool            m_passwordMode;
    bool            m_autoSelAll;
    char            m_passwordChar;
    uint32_t        m_buttonState;
    uint32_t        m_editBkColor;
    TDrawInfo       m_diNormal;
    TDrawInfo       m_diHot;
    TDrawInfo       m_diFocused;
    TDrawInfo       m_diDisabled;
    shared_ptr<UIEditInternal>  m_internalImpl;
};

#endif //DIRECTUI_UIEDIT_H