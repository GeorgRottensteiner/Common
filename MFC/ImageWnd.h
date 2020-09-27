#if !defined(AFX_IMAGEWND_H__FBF4C953_B061_4080_BF76_8A126BAEB85F__INCLUDED_)
#define AFX_IMAGEWND_H__FBF4C953_B061_4080_BF76_8A126BAEB85F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



namespace GR
{
  namespace Graphic
  {
    class Image;
    class GDIPage;
    class GFXPage;
  }
}



class CImageWnd : public CWnd
{
  // Construction
  public:

    #define IMAGEWND_CLASS_NAME _T( "ImageWnd" )

    static BOOL hasclass;
    static BOOL RegisterMe();

	  CImageWnd();


    GR::Graphic::GDIPage*      m_pPage;

    GR::Graphic::Image*     m_pImage;
    GR::Graphic::Image*     m_pOwnedImage;



    void            SetImage( GR::Graphic::Image* pImage );
    void            CopyPage( GR::Graphic::GFXPage* pPage );
    GR::Graphic::Image*     GetImage() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GR::Graphic::ImageWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CImageWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(GR::Graphic::ImageWnd)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
  virtual void PreSubclassWindow();
};

#endif // !defined(AFX_IMAGEWND_H__FBF4C953_B061_4080_BF76_8A126BAEB85F__INCLUDED_)
