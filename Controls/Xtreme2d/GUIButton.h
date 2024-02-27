#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H



#include <string>
#include <vector>

#include <Controls\AbstractButton.h>
#include "GUIComponent.h"


class GUIButton : public AbstractButton<GUIComponent>
{

  protected:

    GR::tPoint                m_PushedTextOffset;


  public:

    DECLARE_CLONEABLE( GUIButton, "Button" )

    enum eCustomTSId
    {
      CTS_IMAGE = GUI::CTS_USER_FIRST,

      CTS_NORMAL,
      CTS_MOUSE_OVER,
      CTS_PUSHED
    };

    GUIButton( int X = 0, int Y = 0, int Width = 20, int Height = 20, GR::u32 Id = 0, GR::u32 Style = BCS_DEFAULT );
    GUIButton( int X, int Y, int Width, int Height, const GR::String& Caption, GR::u32 Id = 0, GR::u32 Style = BCS_DEFAULT );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );
    virtual void              DisplayNonClientOnPage( GUIComponentDisplayer& Displayer );

    void                      SetPushedTextOffset( const GR::tPoint& Offset );

};


#endif // GUI_BUTTON_H



