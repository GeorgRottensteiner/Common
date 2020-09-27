#ifndef GUI_HOTTIP_H
#define GUI_HOTTIP_H



#include <Controls\AbstractHottip.h>
#include "GUIComponent.h"



namespace Interface
{
  struct IFont;
};



class CGUIHottip : public AbstractHottip<GUIComponent>
{

  public:

    DECLARE_CLONEABLE( CGUIHottip, "Hottip" )


    CGUIHottip( const char* szCaption = "", DWORD dwId = 0 );


    virtual void        DisplayOnPage( CD3DViewer* pViewer, int iOffsetX = 0, int iOffsetY = 0 );

};


#endif // GUI_HOTTIP_H



