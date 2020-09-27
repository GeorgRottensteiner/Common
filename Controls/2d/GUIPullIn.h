#ifndef GUI_PULLIN_H
#define GUI_PULLIN_H



#include <string>
#include <vector>

#include <Controls\AbstractPullIn.h>
#include "GUIComponent.h"



class GR::Graphic::GFXPage;



class CGUIPullIn : public CAbstractPullIn<CGUIComponent>
{

  public:

    DECLARE_CLONEABLE( CGUIPullIn, "GUI.PullIn" )


    CGUIPullIn( int iNewX = 0, int iNewY = 0, 
                int iNewWidth = 0, int iNewHeight = 0, 
                const char* szCaption = "", 
                GR::u32 dwId = 0, 
                GR::u32 dfType = PCS_DEFAULT );


    virtual void        DisplayNonClientOnPage( GR::Graphic::GFXPage* pPage );
    virtual void        DisplayOnPage( GR::Graphic::GFXPage* pPage );

};


#endif // GUI_PULLIN_H



