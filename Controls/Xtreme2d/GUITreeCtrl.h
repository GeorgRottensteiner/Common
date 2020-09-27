#ifndef GUI_TREECONTROL_H
#define GUI_TREECONTROL_H


#include <string>
#include <vector>

#include <Controls\AbstractTreeCtrl.h>

#include <Xtreme/X2dRenderer.h>

#include "GUIComponent.h"
#include "GUIScrollBar.h"



class X2dRenderer;

class GUITreeCtrl : public AbstractTreeCtrl<GUIComponent, GUIScrollBar>
{

  protected:

    struct IconInfo
    {
      XTextureSection       Section;
      GR::u32               ColorKey;
      X2dRenderer::eImageDisplayMethod    DisplayMethod;

      IconInfo( const XTextureSection& Section, X2dRenderer::eImageDisplayMethod DisplayMethod = X2dRenderer::IMG_PLAIN, GR::u32 ColorKey = 0xff000000 ) :
        Section( Section ),
        DisplayMethod( DisplayMethod ),
        ColorKey( ColorKey )
      {
      }
    };

    std::vector<IconInfo>       m_vectSections;


  public:

    DECLARE_CLONEABLE( GUITreeCtrl, "GUITreeCtrl" )

    GUITreeCtrl( int iNewX = 0, int iNewY = 0, int iNewWidth = 20, int iNewHeight = 20, GR::u32 dwStyles = TCFT_DEFAULT, GR::u32 dwId = 0 );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );

    virtual int               AddImage( const XTextureSection& Section, X2dRenderer::eImageDisplayMethod DisplayMethod = X2dRenderer::IMG_PLAIN, GR::u32 ColorKey = 0xff000000 );
    virtual void              SetItemImage( TREEITEM hItem, GR::u32 dwImage );

    TREEITEM                  InsertItem( TREEITEM hItemParent, const char* szString, GR::u32 dwItemImage = 0, GR::u32 dwItemData = 0 );
    TREEITEM                  InsertItem( const char* szString, GR::u32 dwItemImage = 0, GR::u32 dwItemData = 0 );

};


#endif // GUI_TREECONTROL_H



