#ifndef ABSTRACT_SCROLLABLE_CONTAINER_H
#define ABSTRACT_SCROLLABLE_CONTAINER_H



#include <string>
#include <vector>

#include <GR/GRTypes.h>

#include <Controls\AbstractScrollBar.h>



template <class BS_, class SB_> class AbstractScrollableContainer : public BS_
{

  public:

    enum ScrollableContainerStyles
    {
      SCS_SHOW_H_SCROLLBAR      = 0x00000001,
      SCS_SHOW_V_SCROLLBAR      = 0x00000002,
      SCS_AUTO_HIDE_SCROLLBAR   = 0x00000004,
      SCS_DEFAULT               = SCS_SHOW_V_SCROLLBAR
    };

    typedef BS_   BASECLASS;
    typedef SB_   SCROLLBARCLASS;
  


  protected:

    int                         m_OffsetX;
    int                         m_OffsetY;

    size_t                      m_VisibleRows,
                                m_FirstVisibleItem;

    int                         m_ItemWidth,
                                m_ItemHeight,
                                m_ItemsPerLine;

    GR::u64                     m_LastClickTicks;

    SCROLLBARCLASS*             m_pScrollBar;
    SCROLLBARCLASS*             m_pScrollHBar;

    bool                        m_ButtonReleased;



    void UpdateScrollBar()
    {
      GR::tRect     virtualClientSize = CalculateVirtualClientSize();

      //dh::Log( "virtualClientSize = %d,%d - %dx%d", virtualClientSize.Left, virtualClientSize.Top, virtualClientSize.width(), virtualClientSize.height() );

      if ( Style() & SCS_SHOW_H_SCROLLBAR )
      {
        m_ItemsPerLine = ( m_ClientRect.Width() - m_pScrollBar->Width() ) / m_ItemWidth;
        if ( m_ItemsPerLine == 0 )
        {
          m_ItemsPerLine = 1;
        }
      }

      if ( ( Style() & SCS_AUTO_HIDE_SCROLLBAR )
      &&   ( virtualClientSize.Height() <= m_ClientRect.Height() ) )
      {
        m_pScrollBar->SetSize( 0, m_pScrollBar->Height() );
      }
      else
      {
        m_pScrollBar->SetSize( 20, m_pScrollBar->Height() );
        m_pScrollBar->SetLocation( m_ClientRect.Width() - 20, 0 );
      }
        
      if ( virtualClientSize.Height() <= m_ClientRect.Height() )
      {
        // nothing to scroll
        m_pScrollBar->SetScrollRange( 0, 0 );
      }
      else
      {
        m_pScrollBar->SetScrollRange( 0, virtualClientSize.Height() - m_ClientRect.Height(), m_ClientRect.Height() );
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
    using BASECLASS::ProcessEvent;



    AbstractScrollableContainer( GR::u32 Type = SCS_DEFAULT, GR::u32 Id = 0 ) :
      BASECLASS()
    {
      m_ClassName         = "ScrollableContainer";

      m_Style             = Type;

      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;

      m_ButtonReleased    = true;

      m_LastClickTicks    = 0;

      m_ItemHeight        = 12;
      m_ItemWidth         = 40;
      m_SelectedItem      = -1;
      m_FirstVisibleItem  = 0;
      m_ItemsPerLine      = 1;
      m_OffsetX           = 0;
      m_OffsetY           = 0;
      m_TextAlignment     = GUI::AF_LEFT | GUI::AF_VCENTER;

      m_pScrollBar = new SCROLLBARCLASS( m_Width - 20, 0, 20, m_Height, SCROLLBARCLASS::SBFT_INVALID, Id );
      m_pScrollBar->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );

      m_pScrollHBar = new SCROLLBARCLASS( 0, m_Height - 20, m_Width, 20, SCROLLBARCLASS::SBFT_HORIZONTAL, Id );
      m_pScrollHBar->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );
      if ( !( Type & SCS_SHOW_H_SCROLLBAR ) )
      {
        m_pScrollHBar->SetVisible( false );
      }

      Add( m_pScrollBar );
      Add( m_pScrollHBar );

      m_pScrollBar->AddEventListener( this );
      m_pScrollHBar->SetScrollRange( 0, 0, 0 );
      m_pScrollHBar->AddListener( this );

      UpdateScrollBar();
    }



    AbstractScrollableContainer( int X, int Y, int Width, int Height, GR::u32 Type = SCS_DEFAULT, GR::u32 Id = 0 ) :
      BASECLASS( X, Y, Width, Height, Id )
    {
      m_ClassName         = "ScrollableContainer";

      m_Style             = Type;

      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;

      m_ButtonReleased    = true;

      m_LastClickTicks    = 0;

      m_ItemHeight        = 12;
      m_TextAlignment     = GUI::AF_LEFT | GUI::AF_VCENTER;
      m_OffsetX           = 0;
      m_OffsetY           = 0;

      RecalcClientRect();
      m_pScrollBar = new SCROLLBARCLASS( m_Width - 20, 0, 20, m_Height, SCROLLBARCLASS::SBFT_DEFAULT, Id );
      m_pScrollBar->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );

      m_pScrollHBar = new SCROLLBARCLASS( 0, m_Height - 20, m_Width, 20, SCROLLBARCLASS::SBFT_HORIZONTAL, Id );
      m_pScrollHBar->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );
      if ( !( Type & SCS_SHOW_H_SCROLLBAR ) )
      {
        m_pScrollHBar->SetVisible( false );
      }

      m_pScrollBar->SetScrollRange( 0, 0, 0 );
      m_pScrollBar->AddListener( this );
      m_pScrollHBar->SetScrollRange( 0, 0, 0 );
      m_pScrollHBar->AddListener( this );

      Add( m_pScrollBar );
      Add( m_pScrollHBar );

      m_ItemWidth        = m_ClientRect.Width() - m_pScrollBar->Width();
      m_ItemsPerLine     = 1;
      if ( Type & SCS_SHOW_H_SCROLLBAR )
      {
        m_ItemsPerLine = ( m_ClientRect.Width() - m_pScrollBar->Width() ) / m_ItemWidth;
        if ( m_ItemsPerLine == 0 )
        {
          m_ItemsPerLine = 1;
        }
      }

      UpdateScrollBar();
    }



    void Scroll( int Direction )
    {
      m_pScrollBar->SetScrollPosition( m_FirstVisibleItem + Direction * m_ItemsPerLine );
    }



    virtual bool ProcessEvent( const GUI::ComponentEvent& Event )
    {
      switch ( Event.Type )
      {
        case CET_ADDED_CHILD:
        case CET_REMOVED_CHILD:
          UpdateScrollBar();
          break;
        case CET_SET_SIZE:
        case CET_SET_CLIENT_SIZE:
          BASECLASS::ProcessEvent( Event );

          m_ItemWidth        = m_ClientRect.Width() - m_pScrollBar->Width();
          m_ItemsPerLine     = 1;
          if ( Style() & SCS_SHOW_H_SCROLLBAR )
          {
            m_ItemsPerLine = ( m_ClientRect.Width() - m_pScrollBar->Width() ) / m_ItemWidth;
            if ( m_ItemsPerLine == 0 )
            {
              m_ItemsPerLine = 1;
            }
          }

          m_pScrollBar->SetLocation( m_ClientRect.Width() - m_pScrollBar->Width(), 0 );
          m_pScrollBar->SetSize( m_pScrollBar->Width(), m_ClientRect.Height() );

          m_pScrollHBar->SetLocation( 0, m_ClientRect.Height() - m_pScrollBar->Height() );
          m_pScrollHBar->SetSize( m_ClientRect.Width(), m_pScrollBar->Height() );

          UpdateScrollBar();
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
        case CET_KEY_DOWN:
          if ( IsEnabled() )
          {
            if ( ( Event.Character == Xtreme::KEY_DOWN )
            ||   ( Event.Character == Xtreme::KEY_NUMPAD_2 ) )
            {
              // Scroll down?
            }
            else if ( ( Event.Character == Xtreme::KEY_UP )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_8 ) )
            {
              // Scroll up?
            }
            else if ( ( Event.Character == Xtreme::KEY_PAGEUP )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_9 ) )
            {
              // scroll page up?
              return true;
            }
            else if ( ( Event.Character == Xtreme::KEY_PAGEDOWN )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_3 ) )
            {
              // scroll page down?
              return true;
            }
            else if ( ( Event.Character == Xtreme::KEY_HOME )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_7 ) )
            {
              // scroll to top
              m_pScrollBar->SetScrollPosition( 0 );
              return true;
            }
            else if ( ( Event.Character == Xtreme::KEY_END )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_1 ) )
            {
              m_pScrollBar->SetScrollPosition( m_pScrollBar->GetMax() );
              return true;
            }
          }
          break;
      }

      return BASECLASS::ProcessEvent( Event );
    }



    GR::tRect CalculateVirtualClientSize()
    {
      GR::tRect     virtualClientSize = m_ClientRect;

      int           offsetX = m_pScrollBar->GetScrollPosition();
      int           offsetY = m_pScrollHBar->GetScrollPosition();

      std::vector<GUI::Component*>::iterator   it( m_Components.begin() );
      while ( it != m_Components.end() )
      {
        Component* pComp = *it;

        if ( ( pComp != m_pScrollBar )
        &&   ( pComp != m_pScrollHBar ) )
        {
          GR::tRect     childBounds;
          
          pComp->GetComponentRect( childBounds );
          childBounds.Offset( pComp->Position() );
          if ( childBounds.Left - offsetX < virtualClientSize.Left )
          {
            virtualClientSize.Left = childBounds.Left - offsetX;
          }
          if ( childBounds.Right - offsetX > virtualClientSize.Right )
          {
            virtualClientSize.Right = childBounds.Right - offsetX;
          }
          if ( childBounds.Top - offsetY < virtualClientSize.Top )
          {
            virtualClientSize.Top = childBounds.Top - offsetY;
          }
          if ( childBounds.Bottom - offsetY > virtualClientSize.Bottom )
          {
            virtualClientSize.Bottom = childBounds.Bottom - offsetY;
          }
        }

        ++it;
      }

      return virtualClientSize;
    }



    void ShiftChilds( int DX, int DY )
    {
      std::vector<GUI::Component*>::iterator   it( m_Components.begin() );
      while ( it != m_Components.end() )
      {
        Component* pComp = *it;

        if ( ( pComp != m_pScrollBar )
        &&   ( pComp != m_pScrollHBar ) )
        {
          GR::tPoint    oldPos( pComp->Position() );
          pComp->SetLocation( oldPos.x + DX, oldPos.y + DY );
        }
        ++it;
      }
    }



    virtual bool ProcessEvent( const GUI::OutputEvent& Event )
    {
      if ( Event.Type == OET_SCROLLBAR_SCROLLED )
      {
        if ( Event.pComponent == m_pScrollBar )
        {
          ShiftChilds( 0, m_OffsetY - Event.Param1 );
          m_OffsetY = Event.Param1;
        }
        else if ( Event.pComponent == m_pScrollHBar )
        {
          ShiftChilds( m_OffsetX - Event.Param1, 0 );
          m_OffsetX = Event.Param1;
        }
      }
      return false;
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
      if ( Style() & SCS_SHOW_H_SCROLLBAR )
      {
        m_ItemWidth = Width;
        if ( m_ItemWidth < 4 )
        {
          m_ItemWidth = 4;
        }
      }
      if ( Style() & SCS_SHOW_H_SCROLLBAR )
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


#endif // ABSTRACT_SCROLLABLE_CONTAINER_H



