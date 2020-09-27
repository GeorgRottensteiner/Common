#ifndef INCLUDE_CSPLITTERWND_H
#define INCLUDE_CSPLITTERWND_H



enum SplitterWndMode
{
  SWM_FREE = 0,
  SWM_RELATIVE,
  SWM_ABSOLUTE_FIRST_PANE,
  SWM_ABSOLUTE_SECOND_PANE,
};



#include <ControlHelper\CWnd.h>



namespace WindowsWrapper
{

  class CSplitterWnd : public CWnd
  {

    protected:

    RECT                  m_rcComplete;


    public:



    CSplitterWnd();
    virtual ~CSplitterWnd();


    virtual BOOL          Create( HWND hwndParent,
                                  const RECT& rect,
                                  bool bHorizontal,
                                  SplitterWndMode swMode,
                                  int iSplitModeValue,
                                  HWND hwndFirstPane,
                                  HWND hwndSecondPane,
                                  DWORD dwStyle = WS_CHILD | WS_VISIBLE,
                                  DWORD dwID = 0 );

    virtual void          SetPane( int iPane, HWND hwndPane );
    virtual void          Resize( int iWidth, int iHeight );

    virtual void          GetSplitterRect( RECT* pRect );

    virtual void          SetSplitMode( SplitterWndMode swMode, int iValue );

    virtual void          AllowDragging( bool bAllow = true );

    void                  SetExtents( int iWidth, int iHeight );


    protected:

    HWND                  m_hwndFirstPane,
      m_hwndSecondPane,
      m_hwndParent;

    bool                  m_bHorizontal,
      m_bDragging,
      m_bAllowDragging;

    int                   m_iSplitModeValue,
      m_iSplitterSize,
      m_iSplitterPos;

    SplitterWndMode       m_swMode;

    virtual LRESULT       WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

    void                  _RegisterClass();

    virtual void          UpdateSplitterPos();

  };

}

#endif // INCLUDE_CSPLITTERWND_H
