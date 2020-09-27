#ifndef INCLUDE_CGROUPBOX_H
#define INCLUDE_CGROUPBOX_H



#include "CButton.h"



namespace WindowsWrapper
{

  class CGroupBox : public CButton
  {

    protected:

    HFONT               m_hFont;

    HBITMAP             m_hbmCheckBox;

    void                _RegisterClass();

    bool                m_bChecked;


    public:

    CGroupBox();
    virtual ~CGroupBox();


    virtual BOOL        PreTranslateMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult );

    virtual void        OnPaint( HDC hdc );

    void                GetCheckBoxRect( RECT& rc );
    void                GetLabelRect( RECT& rc );

    virtual void        SetCheck( int iCheckState );
    virtual int         GetCheck() const;

    void                EnableContainingControls( bool bEnable = true );

    virtual void        OnAttach();

    virtual LRESULT     WindowProc( UINT message, WPARAM wParam, LPARAM lParam );


  };

}

#endif //__INCLUDE_CGROUPBOX_H__



