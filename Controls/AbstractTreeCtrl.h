#ifndef ABSTRACTTREECTRL_H
#define ABSTRACTTREECTRL_H



#include <string>
#include <vector>

#include <Controls\AbstractScrollBar.h>

#include <Collections\tree.h>



struct tTreeItem
{
  GR::String       Text;
  GR::up            ItemData,
                    CtrlItemData;

  tTreeItem() :
    Text( "" ),
    ItemData( 0 ),
    CtrlItemData( 0 )
  {
  }
};

typedef GR::tree<tTreeItem>::iterator   TreeIterator;

typedef TreeIterator                TREEITEM;



template <class BS_, class SB_> class AbstractTreeCtrl : public BS_
{

  public:

    typedef BS_   BASECLASS;
    typedef SB_   SCROLLBARCLASS;

    enum TreeCtrlFlagType
    {
      TCFT_SINGLE_SELECT      = 0x00000001,
      TCFT_SHOW_BUTTONS       = 0x00000002,
      TCFT_HAS_ICONS          = 0x00000004,
      TCFT_AUTOHIDE_SCROLLBAR = 0x00000008,

      TCFT_DEFAULT            = TCFT_SINGLE_SELECT | TCFT_SHOW_BUTTONS | TCFT_AUTOHIDE_SCROLLBAR,
    };



  protected:

    typedef GR::tree<tTreeItem> tTree;

    tTree                       m_TreeItems;

    TreeIterator                m_itFirstVisibleItem,
                                m_itSelectedItem,
                                m_itMouseOverItem;

    int                         m_Offset,
                                m_VisibleItemsInTree,
                                m_LevelIndent,      // Einrückung pro Level
                                m_ExtraIndent,      // extra-Einrückung für Icons etc.
                                m_ItemHeight,
                                m_VisibleItems,
                                m_SelectedItem;

    GR::u64                     m_LastClickTicks;

    SCROLLBARCLASS*             m_pScrollBar;

    bool                        m_ButtonReleased;



    virtual void                UpdateScrollBar()
    {
      int   visibleItems = m_ClientRect.size().y / m_ItemHeight;

      int   openItems = 0;
      tTree::iterator it( m_TreeItems.begin() );
      while ( it != m_TreeItems.end() )
      {
        openItems++;

        it = GetNextVisibleItem( it );
      }

      m_VisibleItemsInTree = openItems;
      if ( visibleItems > openItems )
      {
        visibleItems = openItems;
      }
      m_VisibleItems = visibleItems;

      if ( ( Style() & TCFT_AUTOHIDE_SCROLLBAR )
      &&   ( openItems <= m_VisibleItems ) )
      {
        m_pScrollBar->SetSize( 0, m_pScrollBar->Height() );
        m_pScrollBar->SetLocation( m_ClientRect.width(), 0 );
      }
      else
      {
        m_pScrollBar->SetSize( 20, m_pScrollBar->Height() );
        m_pScrollBar->SetLocation( m_ClientRect.width() - 20, 0 );
      }

      if ( openItems < visibleItems )
      {
        m_pScrollBar->SetScrollRange( 0, 0 );
      }
      else
      {
        m_pScrollBar->SetScrollRange( 0, openItems - visibleItems );
        if ( m_SelectedItem < m_Offset )
        {
          m_pScrollBar->SetScrollPosition( m_SelectedItem - m_Offset );
        }
        if ( m_SelectedItem >= m_Offset + m_VisibleItems )
        {
          int   newOffset = m_SelectedItem - m_VisibleItems + 1;
          if ( newOffset < 0 )
          {
            newOffset = 0;
          }
          m_pScrollBar->SetScrollPosition( newOffset );
        }
      }
      if ( m_VisibleItems > 1 )
      {
        m_pScrollBar->SetPageSteps( m_VisibleItems - 1 );
      }
    }


  public:



    // usings are required for annoying standard conform ignored lookup of base class members
    // alternative would be prefixing everything with "this->" which is even worse
    using BASECLASS::m_ClassName;
    using BASECLASS::m_Style;
    using BASECLASS::m_ComponentFlags;
    using BASECLASS::m_Position;
    using BASECLASS::m_Width;
    using BASECLASS::m_Height;
    using BASECLASS::Width;
    using BASECLASS::Height;
    using BASECLASS::SetSize;
    using BASECLASS::m_ClientRect;
    using BASECLASS::RecalcClientRect;
    using BASECLASS::Style;
    using BASECLASS::IsEnabled;
    using BASECLASS::IsVisible;
    using BASECLASS::LocalToScreen;
    using BASECLASS::SetCapture;
    using BASECLASS::ReleaseCapture;
    using BASECLASS::IsMouseInside;
    using BASECLASS::GenerateEvent;
    using BASECLASS::GenerateEventForParent;
    using BASECLASS::Add;



    AbstractTreeCtrl( GR::u32 lbfType = TCFT_SINGLE_SELECT, GR::u32 dwId = 0 ) :
        BASECLASS()
    {
      m_ClassName         = "TreeCtrl";

      m_Style             = lbfType;

      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;

      m_ButtonReleased    = true;

      m_LastClickTicks    = 0;

      m_ExtraIndent       = 0;

      m_ItemHeight        = 12;
      m_LevelIndent       = m_ItemHeight;
      m_SelectedItem      = -1;
      m_itMouseOverItem   = TREEITEM();
      m_Offset            = 0;
      m_VisibleItemsInTree= 0;

      m_itFirstVisibleItem  = m_TreeItems.end();
      m_itSelectedItem      = m_TreeItems.end();
      m_itMouseOverItem     = m_TreeItems.end();

      m_pScrollBar = new SCROLLBARCLASS( m_Width - 20, 0, 20, m_Height, SCROLLBARCLASS::SBFT_INVALID, dwId );
      m_pScrollBar->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );

      Add( m_pScrollBar );

      m_pScrollBar->AddEventListener( this );

      UpdateScrollBar();
    }
    
        
        
    AbstractTreeCtrl( int X, int Y, int Width, int Height, GR::u32 Type = TCFT_SINGLE_SELECT, GR::u32 Id = 0 ) :
      BASECLASS( X, Y, Width, Height, Id )
    {
      m_ClassName         = "TreeCtrl";

      m_Style             = Type;

      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;

      m_ButtonReleased    = true;

      m_ExtraIndent       = 0;

      m_LastClickTicks    = 0;

      m_ItemHeight        = 12;
      m_LevelIndent       = m_ItemHeight;
      m_SelectedItem      = -1;
      m_Offset            = 0;
      m_VisibleItemsInTree= 0;

      m_itFirstVisibleItem  = m_TreeItems.end();
      m_itSelectedItem      = m_TreeItems.end();
      m_itMouseOverItem     = m_TreeItems.end();

      RecalcClientRect();

      m_pScrollBar = new SCROLLBARCLASS( m_Width - 20, 0, 20, m_Height, SCROLLBARCLASS::SBFT_INVALID, Id );
      m_pScrollBar->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );

      Add( m_pScrollBar );

      m_pScrollBar->SetScrollRange( 0, 0, 0 );

      m_pScrollBar->AddListener( this );

      UpdateScrollBar();
    }



    SCROLLBARCLASS* GetScrollbar()
    {
      return m_pScrollBar;
    }



    // returns level in hierarchy, 0 = root node
    GR::u32 ItemLevel( const TREEITEM hItem ) const
    {
      TREEITEM    hItem2( hItem );

      GR::u32     level = 0;
      while ( hItem2 != TREEITEM() )
      {
        hItem2 = GetParentItem( hItem2 );
        ++level;
      }
      return level;
    }



    void SetItemHeight( size_t ItemHeight )
    {
      m_ItemHeight = (int)ItemHeight;
      if ( m_ItemHeight < 4 )
      {
        m_ItemHeight = 4;
      }
      SetSize( Width(), Height() );
    }



    TREEITEM FindItemByItemData( const GR::up& ItemData )
    {
      TreeIterator     it( m_TreeItems.begin() );
      while ( it != m_TreeItems.end() )
      {
        tTreeItem&   Item = *it;

        if ( Item.ItemData == ItemData )
        {
          return it;
        }

        ++it;
      }
      return m_TreeItems.end();
    }



    virtual TREEITEM FindItemByText( const GR::String& Text )
    {
      TreeIterator     it( m_TreeItems.begin() );
      while ( it != m_TreeItems.end() )
      {
        tTreeItem&   Item = *it;

        if ( Item.Text == Text )
        {
          return it;
        }
        ++it;
      }
      return m_TreeItems.end();
    }



    virtual void SelectItem( const TREEITEM& Item )
    {
      if ( Item == m_TreeItems.end() )
      {
        return;
      }

      if ( m_itSelectedItem == Item )
      {
        return;
      }
      m_itSelectedItem = Item;

      int   itemIndex = 0;
      TreeIterator    it( Item );
      while ( it != m_TreeItems.begin() )
      {
        ++itemIndex;

        it = GetPreviousVisibleItem( it );
      }

      m_SelectedItem = itemIndex;
      if ( m_SelectedItem < m_Offset )
      {
        m_pScrollBar->SetScrollPosition( m_SelectedItem );
      }
      if ( m_SelectedItem >= m_Offset + m_VisibleItems )
      {
        int   newOffset = m_SelectedItem - m_VisibleItems + 1;
        if ( newOffset < 0 )
        {
          newOffset = 0;
        }
        m_pScrollBar->SetScrollPosition( newOffset );
      }

      const tTreeItem&   selItem = *m_itSelectedItem;

      GenerateEventForParent( OET_TREE_ITEM_SELECTED, m_SelectedItem, selItem.ItemData );
    }



    virtual void SelectItem( int ItemIndex )
    {
      if ( ItemIndex == -1 )
      {
        m_SelectedItem = -1;
        m_itSelectedItem = m_TreeItems.end();
        GenerateEventForParent( OET_TREE_ITEM_SELECTED, m_SelectedItem );
        return;
      }

      TreeIterator    it( m_TreeItems.begin() );

      while ( ItemIndex )
      {
        it = GetNextVisibleItem( it );
        --ItemIndex;
      }

      SelectItem( it );
    }



    virtual void ExpandItem( TREEITEM hItem )
    {
      if ( hItem == TreeIterator() )
      {
        return;
      }
      if ( m_TreeItems.child( hItem ) != m_TreeItems.end() )
      {
        tTreeItem&    Item = *hItem;

        GR::u32 scrollBarPos = m_pScrollBar->GetScrollPosition();

        if ( hItem.is_collapsed() )
        {
          hItem.toggle();
        }
        GenerateEventForParent( OET_TREE_ITEM_EXPAND, (GR::up)&hItem, Item.ItemData );
        UpdateScrollBar();

        m_pScrollBar->SetScrollPosition( scrollBarPos );
      }
    }



    virtual void EnsureItemIsVisible( TREEITEM hItem )
    {
      if ( hItem == TreeIterator() )
      {
        return;
      }
      while ( ( hItem = GetParentItem( hItem ) ) != TreeIterator() )
      {
        ExpandItem( hItem );
      }
    }



    virtual void CollapseItem( TREEITEM hItem )
    {
      if ( hItem == TreeIterator() )
      {
        return;
      }
      if ( m_TreeItems.child( hItem ) != m_TreeItems.end() )
      {
        tTreeItem&    Item = *hItem;

        GR::u32 scrollBarPos = m_pScrollBar->GetScrollPosition();

        if ( !hItem.is_collapsed() )
        {
          hItem.toggle();
        }
        GenerateEventForParent( OET_TREE_ITEM_COLLAPSE, (GR::up)&hItem, Item.ItemData );
        UpdateScrollBar();

        m_pScrollBar->SetScrollPosition( scrollBarPos );
      }
    }



    virtual void ToggleItem( TREEITEM hItem )
    {
      if ( hItem == TreeIterator() )
      {
        return;
      }
      // nur wenn das Item Childs hat
      if ( m_TreeItems.child( hItem ) != m_TreeItems.end() )
      {
        tTreeItem&    Item = *hItem;

        GR::u32 scrollBarPos = m_pScrollBar->GetScrollPosition();

        hItem.toggle();
        if ( hItem.is_collapsed() )
        {
          GenerateEventForParent( OET_TREE_ITEM_COLLAPSE, (GR::up)&hItem, Item.ItemData );
        }
        else
        {
          GenerateEventForParent( OET_TREE_ITEM_EXPAND, (GR::up)&hItem, Item.ItemData );
        }
        UpdateScrollBar();

        m_pScrollBar->SetScrollPosition( scrollBarPos );
      }
    }



    virtual bool ProcessEvent( const GUI::ComponentEvent& Event )
    {
      if ( ( !IsVisible() )
      &&   ( !IsEnabled() ) )
      {
        return true;
      }

      switch ( Event.Type )
      {
        case CET_SET_ITEM_HEIGHT:
          BASECLASS::ProcessEvent( Event );
          SetItemHeight( Event.Value );
          break;
        case CET_SET_SIZE:
          BASECLASS::ProcessEvent( Event );
          m_pScrollBar->SetLocation( m_ClientRect.size().x - m_pScrollBar->Width(), 0 );
          m_pScrollBar->SetSize( m_pScrollBar->Width(), m_ClientRect.size().y );
          UpdateScrollBar();
          return true;
        case CET_MOUSE_WHEEL:
          if ( IsMouseInside() )
          {
            GUI::ComponentEvent    newEvent( Event );

            LocalToScreen( newEvent.Position );
            GUI::ComponentContainer::ScreenToLocal( newEvent.Position, m_pScrollBar );
            m_pScrollBar->ProcessEvent( newEvent );
          }
          return true;
        case CET_MOUSE_OUT:
          m_itMouseOverItem = m_TreeItems.end();
          return true;
        case CET_KEY_DOWN:
          if ( !m_TreeItems.empty() )
          {
            if ( ( Event.Character == Xtreme::KEY_ENTER )
            ||   ( Event.Character == Xtreme::KEY_NUMPAD_ENTER ) )
            {
              if ( m_itSelectedItem != m_TreeItems.end() )
              {
                GenerateEventForParent( OET_TREE_ITEM_ACTIVATE, m_SelectedItem );
                return true;
              }
            }
            else if ( ( Event.Character == Xtreme::KEY_RIGHT )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_6 ) )
            {
              if ( m_itSelectedItem != m_TreeItems.end() )
              {
                if ( m_itSelectedItem.is_collapsed() )
                {
                  ToggleItem( m_itSelectedItem );
                  return true;
                }
                else if ( m_itSelectedItem.child() != m_TreeItems.end() )
                {
                  SelectItem( m_itSelectedItem.child() );
                  return true;
                }
              }
            }
            else if ( ( Event.Character == Xtreme::KEY_LEFT )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_4 ) )
            {
              if ( m_itSelectedItem != m_TreeItems.end() )
              {
                if ( ( ItemHasChildren( m_itSelectedItem ) )
                &&   ( !IsItemCollapsed( m_itSelectedItem ) ) )
                {
                  ToggleItem( m_itSelectedItem );
                  return true;
                }
                else
                {
                  TreeIterator  itParent = GetParentItem( m_itSelectedItem );
                  if ( itParent != m_TreeItems.end() )
                  {
                    SelectItem( itParent );
                    return true;
                  }
                }
              }
            }
            else if ( ( Event.Character == Xtreme::KEY_DOWN )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_2 ) )
            {
              if ( m_SelectedItem < m_VisibleItemsInTree )
              {
                SelectItem( m_SelectedItem + 1 );
                return true;
              }
            }
            else if ( ( Event.Character == Xtreme::KEY_UP )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_8 ) )
            {
              if ( m_SelectedItem > 0 )
              {
                SelectItem( m_SelectedItem - 1 );
                return true;
              }
            }
            else if ( ( Event.Character == Xtreme::KEY_PAGEUP )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_9 ) )
            {
              int iNewItem = m_SelectedItem - m_VisibleItems;
              if ( iNewItem < 0 )
              {
                iNewItem = 0;
              }
              SelectItem( iNewItem );
              return true;
            }
            else if ( ( Event.Character == Xtreme::KEY_PAGEDOWN )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_3 ) )
            {
              int newItem = m_SelectedItem + m_VisibleItems;
              if ( newItem >= m_VisibleItemsInTree )
              {
                newItem = m_VisibleItemsInTree - 1;
              }
              SelectItem( newItem );
              return true;
            }
            else if ( ( Event.Character == Xtreme::KEY_HOME )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_7 ) )
            {
              if ( m_SelectedItem > 0 )
              {
                SelectItem( 0 );
                return true;
              }
            }
            else if ( ( Event.Character == Xtreme::KEY_END )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_1 ) )
            {
              if ( m_SelectedItem != m_VisibleItemsInTree - 1 )
              {
                SelectItem( m_VisibleItemsInTree - 1 );
                return true;
              }
            }
          }
          break;
        case CET_MOUSE_UPDATE:
          {
            if ( !( Event.MouseButtons & 1 ) )
            {
              m_ButtonReleased = true;
            }
            
            GR::tRect   rectList;
            GetListRect( rectList );
            if ( rectList.contains( Event.Position ) )
            {
              tTree::iterator   it( m_itFirstVisibleItem );

              GR::tRect   rectItem;

              while ( it != m_TreeItems.end() )
              {
                if ( GetItemRect( it, rectItem ) )
                {
                  if ( rectItem.contains( Event.Position ) )
                  {
                    m_itMouseOverItem = it;

                    if ( Style() & TCFT_SINGLE_SELECT )
                    {
                      if ( ( Event.MouseButtons & 1 )
                      &&   ( m_ButtonReleased ) )
                      {
                        m_ButtonReleased = false;
                  
                        bool    bSendDblClk = false;
                        if ( m_itSelectedItem == m_itMouseOverItem )
                        {
                          if ( GUI::GetTicks() <= GUI::GetDoubleClickTimeMS() )
                          {
                            bSendDblClk = true;
                          }
                        }
                        SelectItem( m_itMouseOverItem );
                        m_LastClickTicks = GUI::GetTicks();
                        if ( bSendDblClk )
                        {
                          ToggleItem( m_itSelectedItem );
                          GenerateEventForParent( OET_TREE_ITEM_DBLCLK, m_SelectedItem );
                        }
                        return true;
                      }
                    }
                    break;
                  }
                  else if ( rectItem.position().y >= Event.Position.y )
                  {
                    break;
                  }
                  else if ( ( Event.MouseButtons & 1 )
                  &&        ( m_ButtonReleased ) )
                  {
                    if ( GetToggleButtonRect( it, rectItem ) )
                    {
                      if ( rectItem.contains( Event.Position ) )
                      {
                        ToggleItem( it );
                        m_ButtonReleased = false;
                      }
                    }
                  }
                }
                else
                {
                  break;
                }

                it = GetNextVisibleItem( it );
              }

              if ( it == m_TreeItems.end() )
              {
                m_itMouseOverItem = m_TreeItems.end();
                m_LastClickTicks = 0;
              }
            }
          }
          break;
      }

      return BASECLASS::ProcessEvent( Event );
    }



    virtual bool ProcessEvent( const GUI::OutputEvent& Event )
    {
      if ( Event.Type == OET_SCROLLBAR_SCROLLED )
      {
        if ( Event.pComponent == m_pScrollBar )
        {
          // Offset des Scrollbars anpassen (zusammengeklappte Items!)
          m_Offset = (int)Event.Param1;

          TreeIterator    It = m_TreeItems.begin();

          int   iDummy = m_Offset;
          while ( iDummy )
          {
            It = GetNextVisibleItem( It );
            iDummy--;
          }

          m_itFirstVisibleItem = It;
        }
      }
      return false;
    }



    virtual TREEITEM GetLastSiblingItem( const TREEITEM treeItem ) const
    {
      if ( treeItem == TREEITEM() )
      {
        return treeItem;
      }
      int   iCurLevel = treeItem.level();

      TREEITEM    nextItem = GetNextItem( treeItem );
      if ( nextItem == TREEITEM() )
      {
        return treeItem;
      }
      if ( nextItem.level() != iCurLevel )
      {
        return treeItem;
      }

      TREEITEM    tempItem = nextItem;
      while ( ( tempItem != TREEITEM() )
      &&      ( tempItem.level() == iCurLevel ) )
      {
        nextItem = tempItem;
        tempItem = GetNextItem( tempItem );
      }

      return nextItem;
    }



    virtual TREEITEM GetSelectedItem() const
    {
      return m_itSelectedItem;
    }



    virtual int GetItemCount() const
    {
      return (int)m_TreeItems.size();
    }



    virtual int GetChildCount( TREEITEM Item ) const
    {
      if ( Item == TREEITEM() )
      {
        return 0;
      }
      return m_TreeItems.child_count( Item );
    }



    virtual TREEITEM InsertItem( const char* pString, GR::up ItemData = 0 )
    {
      TreeIterator  itNew = m_TreeItems.insert( tTreeItem() );

      tTreeItem&    newItem = *itNew;

      newItem.Text      = pString;
      newItem.ItemData  = ItemData;

      if ( m_itFirstVisibleItem == m_TreeItems.end() )
      {
        m_itFirstVisibleItem = m_TreeItems.begin();
      }

      UpdateScrollBar();

      return itNew;
    }



    virtual TREEITEM InsertItem( TREEITEM ItemParent, const char* pString, GR::up ItemData = 0 )
    {
      if ( ItemParent == TreeIterator() )
      {
        return InsertItem( pString, ItemData );
      }

      TreeIterator it = m_TreeItems.insert_child( ItemParent, tTreeItem() );

      tTreeItem&   newItem = *it;

      newItem.Text      = pString;
      newItem.ItemData  = ItemData;

      if ( m_itFirstVisibleItem == m_TreeItems.end() )
      {
        m_itFirstVisibleItem = m_TreeItems.begin();
      }

      UpdateScrollBar();
      return it;
    }



    virtual void ResetContent()
    {
      m_TreeItems.clear();

      m_itFirstVisibleItem  = m_TreeItems.end();
      m_itSelectedItem      = m_TreeItems.end();
      m_itMouseOverItem     = m_TreeItems.end();

      UpdateScrollBar();
    }



    void DeleteItem( TREEITEM hItem )
    {
      if ( m_itFirstVisibleItem == hItem )
      {
        m_itFirstVisibleItem = m_TreeItems.end();
      }
      if ( m_itSelectedItem == hItem )
      {
        m_itSelectedItem = m_TreeItems.end();
      }
      if ( m_itMouseOverItem == hItem )
      {
        m_itMouseOverItem = m_TreeItems.end();
      }
      m_TreeItems.erase( hItem );
    }



    virtual GR::String GetItemText( TREEITEM hItem )
    {
      if ( hItem == m_TreeItems.end() )
      {
        return GR::String();
      }
      tTreeItem&    Item = *hItem;

      return Item.Text;
    }



    virtual TREEITEM GetItem( size_t ItemIndex )
    {
      if ( ItemIndex >= m_TreeItems.size() )
      {
        return TREEITEM();
      }
      tTree::iterator  it( m_TreeItems.begin() );
      while ( it != m_TreeItems.end() )
      {
        if ( ItemIndex == 0 )
        {
          return it;
        }
        --ItemIndex;

        it = GetNextVisibleItem( it );
      }
      return TREEITEM();
    }



    virtual void SetItemText( size_t ItemIndex, const GR::String& Text )
    {
      if ( ItemIndex >= m_TreeItems.size() )
      {
        return;
      }
      tTree::iterator  it( m_TreeItems.begin() );
      while ( it != m_TreeItems.end() )
      {
        if ( ItemIndex == 0 )
        {
          tTreeItem&   listItem = *it;

          listItem.Text = Text;
          return;
        }
        --ItemIndex;
        ++it;
      }
    }



    void SetItemText( TREEITEM& hItem, const GR::String& Text )
    {
      if ( hItem == m_TreeItems.end() )
      {
        return;
      }
      hItem->Text = Text;
    }



    virtual void SetItemData( size_t ItemIndex, GR::up Data )
    {
      if ( ItemIndex >= m_TreeItems.size() )
      {
        return;
      }
      tTree::iterator  it( m_TreeItems.begin() );
      while ( it != m_TreeItems.end() )
      {
        if ( ItemIndex == 0 )
        {
          tTreeItem&   listItem = *it;

          listItem.ItemData = Data;
          return;
        }
        --ItemIndex;
        ++it;
      }
    }



    virtual void SetItemData( TREEITEM hItem, GR::up Data )
    {
      if ( hItem == m_TreeItems.end() )
      {
        return;
      }
      tTreeItem&    listItem = *hItem;

      listItem.ItemData = Data;
    }



    virtual GR::up GetItemData( TREEITEM hItem )
    {
      if ( hItem == m_TreeItems.end() )
      {
        return 0;
      }
      tTreeItem&   listItem = *hItem;

      return listItem.ItemData;
    }



    virtual TREEITEM GetNextItem( const TREEITEM& hItem ) const
    {
      if ( hItem == m_TreeItems.end() )
      {
        return m_TreeItems.end();
      }

      tTree::iterator   newIt( hItem );

      ++newIt;
      if ( newIt == m_TreeItems.end() )
      {
        return tTree::iterator();
      }
      return newIt;
    }



    virtual TREEITEM GetNextSiblingItem( const TREEITEM& hItem ) const
    {
      if ( hItem == m_TreeItems.end() )
      {
        return m_TreeItems.end();
      }

      tTree::iterator   newIt( hItem );

      return newIt.next_sibling();
    }



    virtual TREEITEM GetPreviousSiblingItem( const TREEITEM& hItem ) const
    {
      if ( hItem == m_TreeItems.end() )
      {
        return m_TreeItems.end();
      }

      tTree::iterator   newIt( hItem );

      return newIt.prev_sibling();
    }



    virtual TREEITEM GetNextVisibleItem( const TREEITEM& hItem ) const
    {
      if ( hItem == m_TreeItems.end() )
      {
        return m_TreeItems.end();
      }

      tTree::iterator   newIt( hItem );

      newIt.inc_visible();
      return newIt;
    }



    virtual bool IsItemVisible( const TREEITEM& hItem ) const
    {
      if ( hItem == m_TreeItems.end() )
      {
        return true;
      }

      return hItem.is_visible();
    }



    virtual TREEITEM GetPreviousVisibleItem( const TREEITEM& hItem )
    {
      if ( hItem == m_TreeItems.end() )
      {
        return m_TreeItems.end();
      }

      if ( hItem == m_TreeItems.begin() )
      {
        return m_TreeItems.end();
      }

      tTree::iterator   newIt( hItem );

      do
      {
        if ( newIt == m_TreeItems.begin() )
        {
          return m_TreeItems.end();
        }
        newIt--;
        if ( newIt == m_TreeItems.end() )
        {
          return newIt;
        }
      }
      while ( !IsItemVisible( newIt ) );

      return newIt;
    }



    virtual bool GetItemRect( const TREEITEM& hItem, GR::tRect& rectItem )
    {
      if ( hItem == m_TreeItems.end() )
      {
        return false;
      }
      tTree::iterator   itDummy = hItem;

      int               itemIndex = 0;

      while ( itDummy != m_itFirstVisibleItem )
      {
        ++itemIndex;
        itDummy = GetPreviousVisibleItem( itDummy );
        if ( itDummy == m_TreeItems.end() )
        {
          return false;
        }
      }

      int   iX = hItem.level() * m_LevelIndent + m_ExtraIndent;

      if ( Style() & TCFT_SHOW_BUTTONS )
      {
        // +/- Buttons zum Auf und zuklappen
        iX += m_ItemHeight;
      }

      if ( Style() & TCFT_HAS_ICONS )
      {
        iX += m_ItemHeight;
      }


      if ( itemIndex - m_Offset > m_VisibleItems )
      {
        return false;
      }

      rectItem.position( iX, itemIndex * m_ItemHeight );
      rectItem.size( m_ClientRect.size().x - m_pScrollBar->Width() - rectItem.position().x, m_ItemHeight );

      if ( rectItem.position().y + rectItem.height() < 0 )
      {
        rectItem.size( 0, 0 );
        rectItem.position( 0, 0 );
        return false;
      }
      if ( rectItem.position().y >= m_ClientRect.height() )
      {
        rectItem.size( 0, 0 );
        rectItem.position( 0, 0 );
        return false;
      }
      if ( rectItem.position().y < 0 )
      {
        rectItem.size( rectItem.size().x, rectItem.size().y + rectItem.position().y );
        rectItem.position( rectItem.position().x, 0 );
      }
      /*
      if ( rectItem.position().y + rectItem.size().y >= m_ClientRect.height() )
      {
        rectItem.size( rectItem.width(), m_ClientRect.height() - rectItem.position().y );
      }
      */

      return true;
    }



    virtual bool GetToggleButtonRect( const TREEITEM& hItem, GR::tRect& TargetRect )
    {
      if ( hItem == m_TreeItems.end() )
      {
        return false;
      }
      if ( !( Style() & TCFT_SHOW_BUTTONS ) )
      {
        return false;
      }

      tTree::iterator   itDummy = hItem;

      int               itemIndex = 0;

      while ( itDummy != m_itFirstVisibleItem )
      {
        ++itemIndex;
        itDummy = GetPreviousVisibleItem( itDummy );
        if ( itDummy == m_TreeItems.end() )
        {
          return false;
        }
      }

      int   iX = hItem.level() * m_LevelIndent;

      TargetRect.position( iX, itemIndex * m_ItemHeight );
      TargetRect.size( m_ItemHeight, m_ItemHeight );
      return true;
    }



    virtual bool GetExtraIndentRect( const TREEITEM& hItem, GR::tRect& TargetRect )
    {
      if ( hItem == m_TreeItems.end() )
      {
        return false;
      }
      int   indent = m_ExtraIndent;

      if ( Style() & TCFT_HAS_ICONS )
      {
        indent += m_ItemHeight;
      }
      if ( indent == 0 )
      {
        return false;
      }

      tTree::iterator   itDummy = hItem;

      int               itemIndex = 0;

      while ( itDummy != m_itFirstVisibleItem )
      {
        ++itemIndex;
        itDummy = GetPreviousVisibleItem( itDummy );
        if ( itDummy == m_TreeItems.end() )
        {
          return false;
        }
      }

      int   iX = hItem.level() * m_LevelIndent + m_ItemHeight;

      TargetRect.position( iX, itemIndex * m_ItemHeight );
      TargetRect.size( indent, m_ItemHeight );
      return true;
    }



    virtual void GetListRect( GR::tRect& TargetRect )
    {
      TargetRect.position( 0, 0 );
      TargetRect.size( m_ClientRect.width() - m_pScrollBar->Width(), m_ClientRect.height() );
    }



    virtual TREEITEM GetParentItem( const TREEITEM& hItem ) const
    {
      if ( hItem == m_TreeItems.end() )
      {
        return m_TreeItems.end();
      }
      return m_TreeItems.parent( hItem );
    }



    virtual TREEITEM GetFirstChildItem( const TREEITEM& hItem ) const
    {
      if ( hItem == m_TreeItems.end() )
      {
        return m_TreeItems.end();
      }
      return m_TreeItems.child( hItem );
    }



    virtual bool ItemHasChildren( const TREEITEM& hItem ) const
    {
      if ( hItem == m_TreeItems.end() )
      {
        return false;
      }
      return ( m_TreeItems.child( hItem ) != m_TreeItems.end() );
    }



    virtual bool IsItemCollapsed( const TREEITEM& hItem ) const
    {
      if ( hItem == m_TreeItems.end() )
      {
        return false;
      }
      return hItem.is_collapsed();
    }



    virtual size_t ChildCount( const TREEITEM& hItem ) const
    {
      return m_TreeItems.child_count( hItem );
    }



    virtual GR::u32 ModifyStyle( GR::u32 Add, GR::u32 Remove = 0 )
    {
      GR::u32 result = BASECLASS::ModifyStyle( Add, Remove );
      UpdateScrollBar();
      return result;
    }

};


#endif // ABSTRACTTREECTRL_H