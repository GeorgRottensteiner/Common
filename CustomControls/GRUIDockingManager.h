#pragma once

#include <list>

#include <windows.h>



class GRUIDockFrame;
class GRUIControlBar;
class GRUIMDIApp;

class GRUIDockingManager
{

  public:

    static GRUIMDIApp*                    m_pWndTopLevel;

    static bool                           m_bLoadingState;

    static GRUIDockFrame*                 m_pDockBars[4];

    static std::list<GRUIControlBar*>     m_listControlBars;

    static void                           EnableDocking( DWORD dwDockStyles );

    static void                           DockControlBar( GRUIControlBar* pBar, DWORD dwDockPos, POINT& ptBarPos );

    static void                           AddControlBar( GRUIControlBar* pBar );
    static void                           RemoveControlBar( GRUIControlBar* pBar );

    static DWORD                          CheckForDockingPosition( GRUIControlBar* pBar, POINT& ptPos );

    static void                           ShowControlBar( GRUIControlBar* pBar, BOOL bShow );

    static void                           SaveBarState();
    static void                           LoadBarState();


};
