#ifndef GUI_TREECTRL_H
#define GUI_TREECTRL_H



#include <string>
#include <vector>

#include <Controls\AbstractTreeCtrl.h>
#include "GUIComponent.h"
#include "GUIScrollBar.h"



class GR::Graphic::GFXPage;



class GUITreeCtrl : public AbstractTreeCtrl<GUIComponent,GUIScrollbar>
{

  protected:

    std::vector<GR::Graphic::Image*>     m_VectImages;


  public:

    DECLARE_CLONEABLE( GUITreeCtrl, "TreeCtrl" )


    GUITreeCtrl( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 lbType = TCFT_SINGLE_SELECT | TCFT_SHOW_BUTTONS, GR::u32 dwId = 0 );


    virtual void          DisplayOnPage( GR::Graphic::GFXPage* pPage );
    virtual size_t        AddImage( GR::Graphic::Image* pImage );

    virtual TREEITEM      InsertItem( TREEITEM hItemParent, const char* szString, DWORD dwItemImage = 0, DWORD dwItemData = 0 );
    virtual TREEITEM      InsertItem( const char* szString, DWORD dwItemImage = 0, DWORD dwItemData = 0 );

    virtual void          SetItemImage( TREEITEM hItem, DWORD dwImage );
    GR::u32               GetItemImage( TREEITEM hItem ) const;

};


#endif // GUI_TREECTRL_H



