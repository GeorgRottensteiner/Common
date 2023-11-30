#ifndef GUI_LISTCTRL_H
#define GUI_LISTCTRL_H



#include <string>
#include <vector>

#include <Controls\AbstractListCtrl.h>
#include "GUIComponent.h"
#include "GUIScrollBar.h"



class GR::Graphic::GFXPage;



class GUIListCtrl : public AbstractListCtrl<GUIComponent,GUIScrollbar>
{

  protected:

    GR::Graphic::Image*           m_pImageHdrLeft;
    GR::Graphic::Image*           m_pImageHdrCenter;
    GR::Graphic::Image*           m_pImageHdrRight;
                

  public:

    DECLARE_CLONEABLE( GUIListCtrl, "ListCtrl" )


    GUIListCtrl( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 dwType = LCS_DEFAULT, GR::u32 dwId = 0 );


    virtual void          DisplayOnPage( GR::Graphic::GFXPage* pPage );

    virtual void          SetHeaderImages( GR::Graphic::Image* pImageHdrLeft, GR::Graphic::Image* pImageHdrCenter, GR::Graphic::Image* pImageHdrRight );

};


#endif // GUI_LISTCTRL_H



