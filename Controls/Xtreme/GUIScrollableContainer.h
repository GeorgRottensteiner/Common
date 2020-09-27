#ifndef GUI_SCROLLABLE_CONTAINER_H
#define GUI_SCROLLABLE_CONTAINER_H



#include <string>
#include <vector>

#include <Xtreme/XHugeTexture.h>

#include "GUIComponent.h"
#include "GUIScrollBar.h"
#include <Controls/AbstractScrollableContainer.h>



class GUIScrollableContainer : public AbstractScrollableContainer<GUIComponent,GUIScrollBar>
{

  protected:



  public:

    DECLARE_CLONEABLE( GUIScrollableContainer, "ScrollableContainer" )



    GUIScrollableContainer( int NewX = 0, int NewY = 0, int NewWidth = 0, int NewHeight = 0, GR::u32 Id = 0 );


    virtual void                ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment );


    virtual void                DisplayNonClientOnPage( GUIComponentDisplayer& Displayer );
};


#endif // GUI_SCROLLABLE_CONTAINER_H



