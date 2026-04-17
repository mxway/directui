#include <UIRichEdit.h>
#include "UIRichEditInternal_impl.h"
#include "UIRichEditCaret.h"
#include "UIRenderEngine.h"
#include "UIPaintManager.h"
#include "UIRenderClip.h"
#include <cstring>

#include "UIFont.h"
#include "UIResourceMgr.h"

void SetSegmentRect(UIRect& rect, int left, int top, int right, int bottom) {
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
}

UIRichEdit::UIRichEdit() {
    m_pVerticalScrollBar = nullptr;
    m_enableVerticalScrollBar = false;
    m_verticalScrollCaptured = false;
    m_contentHeight = 0;
    m_layoutDirty = true;
    m_lastLayoutWidth = -1;
    m_caret.reset(new UIRichEditCaret());
}

UIRichEdit::~UIRichEdit() {
    if (m_pVerticalScrollBar != nullptr) {
        m_pVerticalScrollBar->Delete();
        m_pVerticalScrollBar = nullptr;
    }
}

uint32_t UIRichEdit::GetControlFlags() const {
    if (!IsEnabled()) {
        return UILabel::GetControlFlags();
    }
    return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

UIString UIRichEdit::GetClass() const {
    return UIString{DUI_CTR_RICHEDIT};
}

LPVOID UIRichEdit::GetInterface(const UIString& name) {
    if(name==DUI_CTR_RICHEDIT){
        return this;
    }
    return UILabel::GetInterface(name);
}

void UIRichEdit::SetText(const UIString& text) {
    m_document.ClearParagraphs();
    if (m_caret) {
        m_caret->Clear();
    }
    const char *end = text.GetData() + text.GetLength();

    const char *paragraphStart = text.GetData();
    const char *paragraphEnd = paragraphStart;
    while (paragraphStart < end) {
        paragraphEnd = paragraphStart;
        while (paragraphEnd != end && *paragraphEnd != '\r' && *paragraphEnd != '\n') {
            paragraphEnd++;
        }
        if (paragraphStart == paragraphEnd) {
            m_document.AppendParagraph(Paragraph{});
        }else {
            auto textRun = make_shared<TextRun>();
            textRun->SetText(UIString{paragraphStart,static_cast<int>(paragraphEnd-paragraphStart)});
            Paragraph   paragraph;
            paragraph.AppendRun(textRun);
            m_document.AppendParagraph(paragraph);

        }
        paragraphStart = paragraphEnd;

        //\r\n
        if (*paragraphStart=='\r' && (paragraphStart+1) < end && *(paragraphStart+1)=='\n') {
            paragraphStart += 2;
        }else if (*paragraphStart=='\r' || *paragraphStart=='\n') {
            paragraphStart++;
        }
    }

    m_documentLayouts.documentLayouts.assign(m_document.GetParagraphCount(), ParagraphLayout());
    m_layoutDirty = true;
    SyncScrollFromBar();
    Invalidate();
}

int UIRichEdit::GetTextViewWidth() const {
    const RECT rc = GetTextViewRect();
    return std::max(0, static_cast<int>(rc.right - rc.left));
}

void UIRichEdit::MarkLayoutDirty() {
    m_layoutDirty = true;
    if (m_caret) {
        m_caret->Clear();
    }
    for (size_t i = 0; i < m_documentLayouts.documentLayouts.size(); ++i) {
        m_documentLayouts.documentLayouts[i].dirty = true;
    }
}

RECT UIRichEdit::GetTextViewRect() const {
    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.top += m_rcTextPadding.top;
    rc.right -= m_rcTextPadding.right;
    rc.bottom -= m_rcTextPadding.bottom;
    if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
        rc.right -= m_pVerticalScrollBar->GetFixedWidth();
    }
    if (rc.right < rc.left) rc.right = rc.left;
    if (rc.bottom < rc.top) rc.bottom = rc.top;
    return rc;
}

void UIRichEdit::EnsureVerticalScrollBar() {
    if (m_pVerticalScrollBar != nullptr) {
        return;
    }
    m_pVerticalScrollBar = new UIScrollBar();
    m_pVerticalScrollBar->SetScrollRange(0);
    m_pVerticalScrollBar->SetVisible(false);
    // Keep scrollbar event bubbling detached; UIRichEdit forwards needed events explicitly.
    m_pVerticalScrollBar->SetManager(m_manager, nullptr, false);
    if (m_manager) {
        const char* defaultAttr = m_manager->GetDefaultAttributeList("VScrollBar");
        if (defaultAttr) {
            m_pVerticalScrollBar->SetAttributeList(defaultAttr);
        }
    }
    if (!m_vScrollBarStyle.IsEmpty()) {
        m_pVerticalScrollBar->SetAttributeList(m_vScrollBarStyle.GetData());
    }
}

void UIRichEdit::SyncScrollFromBar() {
    if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
        m_pVerticalScrollBar->SetScrollPos(0, false);
    }
}

void UIRichEdit::UpdateVerticalScrollBar() {
    if (!m_enableVerticalScrollBar || m_pVerticalScrollBar == nullptr) {
        m_contentHeight = 0;
        return;
    }

    const RECT viewRc = GetTextViewRect();
    const int viewHeight = std::max(0, static_cast<int>(viewRc.bottom - viewRc.top));
    const int scrollRange = std::max(0, m_contentHeight - viewHeight);
    const bool needVisible = scrollRange > 0;

    m_pVerticalScrollBar->SetVisible(needVisible);
    m_pVerticalScrollBar->SetScrollRange(scrollRange);
    if (!needVisible) {
        m_pVerticalScrollBar->SetScrollPos(0, false);
    }else {
        int pos = m_pVerticalScrollBar->GetScrollPos();
        if (pos > scrollRange) {
            m_pVerticalScrollBar->SetScrollPos(scrollRange,false);
        }else if (pos < 0) {
            m_pVerticalScrollBar->SetScrollPos(0, false);
        }
    }

    RECT rcScroll = m_rcItem;
    rcScroll.left = rcScroll.right - m_pVerticalScrollBar->GetFixedWidth();
    m_pVerticalScrollBar->SetPos(rcScroll, false);
}

void UIRichEdit::SetPos(RECT rc, bool bNeedInvalidate) {
    //const int oldWidth = GetTextViewWidth();
    const RECT oldViewRc = GetTextViewRect();
    UILabel::SetPos(rc, bNeedInvalidate);
    if (m_pVerticalScrollBar) {
        RECT rcScroll = m_rcItem;
        rcScroll.left = rcScroll.right - m_pVerticalScrollBar->GetFixedWidth();
        m_pVerticalScrollBar->SetPos(rcScroll, false);
    }

    const RECT newViewRc = GetTextViewRect();
    const int oldW = std::max(0,static_cast<int>(oldViewRc.right - oldViewRc.left));
    const int newW = std::max(0,static_cast<int>(newViewRc.right - newViewRc.left));
    if (oldW != newW) {
        MarkLayoutDirty();
    }
}

void UIRichEdit::SetManager(UIPaintManager* pManager, UIControl* pParent, bool bInit) {
    if (m_pVerticalScrollBar) {
        m_pVerticalScrollBar->SetManager(pManager, nullptr, bInit);
    }
    UILabel::SetManager(pManager, pParent, bInit);
}

void UIRichEdit::DoEvent(TEventUI& event) {
    if (event.Type == UIEVENT_SETFOCUS) {
        if (m_caret && m_caret->HasCaret()) {
            m_caret->SetVisible(true);
            Invalidate();
        }
    }
    if (event.Type == UIEVENT_KILLFOCUS) {
        if (m_caret && m_caret->IsVisible()) {
            m_caret->SetVisible(false);
            Invalidate();
        }
    }

    if (event.Type == UIEVENT_SCROLLWHEEL && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
        int pos = m_pVerticalScrollBar->GetScrollPos();
        const int line = std::max(1, m_pVerticalScrollBar->GetLineSize());
        switch (LOWORD(event.wParam)) {
            case SB_LINEUP:
                m_pVerticalScrollBar->SetScrollPos(pos - line, false);
                Invalidate();
                return;
            case SB_LINEDOWN:
                m_pVerticalScrollBar->SetScrollPos(pos + line, false);
                Invalidate();
                return;
            default:
                break;
        }
    }

    if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
        if (!m_pVerticalScrollBar->IsMouseEnabled()) {
            UILabel::DoEvent(event);
            return;
        }
        UIRect rcScroll{m_pVerticalScrollBar->GetPos()};
        const bool hitScroll = rcScroll.IsPtIn(event.ptMouse);
        const bool downEvent = (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK);
        if (downEvent && hitScroll) {
            m_verticalScrollCaptured = true;
            m_pVerticalScrollBar->DoEvent(event);
            Invalidate();
            return;
        }
        if (m_verticalScrollCaptured && (event.Type == UIEVENT_MOUSEMOVE || event.Type == UIEVENT_BUTTONUP)) {
            m_pVerticalScrollBar->DoEvent(event);
            if (event.Type == UIEVENT_BUTTONUP) {
                m_verticalScrollCaptured = false;
            }
            Invalidate();
            return;
        }
        if (hitScroll && event.Type == UIEVENT_MOUSEMOVE) {
            m_pVerticalScrollBar->DoEvent(event);
            return;
        }
    }

    const bool activateCaret = (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK);
    if (activateCaret && IsEnabled() && m_manager != nullptr && m_caret) {
        const RECT viewRc = GetTextViewRect();
        UIRect rcView{viewRc};
        if (rcView.IsPtIn(event.ptMouse)) {
            SetFocus();
            HANDLE_DC hdc = m_manager->GetPaintDC();
            if (hdc != nullptr) {
                EnsureLayoutReady(hdc);
                const int scrollY = (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) ? m_pVerticalScrollBar->GetScrollPos() : 0;
                POINT documentPoint = {
                    event.ptMouse.x - viewRc.left,
                    event.ptMouse.y - viewRc.top + scrollY
                };
                m_caret->HitTest(hdc, documentPoint, m_document.GetParagraphs(), m_documentLayouts.documentLayouts);
                m_caret->SetVisible(IsFocused() || (m_manager && m_manager->GetFocus() == this));
                Invalidate();
                return;
            }
        }
    }

    UILabel::DoEvent(event);
}

void UIRichEdit::SetAttribute(const char* pstrName, const char* pstrValue) {
    if (strcasecmp(pstrName, "vscrollbar") == 0) {
        m_enableVerticalScrollBar = (strcasecmp(pstrValue, "true") == 0);
        if (m_enableVerticalScrollBar) {
            EnsureVerticalScrollBar();
        } else if (m_pVerticalScrollBar) {
            m_pVerticalScrollBar->SetVisible(false);
            m_pVerticalScrollBar->SetScrollPos(0, false);
            m_pVerticalScrollBar->SetScrollRange(0);
        }
        MarkLayoutDirty();
        Invalidate();
        return;
    }
    if (strcasecmp(pstrName, "vscrollbarstyle") == 0) {
        m_vScrollBarStyle = UIString{pstrValue};
        m_enableVerticalScrollBar = true;
        EnsureVerticalScrollBar();
        if (m_pVerticalScrollBar) {
            m_pVerticalScrollBar->SetAttributeList(pstrValue);
        }
        MarkLayoutDirty();
        Invalidate();
        return;
    }
    UILabel::SetAttribute(pstrName, pstrValue);
}

void CommitLine(ParagraphLayout& pl, LineLayout& line, int& cursorY) {
    if (line.segs.empty()) {
        line.height = std::max(line.height, 20);
    }

    // finalize vertical positions by baseline
    for (size_t i=0;i<line.segs.size();++i) {
        InlineSegment& s = line.segs[i];
        if (s.segType == SEG_TEXT) {
            int yTop = line.yTop + (line.baseLine - s.ascent);
            int h = s.ascent + s.descent;
            s.rc.top = yTop;
            s.rc.bottom = yTop + h;
            //SetRect(&s.rc, s.rc.left, yTop, s.rc.right, yTop + h);
        } else {
            // image bottom align to baseline (simple policy)
            const int imageHeight = std::max(0, static_cast<int>(s.rc.bottom - s.rc.top));
            int yTop = line.yTop + (line.baseLine - imageHeight);
            s.rc.top = yTop;
            s.rc.bottom = yTop + imageHeight;
            //SetRect(&s.rc, s.rc.left, yTop, s.rc.right, yTop + imageHeight);
        }
    }

    pl.lines.push_back(line);
    //cursorY += line.height + LINE_GAP;
    cursorY += line.height;
    line = LineLayout();
    line.yTop = cursorY;
}

static void LayoutTextRunInline(HANDLE_DC hdc, const TextRun & textRun, size_t runIndex,
                                int contentW,
                                int& cursorX, int& cursorY,
                                LineLayout& line, ParagraphLayout& pl,
                                int stopAfterHeight, bool* stoppedEarly) {
    int asc=0, desc=0, lineH=0;
    GetTextMetricsForStyle(hdc, textRun.style, asc, desc, lineH);

#if defined(_WIN32) || defined(WIN32)
    const std::wstring& s = textRun.text;
#else
    const std::string &s = textRun.text;
#endif
    const size_t n = s.size();
    size_t i = 0;

#if defined(_WIN32) || defined(WIN32)
    while (i < n) {
        size_t newLineBytes = ConsumeNewLine(s, i);
        if (newLineBytes > 0) {

            // explicit line break
            if (line.segs.empty()) {
                line.baseLine = std::max(line.baseLine, asc);
                line.height = std::max(line.height, asc + desc);
            }
            CommitLine(pl, line, cursorY);
            cursorX = 0;
            if (stopAfterHeight >= 0 && cursorY > stopAfterHeight) {
                if (stoppedEarly) {
                    *stoppedEarly = true;
                }
                return;
            }
            i += newLineBytes;
            continue;
        }

        int avail = contentW - cursorX;
        if (avail <= 8) {
            CommitLine(pl, line, cursorY);
            cursorX = 0;
            if (stopAfterHeight >= 0 && cursorY > stopAfterHeight) {
                if (stoppedEarly) {
                    *stoppedEarly = true;
                }
                return;
            }
            avail = contentW;
        }

        size_t cut = i;
        int bestW = 0;
        bool forceCommitAfterSoftBreak = false;
        ComputeTextSliceForLine(hdc, textRun, s, i, avail, cut, bestW, forceCommitAfterSoftBreak);

        InlineSegment seg;
        seg.segType = SEG_TEXT;
        seg.runIndex = runIndex;
        seg.startChar = i;
        seg.charLen = cut - i;
        seg.ascent = asc;
        seg.descent = desc;
        SetSegmentRect(seg.rc, cursorX, line.yTop, cursorX + bestW, line.yTop + lineH);

        line.baseLine = std::max(line.baseLine, asc);
        line.height = std::max(line.height, asc + desc);

        line.segs.push_back(seg);
        cursorX += bestW;

        i = cut;

        if (forceCommitAfterSoftBreak) {
            newLineBytes = ConsumeNewLine(s, i);
            if (newLineBytes > 0) {
                //一行占满时，正好遇到换行，这时直接跳过换行，免得多出一行空行。
                i += newLineBytes;
            }
            CommitLine(pl, line, cursorY);
            cursorX = 0;
            if (stopAfterHeight >= 0 && cursorY > stopAfterHeight) {
                if (stoppedEarly) {
                    *stoppedEarly = true;
                }
                return;
            }
        }
    }
#else
    std::vector<WrappedTextSlice> slices;
    if (textRun.text.length() == 0) {
        return;
    }
    if (cursorX != 0) {
        //一行包含多个run。每个run可能有不同的样式
        ComputeWrappedTextSlices(hdc, textRun.style, s.c_str(), s.length(),contentW-cursorX, slices);
        if (slices.size() == 0 || slices.front().width==0) {
            CommitLine(pl, line, cursorY);
        }else {
            //cursorX += slices.front().width;
            InlineSegment seg;
            seg.segType = SEG_TEXT;
            seg.runIndex = runIndex;
            seg.startChar = slices.front().startChar;
            seg.charLen = slices.front().charLen;
            seg.ascent = asc;
            seg.descent = desc;
            SetSegmentRect(seg.rc, cursorX, line.yTop, cursorX + slices.front().width, line.yTop + lineH);

            cursorX += slices.front().width;
            line.baseLine = std::max(line.baseLine, asc);
            line.height = std::max(line.height, asc + desc);
            line.segs.push_back(seg);
            if (slices.front().charLen == textRun.text.length()) {
                return; //一个run的文本已经全部放到一行中。
            }
            i += slices.front().charLen;
            CommitLine(pl,line,cursorY);
        }
    }
    ComputeWrappedTextSlices(
        hdc,
        textRun.style,
        s.c_str() + i,
        s.length(),contentW,
        slices);
    for (size_t si = 0; si < slices.size(); ++si) {
        const WrappedTextSlice& slice = slices[si];
        if (slice.charLen == 0) {
            CommitLine(pl,line,cursorY);
            continue;
        }

        InlineSegment seg;
        seg.segType = SEG_TEXT;
        seg.runIndex = runIndex;
        seg.startChar = slice.startChar + i;
        seg.charLen = slice.charLen;
        seg.ascent = asc;
        seg.descent = desc;
        SetSegmentRect(seg.rc, 0, line.yTop, contentW, line.yTop + lineH);

        line.baseLine = std::max(line.baseLine, asc);
        line.height = std::max(line.height, asc + desc);
        line.segs.push_back(seg);
        cursorX = 0;
        CommitLine(pl,line, cursorY);
        if (stopAfterHeight >= 0 && cursorY > stopAfterHeight) {
            if (stoppedEarly) {
                *stoppedEarly = true;
                return;
            }
        }
    }
#endif
}

static void LayoutImageRunInline(const ImageRun& ir, size_t runIndex,
                                 int contentW,
                                 int& cursorX, int& cursorY,
                                 LineLayout& line, ParagraphLayout& pl,
                                 int stopAfterHeight, bool* stoppedEarly) {
    int avail = contentW - cursorX;
    if (avail < ir.width && !line.segs.empty()) {
        CommitLine(pl, line, cursorY);
        cursorX = 0;
        if (stopAfterHeight >= 0 && cursorY > stopAfterHeight) {
            if (stoppedEarly) {
                *stoppedEarly = true;
            }
            return;
        }
    }

    InlineSegment seg;
    seg.segType = SEG_IMAGE;
    seg.runIndex = runIndex;
    seg.startChar = 0;
    seg.charLen = 0;
    SetSegmentRect(seg.rc, cursorX, line.yTop, cursorX + ir.width, line.yTop + ir.height);

    line.baseLine = std::max(line.baseLine, ir.height);
    line.height = std::max(line.height, ir.height);

    line.segs.push_back(seg);
    cursorX += ir.width + 2;
}

static void SetDefaultStyle(TextRun *tr, UIFont *defaultFont, uint32_t defaultTextColor) {
    if (tr->style.textColor == 0) {
        tr->style.textColor = defaultTextColor;
    }
    if (tr->style.fontFamily.empty() && (!defaultFont->GetFontName().IsEmpty())) {
        tr->SetFontFamily(defaultFont->GetFontName());
    }
    if (tr->style.fontSize == 0) {
        tr->SetFontSize(defaultFont->GetSize());
    }
}

int UIRichEdit::LayoutOneParagraph(HANDLE_DC hdc, size_t pIndex, int startY, int contentW, int stopAfterHeight, bool* stoppedEarly) {
    std::vector<ParagraphLayout> &paragraphLayout = m_documentLayouts.documentLayouts;
    ParagraphLayout& pl = paragraphLayout[pIndex];
    pl.startY = startY;
    pl.lines.clear();

    uint32_t defaultTextColor = m_textColor;
    if (defaultTextColor == 0) {
        defaultTextColor = (m_manager==nullptr ? 0xff000000 : m_manager->GetDefaultFontColor());
    }
    UIFont *font = UIResourceMgr::GetInstance().GetFont(m_fontId);

    Paragraph& para = m_document.GetParagraphAt(pIndex);
    contentW = std::max(12, contentW);

    int cursorY = startY;
    int cursorX = 0;

    LineLayout line;
    line.yTop = cursorY;

    if (para.GetRuns().empty()) {
        line.height = 20;
        CommitLine(pl, line, cursorY);
        pl.height = std::max(20, cursorY - startY);
        pl.dirty = false;
        return pl.height;
    }

    for (size_t r = 0; r < para.GetRuns().size(); ++r) {
        std::shared_ptr<RunBase>& rb = para.GetRuns()[r];
        if (!rb) continue;

        if (rb->Type() == RUN_TEXT) {
            TextRun *tr = static_cast<TextRun*>(rb.get());
            if (tr) {
                SetDefaultStyle(tr, font, defaultTextColor);
                LayoutTextRunInline(hdc, *tr, r, contentW, cursorX, cursorY, line, pl, stopAfterHeight, stoppedEarly);
                if (stoppedEarly && *stoppedEarly) {
                    pl.height = std::max(20, cursorY - startY);
                    pl.dirty = true;
                    return pl.height;
                }
            }
        } else {
            ImageRun* ir = static_cast<ImageRun*>(rb.get());
            if (ir) {
                LayoutImageRunInline(*ir, r, contentW, cursorX, cursorY, line, pl, stopAfterHeight, stoppedEarly);
                if (stoppedEarly && *stoppedEarly) {
                    pl.height = std::max(20, cursorY - startY);
                    pl.dirty = true;
                    return pl.height;
                }
            }
        }
    }

    if (!line.segs.empty()) CommitLine(pl, line, cursorY);

    pl.height = std::max(20, cursorY - startY);
    pl.dirty = false;
    return pl.height;
}

void UIRichEdit::EnsureLayoutReady(HANDLE_DC hDC) {
    const size_t paragraphCount = m_document.GetParagraphCount();
    if (m_documentLayouts.documentLayouts.size() != paragraphCount) {
        m_documentLayouts.documentLayouts.assign(paragraphCount, ParagraphLayout());
        m_layoutDirty = true;
    }

    if (GetTextViewWidth() != m_lastLayoutWidth) {
        MarkLayoutDirty();
    }

    const bool hadScrollVisibleBeforeLayout = (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible());
    bool handledHiddenToVisibleByPrepass = false;

    if (m_layoutDirty) {
        RECT baseRc = m_rcItem;
        baseRc.left += m_rcTextPadding.left;
        baseRc.top += m_rcTextPadding.top;
        baseRc.right -= m_rcTextPadding.right;
        baseRc.bottom -= m_rcTextPadding.bottom;
        if (baseRc.right < baseRc.left) baseRc.right = baseRc.left;
        if (baseRc.bottom < baseRc.top) baseRc.bottom = baseRc.top;

        const int viewHeight = std::max(0, static_cast<int>(baseRc.bottom - baseRc.top));
        const int noScrollWidth = std::max(12, static_cast<int>(baseRc.right - baseRc.left));
        const int scrollBarWidth = (m_enableVerticalScrollBar && m_pVerticalScrollBar) ? std::max(0, m_pVerticalScrollBar->GetFixedWidth()) : 0;
        const int reducedWidth = std::max(12, noScrollWidth - scrollBarWidth);

        const bool canProbeOverflow =
            (m_enableVerticalScrollBar && m_pVerticalScrollBar && !hadScrollVisibleBeforeLayout && viewHeight > 0 && scrollBarWidth > 0);

        if (canProbeOverflow) {
            bool overflowDetected = false;
            DoIncrementalRelayout(hDC, noScrollWidth, viewHeight, &overflowDetected);
            if (overflowDetected) {
                handledHiddenToVisibleByPrepass = true;
                MarkLayoutDirty();
                DoIncrementalRelayout(hDC, reducedWidth, -1, nullptr);
            }
        } else {
            DoIncrementalRelayout(hDC, std::max(12, GetTextViewWidth()), -1, nullptr);
        }
        m_layoutDirty = false;
    }

    int docBottom = 0;
    for (size_t i = 0; i < m_documentLayouts.documentLayouts.size(); ++i) {
        ParagraphLayout& pl = m_documentLayouts.documentLayouts[i];
        docBottom = std::max(docBottom, pl.startY + pl.height);
    }
    m_contentHeight = std::max(0, docBottom);

    const bool oldScrollVisible = hadScrollVisibleBeforeLayout;
    UpdateVerticalScrollBar();
    const bool newScrollVisible = (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible());

    if (oldScrollVisible != newScrollVisible) {
        const bool hiddenToVisibleAlreadyHandled = (!oldScrollVisible && newScrollVisible && handledHiddenToVisibleByPrepass);
        if (!hiddenToVisibleAlreadyHandled) {
            MarkLayoutDirty();
            DoIncrementalRelayout(hDC, std::max(12, GetTextViewWidth()), -1, nullptr);
            m_layoutDirty = false;

            docBottom = 0;
            for (size_t i = 0; i < m_documentLayouts.documentLayouts.size(); ++i) {
                ParagraphLayout& pl = m_documentLayouts.documentLayouts[i];
                docBottom = std::max(docBottom, pl.startY + pl.height);
            }
            m_contentHeight = std::max(0, docBottom);
            UpdateVerticalScrollBar();
        }
    }

    m_lastLayoutWidth = GetTextViewWidth();
}


void UIRichEdit::DoIncrementalRelayout(HANDLE_DC hdc, int contentW, int stopAfterHeight, bool* stoppedEarly) {

    if (stoppedEarly) {
        *stoppedEarly = false;
    }

    std::vector<ParagraphLayout> &paragraphLayout = m_documentLayouts.documentLayouts;
    if (paragraphLayout.size() == 0) {
        return;
    }
    size_t firstDirty = paragraphLayout.size();
    for (size_t i=0;i<paragraphLayout.size();++i) {
        if (paragraphLayout[i].dirty) { firstDirty = i; break; }
    }
    if (firstDirty == paragraphLayout.size()) return;

    int y = 0;
    if (firstDirty > 0) {
        ParagraphLayout& prev = paragraphLayout[firstDirty - 1];
        y = prev.startY + prev.height;
    }

    for (size_t i=firstDirty; i<paragraphLayout.size(); ++i) {
        ParagraphLayout& pl = paragraphLayout[i];
        if (pl.dirty) {
            LayoutOneParagraph(hdc, i, y, contentW, stopAfterHeight, stoppedEarly);
            if (stoppedEarly && *stoppedEarly) {
                break;
            }
        } else {
            int delta = y - pl.startY;
            if (delta != 0) {
                pl.startY = y;
                for (size_t li=0; li<pl.lines.size(); ++li) {
                    LineLayout& ln = pl.lines[li];
                    ln.yTop += delta;
                    for (size_t si=0; si<ln.segs.size(); ++si) {
                        ln.segs[si].rc.Offset(0,delta);
                        //OffsetRect(&ln.segs[si].rc, 0, delta);
                    }
                }
            }
        }
        y = paragraphLayout[i].startY + paragraphLayout[i].height;
        if (stopAfterHeight >= 0 && y > stopAfterHeight) {
            if (stoppedEarly) {
                *stoppedEarly = true;
            }
            break;
        }
    }
}

static void DrawTextSegment(HANDLE_DC hdc, size_t paragraphIndex, const InlineSegment& seg,std::vector<Paragraph>& paragraphs) {
    //OffsetRect(&seg.rc, 0, -scrollY);
    //if (rc.bottom < 0 || rc.top > viewH) return;

    if (paragraphIndex >=paragraphs.size()) return;
    Paragraph& para = paragraphs[paragraphIndex];
    if (seg.runIndex >= para.GetRuns().size()) return;
    const TextRun* tr = static_cast<const TextRun*>(para.GetRuns()[seg.runIndex].get());
    if (!tr) return;

    size_t start = std::min(seg.startChar, tr->text.size());
    size_t len = std::min(seg.charLen, tr->text.size() - start);
    auto token = tr->text.substr(start, len);
    DrawTextRunSegment(hdc, tr->style, seg.rc, token);
}

static const ImageRun* GetImageRun(Paragraph& para, size_t runIndex) {
    if (runIndex >= para.GetRuns().size()) {
        return nullptr;
    }
    const std::shared_ptr<RunBase>& run = para.GetRuns()[runIndex];
    if (!run || run->Type() != RUN_IMAGE) {
        return nullptr;
    }
    return static_cast<const ImageRun*>(run.get());
}

void UIRichEdit::PaintText(HANDLE_DC hDC) {
    EnsureLayoutReady(hDC);

    const int scrollY = (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) ? m_pVerticalScrollBar->GetScrollPos() : 0;
    RECT viewRc = GetTextViewRect();
    UIRenderClip clip;
    UIRenderClip::GenerateClip(hDC, viewRc, clip);
    std::vector<Paragraph>& paragraphs = m_document.GetParagraphs();

    for (size_t p = 0; p < m_documentLayouts.documentLayouts.size(); ++p) {
        ParagraphLayout& pl = m_documentLayouts.documentLayouts[p];
        const int pTop = pl.startY - scrollY;
        const int pBottom = pTop + pl.height;
        if (pBottom <= 0 || pTop > viewRc.bottom - viewRc.top) {
            continue;
        }
        for (size_t li = 0; li < pl.lines.size(); ++li) {
            LineLayout& lineLayout = pl.lines[li];
            const int lTop = lineLayout.yTop - scrollY;
            const int lBottom = lTop + lineLayout.height;
            if (lBottom <= 0 || lTop > viewRc.bottom - viewRc.top) {
                continue;
            }
            for (size_t si = 0; si < lineLayout.segs.size(); ++si) {
                InlineSegment& seg = lineLayout.segs[si];
                UIRect drawRc{seg.rc};
                drawRc.Offset(viewRc.left, viewRc.top-scrollY);
                if (drawRc.bottom < viewRc.top || drawRc.top > viewRc.bottom) {
                    continue;
                }
                if (seg.segType == SEG_TEXT) {
                    InlineSegment drawSeg = seg;
                    drawSeg.rc = drawRc;
                    DrawTextSegment(hDC, p, drawSeg, paragraphs);
                } else {
                    if (p >= paragraphs.size()) {
                        continue;
                    }
                    Paragraph& para = paragraphs[p];
                    const ImageRun* imageRun = GetImageRun(para, seg.runIndex);
                    if (!imageRun) {
                        continue;
                    }
                    TDrawInfo info;
                    info.sDrawString = UIString{imageRun->id.c_str()};
                    UIRenderEngine::DrawImage(hDC, drawRc, m_rcPaint, info);
                }
            }
        }
    }

    if (IsFocused() && m_caret && m_caret->HasCaret()) {
        const uint32_t caretColor = (m_textColor != 0) ? m_textColor : (m_manager ? m_manager->GetDefaultFontColor() : 0xFF000000);
        m_caret->Paint(hDC, viewRc, scrollY, caretColor);
    }
}

bool UIRichEdit::DoPaint(HANDLE_DC hDC, const RECT& rcPaint, UIControl* pStopControl) {
    if (!UIControl::DoPaint(hDC, rcPaint, pStopControl)) {
        return false;
    }
    if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
        RECT rcTemp = {0};
        if (::UIIntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos())) {
            if (!m_pVerticalScrollBar->Paint(hDC, rcPaint, pStopControl)) {
                return false;
            }
        }
    }
    return true;
}

SIZE UIRichEdit::EstimateSize(SIZE szAvailable) {
    return m_cxyFixed;
}

void UIRichEdit::AppendParagraph(const Paragraph& paragraph) {
    m_document.AppendParagraph(paragraph);
    if (m_caret) {
        m_caret->Clear();
    }
    m_layoutDirty = true;
    Invalidate();
}
