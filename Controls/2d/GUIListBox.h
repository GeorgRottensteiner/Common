#ifndef GUI_LISTBOX_H
#define GUI_LISTBOX_H



#include <string>
#include <vector>

#include <Controls\AbstractListBox.h>
#include "GUIComponent.h"
#include "GUIScrollBar.h"



class GR::Graphic::GFXPage;



class GUIListBox : public AbstractListBox<GUIComponent,GUIScrollbar>
{

  public:

    DECLARE_CLONEABLE( GUIListBox, "ListBox" );


    GUIListBox( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 lbType = LCS_SINGLE_SELECT, GR::u32 dwId = 0 );


    virtual void        DisplayOnPage( GR::Graphic::GFXPage* pPage );

};


#endif // GUI_LISTBOX_H



