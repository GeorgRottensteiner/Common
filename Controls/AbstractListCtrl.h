#ifndef ABSTRACTLISTCONTROL_H
#define ABSTRACTLISTCONTROL_H



#include <string>
#include <vector>

#include <Controls\AbstractScrollBar.h>
#include <Controls\Component.h>

#include <Collections/MultiColumnTable.h>

#include <GR/Database/DatabaseTable.h>

#include <String/StringUtil.h>



template <class BS_, class SB_> class AbstractListCtrl : public BS_
{

  public:

    enum ListCtrlFlagType
    {
      LCS_SINGLE_SELECT      = 0x00000001,
      LCS_SHOW_HEADER        = 0x00000002,
      LCS_AUTOHIDE_SCROLLBAR = 0x00000004,
      LCS_HEADER_SORT        = 0x00000008,
      LCS_SORTING            = 0x00000010,

      LCS_DEFAULT            = LCS_SINGLE_SELECT | LCS_SHOW_HEADER | LCS_AUTOHIDE_SCROLLBAR,
    };


    struct tListCtrlItem
    {
      GR::String       Text; 
      GR::up            ItemData;
      GR::up            InternalItemData;

      tListCtrlItem() : 
        ItemData( 0 ),
        InternalItemData( 0 )
      {
      }
    };


    typedef GR::Collections::MultiColumnTable<tListCtrlItem>  tTableType;

    int                       m_HeaderHeight;


    struct tListCtrlColumn
    {
      GR::String          Description;
      int                     Width,
                              CurrentWidth,
                              CurrentOffset;
      GR::u32                 TextAlignment;
      int                     DBTextID;



      tListCtrlColumn( const GR::String& Desc = GR::String(), int Width = -1, GR::u32 textAlignment = GUI::AF_DEFAULT ) :
        Description( Desc ),
        Width( Width ),
        CurrentWidth( 0 ),
        CurrentOffset( 0 ),
        TextAlignment( textAlignment ),
        DBTextID( -1 )
      {
      }
    };

    typedef std::vector<tListCtrlColumn>              tVectListCtrlColumn;

    typedef fastdelegate::FastDelegate3<size_t,const typename tTableType::MultiColumnRow&, const typename tTableType::MultiColumnRow&,bool>      tCompareFunction;


    typedef BS_   BASECLASS;
    typedef SB_   SCROLLBARCLASS;
  


  protected:

    tTableType                  m_Items;

    std::vector<size_t>         m_ItemIndex;

    tVectListCtrlColumn         m_vectColumns;

    tCompareFunction            m_CompareFunction;

    size_t                      m_VisibleItems,
                                m_SelectedItem,
                                m_MouseOverItem,
                                m_MouseOverSubItem,
                                m_Offset;

    size_t                      m_HeaderPushedIndex;
    size_t                      m_SortColumnIndex;
    bool                        m_SortAscending;

    int                         m_ItemHeight,
                                m_HorzOffset;

    GR::u64                     m_LastClickTicks;

    SCROLLBARCLASS*             m_pScrollBar;
    SCROLLBARCLASS*             m_pScrollBarH;

    bool                        m_ButtonReleased;



    virtual void                UpdateScrollBar()
    {
      int       iUsableHeight = m_ClientRect.height() - m_pScrollBarH->Height();

      if ( Style() & LCS_SHOW_HEADER )
      {
        iUsableHeight -= m_HeaderHeight;
      }
      if ( iUsableHeight < 0 )
      {
        iUsableHeight = 0;
      }

      size_t   iVisibleItems = iUsableHeight / m_ItemHeight;

      if ( iVisibleItems > m_Items.NumRows() )
      {
        iVisibleItems = m_Items.NumRows();
      }
      m_VisibleItems = iVisibleItems;

      if ( ( Style() & LCS_AUTOHIDE_SCROLLBAR )
      &&   ( m_Items.NumRows() <= iVisibleItems ) )
      {
        m_pScrollBar->SetSize( 0, m_ClientRect.height() - m_pScrollBarH->Height() );
      }
      else
      {
        m_pScrollBar->SetSize( 20, m_ClientRect.height() - m_pScrollBarH->Height() );
        m_pScrollBar->SetLocation( m_ClientRect.width() - 20, 0 );
      }

      AdjustColumnOffsets();

      if ( m_Items.NumRows() <= m_VisibleItems )
      {
        m_pScrollBar->SetScrollRange( 0, 0 );
      }
      else
      {
        m_pScrollBar->SetScrollRange( 0, (int)( m_Items.NumRows() - m_VisibleItems ) );
        if ( m_SelectedItem < m_Offset )
        {
          m_pScrollBar->SetScrollPosition( (int)( m_SelectedItem - m_Offset ) );
        }
        if ( m_SelectedItem >= m_Offset + m_VisibleItems )
        {
          int   iNewOffset = (int)( m_SelectedItem - m_VisibleItems + 1 );
          if ( iNewOffset < 0 )
          {
            iNewOffset = 0;
          }
          m_pScrollBar->SetScrollPosition( iNewOffset );
        }
      }
      if ( m_VisibleItems > 1 )
      {
        m_pScrollBar->SetPageSteps( (int)m_VisibleItems - 1 );
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
    using BASECLASS::RecalcClientRect;
    using BASECLASS::Style;
    using BASECLASS::IsEnabled;
    using BASECLASS::IsVisible;
    using BASECLASS::SetCapture;
    using BASECLASS::ReleaseCapture;
    using BASECLASS::IsMouseInside;
    using BASECLASS::GenerateEvent;
    using BASECLASS::GenerateEventForParent;
    using BASECLASS::m_ClientRect;
    using BASECLASS::LocalToScreen;
    using BASECLASS::m_pFont;
    using BASECLASS::Add;
    using BASECLASS::m_DefaultEventScript;
    using BASECLASS::RaiseDefaultEvent;



    AbstractListCtrl( GR::u32 dwStyles = LCS_DEFAULT, GR::u32 dwId = 0 ) :
        BASECLASS(),
        m_HeaderPushedIndex( (size_t)-1 ),
        m_SortColumnIndex( (size_t)-1 ),
        m_SortAscending( true )
    {
      m_ClassName         = "ListCtrl";

      m_Style             = dwStyles;

      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;

      m_ButtonReleased   = true;

      m_LastClickTicks  = 0;

      m_ItemHeight       = 12;
      m_SelectedItem     = -1;
      m_MouseOverItem    = -1;
      m_MouseOverSubItem = -1;
      m_HeaderHeight     = m_ItemHeight;
      m_Offset           = 0;
      m_HorzOffset       = 0;

      m_pScrollBar = new SCROLLBARCLASS( m_Width - 20, 0, 20, m_Height, SCROLLBARCLASS::SBFT_VERTICAL, dwId );
      m_pScrollBar->ModifyFlags( 0, GUI::COMPFT_TAB_STOP );

      m_pScrollBarH = new SCROLLBARCLASS( 0, m_Height - 20, m_Width - 20, 20, SCROLLBARCLASS::SBFT_HORIZONTAL, dwId );
      m_pScrollBarH->ModifyFlags( 0, GUI::COMPFT_TAB_STOP );

      m_pScrollBar->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );
      m_pScrollBarH->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );

      Add( m_pScrollBar );
      Add( m_pScrollBarH );

      m_pScrollBar->AddEventListener( this );
      m_pScrollBarH->AddEventListener( this );

      UpdateScrollBar();
    }



    AbstractListCtrl( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwStyles = LCS_DEFAULT, GR::u32 dwId = 0 ) :
      BASECLASS( iNewX, iNewY, iNewWidth, iNewHeight, dwId ),
      m_HeaderPushedIndex( (size_t)-1 ),
      m_SortColumnIndex( (size_t)-1 ),
      m_SortAscending( true )
    {
      m_ClassName         = "ListCtrl";

      m_Style             = dwStyles;

      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;

      m_ButtonReleased   = true;

      m_LastClickTicks  = 0;

      m_ItemHeight       = 12;
      m_SelectedItem     = (size_t)-1;
      m_MouseOverItem    = -1;
      m_MouseOverSubItem = -1;
      m_HeaderHeight     = m_ItemHeight;
      m_Offset           = 0;
      m_HorzOffset       = 0;

      RecalcClientRect();

      m_pScrollBar = new SCROLLBARCLASS( m_Width - 20, 0, 20, m_Height, SCROLLBARCLASS::SBFT_VERTICAL, dwId );
      m_pScrollBar->ModifyFlags( 0, GUI::COMPFT_TAB_STOP );
      Add( m_pScrollBar );

      m_pScrollBarH = new SCROLLBARCLASS( 0, m_ClientRect.height() - 20, m_Width - 20, 20, SCROLLBARCLASS::SBFT_HORIZONTAL, dwId );
      m_pScrollBarH->ModifyFlags( 0, GUI::COMPFT_TAB_STOP );
      Add( m_pScrollBarH );

      m_pScrollBar->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );
      m_pScrollBarH->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );

      m_pScrollBar->SetScrollRange( 0, 0, 0 );
      m_pScrollBarH->SetScrollRange( 0, 0, 0 );

      m_pScrollBar->AddListener( this );
      m_pScrollBarH->AddListener( this );

      UpdateScrollBar();
    }



    virtual size_t InsertColumn( const GR::String& ColumnName, int iWidth, GR::u32 textAlignment = GUI::AF_LEFT )
    {
      textAlignment |= GUI::AF_VCENTER;
      m_vectColumns.push_back( tListCtrlColumn( ColumnName, iWidth, textAlignment ) );


      m_Items.AddColumn( ColumnName );
      AdjustColumnOffsets();

      return m_vectColumns.size() - 1;
    }



    virtual void AdjustColumnOffsets()
    {
      int   adaptableColumn = -1;

      int   curColumn = 0;

      int   completeWidth = 0;

      // gibt es eine sich anpassende Spalte?
      typename tVectListCtrlColumn::iterator    it( m_vectColumns.begin() );
      while ( it != m_vectColumns.end() )
      {
        tListCtrlColumn&    Column = *it;
        if ( Column.Width == -1 )
        {
          adaptableColumn = curColumn;
          break;
        }
        ++curColumn;
        ++it;
      }

      int   iWidthLeft = m_ClientRect.size().x - m_pScrollBar->Width();

      it = m_vectColumns.begin();
      while ( it != m_vectColumns.end() )
      {
        tListCtrlColumn&    Column = *it;
        if ( Column.Width != -1 )
        {
          Column.CurrentWidth = Column.Width;
          iWidthLeft -= Column.CurrentWidth;

          completeWidth += Column.CurrentWidth;
        }
        ++it;
      }
      if ( adaptableColumn != -1 )
      {
        m_vectColumns[adaptableColumn].CurrentWidth = iWidthLeft;

        completeWidth += iWidthLeft;
      }

      int   iOffset = 0;
      it = m_vectColumns.begin();
      while ( it != m_vectColumns.end() )
      {
        tListCtrlColumn&    Column = *it;

        Column.CurrentOffset = iOffset;

        iOffset += Column.CurrentWidth;
        ++it;
      }

      if ( completeWidth > m_ClientRect.width() )
      {
        // H-Scrollbar wird benötigt
        m_pScrollBarH->SetLocation( 0, m_ClientRect.height() - 20 );
        m_pScrollBarH->SetSize( m_pScrollBarH->Width(), 20 );
        m_pScrollBarH->SetScrollRange( 0, completeWidth - m_ClientRect.width() );
      }
      else
      {
        completeWidth = m_ClientRect.width();
        m_pScrollBarH->SetLocation( 0, m_ClientRect.height() );
        m_pScrollBarH->SetSize( m_pScrollBarH->Width(), 0 );
        m_pScrollBarH->SetScrollRange( 0, 0 );
      }
      if ( m_HorzOffset > completeWidth - m_ClientRect.width() )
      {
        m_HorzOffset = completeWidth - m_ClientRect.width();
      }
      if ( m_HorzOffset < 0 )
      {
        m_HorzOffset = 0;
      }
    }



    virtual int FindItemByText( size_t ColumnIndex, const GR::String& Text )
    {
      if ( ColumnIndex >= m_vectColumns.size() )
      {
        return -1;
      }

      for ( size_t RowIndex = 0; RowIndex < m_Items.NumRows(); ++RowIndex )
      {
        const typename tTableType::MultiColumnRow& row( m_Items.Row( m_ItemIndex[RowIndex] ) );

        if ( row.SubItems[ColumnIndex].Text == Text )
        {
          return (int)RowIndex;
        }
      }
      return -1;
    }



    virtual void SelectItem( size_t iItem )
    {
      if ( iItem == -1 )
      {
        m_SelectedItem = (size_t)-1;
        GenerateEventForParent( OET_LISTBOX_ITEM_SELECTED, m_SelectedItem );
        return;
      }

      if ( ( iItem < 0 )
      ||   ( iItem >= (int)m_Items.NumRows() ) )
      {
        return;
      }
      if ( m_SelectedItem == iItem )
      {
        return;
      }
      m_SelectedItem = iItem;
      if ( m_SelectedItem < m_Offset )
      {
        m_pScrollBar->SetScrollPosition( (int)m_SelectedItem );
      }
      if ( m_SelectedItem >= m_Offset + m_VisibleItems )
      {
        int   iNewOffset = (int)( m_SelectedItem - m_VisibleItems + 1 );
        if ( iNewOffset < 0 )
        {
          iNewOffset = 0;
        }
        m_pScrollBar->SetScrollPosition( iNewOffset );
      }
      GenerateEventForParent( OET_LISTBOX_ITEM_SELECTED, m_SelectedItem );
    }



    bool HitTest( const GR::tPoint& ptPosition, size_t& iItem, size_t& subItem )
    {
      iItem     = (size_t)-1;
      subItem  = (size_t)-1;

      GR::tRect   rectList;
      GetListRect( rectList );
      if ( !rectList.contains( ptPosition ) )
      {
        return false;
      }

      iItem = ptPosition.y / m_ItemHeight + m_Offset;
      if ( Style() & LCS_SHOW_HEADER )
      {
        iItem = ( ptPosition.y - m_HeaderHeight ) / m_ItemHeight + m_Offset;
      }
      if ( iItem >= m_Items.NumRows() )
      {
        iItem = (size_t)-1;
        m_LastClickTicks = 0;
      }

      for ( size_t i = 0; i < m_vectColumns.size(); ++i )
      {
        if ( ( ptPosition.x - rectList.Left >= m_vectColumns[i].CurrentOffset )
        &&   ( ptPosition.x - rectList.Left < m_vectColumns[i].CurrentOffset + m_vectColumns[i].CurrentWidth ) )
        {
          subItem = i;
          break;
        }
      }
      return ( subItem != (size_t)-1 );
    }



    bool HitTestHeader( const GR::tPoint& ptPosition, size_t& ColumnIndex )
    {
      ColumnIndex  = (size_t)-1;

      GR::tRect   rectList;
      GetHeaderRect( rectList );
      if ( !rectList.contains( ptPosition ) )
      {
        return false;
      }
      for ( size_t i = 0; i < m_vectColumns.size(); ++i )
      {
        if ( ( ptPosition.x - rectList.Left >= m_vectColumns[i].CurrentOffset )
        &&   ( ptPosition.x - rectList.Left < m_vectColumns[i].CurrentOffset + m_vectColumns[i].CurrentWidth ) )
        {
          ColumnIndex = i;
          break;
        }
      }
      return true;
    }



    void SortByColumn( size_t ColumnIndex )
    {
      if ( m_SortColumnIndex == ColumnIndex )
      {
        m_SortAscending = !m_SortAscending;
      }
      else
      {
        m_SortColumnIndex = ColumnIndex;
        m_SortAscending = true;
      }
      if ( m_Items.NumRows() > 1 )
      {
        bool    hasCompareFunction = !!( m_CompareFunction );
        size_t  numRows = m_Items.NumRows();

        for ( size_t i = 0; i < numRows - 1; ++i )
        {
          for ( size_t j = i + 1; j < numRows; ++j )
          {
            if ( m_SortAscending )
            {
              const typename tTableType::MultiColumnRow& row1( m_Items.Row( m_ItemIndex[i] ) );
              const typename tTableType::MultiColumnRow& row2( m_Items.Row( m_ItemIndex[j] ) );

              if ( hasCompareFunction )
              {
                if ( !m_CompareFunction( m_SortColumnIndex, row2, row1 ) )
                {
                  size_t    temp = m_ItemIndex[i];
                  m_ItemIndex[i] = m_ItemIndex[j];
                  m_ItemIndex[j] = temp;
                }
              }
              else if ( row2.SubItems[m_SortColumnIndex].Text > row1.SubItems[m_SortColumnIndex].Text )
              {
                size_t    temp = m_ItemIndex[i];
                m_ItemIndex[i] = m_ItemIndex[j];
                m_ItemIndex[j] = temp;
              }
            }
            else
            {
              const typename tTableType::MultiColumnRow& row2( m_Items.Row( m_ItemIndex[i] ) );
              const typename tTableType::MultiColumnRow& row1( m_Items.Row( m_ItemIndex[j] ) );

              if ( hasCompareFunction )
              {
                if ( !m_CompareFunction( m_SortColumnIndex, row2, row1 ) )
                {
                  size_t    temp = m_ItemIndex[i];
                  m_ItemIndex[i] = m_ItemIndex[j];
                  m_ItemIndex[j] = temp;
                }
              }
              else if ( row2.SubItems[m_SortColumnIndex].Text > row1.SubItems[m_SortColumnIndex].Text )
              {
                size_t    temp = m_ItemIndex[i];
                m_ItemIndex[i] = m_ItemIndex[j];
                m_ItemIndex[j] = temp;
              }
            }
          }
        }
      }
      /*
      std::multiset<std::pair<GR::String,size_t> >   tempSortContainer;

      for ( size_t i = 0; i < m_Items.NumRows(); ++i )
      {
        GR::String     textToSort = m_Items.Row( i ).SubItems[ColumnIndex].Text;

        tempSortContainer.insert( std::make_pair( textToSort, i ) );
      }
      */

      /*
      std::vector<size_t>   newSortIndices;

      newSortIndices.resize( m_Items.NumRows() );

      size_t                insertIndex = 0;

      if ( m_SortAscending )
      {
        std::multiset<std::pair<GR::String,size_t> >::const_iterator   itS( tempSortContainer.begin() );
        while ( itS != tempSortContainer.end() )
        {
          newSortIndices[insertIndex] = itS->second;
          ++insertIndex;

          ++itS;
        }
      }
      else
      {
        std::multiset<std::pair<GR::String,size_t> >::const_reverse_iterator itS( tempSortContainer.rbegin() );
        while ( itS != tempSortContainer.rend() )
        {
          newSortIndices[insertIndex] = itS->second;
          ++insertIndex;

          ++itS;
        }
      }
      m_ItemIndex = newSortIndices;
      */
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
        case CET_INIT_AFTER_LOAD:
          {
            GR::Database::Table*  pTable = (GR::Database::Table*)Event.Value;

            for ( size_t i = 0; i < m_vectColumns.size(); ++i )
            {
              if ( m_vectColumns[i].DBTextID != -1 )
              {
                m_vectColumns[i].Description = pTable->Entry( m_vectColumns[i].DBTextID );
              }
            }
          }
          break;
        case CET_MOUSE_WHEEL:
          if ( IsMouseInside() )
          {
            GUI::ComponentEvent    newEvent( Event );

            LocalToScreen( newEvent.Position );
            GUI::ComponentContainer::ScreenToLocal( newEvent.Position, m_pScrollBar );
            m_pScrollBar->ProcessEvent( Event );
          }
          return true;
        case CET_MOUSE_OUT:
          m_MouseOverItem     = (size_t)-1;
          m_MouseOverSubItem  = (size_t)-1;
          break;
        case CET_KEY_DOWN:
          if ( m_Items.NumRows() )
          {
            if ( ( Event.Character == Xtreme::KEY_DOWN )
            ||   ( Event.Character == Xtreme::KEY_NUMPAD_2 ) )
            {
              if ( m_SelectedItem < m_Items.NumRows() - 1 )
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
              int iNewItem = (int)m_SelectedItem - (int)m_VisibleItems;
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
              size_t iNewItem = m_SelectedItem + m_VisibleItems;
              if ( iNewItem >= m_Items.NumRows() )
              {
                iNewItem = m_Items.NumRows() - 1;
              }
              SelectItem( iNewItem );
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
              if ( m_SelectedItem != m_Items.NumRows() - 1 )
              {
                SelectItem( m_Items.NumRows() - 1 );
                return true;
              }
            }
            else if ( ( Event.Character == Xtreme::KEY_ENTER )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_ENTER ) )
            {
              if ( m_SelectedItem != -1 )
              {
                GenerateEventForParent( OET_LISTBOX_ITEM_KEY_ACTIVATE, m_SelectedItem );
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
            size_t    iItem = -1;
            size_t    subItem = -1;

            if ( HitTestHeader( Event.Position, subItem ) )
            {
              if ( m_HeaderPushedIndex != (size_t)-1 )
              {
                if ( subItem == m_HeaderPushedIndex )
                {
                  // released header, re-sort
                  if ( Style() & LCS_HEADER_SORT )
                  {
                    SortByColumn( m_HeaderPushedIndex );
                  }
                }
                m_HeaderPushedIndex = (size_t)-1;
              }
            }

            if ( HitTest( Event.Position, iItem, subItem ) )
            {
              m_MouseOverItem     = iItem;
              m_MouseOverSubItem  = subItem;
            }
            else
            {
              m_MouseOverItem = (size_t)-1;
              m_MouseOverSubItem = (size_t)-1;
            }
          }
          break;
        case CET_MOUSE_DOWN:
          if ( IsEnabled() )
          {
            size_t    iItem = -1;
            size_t    subItem = -1;

            if ( HitTestHeader( Event.Position, subItem ) )
            {
              GenerateEventForParent( OET_LISTBOX_HEADER_CLICK, -1, subItem );
              m_HeaderPushedIndex = subItem;
            }
            else
            {
              m_HeaderPushedIndex = -1;
            }

            if ( HitTest( Event.Position, iItem, subItem ) )
            {
              if ( Style() & LCS_SINGLE_SELECT )
              {
                if ( ( Event.MouseButtons & 1 )
                &&   ( m_ButtonReleased ) )
                {
                  m_ButtonReleased = false;
                  
                  bool    bSendDblClk = false;
                  if ( m_SelectedItem == iItem )
                  {
                    bool    sendDblClk = false;
                    if ( GUI::GetTicks() - m_LastClickTicks <= GUI::GetDoubleClickTimeMS() )
                    {
                      bSendDblClk = true;
                    }
                  }
                  SelectItem( iItem );

                  if ( iItem != (size_t)-1 )
                  {
                    GenerateEventForParent( OET_LISTBOX_ITEM_CLICK, m_ItemIndex[iItem], subItem );
                  }

                  m_LastClickTicks = GUI::GetTicks();
                  if ( bSendDblClk )
                  {
                    if ( m_SelectedItem != (size_t)-1 )
                    {
                      GenerateEventForParent( OET_LISTBOX_ITEM_DBLCLK, m_ItemIndex[m_SelectedItem], subItem );
                    }
                  }
                  return true;
                }
              }
              else
              {
                if ( ( Event.MouseButtons & 1 )
                &&   ( m_ButtonReleased ) )
                {
                  m_ButtonReleased = false;
                  GenerateEventForParent( OET_LISTBOX_ITEM_CLICK, m_ItemIndex[iItem], subItem );
                }
              }
            }
          }
          break;
        case CET_SET_SIZE:
          BASECLASS::ProcessEvent( Event );
          RecalcClientRect();

          m_pScrollBar->SetLocation( m_ClientRect.size().x - m_pScrollBar->Width(), 0 );
          m_pScrollBar->SetSize( m_pScrollBar->Width(), m_ClientRect.height() );

          m_pScrollBarH->SetLocation( 0, m_ClientRect.height() - m_pScrollBarH->Height() );
          m_pScrollBarH->SetSize( m_ClientRect.width(), m_pScrollBarH->Height() );

          UpdateScrollBar();
          AdjustColumnOffsets();
          return true;
      }
      return BASECLASS::ProcessEvent( Event );
    }



    virtual bool ProcessEvent( const GUI::OutputEvent& Event )
    {
      if ( Event.Type == OET_SCROLLBAR_SCROLLED )
      {
        if ( Event.pComponent == m_pScrollBar )
        {
          m_Offset = Event.Param1;
        }
        else if ( Event.pComponent == m_pScrollBarH )
        {
          m_HorzOffset = (int)Event.Param1;
        }
      }
      return false;
    }



    virtual size_t GetSelectedItem() const
    {
      return m_SelectedItem;
    }



    virtual size_t GetItemCount() const
    {
      return m_Items.NumRows();
    }



    void Scroll( int iDirection )
    {
      m_pScrollBar->SetScrollPosition( m_Offset + iDirection );
    }



    virtual size_t AddString( const GR::String& String, GR::up ItemData = 0 )
    {
      if ( m_vectColumns.empty() )
      {
        return (size_t)-1;
      }

      typename tTableType::MultiColumnRow    newRow;


      tListCtrlItem   newItem;

      newItem.Text      = String;
      newItem.ItemData  = ItemData;

      newRow.SubItems.push_back( newItem );

      // fill to number of columns
      while ( newRow.SubItems.size() < m_vectColumns.size() )
      {
        newRow.SubItems.push_back( tListCtrlItem() );
      }
      size_t newIndex = m_Items.AddRow( newRow );
      m_ItemIndex.resize( m_Items.NumRows() );
      m_ItemIndex[newIndex] = newIndex;

      UpdateScrollBar();

      return m_Items.NumRows() - 1;
    }



    virtual void DeleteItem( size_t Item )
    {
      if ( Item >= m_Items.NumRows() )
      {
        return;
      }
      m_Items.DeleteRow( m_ItemIndex[Item] );

      // shuffle old indices
      m_ItemIndex.erase( m_ItemIndex.begin() + Item );
      UpdateScrollBar();
    }



    virtual void DeleteAllColumns()
    {
      m_vectColumns.clear();
      ResetContent();
    }



    virtual void DeleteAllItems()
    {
      m_SelectedItem      = (size_t)-1;
      m_MouseOverItem     = (size_t)-1;
      m_MouseOverSubItem  = (size_t)-1;
      m_Items.ClearItems();
      m_ItemIndex.clear();

      UpdateScrollBar();
    }



    virtual void ResetContent()
    {
      m_SelectedItem      = (size_t)-1;
      m_MouseOverItem     = (size_t)-1;
      m_MouseOverSubItem  = (size_t)-1;
      m_Items.Clear();
      m_ItemIndex.clear();
      m_vectColumns.clear();

      UpdateScrollBar();
    }



    virtual GR::String GetItemText( size_t Item, size_t Column = 0 )
    {
      if ( Item >= m_Items.NumRows() )
      {
        return GR::String();
      }
      if ( ( Column >= m_vectColumns.size() )
      ||   ( Column >= m_Items.NumColumns() ) )
      {
        return GR::String();
      }
      return m_Items.Row( m_ItemIndex[Item] ).SubItems[Column].Text;
    }



    virtual void SetItemText( size_t Item, size_t Column, const GR::String& Text )
    {
      if ( Item >= m_Items.NumRows() )
      {
        return;
      }
      if ( Column >= m_vectColumns.size() )
      {
        return;
      }
      m_Items.Row( m_ItemIndex[Item] ).SubItems[Column].Text = Text;
    }



    virtual void SetItemData( size_t Item, size_t Column, GR::up ItemData )
    {
      if ( Item >= m_Items.NumRows() )
      {
        return;
      }
      if ( Column >= m_vectColumns.size() )
      {
        return;
      }
      m_Items.Row( m_ItemIndex[Item] ).SubItems[Column].ItemData = ItemData;
    }



    virtual GR::up GetItemData( size_t Item, size_t Column = 0 )
    {
      if ( Item >= m_Items.NumRows() )
      {
        return 0;
      }
      if ( Column >= m_vectColumns.size() )
      {
        return 0;
      }
      return m_Items.Row( m_ItemIndex[Item] ).SubItems[Column].ItemData;
    }



    virtual bool GetLineRect( size_t Item, GR::tRect& LineRect )
    {
      if ( Item >= m_Items.NumRows() )
      {
        return false;
      }
      if ( Item < m_Offset )
      {
        return false;
      }

      GR::tRect   rcList;

      GetListRect( rcList );

      LineRect.position( 0, (int)( Item - m_Offset ) * m_ItemHeight );
      LineRect.size( rcList.width(), m_ItemHeight );

      if ( Style() & LCS_SHOW_HEADER )
      {
        LineRect.offset( 0, m_HeaderHeight );
      }

      if ( LineRect.position().y + LineRect.size().y < 0 )
      {
        LineRect.size( 0, 0 );
        LineRect.position( 0, 0 );
        return false;
      }
      if ( LineRect.position().y >= m_ClientRect.size().y )
      {
        LineRect.size( 0, 0 );
        LineRect.position( 0, 0 );
        return false;
      }
      if ( LineRect.position().y < 0 )
      {
        LineRect.size( LineRect.size().x, LineRect.size().y + LineRect.position().y );
        LineRect.position( LineRect.position().x, 0 );
      }
      if ( LineRect.position().y + LineRect.size().y >= m_ClientRect.size().y )
      {
        LineRect.size( LineRect.size().x, m_ClientRect.size().y - LineRect.position().y );
      }

      LineRect.offset( -m_HorzOffset, 0 );
      return true;
    }



    virtual bool GetItemRect( size_t Item, size_t Column, GR::tRect& ItemRect )
    {
      if ( ( Item >= m_Items.NumRows() )
      ||   ( Item < m_Offset ) )
      {
        return false;
      }
      if ( ( Column >= m_vectColumns.size() )
      &&   ( Column != -1 ) )
      {
        return false;
      }

      int     iX = 0;

      if ( Column != -1 )
      {
        size_t  tempColumn = 0;

        while ( tempColumn < Column )
        {
          iX += m_vectColumns[tempColumn].CurrentWidth;
          ++tempColumn;
        }
        ItemRect.position( iX, (int)( Item - m_Offset ) * m_ItemHeight );
        ItemRect.size( m_vectColumns[tempColumn].CurrentWidth, m_ItemHeight );
      }
      else
      {
        ItemRect.position( 0, (int)( Item - m_Offset ) * m_ItemHeight );
        ItemRect.size( m_ClientRect.width() + m_pScrollBarH->GetMax() - m_pScrollBar->Width(), m_ItemHeight );
      }

      if ( Style() & LCS_SHOW_HEADER )
      {
        ItemRect.offset( 0, m_HeaderHeight );
      }

      if ( ItemRect.position().y + ItemRect.size().y < 0 )
      {
        ItemRect.size( 0, 0 );
        ItemRect.position( 0, 0 );
        return false;
      }
      if ( ItemRect.position().y >= m_ClientRect.size().y )
      {
        ItemRect.size( 0, 0 );
        ItemRect.position( 0, 0 );
        return false;
      }
      if ( ItemRect.position().y < 0 )
      {
        ItemRect.size( ItemRect.size().x, ItemRect.size().y + ItemRect.position().y );
        ItemRect.position( ItemRect.position().x, 0 );
      }

      ItemRect.offset( -m_HorzOffset, 0 );

      return true;
    }



    SCROLLBARCLASS* GetScrollbarV()
    {
      return m_pScrollBar;
    }



    SCROLLBARCLASS* GetScrollbarH()
    {
      return m_pScrollBarH;
    }



    void SetItemHeight( size_t iHeight )
    {
      m_ItemHeight = (int)iHeight;
      if ( m_ItemHeight < 4 )
      {
        m_ItemHeight = 4;
      }
    }



    void SetHeaderHeight( size_t iHeight )
    {
      m_HeaderHeight = (int)iHeight;
    }



    virtual void GetListRect( GR::tRect& rectItem )
    {
      if ( Style() & LCS_SHOW_HEADER )
      {
        rectItem.position( 0, m_HeaderHeight );
        rectItem.size( m_ClientRect.width() + m_pScrollBarH->GetMax() - m_pScrollBar->Width(), m_ClientRect.height() - m_HeaderHeight );
      }
      else
      {
        rectItem.position( 0, 0 );
        rectItem.size( m_ClientRect.width() + m_pScrollBarH->GetMax() - m_pScrollBar->Width(), m_ClientRect.height() );
      }
      rectItem.offset( -m_HorzOffset, 0 );
    }



    virtual void GetHeaderRect( size_t iColumn, GR::tRect& rectHeader )
    {
      if ( iColumn >= m_vectColumns.size() )
      {
        rectHeader.position( 0, 0 );
        rectHeader.size( 0, 0 );
        return;
      }
      if ( Style() & LCS_SHOW_HEADER )
      {
        rectHeader.position( m_vectColumns[iColumn].CurrentOffset, 0 );
        rectHeader.size( m_vectColumns[iColumn].CurrentWidth, m_HeaderHeight );
      }
      else
      {
        rectHeader.position( 0, 0 );
        rectHeader.size( m_ClientRect.size().x, 0 );
      }
      rectHeader.offset( -m_HorzOffset, 0 );
    }



    virtual void GetHeaderRect( GR::tRect& rectHeader )
    {
      rectHeader.position( 0, 0 );
      if ( Style() & LCS_SHOW_HEADER )
      {
        rectHeader.size( m_ClientRect.width(), m_HeaderHeight );
      }
      else
      {
        rectHeader.size( 0, 0 );
      }
      rectHeader.offset( -m_HorzOffset, 0 );
    }



    size_t      Columns() const
    {
      return m_vectColumns.size();
    }



    GR::String ColumnText( size_t iIndex ) const
    {
      if ( iIndex >= m_vectColumns.size() )
      {
        return "";
      }
      return m_vectColumns[iIndex].Description;
    }



    virtual void SetFont( Interface::IFont* pFont )
    {
      BASECLASS::SetFont( pFont );

      if ( m_pFont )
      {
        SetItemHeight( m_pFont->TextHeight( "\xC4\xD6\xDC\xDFylgA0" ) );
        m_HeaderHeight = m_ItemHeight + 2;
        UpdateScrollBar();
      }
    }



    virtual GR::u32 ModifyStyle( GR::u32 dwAdd, GR::u32 dwRemove = 0 )
    {
      GR::u32 uResult = BASECLASS::ModifyStyle( dwAdd, dwRemove );

      UpdateScrollBar();
      return uResult;
    }



    void ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment )
    {
      BS_::ParseXML( pElement, Environment );
      if ( pElement == NULL )
      {
        return;
      }
      if ( pElement->HasAttribute( "ItemHeight" ) )
      {
        m_ItemHeight = GR::Convert::ToI32( pElement->Attribute( "ItemHeight" ) );
      }
      if ( pElement->HasAttribute( "HeaderHeight" ) )
      {
        m_HeaderHeight = GR::Convert::ToI32( pElement->Attribute( "HeaderHeight" ) );
      }
      GR::Strings::XML::iterator    it( pElement->FirstChild() );
      while ( it != GR::Strings::XML::iterator() )
      {
        GR::Strings::XMLElement* pElementChild( *it );

        if ( pElementChild->Type() == "Column" )
        {
          if ( pElementChild->HasAttribute( "ID" ) )
          {
            size_t columnNo = InsertColumn( "", 
                          GR::Convert::ToI32( pElementChild->Attribute( "Width" ) ), 
                          GR::Convert::ToI32( pElementChild->Attribute( "TextAlignment" ) ) );
            m_vectColumns[columnNo].DBTextID = GR::Convert::ToI32( pElementChild->Attribute( "ID" ) );
          }
          else
          {
            InsertColumn( pElementChild->Attribute( "Name" ), 
                          GR::Convert::ToI32( pElementChild->Attribute( "Width" ) ), 
                          GR::Convert::ToI32( pElementChild->Attribute( "TextAlignment" ) ) );
          }
        }
        it = it.next_sibling();
      }
    }



    void SetCompareFunction( tCompareFunction Function = tCompareFunction() )
    {
      m_CompareFunction = Function;
    }


  protected:



    void SetInternalItemData( size_t Item, size_t Column, GR::up ItemData )
    {
      if ( Item >= m_Items.NumRows() )
      {
        return;
      }
      if ( Column >= m_vectColumns.size() )
      {
        return;
      }
      m_Items.Row( m_ItemIndex[Item] ).SubItems[Column].InternalItemData = ItemData;
    }



    GR::up GetInternalItemData( size_t Item, size_t Column = 0 )
    {
      if ( Item >= m_Items.NumRows() )
      {
        return 0;
      }
      if ( Column >= m_vectColumns.size() )
      {
        return 0;
      }
      return m_Items.Row( m_ItemIndex[Item] ).SubItems[Column].InternalItemData;
    }



};


#endif // __ABSTRACTLISTCONTROL_H__



