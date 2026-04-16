#ifndef DIRECTUI_UIRICHEDITCARET_H
#define DIRECTUI_UIRICHEDITCARET_H

#include <UIRichEdit.h>

class UIRichEditCaret {
public:
    UIRichEditCaret();

    void Clear();
    bool HasCaret() const;
    void SetVisible(bool visible);
    bool IsVisible() const;

    bool HitTest(HANDLE_DC hdc,
                 const POINT& documentPoint,
                 std::vector<Paragraph>& paragraphs,
                 const std::vector<ParagraphLayout>& layouts);

    void Paint(HANDLE_DC hdc, const RECT& viewRc, int scrollY, uint32_t color) const;

private:
    struct Placement {
        bool valid;
        size_t paragraphIndex;
        size_t lineIndex;
        size_t segIndex;
        size_t runIndex;
        size_t charIndex;
        bool afterImage;
        UIRect rect;

        Placement()
            : valid(false),
              paragraphIndex(0),
              lineIndex(0),
              segIndex(0),
              runIndex(0),
              charIndex(0),
              afterImage(false),
              rect() {
        }
    };

    bool PlaceCaretAtLine(HANDLE_DC hdc,
                          int x,
                          size_t paragraphIndex,
                          size_t lineIndex,
                          Paragraph& paragraph,
                          const LineLayout& line);
    bool PlaceCaretInsideText(HANDLE_DC hdc,
                              size_t paragraphIndex,
                              size_t lineIndex,
                              size_t segIndex,
                              const TextRun& textRun,
                              const InlineSegment& seg,
                              int x);
    bool PlaceCaretBeforeSegment(size_t paragraphIndex,
                                 size_t lineIndex,
                                 size_t segIndex,
                                 const InlineSegment& seg,
                                 const LineLayout& line);
    bool PlaceCaretAfterSegment(size_t paragraphIndex,
                                size_t lineIndex,
                                size_t segIndex,
                                Paragraph& paragraph,
                                const InlineSegment& seg,
                                const LineLayout& line);
    void ApplyPlacement(const Placement& placement);

private:
    Placement m_placement;
    bool m_visible;
};

#endif //DIRECTUI_UIRICHEDITCARET_H

