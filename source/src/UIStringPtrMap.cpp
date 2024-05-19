#include <UIStringPtrMap.h>
#include <cstring>

struct TITEM
{
    UIString    Key;
    LPVOID      Data;
    struct      TITEM* pPrev;
    struct      TITEM* pNext;
};

static uint32_t HashKey(const UIString &key)
{
    uint32_t hashValue = 0;
    int len = key.GetLength();
    while(len-- > 0)
        hashValue = (hashValue << 5) + hashValue + key[len];
    return hashValue;
}

UIStringPtrMap::UIStringPtrMap(int nSize)
    :m_nCount{0}
{
    if(nSize < 16) nSize=16;
    m_nBuckets = nSize;
    m_aT = new TITEM*[nSize];
    memset(m_aT, 0, nSize*sizeof(TITEM*));
}

UIStringPtrMap::~UIStringPtrMap() {
    this->Destroy();
}

void UIStringPtrMap::Resize(int nSize) {
    this->Destroy();
    if(nSize < 0)nSize = 0;
    if(nSize>0){
        m_aT = new TITEM*[nSize];
        memset(m_aT, 0, nSize*sizeof(TITEM*));
    }
    m_nBuckets = nSize;
    m_nCount = 0;
}

LPVOID UIStringPtrMap::Find(const UIString &key, bool optimize) const {
    if( m_nBuckets == 0 || GetSize() == 0 ) return nullptr;

    uint32_t slot = HashKey(key) % m_nBuckets;
    for( TITEM* pItem = m_aT[slot]; pItem; pItem = pItem->pNext ) {
        if( pItem->Key == key ) {
            if (optimize && pItem != m_aT[slot]) {
                if (pItem->pNext) {
                    pItem->pNext->pPrev = pItem->pPrev;
                }
                pItem->pPrev->pNext = pItem->pNext;
                pItem->pPrev = nullptr;
                pItem->pNext = m_aT[slot];
                pItem->pNext->pPrev = pItem;
                m_aT[slot] = pItem;
            }
            return pItem->Data;
        }
    }

    return nullptr;
}

bool UIStringPtrMap::Insert(const UIString &key, LPVOID pData) {
    if( m_nBuckets == 0 ) return false;
    if( Find(key) ) return false;

    // Add first in bucket
    uint32_t slot = HashKey(key) % m_nBuckets;
    auto* pItem = new TITEM;
    pItem->Key = key;
    pItem->Data = pData;
    pItem->pPrev = nullptr;
    pItem->pNext = m_aT[slot];
    if (pItem->pNext)
        pItem->pNext->pPrev = pItem;
    m_aT[slot] = pItem;
    m_nCount++;
    return true;
}

LPVOID UIStringPtrMap::Set(const UIString &key, LPVOID pData) {
    if( m_nBuckets == 0 ) return pData;

    if (GetSize()>0) {
        uint32_t slot = HashKey(key) % m_nBuckets;
        // Modify existing item
        for( TITEM* pItem = m_aT[slot]; pItem; pItem = pItem->pNext ) {
            if( pItem->Key == key ) {
                LPVOID pOldData = pItem->Data;
                pItem->Data = pData;
                return pOldData;
            }
        }
    }

    Insert(key, pData);
    return nullptr;
}

bool UIStringPtrMap::Remove(const UIString &key) {
    if( m_nBuckets == 0 || GetSize() == 0 ) return false;

    uint32_t slot = HashKey(key) % m_nBuckets;
    TITEM** ppItem = &m_aT[slot];
    while( *ppItem ) {
        if( (*ppItem)->Key == key ) {
            TITEM* pKill = *ppItem;
            *ppItem = (*ppItem)->pNext;
            if (*ppItem)
                (*ppItem)->pPrev = pKill->pPrev;
            delete pKill;
            m_nCount--;
            return true;
        }
        ppItem = &((*ppItem)->pNext);
    }

    return false;
}

void UIStringPtrMap::RemoveAll() {
    this->Resize(m_nBuckets);
}

int UIStringPtrMap::GetSize() const {
    return m_nCount;
}

UIString UIStringPtrMap::GetAt(int iIndex) const {
    if( m_nBuckets == 0 || GetSize() == 0 ) return UIString{};

    int pos = 0;
    int len = m_nBuckets;
    while( len-- ) {
        for( TITEM* pItem = m_aT[len]; pItem; pItem = pItem->pNext ) {
            if( pos++ == iIndex ) {
                return pItem->Key;
            }
        }
    }

    return UIString{};
}

UIString UIStringPtrMap::operator[](int nIndex) const {
    return GetAt(nIndex);
}

void UIStringPtrMap::Destroy() {
    if(!m_aT){
        return;
    }
    int len = m_nBuckets;
    while(len--){
        TITEM *pItem = m_aT[len];
        while(pItem){
            TITEM *pKill = pItem;
            pItem = pItem->pNext;
            delete pKill;
        }
    }
    delete []m_aT;
    m_aT = nullptr;
}
