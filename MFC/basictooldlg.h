#if !defined(AFX_BASICTOOLDLG_H__9F96B783_16FD_11D6_AC02_00104B6795C0__INCLUDED_)
#define AFX_BASICTOOLDLG_H__9F96B783_16FD_11D6_AC02_00104B6795C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BasicToolDlg.h : Header-Datei
//


LRESULT HandleNCActivate( HWND hwndMain, HWND hWnd, WPARAM wParam, LPARAM lParam, WNDPROC wndProc );
LRESULT HandleActivate( HWND hwndMain, HWND hWnd, WPARAM wParam, LPARAM lParam, WNDPROC wndProc );

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CBasicToolDlg 

namespace BASICTOOLDLG
{
  void InsertWindow( HWND hWnd );
  void RemoveWindow( HWND hWnd );
}


class CBasicToolDlg : public CDialog
{
// Konstruktion
public:
  CBasicToolDlg(DWORD dwID, CWnd* pParent /*=NULL*/);

// Dialogfelddaten
	//{{AFX_DATA(CBasicToolDlg)
	enum { IDD = 1 };
		// HINWEIS: Der Klassen-Assistent fügt hier Datenelemente ein
	//}}AFX_DATA


  virtual void    Restore();
  virtual void    Minimize();
  void    SetMinimizedPos( int iY );
  void    SetRestoredPosition( int iX, int iY );
  int     GetRestoredX();
  int     GetRestoredY();
  void    SetIcon( HICON hIcon );
  bool    Minimized() { return m_bMinimized; }

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CBasicToolDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CBasicToolDlg)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnNcDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


  bool    m_bMinimized;

  HICON   m_hIcon;

  HMENU   m_hMenu;
  
  RECT    m_rectOrig,
          m_rectSmall;

  int     m_iYOffset;

  DWORD   m_dwMaximizedStyle,
          m_dwMaximizedStyleEx;
      
	virtual void OnOK();
	virtual void OnCancel();


};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_BASICTOOLDLG_H__9F96B783_16FD_11D6_AC02_00104B6795C0__INCLUDED_
