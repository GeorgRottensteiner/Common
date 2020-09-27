#ifndef __INCLUDE_CRESIZEDIALOG_H__
#define __INCLUDE_CRESIZEDIALOG_H__
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
#include "CResizeWndImpl.h"

#include <list>



/*-Prototypen-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CResizeDlg : public CResizeWndImpl<CDialog>
{

  public:

    CResizeDlg( DWORD dwResourceID = 0, HWND hwndParent = NULL )
    {
      m_dwResourceID = dwResourceID;
      m_hwndParent   = hwndParent;
    }

};



#endif //__INCLUDE_CRESIZEDIALOG_H__