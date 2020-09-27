#ifndef GUI_LISTBOX_H
#define GUI_LISTBOX_H



#include <string>
#include <vector>

#include <Controls\AbstractListBox.h>
#include "GUIComponent.h"
#include "GUIScrollBar.h"



class GUIListBox : public AbstractListBox<GUIComponent,GUIScrollBar>
{


  public:

    DECLARE_CLONEABLE( GUIListBox, "ListBox" )


    GUIListBox( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 dwId = 0, GR::u32 lbfType = LCS_DEFAULT );


    virtual void          DisplayOnPage( GUIComponentDisplayer& Displayer );

};


#endif // GUI_LISTBOX_H



