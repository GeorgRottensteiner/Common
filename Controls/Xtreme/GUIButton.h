#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H



#include <string>
#include <vector>

#include <Controls\AbstractButton.h>
#include "GUIComponent.h"



class GUIButton : public AbstractButton<GUIComponent>
{

  public:

    enum eCustomTSId
    {
      CTS_IMAGE = GUI::CTS_USER_FIRST,
    };


    DECLARE_CLONEABLE( GUIButton, "Button" )


    GR::tPoint                m_PushedTextOffset;


    GUIButton( int NewX = 0, int NewY = 0, int NewWidth = 20, int NewHeight = 20, GR::u32 Id = 0, GR::u32 Style = BCS_DEFAULT );
    GUIButton( int NewX, int NewY, int NewWidth, int NewHeight, const GR::String& Caption, GR::u32 Id = 0, GR::u32 Style = BCS_DEFAULT );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );

    void                      SetPushedTextOffset( const GR::tPoint& Offset );

};


#endif // GUI_BUTTON_H



