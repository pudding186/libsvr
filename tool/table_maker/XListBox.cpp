// XListBox.cpp  Version 1.2
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// License:
//     This software is released under the Code Project Open License (CPOL),
//     which may be found here:  http://www.codeproject.com/info/eula.aspx
//     You are free to use this software in any way you like, except that you 
//     may not sell this source code.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     I accept no liability for any damage or loss of business that this 
//     software may cause.
//
// Notes on use:  
//     To use in an MFC project, first create a listbox using the standard 
//     dialog editor.  Be sure to mark the listbox as OWNERDRAW FIXED, and 
//     check the HAS STRINGS box.  Using Class Wizard, create a variable for
//     the listbox.  Finally, manually edit the dialog's .h file and replace 
//     CListBox with CXListBox, and #include XListBox.h.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XListBox.h"
#include "Clipboard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable : 4996)	// disable bogus deprecation warning

//=============================================================================
// set resource handle (in case used in DLL)
//=============================================================================
#ifdef _USRDLL
#define AFXMANAGESTATE AfxGetStaticModuleState
#else
#define AFXMANAGESTATE AfxGetAppModuleState
#endif

//=============================================================================	
// NOTE - following table must be kept in sync with ColorPickerCB.cpp
//=============================================================================	

static COLORREF ColorTable[16] = { RGB(  0,   0,   0),		// Black
								   RGB(255, 255, 255),		// White
								   RGB(128,   0,   0),		// Maroon
								   RGB(  0, 128,   0),		// Green
								   RGB(128, 128,   0),		// Olive
								   RGB(  0,   0, 128),		// Navy
								   RGB(128,   0, 128),		// Purple
								   RGB(  0, 128, 128),		// Teal
								   RGB(192, 192, 192),		// Silver
								   RGB(128, 128, 128),		// Gray
								   RGB(255,   0,   0),		// Red
								   RGB(  0, 255,   0),		// Lime
								   RGB(255, 255,   0),		// Yellow
								   RGB(  0,   0, 255),		// Blue
								   RGB(255,   0, 255),		// Fuschia
								   RGB(  0, 255, 255) };	// Aqua

//=============================================================================	
BEGIN_MESSAGE_MAP(CXListBox, CListBox)
//=============================================================================	
	//{{AFX_MSG_MAP(CXListBox)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//=============================================================================	
CXListBox::CXListBox()
//=============================================================================	
{
	m_ColorWindow        = ::GetSysColor(COLOR_WINDOW);
	m_ColorHighlight     = ::GetSysColor(COLOR_HIGHLIGHT);
	m_ColorWindowText    = ::GetSysColor(COLOR_WINDOWTEXT);
	m_ColorHighlightText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	m_ColorGutter        = RGB(245,245,245);
	m_ColorLineNo        = RGB(80,80,80);
	m_strLogFile         = _T("");
	m_bColor             = TRUE;
	m_cxExtent           = 0;
	m_nTabPosition       = 8;	// tab stops every 8 columns
	m_nSpaceWidth        = 7;
	m_nContextMenuId     = (UINT)-1;
	m_bLineNumbers       = FALSE;
	m_nGutterWidth       = 5;
	for (int i = 0; i < MAXTABSTOPS; i++)
		m_nTabStopPositions[i] = (i+1) * m_nTabPosition * m_nSpaceWidth;
}

//=============================================================================	
CXListBox::~CXListBox()
//=============================================================================	
{
}

//=============================================================================	
void CXListBox::MeasureItem(LPMEASUREITEMSTRUCT)
//=============================================================================	
{
}

//=============================================================================	
int CXListBox::CompareItem(LPCOMPAREITEMSTRUCT)
//=============================================================================	
{
	return 0;
}

//=============================================================================	
void CXListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
//=============================================================================	
{
	COLORREF oldtextcolor, oldbackgroundcolor;

	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	pDC->GetCharWidth((UINT) ' ', (UINT) ' ', &m_nSpaceWidth);
	pDC->GetCharWidth((UINT) 'c', (UINT) 'c', &m_nAveCharWidth);

	for (int i = 0; i < MAXTABSTOPS; i++)
		m_nTabStopPositions[i] = (i+1) * m_nAveCharWidth * m_nTabPosition;

	// draw focus rectangle when no items in listbox
	if (lpDIS->itemID == (UINT)-1)
	{
		if (lpDIS->itemAction & ODA_FOCUS)
			pDC->DrawFocusRect(&lpDIS->rcItem);
		return;
	}
	else
	{
		int selChange   = lpDIS->itemAction & ODA_SELECT;
		int focusChange = lpDIS->itemAction & ODA_FOCUS;
		int drawEntire  = lpDIS->itemAction & ODA_DRAWENTIRE;

		if (selChange || drawEntire)
		{
			BOOL sel = lpDIS->itemState & ODS_SELECTED;

			int nLen = CListBox::GetTextLen(lpDIS->itemID);
			if (nLen != LB_ERR)
			{
				TCHAR *buf = new TCHAR [nLen + 16];
				ASSERT(buf);
				if (buf && (GetTextWithColor(lpDIS->itemID, buf) != LB_ERR))
				{
					CRect rectItem(lpDIS->rcItem);
					CSize size;

					if (m_bLineNumbers)
					{
						// draw gutter & line no.
						UINT index = GetTopIndex();
						UINT n = rectItem.top / rectItem.Height();
						index += n;
						//TRACE("index=%d\n", index);
						CString strLineNo = _T("");
						for (int i = 0; i < m_nGutterWidth; i++)
							strLineNo += _T('9');
						if (strLineNo.IsEmpty())
							strLineNo = _T('9');
						size = pDC->GetTextExtent(strLineNo);
						strLineNo = _T("");
						strLineNo.Format(_T("%d"), index+1);
						rectItem.right = rectItem.left + size.cx + 6;
						pDC->SetBkColor(m_ColorGutter);
						pDC->SetTextColor(m_ColorLineNo);
						// fill the gutter with the gutter color the fast way
						pDC->ExtTextOut(0, 0, ETO_OPAQUE, &rectItem, NULL, 0, NULL);
						rectItem.left += 2;
						rectItem.right -= 2;
						pDC->DrawText(strLineNo, &rectItem, DT_RIGHT);
						// restore drawing rect
						rectItem.left = rectItem.right + 2;
						rectItem.right = lpDIS->rcItem.right;
					}

					// set text color from first character in string -
					// NOTE: 1 was added to color index to avoid asserts by CString
					int itext = int (buf[0] - 1);

					// set background color from second character in string -
					// NOTE: 1 was added to color index to avoid asserts by CString
					int iback = int (buf[1] - 1);
					buf[0] = _T(' ');
					buf[1] = _T(' ');
					COLORREF textcolor = sel ? m_ColorHighlightText : ColorTable[itext];
					oldtextcolor = pDC->SetTextColor(textcolor);
					COLORREF backgroundcolor = sel ? m_ColorHighlight : ColorTable[iback];
					oldbackgroundcolor = pDC->SetBkColor(backgroundcolor);

					// fill the rectangle with the background color the fast way
					pDC->ExtTextOut(0, 0, ETO_OPAQUE, &rectItem, NULL, 0, NULL);

					pDC->TabbedTextOut(rectItem.left+2, rectItem.top, &buf[2],
						(int)_tcslen(&buf[2]), MAXTABSTOPS, (LPINT)m_nTabStopPositions, 0);

					size = pDC->GetOutputTextExtent(&buf[2]);
					int nScrollBarWidth = ::GetSystemMetrics(SM_CXVSCROLL);
					size.cx += nScrollBarWidth;	// in case of vertical scrollbar

					int cxExtent = (size.cx > m_cxExtent) ? size.cx : m_cxExtent;

					if (cxExtent > m_cxExtent)
					{
						m_cxExtent = cxExtent;
						SetHorizontalExtent(m_cxExtent+(m_cxExtent/32));
					}
				}
				if (buf)
					delete [] buf;
			}
		}

		if (focusChange || (drawEntire && (lpDIS->itemState & ODS_FOCUS)))
			pDC->DrawFocusRect(&lpDIS->rcItem);
	}
}

//=============================================================================	
// GetTextWithColor - get text string with color bytes
int CXListBox::GetTextWithColor(int nIndex, LPTSTR lpszBuffer) const
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return LB_ERR;
	}

	ASSERT(lpszBuffer);
	lpszBuffer[0] = 0;
	return CListBox::GetText(nIndex, lpszBuffer);
}

//=============================================================================	
// GetTextWithColor - get text string with color bytes
void CXListBox::GetTextWithColor(int nIndex, CString& rString) const
//=============================================================================	
{
	rString.Empty();

	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return;
	}

	CListBox::GetText(nIndex, rString);
}

//=============================================================================	
// GetText - for compatibility with CListBox (no color bytes)
int CXListBox::GetText(int nIndex, LPTSTR lpszBuffer) const
//=============================================================================	
{
	ASSERT(lpszBuffer);
	if (lpszBuffer == NULL)
		return LB_ERR;

	lpszBuffer[0] = 0;

	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return LB_ERR;
	}

	int nRet = CListBox::GetText(nIndex, lpszBuffer);

	size_t n = _tcslen(lpszBuffer);
	if (n > 2)
		memcpy(&lpszBuffer[0], &lpszBuffer[2], (n-1)*sizeof(TCHAR));	// copy nul too

	return nRet;
}

//=============================================================================	
// GetText - for compatibility with CListBox (no color bytes)
void CXListBox::GetText(int nIndex, CString& rString) const
//=============================================================================	
{
	rString.Empty();

	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return;
	}

	CString str = _T("");
	CListBox::GetText(nIndex, str);
	if ((!str.IsEmpty()) && (str.GetLength() > 2))
		rString = str.Mid(2);
}

//=============================================================================	
// GetTextLen - for compatibility with CListBox (no color bytes)
int CXListBox::GetTextLen(int nIndex) const
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return LB_ERR;
	}

	int n = CListBox::GetTextLen(nIndex);
	if (n != LB_ERR && n >= 2)
		n -= 2;
	return n;
}

//=============================================================================	
int CXListBox::SearchString(int nStartAfter, LPCTSTR lpszItem, BOOL bExact) const
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return LB_ERR;
	}

	// start the search after specified index
	int nIndex = nStartAfter + 1;

	int nCount = GetCount();
	if (nCount == LB_ERR)
		return LB_ERR;

	// convert string to search for to lower case
	CString strItem = lpszItem;
	strItem.MakeLower();
	int nItemSize = strItem.GetLength();

	CString strText = _T("");

	// search until end
	for ( ; nIndex < nCount; nIndex++)
	{
		GetText(nIndex, strText);
		strText.MakeLower();
		if (!bExact)
			strText = strText.Left(nItemSize);
		if (strText == strItem)
			return nIndex;
	}

	// if we started at beginning there is no more to do, search failed
	if (nStartAfter == -1)
		return LB_ERR;

	// search until we reach beginning index
	for (nIndex = 0; (nIndex <= nStartAfter) && (nIndex < nCount); nIndex++)
	{
		GetText(nIndex, strText);
		strText.MakeLower();
		if (!bExact)
			strText = strText.Left(nItemSize);
		if (strText == strItem)
			return nIndex;
	}

	return LB_ERR;
}

//=============================================================================	
int CXListBox::FindString(int nStartAfter, LPCTSTR lpszItem) const
//=============================================================================	
{
	return SearchString(nStartAfter, lpszItem, FALSE);
}

//=============================================================================	
int CXListBox::SelectString(int nStartAfter, LPCTSTR lpszItem)
//=============================================================================	
{
	int rc = SearchString(nStartAfter, lpszItem, FALSE);
	if (rc != LB_ERR)
		SetCurSel(rc);
	return rc;
}

//=============================================================================	
int CXListBox::FindStringExact(int nStartAfter, LPCTSTR lpszItem) const
//=============================================================================	
{
	return SearchString(nStartAfter, lpszItem, TRUE);
}

//=============================================================================	
// InsertString - override to add text color
int CXListBox::InsertString(int nIndex, 
							LPCTSTR lpszItem, 
							Color tc /*= Black*/, 
							Color bc /*= White*/)
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return LB_ERR;
	}

	CString s = lpszItem;

	if (!m_bColor)
	{
		tc = Black;			// to force black-only text
		bc = White;
	}

	UINT nColor = (UINT) tc;
	ASSERT(nColor < 16);
	if (nColor >= 16)
		tc = Black;

	// don't display \r or \n characters
	int i = 0;
	while ((i = s.FindOneOf(_T("\r\n"))) != -1)
		s.SetAt(i, ' ');

	// first character in string is color -- add 1 to color
	// to avoid asserts by CString class
	CString t = _T("");
	t += (char) (tc + 1);
	t += (char) (bc + 1);
	t += s;

	// try to insert the string into the listbox
	i = CListBox::InsertString(nIndex, t);

	return i;
}

//=============================================================================	
// AddString - override to add text color
int CXListBox::AddString(LPCTSTR lpszItem)
//=============================================================================	
{
	return AddLine(CXListBox::Black, CXListBox::White, lpszItem);
}

//=============================================================================	
int CXListBox::AddLine(Color tc, Color bc, LPCTSTR lpszLine)
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return LB_ERR;
	}

	CString s = lpszLine;

	if (!m_bColor)
	{
		tc = Black;			// to force black-only text
		bc = White;
	}

	UINT nColor = (UINT) tc;
	ASSERT(nColor < 16);
	if (nColor >= 16)
		tc = Black;

	// don't display \r or \n characters
	int i = 0;
	while ((i = s.FindOneOf(_T("\r\n"))) != -1)
		s.SetAt(i, _T(' '));

	if (!s.IsEmpty() && !m_strLogFile.IsEmpty())
	{
		CTime t = CTime::GetCurrentTime();
		CString s2 = t.Format(_T("%Y-%m-%d %H:%M:%S "));
		TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH*2];
		szComputerName[0] = _T('\0');
		DWORD dwSize = sizeof(szComputerName)/sizeof(TCHAR)-1;
		GetComputerName(szComputerName, &dwSize);
		s2 += _T('[');
		s2 += szComputerName;
		s2 += _T("] ");
		s2 += s;
		s2 += _T("\r\n");

		// write string to log file
		HANDLE hFile = ::CreateFile(m_strLogFile, GENERIC_WRITE, FILE_SHARE_WRITE,
							NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			// set to end of file - always append
			DWORD dwRC = ::SetFilePointer(hFile,		// handle to file
										  0,			// bytes to move pointer
										  NULL,			// bytes to move pointer
										  FILE_END);	// starting point

			if (dwRC != INVALID_SET_FILE_POINTER)
			{
				DWORD dwWritten = 0;
				::WriteFile(hFile,						// handle to file
							s2,							// data buffer
							(DWORD)s2.GetLength()*sizeof(TCHAR),	// number of bytes to write
							&dwWritten,					// number of bytes written
							NULL);						// overlapped buffer
			}

			::CloseHandle(hFile);
		}
	}

	// first character in string is color -- add 1 to color
	// to avoid asserts by CString class
	CString t = _T("");
	t += (TCHAR) (tc + 1);
	t += (TCHAR) (bc + 1);
	t += s;

	// try to add the string to the listbox
	i = CListBox::AddString(t);
	if (i == LB_ERRSPACE)
	{
		// will get LB_ERRSPACE if listbox is out of memory
		int n = GetCount();

		if (n == LB_ERR)
			return LB_ERR;

		if (n < 2)
			return LB_ERR;

		// try to delete some strings to free up some room --
		// don't spend too much time deleting strings, since
		// we might be getting a burst of messages
		n = (n < 20) ? (n-1) : 20;
		if (n <= 0)
			n = 1;

		SetRedraw(FALSE);
		for (i = 0; i < n; i++)
			DeleteString(0);

		i = CListBox::AddString(t);

		SetRedraw(TRUE);
	}

	if (i >= 0)
	{
		//SetTopIndex(i);
        SetCaretIndex(i, FALSE);
        //m_result_box.SetCaretIndex(m_result_box.GetCount()-1, FALSE);
	}

	SetCurSel(-1);

	return i;
}

//=============================================================================	
int _cdecl CXListBox::Printf(Color tc, Color bc, UINT nID, LPCTSTR lpszFmt, ...)
//=============================================================================	
{
	AFX_MANAGE_STATE(AFXMANAGESTATE())

	TCHAR buf[2048], fmt[1024];
	va_list marker;

	// load format string from string resource if
	// a resource ID was specified
	if (nID)
	{
		CString s = _T("");
		if (!s.LoadString(nID))
		{
			_stprintf(s.GetBufferSetLength(80), _T("Failed to load string resource %u"),
				nID);
			s.ReleaseBuffer(-1);
		}
		_tcsncpy(fmt, s, sizeof(fmt)/sizeof(TCHAR)-1);
	}
	else
	{
		// format string was passed as parameter
		_tcsncpy(fmt, lpszFmt, sizeof(fmt)/sizeof(TCHAR)-1);
	}
	fmt[sizeof(fmt)/sizeof(TCHAR)-1] = 0;

	// combine output string and variables
	va_start(marker, lpszFmt);
	_vsntprintf(buf, (sizeof(buf)/sizeof(TCHAR))-1, fmt, marker);
	va_end(marker);
	buf[sizeof(buf)/sizeof(TCHAR)-1] = 0;

	return AddLine(tc, bc, buf);
}

//=============================================================================	
void CXListBox::EnableColor (BOOL bEnable)
//=============================================================================	
{
	m_bColor = bEnable;
}

//=============================================================================	
void CXListBox::SetTabPosition(int nSpacesPerTab)
//=============================================================================	
{
	ASSERT(nSpacesPerTab > 0 && nSpacesPerTab < 11);

	m_nTabPosition = nSpacesPerTab;

	CDC* pDC = GetDC();

	if (pDC)
	{
		TEXTMETRIC tm;
		pDC->GetTextMetrics(&tm);

		pDC->GetCharWidth((UINT) ' ', (UINT) ' ', &m_nSpaceWidth);
		pDC->GetCharWidth((UINT) '9', (UINT) '9', &m_nAveCharWidth);

		for (int i = 0; i < MAXTABSTOPS; i++)
			m_nTabStopPositions[i] = (i+1) * m_nAveCharWidth * m_nTabPosition;

		ReleaseDC(pDC);
	}
}

//=============================================================================	
int CXListBox::GetVisibleLines()
//=============================================================================	
{
	int nCount = 0;

	CDC* pDC = GetDC();

	if (pDC)
	{
		TEXTMETRIC tm;
		pDC->GetTextMetrics(&tm);
		int h = tm.tmHeight + tm.tmInternalLeading;
		ReleaseDC(pDC);

		CRect rect;
		GetClientRect(&rect);
		nCount = rect.Height() / h;
	}
	return nCount;
}

//=============================================================================	
void CXListBox::ResetContent()
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
	}
	else
	{
		CListBox::ResetContent();

		m_cxExtent = 0;

		SetHorizontalExtent(m_cxExtent);
	}
}

//=============================================================================	
void CXListBox::SetFont(CFont *pFont, BOOL bRedraw)
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
	}
	else
	{
		CListBox::SetFont(pFont, bRedraw);

		CDC* pDC = GetDC();

		if (pDC)
		{
			CFont *pOldFont = pDC->SelectObject(pFont);

			TEXTMETRIC tm;
			pDC->GetTextMetrics(&tm);
			int h = tm.tmHeight;
			SetItemHeight(0, h);

			pDC->SelectObject(pOldFont);

			pDC->GetCharWidth((UINT) ' ', (UINT) ' ', &m_nSpaceWidth);
			pDC->GetCharWidth((UINT) '9', (UINT) '9', &m_nAveCharWidth);

			for (int i = 0; i < MAXTABSTOPS; i++)
				m_nTabStopPositions[i] = (i+1) * m_nAveCharWidth * m_nTabPosition;

			ReleaseDC(pDC);
		}

		m_cxExtent = 0;
	}
}

//=============================================================================	
void CXListBox::OnLButtonDblClk(UINT nFlags, CPoint point)
//=============================================================================	
{
	CListBox::OnLButtonDblClk(nFlags, point);
}

//=============================================================================	
//void CXListBox::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
////=============================================================================	
//{
//	AFX_MANAGE_STATE(AFXMANAGESTATE())
//
//	if (m_nContextMenuId == -1)
//	{
//		TRACE(_T("no context menu\n"));
//		return;
//	}
//
//	CMenu menu;
//	if (!menu.LoadMenu(m_nContextMenuId))
//	{
//		TRACE(_T("ERROR failed to load %d\n"), m_nContextMenuId);
//		return;
//	}
//
//	menu.GetSubMenu(0)->TrackPopupMenu(0,
//		point.x, point.y, this, NULL);
//}

//=============================================================================	
void CXListBox::OnEditCopy()
//=============================================================================	
{
	CString str = _T("");

	int nCount = GetCount();
	int nSel = 0;

	for (int i = 0; i < nCount; i++)
	{
		if (GetSel(i) > 0)
		{
			CString s = _T("");
			GetText(i, s);
			if (!s.IsEmpty())
			{
				nSel++;
				s.TrimLeft(_T("\r\n"));
				s.TrimRight(_T("\r\n"));
				if (s.Find(_T('\n')) == -1)
					s += _T("\n");
				s.Replace(_T("\t"), _T(" "));
				str += s;
			}
		}
	}

	if (!str.IsEmpty())
		CClipboard::SetText(str);
}

//=============================================================================	
void CXListBox::OnEditClear()
//=============================================================================	
{
	ResetContent();
}

//=============================================================================	
void CXListBox::OnEditSelectAll()
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
	}
	else
	{
		SelItemRange(TRUE, 0, GetCount()-1);
	}
}

//=============================================================================	
BOOL CXListBox::OnEraseBkgnd(CDC* pDC) 
//=============================================================================	
{
	if (m_bLineNumbers)
	{
		CFont *pOldFont = NULL;
		CFont *pFont = GetFont();
		if (pFont == NULL)
			pFont = GetParent()->GetFont();
		if (pFont)
			pOldFont = pDC->SelectObject(pFont);

		CRect rect;
		GetClientRect(&rect);
		pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
		// fill the background with the window color the fast way
		pDC->ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

		CString strLineNo = _T("");
		for (int i = 0; i < m_nGutterWidth; i++)
			strLineNo += _T('9');
		if (strLineNo.IsEmpty())
			strLineNo = _T('9');
		CSize size = pDC->GetTextExtent(strLineNo);
		CRect rectGutter(rect);
		rectGutter.right = rectGutter.left + size.cx + 6;
		pDC->SetBkColor(m_ColorGutter);

		// fill the gutter with the gutter color the fast way
		pDC->ExtTextOut(0, 0, ETO_OPAQUE, &rectGutter, NULL, 0, NULL);
		if (pOldFont)
			pDC->SelectObject(pOldFont);

		return TRUE;
	}
	else
	{
		return CListBox::OnEraseBkgnd(pDC);
	}
}

//=============================================================================	
CXListBox::Color CXListBox::GetBackgroundColor(int nIndex) const
//=============================================================================	
{
	Color bc = White;

	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
	}
	else
	{
		int nLen = CListBox::GetTextLen(nIndex);
		if (nLen != LB_ERR)
		{
			TCHAR *buf = new TCHAR [nLen + 16];
			ASSERT(buf);
			if (buf && (GetTextWithColor(nIndex, buf) != LB_ERR))
			{
				// get background color from second character in string -
				// NOTE: 1 was added to color index to avoid asserts by CString
				int iback = int (buf[1] - 1);
				bc = (Color) iback;
			}
			if (buf)
				delete [] buf;
			buf = NULL;
		}
	}

	return bc;
}

//=============================================================================	
CXListBox::Color CXListBox::GetTextColor(int nIndex) const
//=============================================================================	
{
	Color tc = Black;

	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
	}
	else
	{
		int nLen = CListBox::GetTextLen(nIndex);
		if (nLen != LB_ERR)
		{
			TCHAR *buf = new TCHAR [nLen + 16];
			ASSERT(buf);
			if (buf && (GetTextWithColor(nIndex, buf) != LB_ERR))
			{
				// set text color from first character in string -
				// NOTE: 1 was added to color index to avoid asserts by CString
				int itext = int (buf[0] - 1);
				tc = (Color) itext;
			}
			if (buf)
				delete [] buf;
			buf = NULL;
		}
	}

	return tc;
}

//=============================================================================	
void CXListBox::SetBackgroundColor(int nIndex, Color bc)
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
	}
	else
	{
		int nLen = CListBox::GetTextLen(nIndex);
		if (nLen != LB_ERR)
		{
			TCHAR *buf = new TCHAR [nLen + 16];
			ASSERT(buf);
			if (buf && (GetTextWithColor(nIndex, buf) != LB_ERR))
			{
				// get text color from first character in string -
				// NOTE: 1 was added to color index to avoid asserts by CString
				int tc = (int) buf[0];

				// delete old string
				CListBox::DeleteString(nIndex);

				CString s = &buf[2];
				CString t = _T("");
				t += (char) tc;
				t += (char) (bc + 1);	// add 1 to color to avoid asserts by CString class
				t += s;
				CListBox::InsertString(nIndex, t);
			}
			if (buf)
				delete [] buf;
			buf = NULL;
		}
	}
}

//=============================================================================	
void CXListBox::SetTextColor(int nIndex, Color tc)
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
	}
	else
	{
		int nLen = CListBox::GetTextLen(nIndex);
		if (nLen != LB_ERR)
		{
			TCHAR *buf = new TCHAR [nLen + 16];
			ASSERT(buf);
			if (buf && (GetTextWithColor(nIndex, buf) != LB_ERR))
			{
				// get background color from second character in string -
				// NOTE: 1 was added to color index to avoid asserts by CString
				int bc = (int) buf[1];

				// delete old string
				CListBox::DeleteString(nIndex);

				CString s = &buf[2];
				CString t = _T("");
				t += (char) (tc + 1);	// add 1 to color to avoid asserts by CString class
				t += (char) bc;
				t += s;
				CListBox::InsertString(nIndex, t);
			}
			if (buf)
				delete [] buf;
			buf = NULL;
		}
	}
}
