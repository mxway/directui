#ifndef __UTILS_H__
#define __UTILS_H__

#pragma once

namespace DuiLib
{
	/////////////////////////////////////////////////////////////////////////////////////
	//

	char    *Utf8ToUtf16(const char *utf8Str,int maxCharNumber = -1);

	class DUILIB_API STRINGorID
	{
	public:
		STRINGorID(const char *lpString);
		STRINGorID(unsigned int nID);

		const char *m_lpstr;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//
    class CDuiString;
	class DUILIB_API CDuiPoint : public tagPOINT
	{
	public:
		CDuiPoint();
		CDuiPoint(const POINT& src);
		CDuiPoint(long x, long y);
		CDuiPoint(LPARAM lParam);
        CDuiPoint(const char *pstrValue);
        CDuiString ToString();
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class DUILIB_API CDuiSize : public tagSIZE
	{
	public:
		CDuiSize();
		CDuiSize(const SIZE& src);
		CDuiSize(const RECT rc);
		CDuiSize(long cx, long cy);
        CDuiSize(const char *pstrValue);
        CDuiString ToString();
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class DUILIB_API CDuiRect : public tagRECT
	{
	public:
		CDuiRect();
		CDuiRect(const RECT& src);
		CDuiRect(long iLeft, long iTop, long iRight, long iBottom);
        CDuiRect(const char *pstrValue);
        CDuiString ToString();

		int GetWidth() const;
		int GetHeight() const;
		void Empty();
		bool IsNull() const;
		void Join(const RECT& rc);
		void ResetOffset();
		void Normalize();
		void Offset(int cx, int cy);
		void Inflate(int cx, int cy);
		void Deflate(int cx, int cy);
		void Union(CDuiRect& rc);
	};

    /////////////////////////////////////////////////////////////////////////////////////
    //

    class DUILIB_API CDuiString
    {
    public:
        enum { MAX_LOCAL_STRING_LEN = 63 };

        CDuiString();
        CDuiString(const char ch);
        CDuiString(const CDuiString& src);
        CDuiString(const char *lpsz, int nLen = -1);
        ~CDuiString();
        CDuiString ToString();

        void Empty();
        int GetLength() const;
        bool IsEmpty() const;
        char GetAt(int nIndex) const;
        void Append(const char *pstr);
        void Assign(const char * pstr, int nLength = -1);
        const char * GetData() const;

        void SetAt(int nIndex, char ch);
        //operator const char *() const;

        char operator[] (int nIndex) const;
        const CDuiString& operator=(const CDuiString& src);
        const CDuiString& operator=(const char ch);
        const CDuiString& operator=(const char * pstr);
#ifdef _UNICODE
        const CDuiString& operator=(LPCSTR lpStr);
        const CDuiString& operator+=(LPCSTR lpStr);
#else
        const CDuiString& operator=(LPCWSTR lpwStr);
        const CDuiString& operator+=(LPCWSTR lpwStr);
#endif
        CDuiString operator+(const CDuiString& src) const;
        CDuiString operator+(const char * pstr) const;
        const CDuiString& operator+=(const CDuiString& src);
        const CDuiString& operator+=(const char * pstr);
        const CDuiString& operator+=(const char ch);

        bool operator == (const char * str) const;
        bool operator ==(const CDuiString &str)const;
        bool operator != (const char * str) const;
        bool operator != (const CDuiString &str)const;
        bool operator <= (const char * str) const;
        bool operator <  (const char * str) const;
        bool operator >= (const char * str) const;
        bool operator >  (const char * str) const;

        int Compare(const char * pstr) const;
        int CompareNoCase(const char * pstr) const;

        void MakeUpper();
        void MakeLower();

        CDuiString Left(int nLength) const;
        CDuiString Mid(int iPos, int nLength = -1) const;
        CDuiString Right(int nLength) const;

        int Find(char ch, int iPos = 0) const;
        int Find(const char *pstr, int iPos = 0) const;
        int ReverseFind(char ch) const;
        int Replace(const char *pstrFrom, const char *pstrTo);

        int __cdecl Format(const char *pstrFormat, ...);
        int __cdecl SmallFormat(const char *pstrFormat, ...);

    protected:
        char * m_pstr;
        char m_szBuffer[MAX_LOCAL_STRING_LEN + 1];
    };

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class DUILIB_API CDuiPtrArray
	{
	public:
		CDuiPtrArray(int iPreallocSize = 0);
		CDuiPtrArray(const CDuiPtrArray& src);
		~CDuiPtrArray();

		void Empty();
		void Resize(int iSize);
		bool IsEmpty() const;
		int Find(LPVOID iIndex) const;
		bool Add(LPVOID pData);
		bool SetAt(int iIndex, LPVOID pData);
		bool InsertAt(int iIndex, LPVOID pData);
		bool Remove(int iIndex, int iCount = 1);
		int GetSize() const;
		LPVOID* GetData();

		LPVOID GetAt(int iIndex) const;
		LPVOID operator[] (int nIndex) const;

	protected:
		LPVOID* m_ppVoid;
		int m_nCount;
		int m_nAllocated;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class DUILIB_API CDuiValArray
	{
	public:
		CDuiValArray(int iElementSize, int iPreallocSize = 0);
		~CDuiValArray();

		void Empty();
		bool IsEmpty() const;
		bool Add(LPCVOID pData);
		bool Remove(int iIndex,  int iCount = 1);
		int GetSize() const;
		LPVOID GetData();

		LPVOID GetAt(int iIndex) const;
		LPVOID operator[] (int nIndex) const;

	protected:
		LPBYTE m_pVoid;
		int m_iElementSize;
		int m_nCount;
		int m_nAllocated;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

    struct TITEM;
	class DUILIB_API CDuiStringPtrMap
	{
	public:
		CDuiStringPtrMap(int nSize = 83);
		~CDuiStringPtrMap();

		void Resize(int nSize = 83);
		LPVOID Find(const CDuiString &key, bool optimize = true) const;
		bool Insert(const CDuiString &key, LPVOID pData);
		LPVOID Set(const CDuiString &key, LPVOID pData);
		bool Remove(const CDuiString &key);
		void RemoveAll();
		int GetSize() const;
		const char *GetAt(int iIndex) const;
		const char *operator[] (int nIndex) const;

	protected:
		TITEM** m_aT;
		int m_nBuckets;
		int m_nCount;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class DUILIB_API CWaitCursor
	{
	public:
		CWaitCursor();
		~CWaitCursor();

	protected:
		HCURSOR m_hOrigCursor;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CVariant : public VARIANT
	{
	public:
		CVariant() 
		{ 
			VariantInit(this); 
		}
		CVariant(int i)
		{
			VariantInit(this);
			this->vt = VT_I4;
			this->intVal = i;
		}
		CVariant(float f)
		{
			VariantInit(this);
			this->vt = VT_R4;
			this->fltVal = f;
		}
		CVariant(LPOLESTR s)
		{
			VariantInit(this);
			this->vt = VT_BSTR;
			this->bstrVal = s;
		}
		CVariant(IDispatch *disp)
		{
			VariantInit(this);
			this->vt = VT_DISPATCH;
			this->pdispVal = disp;
		}

		~CVariant() 
		{ 
			VariantClear(this); 
		}
	};

}// namespace DuiLib

#endif // __UTILS_H__
