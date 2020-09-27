#ifndef INCLUDE_CAPP_H
#define INCLUDE_CAPP_H



#include "CWnd.h"
#include "CResizeWndImpl.h"



namespace WindowsWrapper
{

  class CApp : public CResizeWndImpl<CWnd>
  {

    protected:

    HINSTANCE             m_hInstance;

    int                   m_ExitCode;


    public:


    CApp();
    virtual ~CApp();

    BOOL                  Create( GR::u32 Style, 
                                  int Width,
                                  int Height,
                                  const char* Name,
                                  UINT Menu = 0 );

    virtual bool          InitInstance();
    virtual void          ExitInstance();
    virtual int           Run();

    void                  ShutDown( int ExitCode = 0 );
    virtual LRESULT       WindowProc( UINT Message, WPARAM wParam, LPARAM lParam );

    HICON                 LoadIcon( UINT IDResource ) const;

  };

}

#endif //__INCLUDE_CAPP_H__



