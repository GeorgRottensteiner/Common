#ifndef GUI_HOTTIP_H
#define GUI_HOTTIP_H



#include <Controls\AbstractHottip.h>
#include "GUIComponent.h"



class GR::Graphic::GFXPage;

namespace Interface
{
  struct IFont;
};



class GUIHottip : public AbstractHottip<GUIComponent>
{

  protected:


  public:


    GUIHottip( int iNewX = 0, int iNewY = 0, 
                int iNewWidth = 0, int iNewHeight = 0, 
                const char* szCaption = "", DWORD dwId = 0 );


    virtual void        DisplayOnPage( GR::Graphic::GFXPage* pPage );

    virtual             ICloneAble* Clone();

};


#endif // GUI_HOTTIP_H



