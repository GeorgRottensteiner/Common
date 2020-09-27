#ifndef MASTERFRAME3D_H_INCLUDED
#define MASTERFRAME3D_H_INCLUDED



#include <string>
#include <Input/Input.h>
#include <Sound/Sound.h>

#include <D3D8.h>
#include <D3DX8.h>
#include <DX8\DX8Viewer.h>

#include <MasterFrame/MasterFrame.h>

#include <Controls/OutputEvent.h>
#include <Controls/Component.h>

#pragma comment( lib, "d3dx8.lib" )
#pragma comment( lib, "d3d8.lib" )
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "dinput8.lib" )



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CMasterFrame3d : public CMasterFrame, public CD3DViewer
{
  private:

    unsigned char                     m_ucCurrentDepth,
                                      m_ucOrigDepth;

    int                               m_imagesT;


    void ResetFont();

  protected:


  public:

    virtual BOOL        Create( int iNewWidth, int iNewHeight, unsigned char ucDepth, DWORD dwFlags );

    virtual bool        InitInstance() = 0;
    virtual void        ExitInstance() = 0;

    virtual void        DisplayFrame() = 0;
    virtual void        UpdatePerDisplayFrame(  const float fElapsedTime ) = 0;
    virtual void        UpdateFrame() = 0;

    BOOL                Run();

    void Reset();
    BOOL CleanUp();

    BOOL                CreatePage();
    void                DestroyPage();

    void                DisplayConsole();
    virtual void        ProcessCommandTokens( GR::TokenSequence& m_TokenSequence, GR::String& strCommand );

    virtual BOOL        ToggleFullScreen();
    virtual void        RestoreAppDeviceObjects();

    virtual LRESULT FAR PASCAL WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );

    CMasterFrame3d();
    virtual ~CMasterFrame3d();


};



/*-Variablen------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Prototypen-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#endif // __MASTERFRAME3D_H_INCLUDED__