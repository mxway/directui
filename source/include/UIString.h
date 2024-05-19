#ifndef DIRECTUI_UISTRING_H
#define DIRECTUI_UISTRING_H
#include <cstdint>
#include <cstdarg>

class UIString
{
    static const uint32_t MAX_LOCAL_STRING_LEN = 63;
public:
    UIString();
    ~UIString();
    explicit UIString(char c);
    explicit UIString(const char *str,int len=-1);
    UIString(const UIString &other);
    UIString(UIString &&other) noexcept;

    const char* GetData()const;
    bool IsEmpty()const;
    void    Empty();
    int GetLength()const;

    char GetAt(int nIndex) const;
    char    operator[](int index)const;
    UIString &Append(const char *str);
    UIString &Append(char c);
    UIString &Append(const UIString &other);

    static UIString ToUIString(int64_t intValue);

    void Assign(const char * pstr, int nLength = -1);

    UIString operator+(const char *str)const;
    UIString operator+(char c)const;
    UIString operator+(const UIString &other)const;

    UIString &operator=(const char *str);
    UIString &operator=(char c);
    UIString &operator=(const UIString &other);
    UIString &operator=(UIString &&other) noexcept ;

    UIString &operator+=(const char *str);
    UIString &operator+=(char c);
    UIString &operator+=(const UIString &other);

    bool operator==(const UIString &other)const;
    bool operator==(const char *str)const;
    bool operator!=(const UIString &other)const;
    bool operator!=(const char *str)const;
    int    CompareNoCase(const UIString &other)const;

    void MakeUpper();
    void MakeLower();

    UIString Left(int length)const;
    UIString Right(int length)const;
    UIString Mid(int pos, int length=-1)const;

    int     Find(const char *subString,int pos=0)const;
    int     Find(char c, int pos=0)const;
    int     Find(const UIString &subString, int pos=0)const;
    int     ReverseFind(const char *subString)const;

    int     Replace(const char *fromString, const char *toString);

    int     Format(const char *formatString,...);

private:
    char *m_pstr;
    char m_szBuffer[MAX_LOCAL_STRING_LEN + 1];
};

#endif //DIRECTUI_UISTRING_H
