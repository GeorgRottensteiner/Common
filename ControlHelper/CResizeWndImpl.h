#ifndef __INCLUDE_CRESIZEDIALOGIMPLEMENTATION_H__
#define __INCLUDE_CRESIZEDIALOGIMPLEMENTATION_H__
/*--------------------+-------------------------------------------------------+
 | Programmname       : Window-Helper                                         |
 +--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 17.01.2002                                            |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include "CDialog.h"

#include <list>



/*-Prototypen-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

template <class T> class CResizeWndImpl : public T
{

  protected:

    struct tControlEntry
    {
      HWND      hwndControl;
      DWORD     dwSizingFlags;
      RECT      rcOrig;

      tControlEntry() :
        hwndControl( NULL ),
        dwSizingFlags( 0 )
      {
      }
    };

    typedef std::list<tControlEntry>    tListResizingControls;

    tListResizingControls               m_listResizingControls;


  public:

    enum eSizeFlags
    {
      SF_NONE = 0,
      SF_POSITION_X     = 0x00000001,
      SF_POSITION_Y     = 0x00000002,
      SF_ALIGN_RIGHT    = 0x00000004,
      SF_ALIGN_BOTTOM   = 0x00000008,
      SF_RELATIVE_W     = 0x00000010,
      SF_RELATIVE_H     = 0x00000020,
      SF_WIDTH          = 0x00000040,
      SF_HEIGHT         = 0x00000080,
    };

    void            AddResizingControl( DWORD dwControlID, DWORD dwSizingFlags )
    {
      tControlEntry     Control;

      Control.hwndControl = GetDlgItem( dwControlID );
      if ( Control.hwndControl == NULL )
      {
        return;
      }
      Control.dwSizingFlags = dwSizingFlags;

      ::GetWindowRect( Control.hwndControl, &Control.rcOrig );
      ScreenToClient( &Control.rcOrig );
      
      m_listResizingControls.push_back( Control );
    }

  protected:

    DWORD           m_dwSizingFlags;

    RECT            m_CurrentSize,
                    m_OrigSize;


    virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
    {

      switch ( uMsg )
      {  
        case WM_CREATE:
          GetClientRect( &m_CurrentSize );
          m_OrigSize = m_CurrentSize;
          break;
        case WM_SIZE:
          {
            RECT    rc;

            GetClientRect( &rc );

            ResizeControls( LOWORD( lParam ), HIWORD( lParam ) );
          }
          break;
      }

      return T::WindowProc( uMsg, wParam, lParam );
    }

    void            ResizeControls( int iWidth, int iHeight )
    {

      int       iOldWidth   = m_CurrentSize.right - m_CurrentSize.left;
      int       iOldHeight  = m_CurrentSize.bottom - m_CurrentSize.top;

      HDWP  hdwp = BeginDeferWindowPos( 8 );

      tListResizingControls::iterator   it( m_listResizingControls.begin() );
      while ( it != m_listResizingControls.end() )
      {
        tControlEntry&    Control = *it;

        RECT    rc;

        ::GetWindowRect( Control.hwndControl, &rc );
        ScreenToClient( &rc );

        if ( Control.dwSizingFlags & SF_RELATIVE_W )
        {
          rc.left   = Control.rcOrig.left * iWidth / m_OrigSize.right;
          rc.right  = rc.left + ( Control.rcOrig.right - Control.rcOrig.left ) * iWidth / m_OrigSize.right;
        }
        if ( Control.dwSizingFlags & SF_RELATIVE_H )
        {
          rc.top   = Control.rcOrig.top * iHeight / m_OrigSize.bottom;
          rc.bottom  = rc.top + ( Control.rcOrig.bottom - Control.rcOrig.top ) * iHeight / m_OrigSize.bottom;
        }
        if ( Control.dwSizingFlags & SF_WIDTH )
        {
          rc.right = iWidth - ( m_OrigSize.right - Control.rcOrig.right );
        }
        if ( Control.dwSizingFlags & SF_HEIGHT )
        {
          rc.bottom = iHeight - ( m_OrigSize.bottom - Control.rcOrig.bottom );
        }
        if ( Control.dwSizingFlags & SF_ALIGN_RIGHT )
        {
          OffsetRect( &rc, iWidth - iOldWidth, 0 );
        }
        if ( Control.dwSizingFlags & SF_ALIGN_BOTTOM )
        {
          OffsetRect( &rc, 0, iHeight - iOldHeight );
        }

        hdwp = DeferWindowPos( hdwp, Control.hwndControl, m_hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOZORDER );

        ++it;
      }

      EndDeferWindowPos( hdwp );

      GetClientRect( &m_CurrentSize );

    }
};



#endif //__INCLUDE_CRESIZEDIALOGIMPLEMENTATION_H__