#ifndef GUI_IMAGELIST_H
#define GUI_IMAGELIST_H



#include <Controls\AbstractListBox.h>

#include <Xtreme/XTextureSection.h>

#include "GUIComponent.h"
#include "GUIScrollBar.h"



class GUIImageList : public AbstractListBox<GUIComponent,GUIScrollBar>
{

  protected:


  public:

    DECLARE_CLONEABLE( GUIImageList, "ImageList" )


    GUIImageList( int NewX = 0, int NewY = 0, int NewWidth = 0, int NewHeight = 0, GR::u32 Id = 0, GR::u32 Type = LCS_SINGLE_SELECT | LCS_MULTI_COLUMN );
    virtual ~GUIImageList();


    virtual void          DisplayOnPage( GUIComponentDisplayer& Displayer );


    virtual void          AddString( const XTextureSection& TexSec, GR::u32 ItemData = 0 );
    virtual void          DeleteString( GR::u32 Item );
    virtual void          ResetContent();

    virtual bool          ProcessEvent( const GUI::ComponentEvent& Event );

    void                  SetItemImage( const size_t Index, const XTextureSection& TexSec );


    void                  UpdateCustomSection( GR::u32 SectionID, GUIComponent* pComponent, GR::u32 CustomSectionID );

};


#endif // GUI_IMAGELIST_H



