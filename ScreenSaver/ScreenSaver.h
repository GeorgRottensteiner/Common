#ifndef SCREENSAVER_H_INCLUDED
#define SCREENSAVER_H_INCLUDED



#ifndef __NO_DIRECT_X__
#define __NO_DIRECT_X__
#pragma message( "__NO_DIRECT_X__ sollte definiert sein!\n" )
#endif



#include <windows.h>
#include <string>

#include <Grafik\GfxPage.h>
#include <Grafik/Palette.h>
#include <Grafik\Font.h>
#include <Grafik\Image.h>



struct tScreenSaverInfo;

class ScreenSaver
{

  protected:

    enum eSaverMode
    {
      SM_INVALID = 0,
      SM_PREVIEW,
      SM_SAVER,
      SM_CONFIGURE,
    };

    eSaverMode          m_Mode;

    bool                m_bInitialized,
                        m_bDialogActive,
                        m_bMButton1Released,
                        m_bMButton2Released,
                        m_bTrueClose,         // Flag, um echte WM_CLOSEs von NT-Closes zu unterscheiden
                        m_bShutDownNow,

                        m_bClicked;

    HANDLE              m_hMutex;

    int                 m_iStartZeit,
                        m_iSaverStartZeit,

                        m_iOldMouseX,
                        m_iOldMouseY,

                        m_iClickedAtX,
                        m_iClickedAtY;

    HCURSOR             m_hCursor;

    DWORD               m_dwFlags,
                        m_dwFrameTime,
                        m_dwNoClose,
                        m_dwMouseLimit;

    GR::String         m_strAppName,
                        m_strAppPath,
                        m_strIniSection;


    void                SSUpdateFrame( void );
    void                CallExit();

    bool                VerifyPassword( void );


  public:

    enum eSaverFlags
    {
      SF_ALLOW_PREVIEW        = 0x0001,       // Preview als eigenen Modus erlauben
      SF_ALLOW_STRETCHING     = 0x0002,       // Stretchen, falls die Auflösung nicht paßt
      SF_ALLOW_CURSOR_MOVE    = 0x0004,       // Maus-Cursor anzeigen lassen
      SF_SHOW_CURSOR          = 0x0008,       // Maus-Cursor anzeigen lassen
      SF_GRAB_DESKTOP         = 0x0010,       // Desktop grabben
      SF_ALLOW_KEYBOARD       = 0x0020,       // Tastendrücke brechen den Saver nicht ab
      SF_ALLOW_MULTI_MONITOR  = 0x0040,       // läßt bei Multi-Monitor ein größeres Bild zu
    };

    HWND                m_hwndMain,
                        m_hwndFakePreview;

    HINSTANCE           m_hInstance;

    GR::Graphic::WinGPage   m_gfxPage;

    GR::Graphic::Palette    m_gfxPalette;

    GR::Graphic::Image*     m_pImageDesktop;

    int                 m_iMouseX,
                        m_iMouseY,
                        m_iWidth,
                        m_iHeight,
                        m_iFullWidth,
                        m_iFullHeight,
                        m_iVirtualDesktopWidth,
                        m_iVirtualDesktopHeight,
                        m_iMButton,

                        m_iXOffset,
                        m_iYOffset;

    BYTE                m_ucDepth;



    ScreenSaver();
    ~ScreenSaver();

    void                Release( void );

    bool                Configure( const tScreenSaverInfo& tssInfo );

    void                AllowStretching( BOOL bAllow = TRUE );
    void                AllowMouseMovement( BOOL bAllow = TRUE );
    void                ShutDown();

    void                SetCursor( HCURSOR hCursor )
    {
      m_hCursor = hCursor;
      ::SetCursor( m_hCursor );
    }

    BOOL                Clicked()
    {
      return m_bClicked;
    }
    void                UnClick()
    {
      m_bClicked = FALSE;
    }

    // zu überschreibende Funktionen
    virtual bool        InitInstance() = 0;
    virtual void        ExitInstance() = 0;

    virtual void        DisplayFrame( GR::Graphic::WinGPage *pPage ) = 0;
    virtual void        UpdateFrame() = 0;

    virtual void        ConfigureSaver() = 0;

    virtual LRESULT CALLBACK  WindowProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );


    void                Run();

    void                Invalidate( int iX = 0, int iY = 0, int iWidth = 0, int iHeight = 0 );

    // Helper
    void                WinPrint( int iX, int iY, const char* szText, DWORD dwColor );

    const char*         AppPath( const char* formatstr, ... );



    friend LRESULT CALLBACK ScreenSaverProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
    friend int PASCAL WinMain( HINSTANCE inst, HINSTANCE previnst, LPSTR cmdline, int cmdshow );

};

struct tScreenSaverInfo
{
  DWORD       dwWidth,
              dwHeight,
              dwFlags;
  BYTE        ucColorDepth;
  GR::String strAppName,
              strIniSection;

  tScreenSaverInfo() :
    dwWidth( 640 ),
    dwHeight( 480 ),
    dwFlags( ScreenSaver::SF_ALLOW_PREVIEW ),
    ucColorDepth( 16 ),
    strAppName( "ScreenSaverName" ),
    strIniSection( "ScreenSaverName" )
  {
  }

};



#endif // SCREENSAVER_H_INCLUDED