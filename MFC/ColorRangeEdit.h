#if !defined(AFX_COLORRANGEEDIT_H__E7AA1AE5_845C_4A53_8EF4_92F8C235D722__INCLUDED_)
#define AFX_COLORRANGEEDIT_H__E7AA1AE5_845C_4A53_8EF4_92F8C235D722__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorRangeEdit.h : header file
//

#include <MFC/RangeEdit.h>



class ColorRangeEdit : public RangeEdit
{

  protected:

    HBITMAP         m_hbmColorBar;

    COLORREF        GetColor( int Pos );

    //{{AFX_MSG(CColorRangeEdit)
    // NOTE - the ClassWizard will add and remove member functions here.
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()


  public:


	  ColorRangeEdit();
    virtual ~ColorRangeEdit();

    virtual void    SetRange( int Min, int Max, int Step = 1 );
    virtual void    SetRange( float Min, float Max, float Step = 0.02f, int DigitsPreComma = -1, int DigitsPostComma = 2 );
    virtual void    SetPosition( float Pos );

    virtual void    DrawPopup( HDC hdc, const RECT& Target );

  // Overrides
	  // ClassWizard generated virtual function overrides
	  //{{AFX_VIRTUAL(CColorRangeEdit)
	  //}}AFX_VIRTUAL

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORRANGEEDIT_H__E7AA1AE5_845C_4A53_8EF4_92F8C235D722__INCLUDED_)
