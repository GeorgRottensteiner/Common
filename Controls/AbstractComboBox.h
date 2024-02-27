#ifndef ABSTRACTCOMBOBOX_H
#define ABSTRACTCOMBOBOX_H


#include <string>
#include <vector>

#include <Controls\AbstractEdit.h>
#include <Controls\AbstractScrollBar.h>



template <class BS_, class EC_, class BT_, class LBC_> class AbstractComboBox : public BS_
{

  public:

    typedef BS_   BASECLASS;
    typedef EC_   EDITCLASS;
    typedef BT_   BUTTONCLASS;
    typedef LBC_  LISTBOXCLASS;
  
    enum ComboFlagType
    {
      CFT_DEFAULT         = 0x00000000l,
    };

  protected:

    GR::u32               m_Type;

    BUTTONCLASS*          m_pButtonDropDown;
    EDITCLASS*            m_pEdit;
    LISTBOXCLASS*         m_pListBox;

    bool                  m_ListBoxOpened;
    bool                  m_ButtonReleased;


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
    using BASECLASS::GetComponentAt;
    using BASECLASS::GetWindowRect;
    using BASECLASS::SetFocus;
    using BASECLASS::IsIndirectHidden;
    using BASECLASS::Add;



    AbstractComboBox( GR::u32 Id = 0 ) :
        BASECLASS( Id )
    {
      m_ClassName         = "ComboBox";

      m_Type              = CFT_DEFAULT;

      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;

      m_ListBoxOpened     = false;
      m_ButtonReleased    = true;

      m_pEdit             = new EDITCLASS();
      m_pButtonDropDown   = new BUTTONCLASS();
      m_pListBox          = new LISTBOXCLASS();

      m_pEdit->ModifyStyle( EDITCLASS::ECS_READ_ONLY, EDITCLASS::ECS_AUTO_H_SCROLL );
      m_pEdit->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );
      m_pEdit->ModifyVisualStyle( 0, GUI::VFT_RAISED_BORDER | GUI::VFT_SUNKEN_BORDER );
      m_pButtonDropDown->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );
      m_pListBox->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );

      m_pListBox->ModifyVisualStyle( GUI::VFT_FLAT_BORDER, GUI::VFT_RAISED_BORDER | GUI::VFT_SUNKEN_BORDER );

      Add( m_pEdit );
      Add( m_pButtonDropDown );

      m_pEdit->AddEventListener( this );
      m_pButtonDropDown->AddEventListener( this );
      m_pListBox->AddEventListener( this );

      m_pListBox->ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
    }



    AbstractComboBox( int X, int Y, int Width, int Height, GR::u32 Type, GR::u32 Id = 0 ) :
      BASECLASS( X, Y, Width, Height, Id )
    {
      m_ClassName         = "ComboBox";

      RecalcClientRect();

      m_Type              = Type;

      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;

      m_ListBoxOpened     = false;
      m_ButtonReleased    = true;

      m_pEdit             = new EDITCLASS( 0, 0, Width - 20, Height, EDITCLASS::ECS_DEFAULT );
      m_pButtonDropDown   = new BUTTONCLASS( Width - 20, 0, 20, Height );
      m_pListBox          = new LISTBOXCLASS( 0, 0, 0, 0 );
      m_pEdit->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );
      m_pEdit->ModifyVisualStyle( 0, GUI::VFT_RAISED_BORDER | GUI::VFT_SUNKEN_BORDER );
      m_pButtonDropDown->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );
      m_pListBox->ModifyFlags( GUI::COMPFT_NOT_SERIALIZABLE );

      m_pListBox->SetVisible( false );
      m_pListBox->SetCaption( "ComboLBox" );
      m_pButtonDropDown->SetCaption( "V" );
      m_pEdit->ModifyStyle( EDITCLASS::ECS_READ_ONLY, EDITCLASS::ECS_AUTO_H_SCROLL );

      m_pListBox->ModifyVisualStyle( GUI::VFT_FLAT_BORDER, GUI::VFT_RAISED_BORDER | GUI::VFT_SUNKEN_BORDER );

      Add( m_pEdit );
      Add( m_pButtonDropDown );

      m_pEdit->AddListener( this );
      m_pButtonDropDown->AddListener( this );
      m_pListBox->AddListener( this );
    }



    virtual ~AbstractComboBox()
    {
      if ( !m_ListBoxOpened )
      {
        if ( m_pListBox )
        {
          delete m_pListBox;
          m_pListBox = NULL;
        }
      }
    }



    virtual bool ProcessEvent( const GUI::ComponentEvent& Event )
    {
      switch ( Event.Type )
      {
        case CET_SET_SIZE:
        case CET_SET_CLIENT_SIZE:
          BASECLASS::ProcessEvent( Event );

          m_pEdit->SetLocation( 0, 0 );
          m_pEdit->SetSize( m_ClientRect.Width() - m_pButtonDropDown->Width(), m_ClientRect.Height() );
          m_pButtonDropDown->SetLocation( m_ClientRect.Width() - m_pButtonDropDown->Width(), 0 );
          m_pButtonDropDown->SetSize( m_pButtonDropDown->Width(), m_ClientRect.Height() );
          return true;
        case CET_MOUSE_DOWN:
          if ( ( m_ListBoxOpened )
          &&   ( IsEnabled() ) )
          {
            if ( !IsMouseInside( Event.Position ) )
            {
              CloseListBox();
            }
          }
          if ( ( !m_ListBoxOpened )
          &&   ( GetComponentAt( Event.Position ) == m_pEdit ) )
          {
            PopupListBox();
            return true;
          }
          break;
        case CET_ENABLE:
          {
            m_pListBox->Enable( !!Event.Value );
            m_pEdit->Enable( !!Event.Value );
            m_pButtonDropDown->Enable( !!Event.Value );
          }
          break;
        case CET_MOUSE_WHEEL:
          if ( m_ListBoxOpened )
          {
            return m_pListBox->ProcessEvent( Event );
          }
          else
          {
            if ( Event.Value < 0 )
            {
              if ( ( m_pListBox->GetSelectedItem() != -1 )
              &&   ( m_pListBox->GetSelectedItem() > 0 ) )
              {
                SelectItem( m_pListBox->GetSelectedItem() - 1 );
              }
            }
            else if ( Event.Value > 0 )
            {
              if ( ( m_pListBox->GetSelectedItem() != -1 )
              &&   ( m_pListBox->GetSelectedItem() + 1 < m_pListBox->GetItemCount() ) )
              {
                SelectItem( m_pListBox->GetSelectedItem() + 1 );
              }
            }
          }
          break;
        case CET_MOUSE_UPDATE:
          {
            if ( ( Event.MouseButtons & 1 ) == 0 )
            {
              m_ButtonReleased = true;
            }
          }
          break;
      }
      return BASECLASS::ProcessEvent( Event );
    }



    virtual bool ProcessEvent( const GUI::OutputEvent& Event )
    {
      if ( Event.Type == OET_BUTTON_PUSHED )
      {
        if ( Event.pComponent == m_pButtonDropDown )
        {
          if ( m_ListBoxOpened )
          {
            CloseListBox();
          }
          else
          {
            PopupListBox();
          }
        }
      }
      else if ( Event.Type == OET_FOCUS_GET )
      {
        if ( Event.pComponent == m_pEdit )
        {
          PopupListBox();
          return true;
        }
      }
      else if ( ( Event.Type == OET_FOCUS_LOOSE )
      ||        ( Event.Type == OET_CAPTURE_END ) )
      {
        if ( ( Event.pComponent == m_pListBox )
        ||   ( Event.pComponent == this ) )
        {
          GUI::Component* pNewFocusComponent = (GUI::Component*)Event.Param1;

          if ( ( pNewFocusComponent )
          &&   ( !pNewFocusComponent->IsIndirectChild( this ) ) )
          {
            CloseListBox();
          }
        }
      }
      else if ( Event.Type == OET_LISTBOX_ITEM_SELECTED )
      {
        if ( Event.pComponent == m_pListBox )
        {
          SelectItem( (int)Event.Param1 );
          CloseListBox();
          m_ButtonReleased = false;
        }
      }
      return false;
    }



    virtual void PopupListBox()
    {
      if ( m_ListBoxOpened )
      {
        return;
      }
      if ( !m_ButtonReleased )
      {
        return;
      }

      m_ButtonReleased = false;
      m_ListBoxOpened  = true;


      GR::tRect   compRect;

      GetWindowRect( compRect );

      GUI::ComponentContainer* pContainer = GetTopLevelParent();

      pContainer->Add( m_pListBox );
      m_pListBox->SetOwner( this );

      GR::tPoint    ptPos( compRect.Left, compRect.Top + m_pEdit->Height() + m_ClientRect.Top );

      int     completeHeight = m_pListBox->ItemHeight() * (int)m_pListBox->GetItemCount();

      GR::tRect   rcTopLevel;
      pContainer->GetClientRect( rcTopLevel );

      if ( completeHeight > rcTopLevel.Bottom - ptPos.y )
      {
        // zuviele Items für unten
        int   visibleItems = 1;
        
        if ( m_pListBox->ItemHeight() )
        {
          visibleItems = ( rcTopLevel.Bottom - ptPos.y ) / m_pListBox->ItemHeight();
        }

        if ( visibleItems <= 0 )
        {
          // yeah sure, show a non existing list
          // open list above?
          GR::tPoint    ptBottomPos( compRect.Left, compRect.Top );

          visibleItems = ptBottomPos.y / m_pListBox->ItemHeight();

          if ( visibleItems > (int)m_pListBox->GetItemCount() )
          {
            visibleItems = (int)m_pListBox->GetItemCount();
          }
          ptPos.y = ptBottomPos.y - visibleItems * m_pListBox->ItemHeight();
        }

        completeHeight = visibleItems * m_pListBox->ItemHeight();
      }

      m_pListBox->SetVisible();
      m_pListBox->Enable();

      m_pListBox->SetLocation( ptPos.x, ptPos.y );
      m_pListBox->SetClientSize( m_ClientRect.Width(), completeHeight );
      m_pListBox->SetFocus();
    }



    virtual void CloseListBox()
    {
      if ( !m_ListBoxOpened )
      {
        return;
      }
      m_ListBoxOpened = false;

      GUI::ComponentContainer* pContainer = GetTopLevelParent();
      
      pContainer->SetModalMode( NULL );
      pContainer->Remove( m_pListBox );
      m_pListBox->SetVisible( false );

      // this should force a MOUSE_IN event on the new control under the mouse
      /*
      Xtreme::XInput* pInput = (Xtreme::XInput*)GR::Service::Environment::Instance().Service( "Input" );
      if ( pInput )
      {
        GUI::ComponentEvent   eventFakeMouse( CET_MOUSE_UPDATE );

        eventFakeMouse.MouseButtons = pInput->MouseButton();
        eventFakeMouse.Position     = pInput->MousePos();

        pContainer->ProcessEvent( eventFakeMouse );
      }*/

      SetFocus();
    }



    BUTTONCLASS* GetDropButton()
    {
      return m_pButtonDropDown;
    }



    LISTBOXCLASS* GetListBox()
    {
      return m_pListBox;
    }



    EDITCLASS* GetEdit()
    {
      return m_pEdit;
    }



    virtual size_t AddString( const GR::String& String, GR::up ItemData = 0 )
    {
      return m_pListBox->AddString( String, ItemData );
    }



    void DeleteItem( size_t ItemIndex )
    {
      m_pListBox->DeleteString( ItemIndex );
    }



    virtual void Clear()
    {
      m_pListBox->ResetContent();
      m_pEdit->SetCaption( "" );
    }



    virtual GR::String GetItemText( size_t ItemIndex ) const
    {
      return m_pListBox->GetItemText( ItemIndex );
    }

    
    
    virtual GR::String GetSelectedItemText() const
    {
      return m_pListBox->GetItemText( m_pListBox->GetSelectedItem() );
    }

    
    
    void SetItemText( size_t ItemIndex, const GR::String& Text )
    {
      m_pListBox->SetItemText( ItemIndex, Text );
    }



    
    size_t SelectItemByText( const GR::String& Text )
    {
      size_t    itemIndex = m_pListBox->FindItemByText( Text );
      SelectItem( itemIndex );
      return itemIndex;
    }



    size_t SelectItemByItemData( GR::up ItemData )
    {
      size_t    itemIndex = m_pListBox->SelectItemByItemData( ItemData );
      SelectItem( itemIndex );
      return itemIndex;
    }



    size_t FindItem( const GR::String& ItemText ) const
    {
      return m_pListBox->FindItemByText( ItemText );
    }



    size_t FindItemByItemData( const GR::up ItemData ) const
    {
      return m_pListBox->FindItemByItemData( ItemData );
    }



    virtual void SelectItem( size_t ItemIndex )
    {
      if ( m_pListBox->GetSelectedItem() != ItemIndex )
      {
        m_pListBox->SelectItem( ItemIndex );
      }
      size_t selectedItem = m_pListBox->GetSelectedItem();
      if ( selectedItem != -1 )
      {
        m_pEdit->SetCaption( m_pListBox->GetItemText( selectedItem ) );

        GenerateEventForParent( OET_COMBO_SELCHANGE, selectedItem, GetItemData( selectedItem ) );
      }
      else
      {
        m_pEdit->SetCaption( "" );
        GenerateEventForParent( OET_COMBO_SELCHANGE, selectedItem );
      }
    }



    size_t GetSelectedItem() const
    {
      return m_pListBox->GetSelectedItem();
    }
    
    
    
    GR::up GetSelectedItemData() const
    {
      return m_pListBox->GetItemData( GetSelectedItem() );
    }
    
    
    
    GR::up GetItemData( size_t iItem ) const
    {
      return m_pListBox->GetItemData( iItem );
    }



    virtual void Update( float ElapsedTime )
    {
      if ( ( m_ListBoxOpened )
      &&   ( IsIndirectHidden() ) )
      {
        CloseListBox();
      }

      BASECLASS::Update( ElapsedTime );
    }



    virtual GR::String GetCaption() const
    {
      return GetSelectedItemText();
    }



};


#endif // __ABSTRACTSCROLLBAR_H__



