#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H



#include <string>
#include <vector>

#include <Controls\AbstractButton.h>
#include "GUIComponent.h"


class X2dRenderer;

class GUIButton : public AbstractButton<GUIComponent>
{

  protected:

    GR::tPoint                m_ptPushedTextOffset;


  public:

    DECLARE_CLONEABLE( GUIButton, "Button" )

    enum eCustomTSId
    {
      CTS_IMAGE = GUI::CTS_USER_FIRST,

      CTS_NORMAL,
      CTS_MOUSE_OVER,
      CTS_PUSHED
    };

    GUIButton( int iNewX = 0, int iNewY = 0, int iNewWidth = 20, int iNewHeight = 20, GR::u32 dwId = 0, GR::u32 dwStyle = BCS_DEFAULT );
    GUIButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const GR::String& strCaption, GR::u32 dwId = 0, GR::u32 dwStyle = BCS_DEFAULT );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );
    virtual void              DisplayNonClientOnPage( GUIComponentDisplayer& Displayer );

    void                      SetPushedTextOffset( const GR::tPoint& ptOffset );

};


#endif // GUI_BUTTON_H



