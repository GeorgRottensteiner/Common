#ifndef MASTERFRAME2D_H_INCLUDED
#define MASTERFRAME2D_H_INCLUDED



#pragma comment ( lib, "dxguid.lib" )
#include <string>
#include <Grafik/GfxPage.h>
#include <Grafik/Font.h>
#include <Xtreme/Input/DXInput.h>

#include <MasterFrame/MasterFrame.h>

#include <Lang/EventQueue.h>

#include <GR\GameState\MFGameState.h>


namespace GR
{
  namespace Graphic
  {
    class GDIPage;
    class DirectXPage;
  }
}


class MasterFrame2d : public MasterFrame, public IGlobalEventListener, public MFGameStateManager<GR::Graphic::GFXPage>
{
  private:

    std::map<GR::String,GR::Graphic::Image>   m_ImageManager;

    unsigned char                     m_CurrentDepth,
                                      m_OrigDepth;

    int                               m_imagesT;

    GR::String                        m_SnapShotFile;

    bool                              m_SaveSnapShot;

    GR::u32                           m_ETChangeState,
                                      m_ETPushState,
                                      m_ETPopState,
                                      m_ETShutDown,
                                      m_ETToggleWindow;

    GR::Graphic::Image                m_CustomMouseCursor;

    GR::tPoint                        m_MouseCursorOffset;

    bool                              m_UseCustomMouseCursor;



    void              ResetFont();
    void              DoSaveSnapShot( const char* szFile );


  public:

    GR::Graphic::GDIPage*             m_pWinGPage;

    GR::Graphic::DirectXPage*         m_pDirectXPage;

    GR::Graphic::Palette              m_Palette;

    GR::Font*        m_pConsoleFont;



    MasterFrame2d();
    virtual ~MasterFrame2d();

    virtual bool      InitInstance() = 0;
    virtual void      ExitInstance() = 0;

    virtual void      UpdatePerDisplayFrame( const float ElapsedTime ) = 0;
    virtual void      DisplayFrame( GR::Graphic::GFXPage* pPage ) = 0;
    virtual void      UpdateFrame() = 0;

    int               Run();

    void              Print( int X, int Y, const char* szText );
    void              Print( int X, int Y, const GR::String& strText );

    void              Reset();
    bool              CleanUp();

    bool              CreatePage();
    void              DestroyPage();

    void              SaveSnapShot( const char *szFile );
    void              SetCustomMouseCursor( GR::Graphic::Image* pImage = NULL, const GR::tPoint& ptCursorHotSpot = GR::tPoint() );

    void              DisplayConsole();
    virtual void      ProcessCommandTokens( GR::Tokenizer::TokenSequence& m_TokenSequence, GR::String& strCommand );

    virtual void      ProcessEvent( const tGlobalEvent& Event );

    virtual bool      ToggleFullScreen();

    virtual GR::Graphic::GFXPage* GetPage();
    virtual void      SetPalette( GR::Graphic::Palette *pPalette );

    virtual bool      Create( int NewWidth, int NewHeight, unsigned char Depth, GR::u32 Flags );
    virtual BOOL      WindowProc2d( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );


    // Image-Manager-related
    GR::Graphic::Image*       LoadImage( const char* szName );
    void              AddImage( const char* szName, GR::Graphic::Image* pImage );
    void              DeleteImage( const char* szName );

    friend int PASCAL WinMain( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow );

};



#endif 