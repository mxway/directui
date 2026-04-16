#include "UIRichEditCaret.h"

#include <algorithm>
#include <limits>

#include <UIRenderEngine.h>

#include "UIRichEditInternal_impl.h"

namespace {

void SetCaretRect(UIRect& rect, int x, int top, int bottom) {
    rect.left = x;
    rect.right = x + 1;
    rect.top = top;
    rect.bottom = std::max(top + 1, bottom);
}

int DistanceToRange(int value, int start, int end) {
    if (value < start) {
        return start - value;
    }
    if (value > end) {
        return value - end;
    }
    return 0;
}

TextRun* GetTextRun(Paragraph& paragraph, size_t runIndex) {
    if (runIndex >= paragraph.GetRuns().size()) {
        return nullptr;
    }
    std::shared_ptr<RunBase>& run = paragraph.GetRuns()[runIndex];
    if (!run || run->Type() != RUN_TEXT) {
        return nullptr;
    }
    return static_cast<TextRun*>(run.get());
}

bool FindNearestLine(const std::vector<ParagraphLayout>& layouts,
                     int y,
                     size_t& paragraphIndex,
                     size_t& lineIndex) {
    int bestDistance = std::numeric_limits<int>::max();
    bool found = false;
    for (size_t p = 0; p < layouts.size(); ++p) {
        const ParagraphLayout& paragraphLayout = layouts[p];
        for (size_t li = 0; li < paragraphLayout.lines.size(); ++li) {
            const LineLayout& line = paragraphLayout.lines[li];
            const int lineTop = line.yTop;
            const int lineBottom = line.yTop + std::max(1, line.height);
            const int distance = DistanceToRange(y, lineTop, lineBottom);
            if (distance < bestDistance) {
                bestDistance = distance;
                paragraphIndex = p;
                lineIndex = li;
                found = true;
                if (distance == 0) {
                    return true;
                }
            }
        }
    }
    return found;
}

} // namespace

UIRichEditCaret::UIRichEditCaret()
    : m_placement(),
      m_visible(false) {
}

void UIRichEditCaret::Clear() {
    m_placement = Placement();
    m_visible = false;
}

bool UIRichEditCaret::HasCaret() const {
    return m_placement.valid;
}

void UIRichEditCaret::SetVisible(bool visible) {
    m_visible = visible;
}

bool UIRichEditCaret::IsVisible() const {
    return m_visible;
}

bool UIRichEditCaret::HitTest(HANDLE_DC hdc,
                              const POINT& documentPoint,
                              std::vector<Paragraph>& paragraphs,
                              const std::vector<ParagraphLayout>& layouts) {
    if (hdc == nullptr || layouts.empty() || paragraphs.empty()) {
        Clear();
        return false;
    }

    size_t paragraphIndex = 0;
    size_t lineIndex = 0;
    if (!FindNearestLine(layouts, documentPoint.y, paragraphIndex, lineIndex)) {
        Clear();
        return false;
    }
    if (paragraphIndex >= paragraphs.size() || paragraphIndex >= layouts.size()) {
        Clear();
        return false;
    }

    ParagraphLayout const& paragraphLayout = layouts[paragraphIndex];
    if (lineIndex >= paragraphLayout.lines.size()) {
        Clear();
        return false;
    }

    return PlaceCaretAtLine(hdc,
                            documentPoint.x,
                            paragraphIndex,
                            lineIndex,
                            paragraphs[paragraphIndex],
                            paragraphLayout.lines[lineIndex]);
}

void UIRichEditCaret::Paint(HANDLE_DC hdc, const RECT& viewRc, int scrollY, uint32_t color) const {
    if (!m_visible || !m_placement.valid) {
        return;
    }
    UIRect drawRc = m_placement.rect;
    drawRc.Offset(viewRc.left, viewRc.top - scrollY);
    if (drawRc.right <= viewRc.left || drawRc.left >= viewRc.right ||
        drawRc.bottom <= viewRc.top || drawRc.top >= viewRc.bottom) {
        return;
    }

    RECT lineRc = {drawRc.left, drawRc.top, drawRc.left, drawRc.bottom};
    UIRenderEngine::DrawLine(hdc, lineRc, 1, color, 0);
}

bool UIRichEditCaret::PlaceCaretAtLine(HANDLE_DC hdc,
                                       int x,
                                       size_t paragraphIndex,
                                       size_t lineIndex,
                                       Paragraph& paragraph,
                                       const LineLayout& line) {
    if (line.segs.empty()) {
        Placement placement;
        placement.valid = true;
        placement.paragraphIndex = paragraphIndex;
        placement.lineIndex = lineIndex;
        placement.runIndex = 0;
        placement.charIndex = 0;
        SetCaretRect(placement.rect, 0, line.yTop, line.yTop + std::max(1, line.height));
        ApplyPlacement(placement);
        return true;
    }

    const InlineSegment& firstSeg = line.segs.front();
    if (x <= firstSeg.rc.left) {
        return PlaceCaretBeforeSegment(paragraphIndex, lineIndex, 0, firstSeg, line);
    }

    for (size_t si = 0; si < line.segs.size(); ++si) {
        const InlineSegment& seg = line.segs[si];
        if (x < seg.rc.left) {
            return PlaceCaretBeforeSegment(paragraphIndex, lineIndex, si, seg, line);
        }

        if (seg.segType == SEG_TEXT && x <= seg.rc.right) {
            TextRun* textRun = GetTextRun(paragraph, seg.runIndex);
            if (textRun != nullptr) {
                return PlaceCaretInsideText(hdc, paragraphIndex, lineIndex, si, *textRun, seg, x);
            }
            return PlaceCaretBeforeSegment(paragraphIndex, lineIndex, si, seg, line);
        }

        if (seg.segType == SEG_IMAGE && x <= seg.rc.right) {
            const int midX = seg.rc.left + (seg.rc.right - seg.rc.left) / 2;
            if (x <= midX) {
                return PlaceCaretBeforeSegment(paragraphIndex, lineIndex, si, seg, line);
            }
            return PlaceCaretAfterSegment(paragraphIndex, lineIndex, si, paragraph, seg, line);
        }

        if (si + 1 < line.segs.size()) {
            const InlineSegment& nextSeg = line.segs[si + 1];
            if (x < nextSeg.rc.left) {
                const int midGap = seg.rc.right + (nextSeg.rc.left - seg.rc.right) / 2;
                if (x <= midGap) {
                    return PlaceCaretAfterSegment(paragraphIndex, lineIndex, si, paragraph, seg, line);
                }
                return PlaceCaretBeforeSegment(paragraphIndex, lineIndex, si + 1, nextSeg, line);
            }
        }
    }

    return PlaceCaretAfterSegment(paragraphIndex,
                                  lineIndex,
                                  line.segs.size() - 1,
                                  paragraph,
                                  line.segs.back(),
                                  line);
}

bool UIRichEditCaret::PlaceCaretInsideText(HANDLE_DC hdc,
                                           size_t paragraphIndex,
                                           size_t lineIndex,
                                           size_t segIndex,
                                           const TextRun& textRun,
                                           const InlineSegment& seg,
                                           int x) {
    if (seg.startChar > textRun.text.size()) {
        return false;
    }

    const size_t maxLen = textRun.text.size() - seg.startChar;
    const size_t segLen = std::min(seg.charLen, maxLen);
    const int relativeX = std::max(0, x - static_cast<int>(seg.rc.left));
    int caretOffsetX = 0;
    const size_t relCharIndex = HitTestTextCaret(hdc,
                                                 textRun.style,
                                                 textRun.text.c_str() + seg.startChar,
                                                 static_cast<int>(segLen),
                                                 relativeX,
                                                 &caretOffsetX);

    Placement placement;
    placement.valid = true;
    placement.paragraphIndex = paragraphIndex;
    placement.lineIndex = lineIndex;
    placement.segIndex = segIndex;
    placement.runIndex = seg.runIndex;
    placement.charIndex = seg.startChar + std::min(segLen, relCharIndex);
    SetCaretRect(placement.rect, seg.rc.left + caretOffsetX, seg.rc.top, seg.rc.bottom);
    ApplyPlacement(placement);
    return true;
}

bool UIRichEditCaret::PlaceCaretBeforeSegment(size_t paragraphIndex,
                                              size_t lineIndex,
                                              size_t segIndex,
                                              const InlineSegment& seg,
                                              const LineLayout& line) {
    Placement placement;
    placement.valid = true;
    placement.paragraphIndex = paragraphIndex;
    placement.lineIndex = lineIndex;
    placement.segIndex = segIndex;
    placement.runIndex = seg.runIndex;
    placement.afterImage = false;

    int top = line.yTop;
    int bottom = line.yTop + std::max(1, line.height);
    if (seg.segType == SEG_TEXT) {
        placement.charIndex = seg.startChar;
        top = seg.rc.top;
        bottom = seg.rc.bottom;
    }
    SetCaretRect(placement.rect, seg.rc.left, top, bottom);
    ApplyPlacement(placement);
    return true;
}

bool UIRichEditCaret::PlaceCaretAfterSegment(size_t paragraphIndex,
                                             size_t lineIndex,
                                             size_t segIndex,
                                             Paragraph& paragraph,
                                             const InlineSegment& seg,
                                             const LineLayout& line) {
    Placement placement;
    placement.valid = true;
    placement.paragraphIndex = paragraphIndex;
    placement.lineIndex = lineIndex;
    placement.segIndex = segIndex;
    placement.runIndex = seg.runIndex;

    int top = line.yTop;
    int bottom = line.yTop + std::max(1, line.height);
    if (seg.segType == SEG_TEXT) {
        TextRun* textRun = GetTextRun(paragraph, seg.runIndex);
        if (textRun != nullptr) {
            const size_t runSize = textRun->text.size();
            placement.charIndex = std::min(runSize, seg.startChar + seg.charLen);
        } else {
            placement.charIndex = seg.startChar + seg.charLen;
        }
        top = seg.rc.top;
        bottom = seg.rc.bottom;
    } else {
        placement.afterImage = true;
    }
    SetCaretRect(placement.rect, seg.rc.right, top, bottom);
    ApplyPlacement(placement);
    return true;
}

void UIRichEditCaret::ApplyPlacement(const Placement& placement) {
    m_placement = placement;
    m_visible = placement.valid;
}

