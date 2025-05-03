#include <UIValArray.h>
#include <cstring>

UIValArray::UIValArray(int elementSize, int preAllocSize)
    :m_pVoid(nullptr),
    m_nCount{0},
    m_elementSize{elementSize},
    m_nAllocated {preAllocSize}
{
    if( preAllocSize > 0 ) m_pVoid = static_cast<LPBYTE>(malloc(preAllocSize * m_elementSize));
}

UIValArray::~UIValArray() {
    if(m_pVoid != nullptr)free(m_pVoid);
}

void UIValArray::Empty() {
    m_nCount = 0;
}

bool UIValArray::IsEmpty() {
   return m_nCount == 0;
}

bool UIValArray::Add(LPVOID data) {
    if( ++m_nCount >= m_nAllocated) {
        int nAllocated = m_nAllocated * 2;
        if( nAllocated == 0 ) nAllocated = 11;
        auto pVoid = static_cast<LPBYTE>(realloc(m_pVoid, nAllocated * m_elementSize));
        if( pVoid != nullptr ) {
            m_nAllocated = nAllocated;
            m_pVoid = pVoid;
        }
        else {
            --m_nCount;
            return false;
        }
    }
    memmove(m_pVoid + ((m_nCount - 1) * m_elementSize), data, m_elementSize);
    return true;
}

bool UIValArray::Remove(int index, int count) {
    if( index < 0 || count <= 0 || index + count > m_nCount ) return false;
    if (index + count < m_nCount) memmove(m_pVoid + (index * m_elementSize), m_pVoid + (index + count) * m_elementSize, (m_nCount - index - count) * m_elementSize);
    m_nCount -= count;
    return true;
}

int UIValArray::GetSize() const {
    return m_nCount;
}

LPVOID UIValArray::GetData() {
    return static_cast<LPVOID>(m_pVoid);
}

LPVOID UIValArray::GetAt(int index) const {
    if( index < 0 || index >= m_nCount ) return nullptr;
    return m_pVoid + (index * m_elementSize);
}

LPVOID UIValArray::operator[](int index) const {
    return m_pVoid + (index * m_elementSize);
}
