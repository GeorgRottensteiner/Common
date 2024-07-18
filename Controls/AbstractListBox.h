#ifndef ABSTRACTLISTBOX_H
#define ABSTRACTLISTBOX_H



#include <string>
#include <vector>

#include <GR/GRTypes.h>

#include <Controls/AbstractScrollBar.h>
#include <Controls/ComponentDisplayerBase.h>



template <class BS_, class SB_> class AbstractListBox : public BS_
{

  public:

    enum ListBoxFlagType
    {
      LCS_SINGLE_SELECT         = 0x00000001,
      LCS_MULTI_COLUMN          = 0x00000002,
      LCS_AUTOHIDE_SCROLLBAR    = 0x00000004,
      LCS_HIDE_HOVER_ITEM       = 0x00000008,     // MouseOver-Item nicht anzeigen
      LCS_DEFAULT               = LCS_SINGLE_SELECT | LCS_AUTOHIDE_SCROLLBAR,
    };


    typedef BS_   BASECLASS;
    typedef SB_   SCROLLBARCLASS;
  


  protected:

    struct tListBoxItem
    {
      GR::String    Text;
      GR::up        ItemData;
    };



    typedef std::list<tListBoxItem>   tAbstractListBoxItemList;


    tAbstractListBoxItemList    m_Items;

    size_t                      m_VisibleRows,
                                m_FirstVisibleItem,
                                m_MouseOverItem,
                                m_SelectedItem;

    int                         m_ItemWidth,
                                m_ItemHeight,
                                m_ItemsPerLine;

    GR::u64                     m_LastClickTicks;

    SCROLLBARCLASS*             m_pScrollBar;

    bool                        m_ButtonReleased;



    virtual void                UpdateScrollBar()
    {
      if ( Style() & LCS_MULTI_COLUMN )
      {
        m_ItemsPerLine = ( m_ClientRect.Width() - m_pScrollBar->Width() ) / m_ItemWidth;
        if ( m_ItemsPerLine == 0 )
        {
          m_ItemsPerLine = 1;
        }
      }

      size_t   visibleItems = m_ClientRect.Height() / m_ItemHeight;

      if ( visibleItems > m_Items.size() )
      {
        visibleItems = m_Items.size();
      }
      m_VisibleRows = visibleItems;

      size_t   iScrollMaxItems = m_Items.size() / m_ItemsPerLine + 1;
      if ( m_Items.empty() )
      {
        iScrollMaxItems = 0;
      }

      if ( ( Style() & LCS_AUTOHIDE_SCROLLBAR )
      &&   ( m_Items.size() <= visibleItems ) )
      {
        m_pScrollBar->SetSize( 0, m_pScrollBar->Height() );
      }
      else
      {
        m_pScrollBar->SetSize( 20, m_pScrollBar->Height() );
        m_pScrollBar->SetLocation( m_ClientRect.Width() - 20, 0 );
      }
        
      if ( ( iScrollMaxItems < visibleItems )
      ||   ( m_Items.size() <= visibleItems ) )
      {
        m_pScrollBar->SetScrollRange( 0, 0 );
      }
      else
      {
        m_pScrollBar->SetScrollRange( 0, (int)( iScrollMaxItems - visibleItems ), (int)visibleItems );
        if ( m_SelectedItem != -1 )
        {
          if ( m_SelectedItem < m_FirstVisibleItem )
          {
            m_pScrollBar->SetScrollPosition( (int)( m_SelectedItem / m_ItemsPerLine - m_FirstVisibleItem ) );
          }
          if ( m_SelectedItem >= m_FirstVisibleItem + m_VisibleRows )
          {
            int   Offset = (int)( m_SelectedItem - m_VisibleRows + 1 );
            if ( Offset < 0 )
            {
              Offset = 0;
            }
            m_pScrollBar->SetScrollPosition( Offset / m_ItemsPerLine );
          }
        }
      }
    }


  public:

    // usings are required for annoying standard conform ignored lookup of base class members
    // alternative would be prefixing everything with "this->" which is even worse
    using BASECLASS::m_ClassName;
    using BASECLASS::m_Style;
    using BASECLASS::m_ComponentFlags;
    using BASECLASS::m_Position;
    using BASECLASS::m_TextAlignment;
    using BASECLASS::m_Width;
    using BASECLASS::m_Height;
    using BASECLASS::m_ClientRect;
    using BASECLASS::m_Caption;
    using BASECLASS::m_pFont;
    using BASECLASS::m_pParentContainer;
    using BASECLASS::RecalcClientRect;
    using BASECLASS::Style;
    using BASECLASS::IsEnabled;
    using BASECLASS::IsVisible;
    using BASECLASS::SetVisible;
    using BASECLASS::SetCapture;
    using BASECLASS::SetSize;
    using BASECLASS::ReleaseCapture;
    using BASECLASS::IsMouseInside;
    using BASECLASS::ModifyStyle;
    using BASECLASS::GenerateEvent;
    using BASECLASS::GenerateEventForParent;
    using BASECLASS::GetTopLevelParent;
    using BASECLASS::GetComponentParent;
    using BASECLASS::LocalToScreen;
    using BASECLASS::Add;



    AbstractListBox( GR::u32 Type = LCS_DEFAULT, GR::u32 Id = 0 ) :
      BASECLASS()
    {
      m_ClassName         = "ListBox";

      m_Style             = Type;

      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;

      m_ButtonReleased    = true;

      m_LastClickTicks    = 0;

      m_ItemHeight        = 12;
      m_ItemWidth         = 40;
      m_SelectedItem      = -1;
      m_MouseOverItem     = -1;
      m_FirstVisibleItem  = 0;
      m_ItemsPerLine      = 1;
      m_TextAlignment     = GUI::AF_LEFT | GUI::AF_VCENTER;

      m_pScrollBar = new SCROLLBARCLASS( m_Width - 20, 0, 20, m_Height, SCROLLBARCLASS::SBFT_INVALID, Id );
      m_pScrollBar->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );

      Add( m_pScrollBar );

      m_pScrollBar->AddEventListener( this );

      UpdateScrollBar();
      BASECLASS::SetBaseColors();
    }



    AbstractListBox( int X, int Y, int Width, int Height, GR::u32 Type = LCS_DEFAULT, GR::u32 Id = 0 ) :
      BASECLASS( X, Y, Width, Height, Id )
    {
      m_ClassName         = "ListBox";

      m_Style             = Type;

      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;

      m_ButtonReleased    = true;

      m_LastClickTicks    = 0;

      m_ItemHeight        = 12;
      m_SelectedItem      = -1;
      m_MouseOverItem     = -1;
      m_FirstVisibleItem  = 0;
      m_TextAlignment     = GUI::AF_LEFT | GUI::AF_VCENTER;

      RecalcClientRect();
      m_pScrollBar = new SCROLLBARCLASS( m_Width - 20, 0, 20, m_Height, SCROLLBARCLASS::SBFT_DEFAULT, Id );
      m_pScrollBar->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );

      Add( m_pScrollBar );

      m_pScrollBar->SetScrollRange( 0, 0, 0 );

      m_pScrollBar->AddListener( this );

      m_ItemWidth        = m_ClientRect.Width() - m_pScrollBar->Width();
      m_ItemsPerLine     = 1;
      if ( Type & LCS_MULTI_COLUMN )
      {
        m_ItemsPerLine = ( m_ClientRect.Width() - m_pScrollBar->Width() ) / m_ItemWidth;
        if ( m_ItemsPerLine == 0 )
        {
          m_ItemsPerLine = 1;
        }
      }

      UpdateScrollBar();
      BASECLASS::SetBaseColors();
    }



    void Scroll( int iDirection )
    {
      m_pScrollBar->SetScrollPosition( m_FirstVisibleItem + iDirection * m_ItemsPerLine );
    }



    virtual int FindItemByText( const GR::String& Text ) const
    {
      int   iIndex = 0;
      typename tAbstractListBoxItemList::const_iterator    it( m_Items.begin() );
      while ( it != m_Items.end() )
      {
        const tListBoxItem&   Item = *it;

        if ( Item.Text == Text )
        {
          return iIndex;
        }

        iIndex++;
        ++it;
      }
      return -1;
    }



    virtual size_t FindItemByItemData( const GR::up ItemData ) const
    {
      int   iIndex = 0;
      typename tAbstractListBoxItemList::const_iterator    it( m_Items.begin() );
      while ( it != m_Items.end() )
      {
        const tListBoxItem&   Item = *it;

        if ( Item.ItemData == ItemData )
        {
          return iIndex;
        }

        iIndex++;
        ++it;
      }
      return -1;
    }



    size_t SelectItemByItemData( GR::up ItemData )
    {
      size_t    iItem = 0;

      typename tAbstractListBoxItemList::iterator    it( m_Items.begin() );
      while ( it != m_Items.end() )
      {
        tListBoxItem&   Item = *it; 

        if ( Item.ItemData == ItemData )
        {
          SelectItem( iItem );
          return iItem;
        }

        ++iItem;
        ++it;
      }
      return -1;
    }



    virtual void SelectItem( size_t ItemIndex )
    {
      if ( ItemIndex == -1 )
      {
        if ( m_SelectedItem != -1 )
        {
          m_SelectedItem = (size_t)-1;
          GenerateEventForParent( OET_LISTBOX_ITEM_SELECTED, m_SelectedItem );
          BASECLASS::Invalidate();
        }
        return;
      }

      if ( ItemIndex >= m_Items.size() )
      {
        return;
      }
      if ( m_SelectedItem == ItemIndex )
      {
        return;
      }
      m_SelectedItem = ItemIndex;
      if ( m_SelectedItem < m_FirstVisibleItem )
      {
        m_pScrollBar->SetScrollPosition( (int)( m_SelectedItem / m_ItemsPerLine ) );
      }
      if ( m_SelectedItem >= m_FirstVisibleItem + m_VisibleRows * m_ItemsPerLine )
      {
        int   Offset = (int)( m_SelectedItem - m_VisibleRows * m_ItemsPerLine );
        if ( Offset < 0 )
        {
          Offset = 0;
        }
        m_pScrollBar->SetScrollPosition( Offset / m_ItemsPerLine );
      }
      typename tAbstractListBoxItemList::iterator    it( m_Items.begin() );
      std::advance( it, m_SelectedItem );

      tListBoxItem&   Item = *it;

      GenerateEventForParent( OET_LISTBOX_ITEM_SELECTED, m_SelectedItem, Item.ItemData );
      BASECLASS::Invalidate();
    }



    virtual bool ProcessEvent( const GUI::ComponentEvent& Event )
    {
      switch ( Event.Type )
      {
        case CET_SET_SIZE:
        case CET_SET_CLIENT_SIZE:
          BASECLASS::ProcessEvent( Event );

          m_ItemWidth        = m_ClientRect.Width() - m_pScrollBar->Width();
          m_ItemsPerLine     = 1;
          if ( Style() & LCS_MULTI_COLUMN )
          {
            m_ItemsPerLine = ( m_ClientRect.Width() - m_pScrollBar->Width() ) / m_ItemWidth;
            if ( m_ItemsPerLine == 0 )
            {
              m_ItemsPerLine = 1;
            }
          }

          m_pScrollBar->SetLocation( m_ClientRect.Width() - m_pScrollBar->Width(), 0 );
          m_pScrollBar->SetSize( m_pScrollBar->Width(), m_ClientRect.Height() );

          UpdateScrollBar();
          BASECLASS::Invalidate();
          return true;
        case CET_MOUSE_WHEEL:
          if ( ( IsMouseInside() )
          &&   ( IsEnabled() ) )
          {
            GUI::ComponentEvent    newEvent( Event );

            LocalToScreen( newEvent.Position );
            GUI::ComponentContainer::ScreenToLocal( newEvent.Position, m_pScrollBar );
            m_pScrollBar->ProcessEvent( Event );
          }
          return true;
        case CET_MOUSE_OUT:
          if ( m_MouseOverItem != -1 )
          {
            m_MouseOverItem = (size_t)-1;
            GenerateEventForParent( OET_ITEM_HOVER, -1 );
            BASECLASS::Invalidate();
          }
          return true;
        case CET_KEY_DOWN:
          if ( ( !m_Items.empty() )
          &&   ( IsEnabled() ) )
          {
            if ( ( Event.Character == Xtreme::KEY_DOWN )
            ||   ( Event.Character == Xtreme::KEY_NUMPAD_2 ) )
            {
              if ( m_SelectedItem == -1 )
              {
                SelectItem( 0 );
                return true;
              }
              if ( m_SelectedItem < m_Items.size() - 1 )
              {
                SelectItem( (int)( m_SelectedItem + m_ItemsPerLine ) );
                return true;
              }
            }
            else if ( ( Event.Character == Xtreme::KEY_UP )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_8 ) )
            {
              if ( m_SelectedItem == -1 )
              {
                SelectItem( 0 );
                return true;
              }
              else if ( m_SelectedItem > 0 )
              {
                SelectItem( (int)( m_SelectedItem - m_ItemsPerLine ) );
                return true;
              }
            }
            else if ( ( Event.Character == Xtreme::KEY_PAGEUP )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_9 ) )
            {
              int Item = (int)( m_SelectedItem - m_VisibleRows * m_ItemsPerLine );
              if ( Item < 0 )
              {
                Item = 0;
              }
              SelectItem( Item );
              return true;
            }
            else if ( ( Event.Character == Xtreme::KEY_PAGEDOWN )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_3 ) )
            {
              size_t Item = m_SelectedItem + m_VisibleRows * m_ItemsPerLine;
              if ( Item >= m_Items.size() )
              {
                Item = m_Items.size() - 1;
              }
              SelectItem( Item );
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
              if ( m_SelectedItem != m_Items.size() - 1 )
              {
                SelectItem( m_Items.size() - 1 );
                return true;
              }
            }
          }
          break;
        case CET_MOUSE_UPDATE:
          if ( IsEnabled() )
          {
            if ( !( Event.MouseButtons & 1 ) )
            {
              m_ButtonReleased = true;
            }
            GR::tRect   rectList;
            GetListRect( rectList );
            if ( rectList.Contains( Event.Position ) )
            {
              size_t   iItem = ( Event.Position.y / m_ItemHeight ) * m_ItemsPerLine + m_FirstVisibleItem;

              if ( Event.Position.x >= m_ItemWidth * m_ItemsPerLine )
              {
                iItem = (size_t)-1;
                m_LastClickTicks = 0;
              }
              else
              {
                iItem += ( Event.Position.x / m_ItemWidth );

                if ( iItem >= m_Items.size() )
                {
                  iItem = (size_t)-1;
                  m_LastClickTicks = 0;
                }
              }

              if ( m_MouseOverItem != iItem )
              {
                m_MouseOverItem = iItem;
                GenerateEventForParent( OET_ITEM_HOVER, m_MouseOverItem, GetItemData( m_MouseOverItem ) );
                BASECLASS::Invalidate();
              }
            }
          }
          break;
        case CET_MOUSE_DOWN:
          if ( IsEnabled() )
          {
            GR::tRect   rectList;
            GetListRect( rectList );
            if ( rectList.Contains( Event.Position ) )
            {
              size_t   itemIndex = ( Event.Position.y / m_ItemHeight ) * m_ItemsPerLine + m_FirstVisibleItem;

              if ( Event.Position.x >= m_ItemWidth * m_ItemsPerLine )
              {
                itemIndex = (size_t)-1;
                m_LastClickTicks = 0;
              }
              else
              {
                itemIndex += ( Event.Position.x / m_ItemWidth );

                if ( itemIndex >= m_Items.size() )
                {
                  itemIndex = (size_t)-1;
                  m_LastClickTicks = 0;
                }
              }

              if ( Style() & LCS_SINGLE_SELECT )
              {
                if ( ( Event.MouseButtons & 1 )
                &&   ( m_ButtonReleased ) )
                {
                  m_ButtonReleased = false;
                  
                  bool    sendDblClk = false;
                  if ( m_SelectedItem == itemIndex )
                  {
                    GR::u64     deltaTicks = GUI::GetTicks() - m_LastClickTicks;
                    if ( deltaTicks <= GUI::GetDoubleClickTimeMS() )
                    {
                      sendDblClk = true;
                    }
                  }
                  SelectItem( itemIndex );
                  m_LastClickTicks = GUI::GetTicks();
                  if ( sendDblClk )
                  {
                    GenerateEventForParent( OET_LISTBOX_ITEM_DBLCLK, m_SelectedItem, 0 );
                  }
                }
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
          if ( m_FirstVisibleItem != Event.Param1 * m_ItemsPerLine )
          {
            m_FirstVisibleItem = Event.Param1 * m_ItemsPerLine;
            BASECLASS::Invalidate();
          }
        }
      }
      return false;
    }



    virtual size_t GetSelectedItem() const
    {
      return m_SelectedItem;
    }



    virtual size_t SelectedItemData() const
    {
      return GetItemData( m_SelectedItem );
    }



    virtual size_t GetItemCount() const
    {
      return m_Items.size();
    }



    virtual size_t AddString( const GR::String& String, GR::up ItemData = 0 )
    {
      m_Items.push_back( tListBoxItem() );

      tListBoxItem&   newItem = m_Items.back();

      newItem.Text      = String;
      newItem.ItemData  = ItemData;

      UpdateScrollBar();
      BASECLASS::Invalidate();
      return m_Items.size() - 1;
    }



    void DeleteString( const size_t ItemIndex )
    {
      if ( ItemIndex >= m_Items.size() )
      {
        return;
      }

      typename tAbstractListBoxItemList::iterator    it( m_Items.begin() );
      std::advance( it, ItemIndex );

      m_Items.erase( it );
      BASECLASS::Invalidate();

      UpdateScrollBar();
    }



    virtual void ResetContent()
    {
      m_Items.clear();
      m_SelectedItem = (size_t)-1;
      if ( m_MouseOverItem != (size_t)-1 )
      {
        GenerateEventForParent( OET_ITEM_HOVER, m_MouseOverItem, GetItemData( m_MouseOverItem ) );
      }
      m_MouseOverItem = (size_t)-1;
      BASECLASS::Invalidate();

      UpdateScrollBar();
    }



    virtual GR::String GetItemText( size_t ItemIndex )
    {
      if ( ItemIndex >= m_Items.size() )
      {
        return GR::String();
      }
      typename tAbstractListBoxItemList::iterator  it( m_Items.begin() );
      while ( it != m_Items.end() )
      {
        if ( ItemIndex == 0 )
        {
          tListBoxItem&   listItem = *it;

          return listItem.Text;
        }
        --ItemIndex;
        ++it;
      }
      return GR::String();
    }



    virtual void SetItemText( size_t ItemIndex, const GR::String& Text )
    {
      if ( ItemIndex >= m_Items.size() )
      {
        return;
      }
      typename tAbstractListBoxItemList::iterator  it( m_Items.begin() );
      while ( it != m_Items.end() )
      {
        if ( ItemIndex == 0 )
        {
          tListBoxItem&   listItem = *it;

          listItem.Text = Text;
          BASECLASS::Invalidate();
          return;
        }
        --ItemIndex;
        ++it;
      }
    }



    virtual void SetItemData( size_t ItemIndex, GR::up Data )
    {
      if ( ItemIndex >= m_Items.size() )
      {
        return;
      }
      typename tAbstractListBoxItemList::iterator  it( m_Items.begin() );
      while ( it != m_Items.end() )
      {
        if ( ItemIndex == 0 )
        {
          tListBoxItem&   listItem = *it;

          listItem.ItemData = Data;
          return;
        }
        --ItemIndex;
        ++it;
      }
    }



    virtual GR::up GetItemData( size_t ItemIndex ) const
    {
      if ( ItemIndex >= m_Items.size() )
      {
        return 0;
      }
      typename tAbstractListBoxItemList::const_iterator  it( m_Items.begin() );
      while ( it != m_Items.end() )
      {
        if ( ItemIndex == 0 )
        {
          const tListBoxItem&   listItem = *it;

          return listItem.ItemData;
        }
        --ItemIndex;
        ++it;
      }
      return 0;
    }



    virtual bool GetItemRect( size_t iItem, GR::tRect& rectItem )
    {
      if ( iItem >= m_Items.size() )
      {
        return false;
      }
      rectItem.Position( (int)( ( iItem - m_FirstVisibleItem ) % m_ItemsPerLine ) * m_ItemWidth, (int)( iItem - m_FirstVisibleItem ) / m_ItemsPerLine * m_ItemHeight );
      if ( m_ItemsPerLine == 1 )
      {
        rectItem.Size( m_ClientRect.Width() - m_pScrollBar->Width(), m_ItemHeight );
      }
      else
      {
        rectItem.Size( m_ItemWidth, m_ItemHeight );
      }

      if ( rectItem.Top + rectItem.Height() <= 0 )
      {
        rectItem.Size( 0, 0 );
        rectItem.Position( 0, 0 );
        return false;
      }
      if ( rectItem.Top >= m_ClientRect.Height() )
      {
        rectItem.Size( 0, 0 );
        rectItem.Position( 0, 0 );
        return false;
      }
      if ( rectItem.Top < 0 )
      {
        rectItem.Size( rectItem.Width(), rectItem.Height() + rectItem.Top );
        rectItem.Position( rectItem.Left, 0 );
      }
      return true;
    }



    int ItemHeight() const
    {
      return m_ItemHeight;
    }



    virtual void GetListRect( GR::tRect& rectItem )
    {
      rectItem.Position( 0, 0 );
      rectItem.Size( m_ClientRect.Width() - m_pScrollBar->Width(), m_ClientRect.Height() );
    }



    virtual GR::u32 ModifyStyle( GR::u32 dwAdd, GR::u32 dwRemove = 0 )
    {
      GR::u32 uResult = BASECLASS::ModifyStyle( dwAdd, dwRemove );

      UpdateScrollBar();
      BASECLASS::Invalidate();
      return uResult;
    }



    virtual void SetItemSize( int Height, int Width = 0 )
    {
      m_ItemHeight = Height;
      if ( m_ItemHeight <= 0 )
      {
        m_ItemHeight = 1;
      }
      m_ItemsPerLine     = 1;
      if ( Style() & LCS_MULTI_COLUMN )
      {
        m_ItemWidth = Width;
        if ( m_ItemWidth < 4 )
        {
          m_ItemWidth = 4;
        }
      }
      if ( Style() & LCS_MULTI_COLUMN )
      {
        m_ItemsPerLine = ( m_ClientRect.Width() - m_pScrollBar->Width() ) / m_ItemWidth;
        if ( m_ItemsPerLine == 0 )
        {
          m_ItemsPerLine = 1;
        }
      }
      else
      {
        m_ItemWidth = m_ClientRect.Width() - m_pScrollBar->Width();
      }
      UpdateScrollBar();
      BASECLASS::Invalidate();
    }



    SCROLLBARCLASS* GetScrollbar() const
    {
      return m_pScrollBar;
    }



    virtual void ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment )
    {
      BS_::ParseXML( pElement, Environment );

      if ( pElement == NULL )
      {
        return;
      }
      if ( pElement->HasAttribute( "ItemWidth" ) )
      {
        m_ItemWidth = GR::Convert::ToI32( pElement->Attribute( "ItemWidth" ) );
      }
      if ( pElement->HasAttribute( "ItemHeight" ) )
      {
        m_ItemHeight = GR::Convert::ToI32( pElement->Attribute( "ItemHeight" ) );
      }
      UpdateScrollBar();
    }


};


#endif // ABSTRACTLISTBOX_H



