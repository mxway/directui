#ifndef DIRECTUI_UIRICHEDIT_H
#define DIRECTUI_UIRICHEDIT_H
#include <string>
#include <UILabel.h>
#include <vector>
#include <memory>
#include <stdexcept>

#include "UIRect.h"

using namespace std;

enum RunType{RUN_TEXT=1,RUN_IMAGE=2};

class RunBase {
public:
    virtual ~RunBase() {}
    virtual RunType Type() const = 0;
};

struct TextStyle {
#if defined(_WIN32) || defined(WIN32)
    std::wstring fontFamily;
#else
    std::string  fontFamily;
#endif
    int          fontSize;
    bool         bold;
    bool         italic;
    uint32_t     textColor;
    int          lineExtra;
    TextStyle()
        : fontFamily(),
            fontSize(12),
            bold(false),
            italic(false),
            textColor(0xFF000000),
            lineExtra(12) {}
};

class TextRun : public RunBase {
public:
#if defined(_WIN32) || defined(WIN32)
    std::wstring text;
#else
    std::string text;
#endif

    TextStyle    style;

    RunType Type() const override {
        return RunType::RUN_TEXT;
    }
    void        SetFontFamily(const UIString& fontFamily) ;
    void        SetFontSize(int fontSize) {
        style.fontSize = fontSize;
    }
    void        SetBold(bool bold) {
        style.bold = bold;
    }
    void        SetItalic(bool italic) {
        style.italic = italic;
    }
    void        SetTextColor(uint32_t color) {
        style.textColor = color;
    }
    void        SetLineExtra(int extra) {
        style.lineExtra = extra;
    }
    void        SetText(const UIString& utf8Text);
};

class ImageRun : public RunBase {
public:
    std::string    id;
    int             width;
    int             height;
    ImageRun()
        :id{""},
        width{1},
        height{1} {
    }

    RunType Type() const override {
        return RunType::RUN_IMAGE;
    }
};

class Paragraph {
public:
    void    AppendRun(std::shared_ptr<RunBase> run) {
        m_runs.push_back(run);
    }
    std::vector<std::shared_ptr<RunBase>> &GetRuns() {
        return m_runs;
    };
private:
    std::vector<std::shared_ptr<RunBase>> m_runs;
};

class RichDocument {
public:
    void    AppendParagraph(const Paragraph& para) {
        m_paragraphs.push_back(para);
    }
    void    ClearParagraphs() {
        m_paragraphs.clear();
    }
    uint32_t    GetParagraphCount() const {
        return m_paragraphs.size();
    }
    Paragraph   &GetParagraphAt(size_t index) {
        if (index < 0 || index >= m_paragraphs.size()) {
            throw std::out_of_range("paragraph index out of range");
        }
        return m_paragraphs[index];
    }
    std::vector<Paragraph> &GetParagraphs() {
        return m_paragraphs;
    }
private:
    std::vector<Paragraph> m_paragraphs;
};

// --------------------- layout structures ---------------------
enum SegmentType { SEG_TEXT = 1, SEG_IMAGE = 2 };

struct InlineSegment {
    SegmentType segType;
    size_t runIndex;
    size_t startChar;   // text range: UTF-16 code units on Win32, UTF-8 byte offset on Linux
    size_t charLen;     // text range length in the same unit as startChar
    UIRect rc;            // document coordinate
    int     ascent;
    int     descent;
    InlineSegment()
        : segType(SEG_TEXT), runIndex(0), startChar(0), charLen(0),
          ascent(0), descent(0) {
        rc.left = rc.right = rc.top = rc.bottom = 0;
    }
};

struct LineLayout {
    int yTop;
    int height;
    int baseLine;
    std::vector<InlineSegment> segs;
    LineLayout():
        yTop{0},
        height {0},
        baseLine{0} {

    }
};

struct ParagraphLayout {
    int         startY;
    int         height;
    bool        dirty;
    std::vector<LineLayout> lines;
    ParagraphLayout():startY{0},height{0},dirty{true} {

    }
    LineLayout  &GetLineLayoutAt(int index) {
        if (index < 0 || index >= lines.size()) {
            throw std::out_of_range("line index out of range");
        }
        return lines[index];
    }
};

struct RichDocumentLayout {
    std::vector<ParagraphLayout> documentLayouts;
};

class UIRichEdit : public UILabel {
public:
    UIRichEdit();
    ~UIRichEdit() override;

    UIString GetClass() const override;

    LPVOID GetInterface(const UIString &name) override;

    void SetText(const UIString& text) override;

    void PaintText(HANDLE_DC hDC) override;

    SIZE EstimateSize(SIZE szAvailable) override;

    void AppendParagraph(const Paragraph& paragraph);

private:
    void    DoIncrementalRelayout(HANDLE_DC hDC);
    int     LayoutOneParagraph(HANDLE_DC hdc, size_t pIndex, int startY);
    //void    CommitLine(ParagraphLayout& pl, LineLayout& line, int& cursorY);
protected:
    RichDocument m_document;
    RichDocumentLayout m_documentLayouts;
};

#endif //DIRECTUI_UIRICHEDIT_H