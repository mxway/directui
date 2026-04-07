//
// Created by mxllcy on 2026/3/16.
//

#ifndef DIRECTUI_UIRICHEDITINTERNAL_IMPL_H
#define DIRECTUI_UIRICHEDITINTERNAL_IMPL_H
#include "UIRichEdit.h"

//int MeasureTextWidth(HANDLE_DC hdc, const TextStyle& st, const std::wstring& text);

//void GetTextMetricsForStyle(HANDLE_DC hdc, const TextStyle& st, int& ascent, int& descent, int& lineHeight);

HANDLE_FONT CreateFontFromStyle(const TextStyle& s);
void GetTextMetricsForStyle(HANDLE_DC hdc, const TextStyle& st, int& ascent, int& descent, int& lineHeight);

#ifdef _WIN32
bool IsNewLine(wchar_t ch);
bool IsBreakable(wchar_t c) ;
size_t ConsumeNewLine(const std::wstring& text, size_t index);
size_t NextTextUnit(const std::wstring& text, size_t index);
bool IsBreakableAt(const std::wstring& text, size_t index);
int MeasureTextWidth(HANDLE_DC hdc, const TextStyle& st, const std::wstring& text);
int MeasureTextWidthRange(HANDLE_DC hdc, const TextStyle& st, const wchar_t* text, int length);
int GetTextFitCount(HANDLE_DC hdc, const TextStyle& st, const wchar_t* text, int length, int maxWidth);
int GetTextFitMetrics(HANDLE_DC hdc, const TextStyle& st, const wchar_t* text, int length, int maxWidth,
					  int* fitWidth);
void DrawTextRunSegment(HANDLE_DC hdc, const TextStyle& st, const UIRect& rc, const std::wstring& text);
#else
struct WrappedTextSlice {
	size_t startChar;
	size_t charLen;
	int width;
	WrappedTextSlice() : startChar(0), charLen(0), width(0) {}
	WrappedTextSlice(size_t start, size_t len, int w) : startChar(start), charLen(len), width(w) {}
};

bool IsNewLine(char ch);
bool IsBreakable(char c);
bool IsBreakableAt(const std::string& text, size_t index);
size_t ConsumeNewLine(const std::string& text, size_t index);
size_t NextTextUnit(const std::string& text, size_t index);

int MeasureTextWidth(HANDLE_DC hdc, const TextStyle& st, const std::string& text);
int MeasureTextWidthRange(HANDLE_DC hdc, const TextStyle& st, const char* text, int length);
int GetTextFitMetrics(HANDLE_DC hdc, const TextStyle& st, const char* text, int length, int maxWidth,
					  int* fitWidth);
void ComputeWrappedTextSlices(HANDLE_DC hdc,
							  const TextStyle& st,
							  const char* text,
							  int length,
							  int lineWidth,
							  std::vector<WrappedTextSlice>& outSlices);
void DrawTextRunSegment(HANDLE_DC hdc, const TextStyle& st, const UIRect& rc, const std::string& text);
#endif



#endif //DIRECTUI_UIRICHEDITINTERNAL_IMPL_H