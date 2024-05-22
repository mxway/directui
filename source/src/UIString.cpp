#include <UIString.h>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <cstring>
#include "UIDefine.h"

UIString::UIString()
    :m_pstr {m_szBuffer},
    m_szBuffer {0}
{

}

UIString::~UIString() {
    if(m_pstr != m_szBuffer){
        delete []m_pstr;
    }
}

UIString::UIString(char c)
    : UIString()
{
    m_szBuffer[0] = c;
    m_szBuffer[1] = 0;
}

UIString::UIString(const char *str, int len)
    :UIString()
{
    Assign(str, len);
}

UIString::UIString(const UIString &other)
    :UIString()
{
    Assign(other.m_pstr);
}

UIString::UIString(UIString &&other)noexcept
    :UIString()
{
    if(other.GetLength() >= MAX_LOCAL_STRING_LEN){
        m_pstr = other.m_pstr;
    }else{
        memcpy(m_pstr, other.m_pstr, other.GetLength()+1);
    }
    other.m_pstr = other.m_szBuffer;
    other.m_pstr[0] = '\0';
}

bool UIString::IsEmpty() const {
    return m_pstr[0] == '\0';
}

void UIString::Empty() {
    if( m_pstr != m_szBuffer )
    {
        delete []m_pstr;
    }
    m_pstr = m_szBuffer;
    m_pstr[0] = '\0';
}

const char *UIString::GetData() const {
    return m_pstr;
}

int UIString::GetLength() const {
    return (int)strlen(m_pstr);
}

char UIString::GetAt(int nIndex) const {
    if(nIndex >= (int)strlen(m_pstr)){
        throw std::out_of_range("Invalid Index");
    }
    return m_pstr[nIndex];
}

char UIString::operator[](int index) const {
    return this->GetAt(index);
}

void UIString::Assign(const char *pstr, int nLength) {
    if( pstr == nullptr ) pstr = "";
    nLength = (nLength < 0 ? (int) strlen(pstr) : nLength);
    if( nLength < MAX_LOCAL_STRING_LEN ) {
        if( m_pstr != m_szBuffer ) {
            delete []m_pstr;
            m_pstr = m_szBuffer;
        }
    }
    else if( nLength > GetLength() || m_pstr == m_szBuffer ) {
        if( m_pstr == m_szBuffer ) m_pstr = nullptr;
        char *newBuffer = new char[nLength+1];
        delete []m_pstr;
        m_pstr = newBuffer;
    }
    memcpy(m_pstr,pstr,nLength);
    m_pstr[nLength] = '\0';
}

UIString &UIString::Append(const char *str) {
    if(str == nullptr){
        return *this;
    }
    int nNewLength = GetLength() + (int) strlen(str);
    if( nNewLength >= MAX_LOCAL_STRING_LEN ) {
        if( m_pstr == m_szBuffer ) {
            uint32_t    originStringLen = strlen(m_szBuffer);
            m_pstr = new char[nNewLength+1];
            memset(m_pstr, 0, nNewLength+1);
            memcpy(m_pstr,m_szBuffer,originStringLen);
            memcpy(m_pstr + originStringLen,str, strlen(str)+1);
        }
        else {
            char *newBuffer = new char[nNewLength + 1];
            memset(newBuffer, 0, nNewLength+1);
            memcpy(newBuffer, m_pstr, GetLength());
            memcpy(newBuffer + GetLength(), str, strlen(str)+1);
            delete []m_pstr;
            m_pstr = newBuffer;
        }
    }
    else {
        if( m_pstr != m_szBuffer ) {
            delete []m_pstr;
            m_pstr = m_szBuffer;
        }
        memcpy(m_szBuffer+GetLength(),str,strlen(str)+1);
    }
    return *this;
}

UIString &UIString::Append(char c) {
    char string[2] = {c,'\0'};
    return this->Append(string);
}

UIString &UIString::Append(const UIString &other) {
    return this->Append(other.GetData());
}

UIString UIString::ToUIString(int64_t intValue) {
    char str[64] = {0};
#ifdef WIN32
    snprintf(str, 60, "%lld",intValue);
#else
    snprintf(str, 60, "%ld", intValue);
#endif
    return UIString {str};
}

UIString UIString::operator+(const char *str) const {
    UIString temp;
    temp.Append(*this);
    temp.Append(str);
    return temp;
}

UIString UIString::operator+(char c) const {
    char str[2] = {c,'\0'};
    return this->operator+(str);
}

UIString UIString::operator+(const UIString &other) const {
    return this->operator+(other.m_pstr);
}

UIString &UIString::operator=(const char *str) {
    Assign(str);
    return *this;
}

UIString &UIString::operator=(char c) {
    char str[2] = {c,'\0'};
    this->operator=(str);
    return *this;
}

UIString &UIString::operator=(const UIString &other) {
    if(m_pstr == other.m_pstr){
        return *this;
    }
    this->operator=(other.m_pstr);
    return *this;
}

UIString &UIString::operator=(UIString &&other)noexcept {
    if(GetLength() >= MAX_LOCAL_STRING_LEN){
        delete []m_pstr;
        m_pstr = m_szBuffer;
    }
    if(other.GetLength()>=MAX_LOCAL_STRING_LEN){
        m_pstr = other.m_pstr;
    }else{
        memcpy(m_pstr, other.m_pstr, other.GetLength()+1);
    }
    other.m_pstr = other.m_szBuffer;
    other.m_pstr[0] = '\0';
    return *this;
}

UIString &UIString::operator+=(const char *str) {
    this->Append(str);
    return *this;
}

UIString &UIString::operator+=(char c) {
    this->Append(c);
    return *this;
}

UIString &UIString::operator+=(const UIString &other) {
    this->Append(other);
    return *this;
}

bool UIString::operator==(const UIString &other) const {
    return strcmp(m_pstr, other.m_pstr) == 0;
}

bool UIString::operator==(const char *str) const {
    return strcmp(m_pstr, str) == 0;
}

bool UIString::operator!=(const UIString &other) const {
    return strcmp(m_pstr, other.m_pstr) != 0;
}

bool UIString::operator!=(const char *str) const {
    return strcmp(m_pstr, str) != 0;
}

int UIString::CompareNoCase(const UIString &other) const {
    return strcasecmp(m_pstr, other.m_pstr);
}

void UIString::MakeUpper() {
    int length = GetLength();
    for(int i=0;i<length;++i){
        if(m_pstr[i]>='a' && m_pstr[i]<='z'){
            m_pstr[i] &= ~0x20;
        }
    }
}

void UIString::MakeLower() {
    int length = GetLength();
    for(int i=0;i<length;++i){
        if(m_pstr[i]>='A' && m_pstr[i]<='Z'){
            m_pstr[i] |= 0x20;
        }
    }
}

UIString UIString::Left(int length) const {
    length = length>GetLength()?GetLength():length;
    return UIString{m_pstr, length};
}

UIString UIString::Right(int length) const {
    length = length>GetLength()?GetLength():length;
    return UIString{m_pstr + GetLength() - length, length};
}

UIString UIString::Mid(int pos, int length) const {
    pos = pos<0?0:pos;
    if(pos >= GetLength()){
        return UIString{""};
    }
    length = pos + length >= GetLength()?GetLength()-pos:length;
    return UIString{m_pstr + pos , length};
}

int UIString::Find(const char *subString, int pos) const {
    if(pos < 0 || pos>=GetLength()){
        return -1;
    }
    char *p = strstr(m_pstr + pos, subString);
    if(nullptr == p){
        return -1;
    }
    return p - m_pstr;
}

int UIString::Find(char c, int pos) const {
    char str[2] = {c, '\0'};
    return this->Find(str, pos);
}

int UIString::Find(const UIString &subString, int pos) const {
    return this->Find(subString.GetData(), pos);
}

int UIString::ReverseFind(const char *subString) const {
    int subStringLength = strlen(subString);
    int length = GetLength();
    for(int i=length-subStringLength;i>=0;i--){
        if(strncmp(m_pstr+i, m_pstr,subStringLength)==0){
            return i;
        }
    }
    return 0;
}

int UIString::Replace(const char *fromString, const char *toString) {
    UIString sTemp;
    int nCount = 0;
    int iPos = Find(fromString);
    if( iPos < 0 ) return 0;
    int cchFrom = (int) strlen(fromString);
    int cchTo = (int) strlen(toString);
    while( iPos >= 0 ) {
        sTemp = Left(iPos);
        sTemp += toString;
        sTemp += Mid(iPos + cchFrom);
        Assign(sTemp.GetData());
        iPos = Find(fromString, iPos + cchTo);
        nCount++;
    }
    return nCount;
}

int UIString::Format(const char *formatString, ...) {
    char *szSprintf = nullptr;
    va_list argList;
    int nLen;
    va_start(argList, formatString);
    nLen = vsnprintf(nullptr, 0, formatString, argList);
    va_end(argList);
    szSprintf = new char[nLen+1];
    memset(szSprintf, 0, (nLen + 1) * sizeof(char));
    va_start(argList, formatString);
    int iRet = vsnprintf(szSprintf, nLen + 1, formatString, argList);
    va_end(argList);
    Assign(szSprintf);
    delete []szSprintf;
    return iRet;
}
