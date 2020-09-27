#pragma once

#include <ControlHelper/CWnd.h>

#include "ClassicDisplayClass.h"



class CGRUIMenu;

class CCustomWnd : public CWnd
{

  public:

    CCustomWnd();
    virtual ~CCustomWnd();

    BOOL              Create( LPCTSTR lpszWindowName, 
                              DWORD dwStyle, 
                              const RECT& rect, 
                              HWND hwndParent, 
                              UINT_PTR nID, 
                              LPVOID lpParam = 0 );
    BOOL              CreateEx( DWORD dwExtendedStyle,
                                LPCTSTR lpszWindowName, 
                                DWORD dwStyle, 
                                const RECT& rect, 
                                HWND hwndParent, 
                                UINT_PTR nID, 
                                LPVOID lpParam = 0 );
    BOOL              CreateEx( DWORD dwExtendedStyle,
                                const char* szClassName,
                                LPCTSTR lpszWindowName, 
                                DWORD dwStyle, 
                                const RECT& rect, 
                                HWND hwndParent, 
                                UINT_PTR nID, 
                                LPVOID lpParam = 0 );

    void                                SetBackgroundProducer( CCustomWnd* pWndProducer = NULL );

    virtual void                        OnParentResized();
    virtual void                        OnParentRecalcClientSize( RECT& rcClient );


  protected:

    GR::u32                             m_dwWantedStyles,
                                        m_dwWantedExtendedStyle;

    HBITMAP                             m_hbmDoubleBuffer;

    HDC                                 m_hdcPaint;

    CCustomWnd*                         m_pWndBackgroundProducer;

    bool                                m_bActive;

    static CClassicDisplayClass         m_BasicDisplayClass;

    static CClassicDisplayClass*        m_pDisplayClass;


    virtual LRESULT                     WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

    virtual void                        DrawBackground( HDC hdc, const RECT& rcRedraw );
    virtual void                        Draw( HDC hdc, const RECT& rc );

    bool                                GetCaptionRect( RECT& rc );
    bool                                GetCaptionCloseButtonRect( RECT& rc );
    bool                                GetCaptionMaximizeButtonRect( RECT& rc );
    bool                                GetCaptionMinimizeButtonRect( RECT& rc );
    bool                                GetSysMenuRect( RECT& rc );

    void                                CalculateClientRectFromWindowRect( RECT& rc );
    void                                CalculateWindowRectFromClientRect( RECT& rc );

    void                                CreateSysMenu( CGRUIMenu& menuSystem );

    DWORD                               NCHitTest( const POINT& pt );

    void                                DrawCaption( HDC hdc, bool bActive );


  protected:

    POINT                               m_ptDragOffset;

    bool                                m_bDraggingNC;

    bool                                m_bCaptionCloseButtonDown,
                                        m_bCaptionMaxButtonDown,
                                        m_bCaptionMinButtonDown;

    DWORD                               m_dwDraggingHT;

    HDC                                 m_HDCNCArea;
    HDC                                 m_HDCNCDoubleBuffer;
    HBITMAP                             m_hbmNCOld;

    PAINTSTRUCT                         m_psPaint;


    void                                _RegisterClass();

    virtual HDC                         BeginNCPaint();
    virtual void                        EndNCPaint();

    virtual HDC                         StartPaint();
    virtual void                        StopPaint();

};
