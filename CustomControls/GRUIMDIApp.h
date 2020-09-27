#pragma once


#include <list>

#include "CustomWnd.h"
#include "GRUIStatusBar.h"


class GRUIMDIDocument;
class GRUIMDIView;

class GRUIMDIApp : public CCustomWnd
{

  protected:

    static GRUIMDIApp*    m_pMDIApp;

    static std::list<HWND>  m_listSyncWindows;

    typedef std::list<GRUIMDIDocument*>   tListDocuments;

    GRUIMDIView*          m_pActiveMDIChild;

    GRUIStatusBar         m_wndStatusBar;



    tListDocuments        m_listDocuments;

    GR::String           m_strCaption;

    CCustomWnd            m_wndMDIClient;


    virtual LRESULT       WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual void          Draw( HDC hdc, const RECT& rc );
    virtual void          DrawBackground( HDC hdc, const RECT& rc );


  public:


    GRUIMDIApp();
    virtual ~GRUIMDIApp();


    virtual bool          InitInstance();
    virtual void          ExitInstance();

    virtual void          RecalcLayout();

    int                   Run();

    bool                  NewDocument( GRUIMDIDocument* pDoc );

    void                  SetActiveView( GRUIMDIView* pView );


  private:

    static void           GetPopupList( HWND hwndMain, std::list<HWND>& listWindows, BOOL fIncMain );

    // Caption-Spielereien
    void                  SyncActivate( BOOL bActivate );
    void                  SyncEnable( BOOL bEnable );
    bool                  IsCaptionActive();

    friend class GRUIDockFrame;
    friend class GRUIMDIDocument;
    friend class GRUIMDIView;
    friend class GRUIMenuBar;

};



