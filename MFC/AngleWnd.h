#if !defined(AFX_ANGLEWND_H__84015923_8108_11D5_AC02_00104B6795C0__INCLUDED_)
#define AFX_ANGLEWND_H__84015923_8108_11D5_AC02_00104B6795C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AngleWnd.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Fenster CAngleWnd 

class CAngleWnd : public CWnd
{
// Konstruktion
public:
	CAngleWnd();

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CAngleWnd)
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CAngleWnd();

	// Generierte Nachrichtenzuordnungsfunktionen
    #define ANGLEWND_CLASS_NAME _T( "GRAngleWnd" )

    static BOOL hasclass;
    static BOOL RegisterMe();

    void        SetAngle( float fAngle );
    float       GetAngle() const;

protected:


  float       m_fAngle;
	//{{AFX_MSG(CAngleWnd)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnKillFocus(CWnd* pNewWnd);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_ANGLEWND_H__84015923_8108_11D5_AC02_00104B6795C0__INCLUDED_
