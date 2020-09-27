#if !defined(AFX_NONMODALDIALOG_H__4E6FC712_ED4D_11D4_BFB4_00A024ACB50F__INCLUDED_)
#define AFX_NONMODALDIALOG_H__4E6FC712_ED4D_11D4_BFB4_00A024ACB50F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// nonmodaldialog.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CNonModalDialog 
#include <PJLib/DesignPatterns/Observer.h>
#include <FREDCommon/FRED_NamedObject.h>

class CNonModalDialog : public CDialog, public CObserver
{
// Konstruktion
public:
// 	CNonModalDialog(CWnd* pParent = NULL);   // Standardkonstruktor
	CNonModalDialog(  FRED::CNamedObject& subject, 
                    const CString& title,  
                    const CString& message, 
                    CWnd* pParent = NULL );   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CNonModalDialog)
	enum { IDD = IDD_NOMODAL_DIALOG };
	CEdit	m_ctlEdit1;
	//}}AFX_DATA

  void OnCancel();

  FRED::CNamedObject* m_pSubject;
	virtual void OnUpdate();		// Virtual function to do the updating. Call Notify() if the value changes.

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CNonModalDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementierung
protected:

  virtual void Invalidate(CObserver *from = NULL);

  bool m_bChanging;
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CNonModalDialog)
	afx_msg void OnChangeEdit1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_NONMODALDIALOG_H__4E6FC712_ED4D_11D4_BFB4_00A024ACB50F__INCLUDED_
