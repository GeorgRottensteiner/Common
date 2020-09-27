#pragma once

#include <list>
#include <windows.h>


class GRUIMDIView;

class GRUIMDIDocument
{

  public:

    typedef std::list<GRUIMDIView*>   tListViews;


    tListViews                    m_listViews;

    HWND                          m_hwndMDIParent;
    
    bool                          m_bChanged;


    GRUIMDIDocument( HWND hwndMDIParent );
    virtual ~GRUIMDIDocument();


    void                          OnInitInstance();
    void                          OnExitInstance();


    void                          AddView( GRUIMDIView* pView );

    size_t                        ViewCount() const;
    virtual bool                  CanClose();

};
