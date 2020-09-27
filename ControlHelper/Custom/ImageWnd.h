#if !defined(AFX_IMAGEWND_H__FBF4C953_B061_4080_BF76_8A126BAEB85F__INCLUDED_)
#define AFX_IMAGEWND_H__FBF4C953_B061_4080_BF76_8A126BAEB85F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImageWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImageWnd window

#include <ControlHelper/CWnd.h>



namespace GR
{
  class CImage;
};
class CWinGPage;
class CGfxPage;

class CImageWnd : public CWnd
{
  // Construction
  public:

    #define IMAGEWND_CLASS_NAME "ImageWnd"

    static BOOL hasclass;
    static BOOL RegisterMe();

	  CImageWnd();


    CWinGPage*      m_pPage;

    GR::CImage*     m_pImage;
    GR::CImage*     m_pOwnedImage;



    void            SetImage( GR::CImage* pImage );
    void            CopyPage( CGfxPage* pPage );
    GR::CImage*     GetImage() const;

    bool            Create( DWORD dwStyle, const RECT& rect, HWND hwndParent, UINT_PTR nID, LPVOID lpParam = 0 );


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CImageWnd();

	// Generated message map functions
protected:
	void OnPaint();
  virtual void OnAttach();


    virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGEWND_H__FBF4C953_B061_4080_BF76_8A126BAEB85F__INCLUDED_)
