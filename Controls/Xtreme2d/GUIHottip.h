#ifndef GUI_HOTTIP_H
#define GUI_HOTTIP_H



#include <Controls\AbstractHottip.h>
#include "GUIComponent.h"



namespace Interface
{
  struct IFont;
};



class GUIHottip : public AbstractHottip<GUIComponent>
{

  public:

    DECLARE_CLONEABLE( GUIHottip, "Hottip" )


    GUIHottip( const GR::String& strCaption = "", GR::u32 dwId = 0 );


    virtual void        DisplayOnPage( GUIComponentDisplayer& Displayer );

};


#endif // GUI_HOTTIP_H



