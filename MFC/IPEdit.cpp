#include "stdafx.h"
#include "IPEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Style bits for the individual edit controls
const int WS_EDIT = WS_CHILD | WS_VISIBLE | ES_CENTER | ES_MULTILINE;
const TCHAR szDialogClass[] = _T("#32770");			// Special window class for dialogs

BOOL CIPEdit::m_bRegistered = Register();		// Register the control during class initialization

/////////////////////////////////////////////////////////////////////////////
// CIPAddrCtl

IMPLEMENT_DYNCREATE(CIPEdit, CWnd)

CIPEdit::CIPEdit()
{
	m_bEnabled = TRUE;								// Window enabled flag (TRUE by default)
	m_bReadOnly = FALSE;							// Read only flag (FALSE by default)
	m_bNoValidate = FALSE;							// Don't do immediate field validation on input
}

CIPEdit::~CIPEdit()
{
}


BEGIN_MESSAGE_MAP(CIPEdit, CWnd)
	//{{AFX_MSG_MAP(CIPEdit)
	ON_WM_CREATE()
	ON_WM_NCDESTROY()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_PAINT()
	ON_WM_ENABLE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(IPAM_GETADDRESS, OnGetAddress)
	ON_MESSAGE(IPAM_SETADDRESS, OnSetAddress)
	ON_MESSAGE(IPAM_SETREADONLY, OnSetReadOnly)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CIPAddrCtl message handlers

BOOL CIPEdit::Register()
{
	// Register the window class of the control
	WNDCLASS	wc;
	wc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;	// Usual style bits
  wc.lpfnWndProc = ::DefWindowProc;					// Message processing code
	wc.cbClsExtra = 0;								// No extra bytes needed
	wc.cbWndExtra = 0;
	wc.hInstance = NULL;							// No instance handle
	wc.hIcon = NULL;								// No icon
	wc.hCursor = ::LoadCursor(NULL, IDC_IBEAM);		// Use I-beam cursor (like edit control)
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);	// Use default window color (overriden in OnEraseBkgnd)
	wc.lpszMenuName = NULL;							// No menus
	wc.lpszClassName = _T("IPEdit");				// Class name
	if (!::RegisterClass(&wc))						// If registration failed, subsequent dialogs will fail
	{
		ASSERT(FALSE);
		return FALSE;
	}
	else
		return TRUE;
}

BOOL CIPEdit::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwExStyle/*=0*/)
{
	// Create a window class that has the properties we want
	CString szWndClass = AfxRegisterWndClass(CS_GLOBALCLASS | CS_VREDRAW | CS_HREDRAW,
		::LoadCursor(NULL, IDC_IBEAM), (HBRUSH) COLOR_WINDOW+1);

	// Create using the extended window style
#if _MSC_VER >= 1100
	// Original VC 5.0 stuff
	return CWnd::CreateEx(dwExStyle, szWndClass, NULL, dwStyle, rect, pParentWnd, nID);
#else
	// Back ported to VC 4.2
	return CWnd::CreateEx(dwExStyle, szWndClass, NULL, dwStyle,
		rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top,
		pParentWnd->GetSafeHwnd(), (HMENU) nID);
#endif
}

int CIPEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)	
		return -1;

	// Save the "no immediate validation on input" style setting
	m_bNoValidate = (lpCreateStruct->style & IPAS_NOVALIDATE);

	// Set the styles for the parent control
	ModifyStyleEx(0, WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY);

	// Create the four edit controls used to obtain the four parts of the IP address (size
	// of controls gets set during OnSize)
	for (int ii = 0; ii < 4; ii++)
	{
		m_Addr[ii].Create(WS_EDIT, CRect(0,0,0,0), this, IDC_ADDR1 + ii);
		m_Addr[ii].LimitText(3);
		m_Addr[ii].SetParent(this);
	}
	
	return 0;
}

void CIPEdit::OnNcDestroy() 
{
	CWnd::OnNcDestroy();
	
	// Make sure the window was destroyed
	ASSERT(NULL == m_hWnd);

	// Destroy this object since it won't be destroyed otherwise
}

void CIPEdit::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	// Get the width of a "." drawn in the control
	CDC*	pDC = GetDC();
	CSize	szDot = pDC->GetTextExtent(_T("."), 1);
	int nDotWidth = szDot.cx;
	ReleaseDC(pDC);	

	// Based on the size of the parent window, compute the width & height of the edit
	// controls.  Leave room for the three "." which will be drawn on the parent window
	// to separate the four parts of the IP address.
	CRect	rcClient;
	GetClientRect(&rcClient);
	int nEditWidth = (rcClient.Width() - (3 * nDotWidth)) / 4;
	int nEditHeight = rcClient.Height();
	int cyEdge = ::GetSystemMetrics(SM_CYEDGE);

	// Compute rectangles for the edit controls, then move the controls into place
	CRect rect = CRect(0, cyEdge, nEditWidth, nEditHeight);
	for (int ii = 0; ii < 4; ii++)
	{
		m_rcAddr[ii] = rect;
		m_Addr[ii].MoveWindow(rect);
		rect.OffsetRect(nEditWidth + nDotWidth, 0);
	}

	rect = CRect(nEditWidth, 0, nEditWidth + nDotWidth, nEditHeight);
	for (ii = 0; ii < 3; ii++)
	{
		m_rcDot[ii] = rect;
		rect.OffsetRect(nEditWidth + nDotWidth, 0);
	}
}

void CIPEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
	m_Addr[0].SetFocus();							// Set focus to first edit control
	m_Addr[0].SetSel(0, -1);						// Select entire contents
}

// Protected function called by the edit control (friend class) when it receives a
// character which should be processed by the parent
void CIPEdit::OnChildChar(UINT nChar, UINT nRepCnt, UINT nFlags, CIPSingleEdit& child)
{
	switch (nChar)
	{
	case '.':										// Dot means advance to next edit control (if in first 3)
	case VK_RIGHT:									// Ditto for right arrow at end of text
	case ' ':										// Ditto for space
		{
		UINT nIDC = child.GetDlgCtrlID();			// Get control ID of the edit control
		if (nIDC < IDC_ADDR4)						// Move focus to appropriate edit control and select entire contents
		{
			m_Addr[nIDC - IDC_ADDR1 + 1].SetFocus();
			if (VK_RIGHT != nChar)					// Re-select text unless arrow key entered
				m_Addr[nIDC - IDC_ADDR1 + 1].SetSel(0, -1);
		}		
		break;
		}

	case VK_LEFT:									// Left arrow means move to previous edit control (if in last 3)
		{
		UINT nIDC = child.GetDlgCtrlID();			// Get control ID of the edit control
		if (nIDC > IDC_ADDR1)						// Move focus to appropriate edit control
			m_Addr[nIDC - IDC_ADDR1 - 1].SetFocus();
		break;
		}

	case VK_TAB:									// Tab moves between controls in the dialog
		{
		CWnd*	pWnd;
		SHORT nShift = ::GetKeyState(VK_SHIFT);		// Get state of shift key
		if (nShift < 0)
			pWnd = GetParent()->GetNextDlgTabItem(this, TRUE);
		else
			pWnd = GetParent()->GetNextDlgTabItem(this, FALSE);
		if (NULL != pWnd)							// If there's a control, set focus to it
			pWnd->SetFocus();
		break;
		}

    /*
	case VK_RETURN:									// Return implies default pushbutton press
		{
		DWORD dw = ((CDialog*) GetParent())->GetDefID();	// Get ID of default pushbutton
		if (DC_HASDEFID == HIWORD(dw))				// If there is a default pushbutton, simulate pressing it
		{
			CWnd* pWnd = GetParent()->GetDlgItem(LOWORD(dw));	// Get the control
			WPARAM wp = MAKEWPARAM(LOWORD(dw), BN_CLICKED);		// Build wParam for WM_COMMAND
			GetParent()->SendMessage(WM_COMMAND, wp, (LPARAM) pWnd->m_hWnd);	// Fake like button was pressed
		}
		}
    */
		break;

	case '-':										// "Field full" indication
		// Validate the contents for proper values (unless suppressed)
		if (!m_bNoValidate)							// If not suppressing immediate validation
		{
			CString		szText;

			child.GetWindowText(szText);			// Get text from edit control
			int n = _ttoi(szText);					// Get numeric value from edit control
			if (n < 0 || n > 255)					// If out of range, notify parent
			{
        n = 255;
        child.SetWindowText( "255" );
				child.SetFocus();					// Set focus to offending field
				child.SetSel(0, -1);				// Select all text
				return;
			}
		}

		// Advance to next field
		OnChildChar('.', 0, nFlags, child);
		break;

	default:
		TRACE(_T("Unexpected call to CIPAddrCtl::OnChildChar!\n"));
	}
}

void CIPEdit::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// Save mode and set to transparent (so background remains)
	int nOldMode = dc.SetBkMode(TRANSPARENT);

	// If disabled, set text color to COLOR_GRAYTEXT, else use COLOR_WINDOWTEXT
	COLORREF crText;
	if (m_bEnabled)
		crText = ::GetSysColor(COLOR_WINDOWTEXT);
	else
		crText = ::GetSysColor(COLOR_GRAYTEXT);
	COLORREF crOldText = dc.SetTextColor(crText);

	// Draw the three "." which separate the four edit controls
	for (int ii = 0; ii < 3; ii++)
		dc.DrawText(_T("."), 1, m_rcDot[ii], DT_CENTER | DT_SINGLELINE | DT_BOTTOM);

	// Restore old mode and color
	dc.SetBkMode(nOldMode);
	dc.SetTextColor(crOldText);

	// Do not call CWnd::OnPaint() for painting messages
}

BOOL CIPEdit::OnEraseBkgnd(CDC* pDC) 
{
	CRect	rcClient;
	GetClientRect(&rcClient);

	if (m_bEnabled && !m_bReadOnly)
		::FillRect(pDC->m_hDC, rcClient, (HBRUSH) (COLOR_WINDOW+1));
	else
		::FillRect(pDC->m_hDC, rcClient, (HBRUSH) (COLOR_BTNFACE+1));

	return TRUE;
}

void CIPEdit::OnEnable(BOOL bEnable) 
{
	CWnd::OnEnable(bEnable);
	
	// Nothing to do unless the window state has changed
	if (bEnable != m_bEnabled)
	{
		// Save new state
		m_bEnabled = bEnable;

		// Adjust child controls appropriately
		for (int ii = 0; ii < 4; ii++)
			m_Addr[ii].EnableWindow(bEnable);
	}

	Invalidate();
}

LONG CIPEdit::OnSetFont(UINT wParam, LONG lParam)
{
	// Note: font passed on to children, but we don't
	// use it, the system font is much nicer for printing
	// the dots, since they show up much better

	for (int ii = 0; ii < 4; ii++)
		m_Addr[ii].SendMessage(WM_SETFONT, wParam, lParam);

	return 0;
}

LONG CIPEdit::OnGetAddress(UINT wParam, LONG lParam)
{
	BOOL bStatus;
	int	i, nAddr[4], nInError = 0;

	BOOL bPrintErrors = (BOOL) wParam;				// Cast wParam as a flag
	IPA_ADDR* lpIPAddr = (IPA_ADDR*) lParam;		// Cast lParam as an IPA_ADDR structure
	if (NULL == lpIPAddr)							// If it's a bad pointer, return an error
		return FALSE;
	memset(lpIPAddr, 0, sizeof(IPA_ADDR));			// Zero out the returned data

	// Parse the fields and return an error indication if something bad was detected
	for (i = 0; i < 4; i++)
	{
		bStatus = ParseAddressPart(m_Addr[i], nAddr[i]);
		if (!bStatus)								// If it failed to parse, quit now
		{
			nInError = i + 1;						// Remember which address part was in error
			break;									// Break out of for loop
		}
	}
	if (!bStatus)									// Error detected during parse?
	{
		lpIPAddr->nInError = nInError;				// Show where it occurred
		if (bPrintErrors)							// If they want us to print error messages
		{
			CString	szText;
			if (nAddr[i] < 0)
				szText = _T("Missing value in IP address");
			else
				szText.Format(_T("%d is not a valid entry.  Please specify a value between 0 and 255 for this field."), nAddr[i]);
			MessageBox(szText, _T("Error"), MB_OK | MB_ICONEXCLAMATION);
		}
		m_Addr[i].SetFocus();						// Set focus to offending byte
		m_Addr[i].SetSel(0, -1);					// Select entire contents
		return FALSE;								// Return an error
	}

	lpIPAddr->nAddr1 = nAddr[0];					// Return the pieces to the caller
	lpIPAddr->nAddr2 = nAddr[1];
	lpIPAddr->nAddr3 = nAddr[2];
	lpIPAddr->nAddr4 = nAddr[3];
	lpIPAddr->nInError = 0;							// No error to report
	return TRUE;									// Return success
}

LONG CIPEdit::OnSetAddress( UINT wParam, LONG lParam )
{
	CString		szText;

	IPA_ADDR* lpIPAddr = (IPA_ADDR*) lParam;		// Cast lParam as an IPA_ADDR structure

	// Format their data and load the edit controls
	szText.Format(_T("%u"), lpIPAddr->nAddr1);
	m_Addr[0].SetWindowText(szText);
	szText.Format(_T("%u"), lpIPAddr->nAddr2);
	m_Addr[1].SetWindowText(szText);
	szText.Format(_T("%u"), lpIPAddr->nAddr3);
	m_Addr[2].SetWindowText(szText);
	szText.Format(_T("%u"), lpIPAddr->nAddr4);
	m_Addr[3].SetWindowText(szText);
	return TRUE;
}


BOOL CIPEdit::SetAddress( BYTE ucAddr1, BYTE ucAddr2, BYTE ucAddr3, BYTE ucAddr4 )
{

  IPA_ADDR    ipAddr;

  ipAddr.nAddr1 = ucAddr1;
  ipAddr.nAddr2 = ucAddr2;
  ipAddr.nAddr3 = ucAddr3;
  ipAddr.nAddr4 = ucAddr4;
  ipAddr.nInError = 0;

  return SetAddress( &ipAddr );

}



BOOL CIPEdit::GetAddress( BYTE& ucAddr1, BYTE& ucAddr2, BYTE& ucAddr3, BYTE& ucAddr4 )
{

  IPA_ADDR    ipAddr;

  GetAddress( FALSE, &ipAddr );

  ucAddr1 = ipAddr.nAddr1;
  ucAddr2 = ipAddr.nAddr2;
  ucAddr3 = ipAddr.nAddr3;
  ucAddr4 = ipAddr.nAddr4;

  return TRUE;

}



LONG CIPEdit::OnSetReadOnly(UINT wParam, LONG lParam)
{
	m_bReadOnly = (BOOL) wParam;

	for (int ii = 0; ii < 4; ii++)
		m_Addr[ii].SetReadOnly(m_bReadOnly);

	Invalidate();
	return TRUE;
}

BOOL CIPEdit::ParseAddressPart(CEdit& edit, int& n)
{
	CString		szText;

	edit.GetWindowText(szText);						// Get text from edit control
	if (szText.IsEmpty())							// Empty text is an error
	{
		n = -1;										// Return bogus value
		return FALSE;								// Return parse failure to caller
	}

	n = _ttoi(szText);								// Grab a decimal value from edit text
	if (n < 0 || n > 255)							// If it is out of range, return an error
		return FALSE;

	return TRUE;									// Looks acceptable, return success
}

CIPSingleEdit* CIPEdit::GetEditControl(int nIndex)
{
	if (nIndex < 1 || nIndex > 4)
		return NULL;
	return &m_Addr[nIndex - 1];
}


/////////////////////////////////////////////////////////////////////////////

// CIPSingleEdit

CIPSingleEdit::CIPSingleEdit()
{
}

CIPSingleEdit::~CIPSingleEdit()
{
}

void CIPSingleEdit::SetParent(CIPEdit* pParent)
{
	m_pParent = pParent;							// Save pointer to parent control
}


BEGIN_MESSAGE_MAP(CIPSingleEdit, CEdit)
	//{{AFX_MSG_MAP(CIPSingleEdit)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIPSingleEdit message handlers

void CIPSingleEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// Logic for this function:
	// Tab and dot are forwarded to the parent.  Tab (or shift-tab) operate
	// just like expected (focus moves to next control after the parent).
	// Dot or space causes the parent to set focus to the next child edit (if
	// focus is currently set to one of the first three edit controls).
	// Numerics (0..9) and control characters are forwarded to the standard
	// CEdit OnChar method; all other characters are dropped.
	if (VK_TAB == nChar ||
		'.' == nChar ||
		' ' == nChar ||
		VK_RETURN == nChar)
		m_pParent->OnChildChar(nChar, nRepCnt, nFlags, *this);
	else if (('0' <= nChar && '9'>= nChar) || iscntrl(nChar))
	{
		CEdit::OnChar(nChar, nRepCnt, nFlags);

		// Automatically advance to next child control if 3 characters were entered;
		// use "-" to indicate field was full to OnChildChar
		if (3 == GetWindowTextLength())
			m_pParent->OnChildChar('-', 0, nFlags, *this);
	}
	else
		::MessageBeep(0xFFFFFFFF);
}

void CIPSingleEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// Handle the left and right arrow keys.  If the left arrow key is pressed
	// with the caret at the left of the input text, shift focus to the previous
	// control (if in edit controls 2-4).  Likewise for the right arrow key.
	// This is done by calling the parent's OnChildChar method.
	// If not left or right arrow, or not at beginning or end, call default
	// OnKeyDown processor so key down gets passed to edit control.
	if (VK_LEFT == nChar || VK_RIGHT == nChar)
	{
		CPoint ptCaret = GetCaretPos();
		int nCharPos = LOWORD(CharFromPos(ptCaret));
		if ((VK_LEFT == nChar && nCharPos == 0) ||
			(VK_RIGHT == nChar && nCharPos == GetWindowTextLength()))
			m_pParent->OnChildChar(nChar, nRepCnt, nFlags, *this);
	}
	else if (VK_ESCAPE == nChar)
	{
		// Must handle VK_ESCAPE specially.
		// In a normal dialog, VK_ESCAPE gets handled by the dialog box window process,
		// and the edit control never sees the character.  When the edit control lives
		// on a property page, this doesn't happen.  If VK_ESCAPE is not handled here
		// then when the escape key is pressed the entire control disappears from the
		// dialog!  If the parent of the control is a dialog, and the dialog's parent
		// is also a dialog, that's a sign that the control is on a property page.
		TCHAR	cClass1[12];
		TCHAR	cClass2[12];

		// Clear out window class name arrays
		memset(cClass1, 0, sizeof(cClass1));
		memset(cClass2, 0, sizeof(cClass2));

		// Get parent and parent's parent window pointers
		CWnd* pDialog = m_pParent->GetParent();		// Get parent of the IP address edit control
		CWnd* pDialogParent = pDialog->GetParent();	// Get its parent

		// Get class names of the windows that own the IP address edit control and its parent
		if (NULL != pDialog)
			::GetClassName(pDialog->m_hWnd, cClass1, sizeof(cClass1)/sizeof(TCHAR));
		if (NULL != pDialogParent)
			::GetClassName(pDialogParent->m_hWnd, cClass2, sizeof(cClass2)/sizeof(TCHAR));

		// If parent is a dialog, and parent's parent is a dialog, simulate Cancel button press
		if (0 == _tcscmp(cClass1, szDialogClass) && 0 == _tcscmp(cClass2, szDialogClass))
		{
			CWnd* pCancel = pDialogParent->GetDlgItem(IDCANCEL);	// Get the Cancel button
			pDialogParent->SendMessage(WM_COMMAND,
				MAKEWPARAM(IDCANCEL, BN_CLICKED),
				(LPARAM) (NULL != pCancel->m_hWnd ? pCancel->m_hWnd : NULL));
			return;
		}
	}
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

