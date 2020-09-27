#ifndef _IPADDR_H							// Only include this file once
#define	_IPADDR_H

typedef struct tagIPA_ADDR {
	unsigned char	nAddr1;	// The four bytes of the IP address
	unsigned char	nAddr2;
	unsigned char	nAddr3;
	unsigned char	nAddr4;
	UINT	nInError;		// Address byte with erroroneous/missing data (1..4)
} IPA_ADDR;

// IP Address control style bits

#define	IPAS_NOVALIDATE	0x0001	// Don't validate fields as they are input

// Messages to/from the IP Address control
enum IPAM {
	IPAM_GETADDRESS = WM_USER,
	IPAM_SETADDRESS,
	IPAM_SETREADONLY
};

// Resource IDs for our edit controls
enum RID {
	IDC_ADDR1 = 1,
	IDC_ADDR2,
	IDC_ADDR3,
	IDC_ADDR4
};

/////////////////////////////////////////////////////////////////////////////
// CIPAddrEdit window

class CIPEdit;

class CIPSingleEdit : public CEdit
{
// Construction
public:
	CIPSingleEdit();

// Attributes
public:
	friend class CIPEdit;

	// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIPAddrEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIPSingleEdit();

	// Generated message map functions
protected:
	CIPEdit* m_pParent;
	void SetParent(CIPEdit* pParent);
	//{{AFX_MSG(CIPAddrEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CIPAddrCtl window

class CIPEdit : public CWnd
{
    DECLARE_DYNCREATE(CIPEdit)
// Construction
public:
	static BOOL Register();
	CIPEdit();

// Attributes
public:
	friend class CIPSingleEdit;

// Operations
public:
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwExStyle=0);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIPAddrCtl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIPEdit();
	BOOL GetAddress(BOOL bPrintErrors, IPA_ADDR* lpIPAddr)
		{ return (BOOL) ::SendMessage(m_hWnd, IPAM_GETADDRESS, (WPARAM) bPrintErrors, (LPARAM) lpIPAddr); };
	BOOL SetAddress(IPA_ADDR* lpIPAddr)
		{ return (BOOL) ::SendMessage(m_hWnd, IPAM_SETADDRESS, 0, (LPARAM) lpIPAddr); };
	BOOL SetAddress( BYTE ucAddr1, BYTE ucAddr2, BYTE ucAddr3, BYTE ucAddr4 );
  BOOL GetAddress( BYTE& ucAddr1, BYTE& ucAddr2, BYTE& ucAddr3, BYTE& ucAddr4 );
	BOOL SetReadOnly(BOOL bReadOnly = TRUE)
		{ return (BOOL) ::SendMessage(m_hWnd, IPAM_SETREADONLY, (WPARAM) bReadOnly, 0); };
	CIPSingleEdit* GetEditControl(int nIndex);

	// Generated message map functions
protected:
	void OnChildChar(UINT nChar, UINT nRepCnt, UINT nFlags, CIPSingleEdit& child);
	//{{AFX_MSG(CIPAddrCtl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnPaint();
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg LONG OnSetFont(UINT wParam, LONG lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LONG OnGetAddress(UINT wParam, LONG lParam);
	afx_msg LONG OnSetAddress(UINT wParam, LONG lParam);
	afx_msg LONG OnSetReadOnly(UINT wParam, LONG lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	CIPSingleEdit m_Addr[4];
	CRect	m_rcAddr[4];
	CRect	m_rcDot[3];

	BOOL ParseAddressPart(CEdit& edit, int& n);

	BOOL m_bEnabled;
	BOOL m_bReadOnly;
	BOOL m_bNoValidate;

	static BOOL m_bRegistered;
};

// WNDPROC

/////////////////////////////////////////////////////////////////////////////

#endif	// _IPADDR_H
