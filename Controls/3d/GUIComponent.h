#ifndef GUI_COMPONENT_H
#define GUI_COMPONENT_H



#include <string>
#include <vector>

#include <Grafik/Font.h>

#include <DX8\DX8Viewer.h>
#include <DX8\TextureSection.h>

#include <Controls\AbstractComponentDisplayer.h>
#include <Controls/Component.h>



class GUIComponent : public GUI::Component
{

  protected:

    tTextureSection     m_TextureSection[GUI::BT_EDGE_LAST_ENTRY];


  public:

    GUIComponent( GR::u32 dwId = 0 );
    GUIComponent( int iX, int iY, int iWidth, int iHeight, GR::u32 dwId = 0 );
    GUIComponent( int iX, int iY, int iWidth, int iHeight, const char* szCaption, GR::u32 dwId = 0 );

    virtual void                Display( GUI::AbstractComponentDisplayer<GUIComponent>* pDisplayer );
    virtual void                DisplayOnPage( CD3DViewer* pViewer, int iOffsetX = 0, int iOffsetY = 0 ) = 0;

    virtual void                DisplayNonClient( GUI::AbstractComponentDisplayer<GUIComponent>* pDisplayer );
    virtual void                DisplayNonClientOnPage( CD3DViewer* pViewer, int iOffsetX = 0, int iOffsetY = 0 );

    virtual void                DrawEdge( CD3DViewer* pViewer, GR::u32 edgeType, GR::tRect& rectEdge );

    virtual void                DisplayText( CD3DViewer* pViewer, 
                                             int iXOffset, int iYOffset,
                                             const char* szText,
                                             GR::u32 textAlignment = GUI::AF_DEFAULT,
                                             GR::u32 dwColor = 0xffffffff,
                                             GR::tRect* pRect = NULL );

    virtual void                DrawFocusRect( CD3DViewer* pViewer,
                                               int iXOffset, int iYOffset,
                                               GR::tRect& rc );

    // Helper
    virtual void                DrawMultiTexture( CD3DViewer* pViewer, CDX8MultiTexture* pTexture, int iOffsetX = 0, int iOffsetY = 0 );
    virtual void                DrawFrame( CD3DViewer* pViewer, CDX8Texture* pTexture, GR::u32 dwFrameColor,
                                           int iOffsetX = 0, int iOffsetY = 0,
                                           int iX = 0, int iY = 0, int iWidth = 0, int iHeight = 0 );
    virtual void                DrawSunkenFrame( CD3DViewer* pViewer, CDX8Texture* pTexture, GR::u32 dwFrameColor,
                                                 int iOffsetX = 0, int iOffsetY = 0,
                                                 int iX = 0, int iY = 0, int iWidth = 0, int iHeight = 0 );

    virtual void                PrepareStatesForAlpha( CD3DViewer* pViewer, GR::u32 dwColor );

    virtual void                SetFont( Interface::IFont* pFont );

    virtual void                SetTextureSection( const GUI::eBorderType eType, const tTextureSection& TexSection );

    virtual void                RecalcClientRect();

    GR::u32                     GetSysColor( GUI::eColorIndex eColor );

    virtual int                 GetBorderWidth( const GUI::eBorderType eType ) const;
    virtual int                 GetBorderHeight( const GUI::eBorderType eType ) const;

    
};


#endif // GUI_BUTTON_H



