#include <UIPtrArray.h>
#include <cstring>

UIPtrArray::UIPtrArray(int preAllocSize)
    : m_ppVoid{nullptr},
    m_nCount {0},
    m_nAllocated {preAllocSize}
{
    if(preAllocSize>0)m_ppVoid = static_cast<LPVOID*>(malloc(preAllocSize * sizeof(LPVOID)));
}

UIPtrArray::UIPtrArray(const UIPtrArray &src)
    :m_ppVoid{nullptr},
    m_nCount {0},
    m_nAllocated {0}
{
    for(int i=0;i<src.GetSize();i++){
        Add(src.GetAt(i));
    }
}

UIPtrArray::~UIPtrArray() {
    if(m_ppVoid != nullptr)free(m_ppVoid);
}

void UIPtrArray::Empty() {
    if(m_ppVoid != nullptr)free(m_ppVoid);
    m_ppVoid = nullptr;
    m_nCount = m_nAllocated = 0;
}

void UIPtrArray::Resize(int size) {
    Empty();
    m_ppVoid = static_cast<LPVOID*>(malloc(size*sizeof(LPVOID)));
    memset(m_ppVoid, 0, size*sizeof(LPVOID));
    m_nAllocated = size;
    m_nCount = size;
}

bool UIPtrArray::IsEmpty() const {
    return m_nCount == 0;
}

int UIPtrArray::Find(LPVOID data) const {
    for(int i=0;i<m_nCount; ++i){
        if(m_ppVoid[i] == data){
            return i;
        }
    }
    return -1;
}

bool UIPtrArray::Add(LPVOID data) {
    if( ++m_nCount >= m_nAllocated) {
        int nAllocated = m_nAllocated * 2;
        if( nAllocated == 0 ) nAllocated = 11;
        auto* ppVoid = static_cast<LPVOID*>(realloc(m_ppVoid, nAllocated * sizeof(LPVOID)));
        if( ppVoid != nullptr ) {
            m_nAllocated = nAllocated;
            m_ppVoid = ppVoid;
        }
        else {
            --m_nCount;
            return false;
        }
    }
    m_ppVoid[m_nCount - 1] = data;
    return true;
}

bool UIPtrArray::SetAt(int index, LPVOID data) {
    if( index < 0 || index >= m_nCount ) return false;
    m_ppVoid[index] = data;
    return true;
}

bool UIPtrArray::InsertAt(int index, LPVOID data) {
    if( index == m_nCount ) return Add(data);
    if( index < 0 || index > m_nCount ) return false;
    if( ++m_nCount >= m_nAllocated) {
        int nAllocated = m_nAllocated * 2;
        if( nAllocated == 0 ) nAllocated = 11;
        LPVOID* ppVoid = static_cast<LPVOID*>(realloc(m_ppVoid, nAllocated * sizeof(LPVOID)));
        if( ppVoid != NULL ) {
            m_nAllocated = nAllocated;
            m_ppVoid = ppVoid;
        }
        else {
            --m_nCount;
            return false;
        }
    }
    memmove(&m_ppVoid[index + 1], &m_ppVoid[index], (m_nCount - index - 1) * sizeof(LPVOID));
    m_ppVoid[index] = data;
    return true;
}

bool UIPtrArray::Remove(int index, int count) {
    if( index < 0 || count <= 0 || index + count > m_nCount ) return false;
    if (index + count < m_nCount) memmove(m_ppVoid + index, m_ppVoid + index + count, (m_nCount - index - count) * sizeof(LPVOID));
    m_nCount -= count;
    return true;
}

int UIPtrArray::GetSize() const {
    return m_nCount;
}

LPVOID *UIPtrArray::GetData() {
    return m_ppVoid;
}

LPVOID UIPtrArray::GetAt(int index) const {
    if( index < 0 || index >= m_nCount ) return nullptr;
    return m_ppVoid[index];
}

LPVOID UIPtrArray::operator[](int index) const {
    return m_ppVoid[index];
}
