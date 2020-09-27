#if !defined(AFX_GRADIENTRANGEEDIT_H__E7AA1AE5_845C_4A53_8EF4_92F8C235D722__INCLUDED_)
#define AFX_GRADIENTRANGEEDIT_H__E7AA1AE5_845C_4A53_8EF4_92F8C235D722__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GradientRangeEdit.h : header file
//

#include <MFC/RangeEdit.h>



class CGradientRangeEdit : public RangeEdit
{
// Construction
public:
	CGradientRangeEdit();

protected:

    HBITMAP         m_hbmColorBar;

    GR::u32         m_ColorGradient1,
                    m_ColorGradient2;

    COLORREF        GetColor( int Pos );

// Operations
public:

    virtual void    SetRange( int Min, int Max, int Step = 1 );
    virtual void    SetRange( float Min, float Max, float Step = 0.02f, int DigitsPreComma = -1, int DigitsPostComma = 2 );

    virtual void    SetPosition( float Pos );

    virtual void    DrawPopup( HDC hdc, const RECT& Target );

    void            SetColors( GR::u32 Color1, GR::u32 Color2 );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGradientRangeEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGradientRangeEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGradientRangeEdit)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORRANGEEDIT_H__E7AA1AE5_845C_4A53_8EF4_92F8C235D722__INCLUDED_)
