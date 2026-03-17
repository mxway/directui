#include <UIRichEdit.h>
#include "UIRichEditInternal_impl.h"
#include "UIRenderEngine.h"

void SetSegmentRect(UIRect& rect, int left, int top, int right, int bottom) {
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
}

UIRichEdit::UIRichEdit() {
}

UIRichEdit::~UIRichEdit() {
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
    Paragraph   paragraph;
    auto textRun = make_shared<TextRun>();
    textRun->SetText(text);
    textRun->style.textColor = m_textColor;
    paragraph.AppendRun(textRun);
    m_document.AppendParagraph(paragraph);
    Invalidate();
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
                                int contentX, int contentW,
                                int& cursorX, int& cursorY,
                                LineLayout& line, ParagraphLayout& pl) {
    int asc=0, desc=0, lineH=0;
    GetTextMetricsForStyle(hdc, textRun.style, asc, desc, lineH);

#if defined(_WIN32) || defined(WIN32)
    const std::wstring& s = textRun.text;
#else
    const std::string &s = textRun.text;
#endif
    const size_t n = s.size();
    size_t i = 0;

    while (i < n) {
        size_t newLineBytes = ConsumeNewLine(s, i);
        if (newLineBytes > 0) {

            // explicit line break
            if (line.segs.empty()) {
                line.baseLine = std::max(line.baseLine, asc);
                line.height = std::max(line.height, asc + desc);
            }
            CommitLine(pl, line, cursorY);
            cursorX = contentX;
            i += newLineBytes;
            continue;
        }

        int avail = contentX + contentW - cursorX;
        if (avail <= 8) {
            CommitLine(pl, line, cursorY);
            cursorX = contentX;
            avail = contentW;
        }

        // greedy fit char range
        size_t j = i;
        size_t lastBreak = (size_t)-1;
        int bestW = 0;

        while (j < n && ConsumeNewLine(s, j) == 0) {
            const size_t next = NextTextUnit(s, j);
            if (next <= j) {
                break;
            }

            if (IsBreakableAt(s, j)) {
                lastBreak = next;
            }

            auto piece = s.substr(i, next - i);
            int w = MeasureTextWidth(hdc, textRun.style, piece);
            if (w <= avail) {
                bestW = w;
                j = next;
            } else {
                break;
            }
        }

        size_t cut = j;
        if (cut == i) {
            // at least one char
            cut = NextTextUnit(s, i);
            auto piece = s.substr(i, cut - i);
            bestW = MeasureTextWidth(hdc, textRun.style, piece);
        } else if (cut < n && ConsumeNewLine(s, cut) == 0 && lastBreak != (size_t)-1 && lastBreak > i) {
            cut = lastBreak;
            auto piece = s.substr(i, cut-i);
            bestW = MeasureTextWidth(hdc, textRun.style, piece);
        }

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

        // if next is newline, consume and commit line

        newLineBytes = ConsumeNewLine(s, i);
        if (newLineBytes > 0) {
            i += newLineBytes;
            CommitLine(pl, line, cursorY);
            cursorX = contentX;
        }
    }
}

static void LayoutImageRunInline(const ImageRun& ir, size_t runIndex,
                                 int contentX, int contentW,
                                 int& cursorX, int& cursorY,
                                 LineLayout& line, ParagraphLayout& pl) {
    int avail = contentX + contentW - cursorX;
    if (avail < ir.width && !line.segs.empty()) {
        CommitLine(pl, line, cursorY);
        cursorX = contentX;
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

int UIRichEdit::LayoutOneParagraph(HANDLE_DC hdc, size_t pIndex, int startY) {
    std::vector<ParagraphLayout> &paragraphLayout = m_documentLayouts.documentLayouts;
    ParagraphLayout& pl = paragraphLayout[pIndex];
    pl.startY = startY;
    pl.lines.clear();

    Paragraph& para = m_document.GetParagraphAt(pIndex);
    int contentX = m_rcItem.left + m_rcTextPadding.left;
    int contentW = std::max(12, (int)(m_rcItem.right - m_rcItem.left - m_rcTextPadding.left - m_rcTextPadding.right));

    int cursorY = startY;
    int cursorX = contentX;

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
            if (tr) LayoutTextRunInline(hdc, *tr, r, contentX, contentW, cursorX, cursorY, line, pl);
        } else {
            ImageRun* ir = static_cast<ImageRun*>(rb.get());
            if (ir) LayoutImageRunInline(*ir, r, contentX, contentW, cursorX, cursorY, line, pl);
        }
    }

    if (!line.segs.empty()) CommitLine(pl, line, cursorY);

    pl.height = std::max(20, cursorY - startY);
    pl.dirty = false;
    return pl.height;
}

void UIRichEdit::DoIncrementalRelayout(HANDLE_DC hdc) {

    std::vector<ParagraphLayout> &paragraphLayout = m_documentLayouts.documentLayouts;
    if (paragraphLayout.size() == 0) {
        return;
    }
    size_t firstDirty = paragraphLayout.size();
    for (size_t i=0;i<paragraphLayout.size();++i) {
        if (paragraphLayout[i].dirty) { firstDirty = i; break; }
    }
    if (firstDirty == paragraphLayout.size()) return;

    int y = m_rcItem.top + m_rcTextPadding.top;
    if (firstDirty > 0) {
        ParagraphLayout& prev = paragraphLayout[firstDirty - 1];
        y = prev.startY + prev.height;
    }

    for (size_t i=firstDirty; i<paragraphLayout.size(); ++i) {
        ParagraphLayout& pl = paragraphLayout[i];
        if (pl.dirty) {
            LayoutOneParagraph(hdc, i, y);
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
    m_documentLayouts.documentLayouts.assign(m_document.GetParagraphCount(), ParagraphLayout());
    DoIncrementalRelayout(hDC);
    for (size_t p=0; p<m_documentLayouts.documentLayouts.size(); ++p) {
        ParagraphLayout& pl = m_documentLayouts.documentLayouts[p];
        //if (pBot < viewTop || pTop > viewBottom) continue;
        for (size_t li = 0; li<pl.lines.size(); ++li) {
            LineLayout& lineLayout = pl.lines[li];
            //if (lBot < viewTop || lTop > viewBottom)continue;
            for (size_t si=0;si<lineLayout.segs.size();++si) {
                InlineSegment &seg = lineLayout.segs[si];
                if (seg.segType == SEG_TEXT) {
                    DrawTextSegment(hDC,p, seg,m_document.GetParagraphs());
                }else {
                    if (p >= m_document.GetParagraphs().size()) {
                        continue;
                    }
                    Paragraph& para = m_document.GetParagraphs()[p];
                    const ImageRun* imageRun = GetImageRun(para, seg.runIndex);
                    if (!imageRun) {
                        continue;
                    }
                    TDrawInfo info;
                    info.sDrawString = UIString{imageRun->id.c_str()};
                    UIRenderEngine::DrawImage(hDC,m_manager,seg.rc,m_rcPaint,info);
                }
            }
        }
    }
}

SIZE UIRichEdit::EstimateSize(SIZE szAvailable) {
    return m_cxyFixed;
}

void UIRichEdit::AppendParagraph(const Paragraph& paragraph) {
    m_document.AppendParagraph(paragraph);
    Invalidate();
}
