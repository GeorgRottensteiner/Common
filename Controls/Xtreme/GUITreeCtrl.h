#ifndef GUI_TREECONTROL_H
#define GUI_TREECONTROL_H


#include <string>
#include <vector>

#include <Controls\AbstractTreeCtrl.h>

#include "GUIComponent.h"
#include "GUIScrollBar.h"



class GUITreeCtrl : public AbstractTreeCtrl<GUIComponent, GUIScrollBar>
{

  protected:

    std::vector<XTextureSection>      m_vectSections;



    void                      UpdateCustomSection( GR::u32 SectionID, GUIComponent* pComponent, GR::u32 CustomSectionID );


  public:

    DECLARE_CLONEABLE( GUITreeCtrl, "TreeControl" )



    GUITreeCtrl( int NewX = 0, int NewY = 0, int NewWidth = 20, int NewHeight = 20, GR::u32 Styles = TCFT_DEFAULT, GR::u32 Id = 0 );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );

    virtual int               AddImage( const XTextureSection& Section );
    virtual void              SetItemImage( TREEITEM hItem, GR::u32 Image );
    GR::u32                   ItemImage( TREEITEM hItem );

    TREEITEM                  InsertItem( TREEITEM hItemParent, const GR::String& String, GR::u32 ItemImage = 0, GR::up ItemData = 0 );
    TREEITEM                  InsertItem( const GR::String& String, GR::u32 ItemImage = 0, GR::up ItemData = 0 );


    virtual bool              ProcessEvent( const GUI::ComponentEvent& Event );

};


#endif // GUI_TREECONTROL_H



