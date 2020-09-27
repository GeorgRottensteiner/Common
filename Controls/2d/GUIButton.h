#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H



#include <Grafik/GfxPage.h>
#include <Grafik/Font.h>

#include <string>
#include <vector>

#include <Controls/Component.h>
#include <Controls/AbstractButton.h>

#include "GUIComponent.h"



class GUIButton: public AbstractButton<GUIComponent>
{

  protected:

    GR::Graphic::Image* m_pImage;
    GR::Graphic::Image* m_pImageMouseOver;
    GR::Graphic::Image* m_pImagePushed;
    GR::Graphic::Image* m_pImageDisabled;

    GR::tPoint          m_ptTextDisplayOffset;
    GR::tPoint          m_ptPushedTextOffset;


  public:


    DECLARE_CLONEABLE( GUIButton, "Button" )


    GUIButton( GR::u32 ID = 0 );
    GUIButton( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 ID = 0 );
    GUIButton( int NewX, int NewY, int NewWidth, int NewHeight, const char* pCaption, GR::u32 ID = 0 );
    GUIButton( int NewX, int NewY, GR::Graphic::Image* pImage = 0, GR::Graphic::Image* pImageMO = 0, GR::Graphic::Image* pImageP = 0, GR::u32 Id = 0 );

    virtual void                  DisplayOnPage( GR::Graphic::GFXPage* pPage );
    virtual void                  DisplayNonClientOnPage( GR::Graphic::GFXPage* pPage );

    void                          SetImages( GR::Graphic::Image* pImage, GR::Graphic::Image* pImageMO, GR::Graphic::Image* pImageP, GR::Graphic::Image* pImageDisabled = NULL );

    virtual bool                  IsMouseInside( const GR::tPoint& ptMousePos );
    virtual bool                  IsMouseInsideNonClientArea( const GR::tPoint& ptMousePos );

    void                          SetPushedTextOffset( const GR::tPoint& ptOffset );
    void                          SetDisplayTextOffset( const GR::tPoint& ptOffset );

};


#endif // GUI_BUTTON_H



