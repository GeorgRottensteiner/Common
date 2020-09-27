#ifndef GUI_LISTCONTROL_H
#define GUI_LISTCONTROL_H



#include <string>
#include <vector>

#include <Controls\AbstractListCtrl.h>

#include "GUIComponent.h"
#include "GUIScrollBar.h"



class GUIListControl : public AbstractListCtrl<GUIComponent, GUIScrollBar>
{

  public:

    DECLARE_CLONEABLE( GUIListControl, "ListControl" )


    GUIListControl( int NewX = 0, int NewY = 0, int NewWidth = 20, int NewHeight = 20, GR::u32 Styles = LCS_DEFAULT, GR::u32 Id = 0 );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );
    virtual void              SetTextureSection( const GUI::eBorderType Type, const XTextureSection& TexSection );
    virtual bool              ProcessEvent( const GUI::ComponentEvent& Event );

    void                      SetItemImage( size_t ItemIndex, size_t SubItemIndex, size_t ImageIndex );

    void                      SetImageListImage( size_t ImageIndex, XTextureSection Section );


  private:

    std::map<int,XTextureSection>       m_ItemImages;



    void                      UpdateCustomSection( GR::u32 SectionID, GUIComponent* pComponent, GR::u32 CustomSectionID );

};


#endif // GUI_LISTCONTROL_H



