#ifndef ABSTRACTBUTTON_H
#define ABSTRACTBUTTON_H



#include <string>
#include <vector>

#include <Controls/InputEvent.h>

#include <Controls/ComponentContainer.h>
#include <Controls/ComponentEnums.h>

#include <Xtreme/XInput.h>



template <class BASECLASS> class AbstractButton : public BASECLASS
{

  protected:

    GR::up                m_ItemData;

  public:

    enum eButtonStyle
    {
      BF_PUSHED            = 0x00000001,
      BF_MBUTTON_RELEASED  = 0x00000002,
      BF_CAPTURE_PUSHED    = 0x00000004,
    };

    enum eButtonControlStyle
    {
      BCS_DEFAULT           = 0x00000000,
      BCS_NO_BORDER         = 0x80000000,
    };

    // usings are required for annoying standard conform ignored lookup of base class members
    // alternative would be prefixing everything with "this->" which is even worse
    using BASECLASS::m_ClassName;
    using BASECLASS::m_Style;
    using BASECLASS::m_ComponentFlags;
    using BASECLASS::m_TextAlignment;
    using BASECLASS::ModifyStyle;
    using BASECLASS::ModifyVisualStyle;
    using BASECLASS::VisualStyle;
    using BASECLASS::GetSysColor;
    using BASECLASS::SetColor;
    using BASECLASS::RecalcClientRect;
    using BASECLASS::Style;
    using BASECLASS::IsEnabled;
    using BASECLASS::IsVisible;
    using BASECLASS::SetCapture;
    using BASECLASS::ReleaseCapture;
    using BASECLASS::IsMouseInside;
    using BASECLASS::GenerateEventForParent;
    using BASECLASS::m_DefaultEventScript;
    using BASECLASS::RaiseDefaultEvent;
    using BASECLASS::Invalidate;



    AbstractButton( GR::u32 Id = 0, GR::u32 Style = BCS_DEFAULT ) :
        BASECLASS( Id ),
        m_ItemData( 0 )
    {
      m_ClassName       = "Button";

      m_Style           = Style;
      m_ComponentFlags |= GUI::COMPFT_TAB_STOP;

      m_TextAlignment = GUI::AF_CENTER | GUI::AF_VCENTER;

      if ( m_Style & BCS_NO_BORDER )
      {
        ModifyVisualStyle( 0, GUI::VFT_FLAT_BORDER | GUI::VFT_RAISED_BORDER | GUI::VFT_SUNKEN_BORDER );
      }

      SetColor( GUI::COL_BACKGROUND, GetSysColor( GUI::COL_BTNFACE ) );
    }



    AbstractButton( int X, int Y, int Width, int Height, GR::u32 Id = 0, GR::u32 Style = BCS_DEFAULT ) :
        BASECLASS( X, Y, Width, Height, Id ),
        m_ItemData( 0 )
    {
      m_ClassName       = "Button";

      m_Style           = Style;
      m_ComponentFlags  |= GUI::COMPFT_TAB_STOP;
      m_TextAlignment   = GUI::AF_CENTER | GUI::AF_VCENTER;
      if ( m_Style & BCS_NO_BORDER )
      {
        ModifyVisualStyle( 0, GUI::VFT_FLAT_BORDER | GUI::VFT_RAISED_BORDER | GUI::VFT_SUNKEN_BORDER );
      }
      RecalcClientRect();

      SetColor( GUI::COL_BACKGROUND, GetSysColor( GUI::COL_BTNFACE ) );
    }



    AbstractButton( int X, int Y, int Width, int Height, const GR::String& Caption, GR::u32 Id = 0, GR::u32 Style = BCS_DEFAULT ) :
        BASECLASS( X, Y, Width, Height, Caption, Id ),
        m_ItemData( 0 )
    {
      m_ClassName       = "Button";

      m_Style           = Style;
      m_ComponentFlags  |= GUI::COMPFT_TAB_STOP;
      m_TextAlignment   = GUI::AF_CENTER | GUI::AF_VCENTER;
      if ( m_Style & BCS_NO_BORDER )
      {
        ModifyVisualStyle( 0, GUI::VFT_FLAT_BORDER | GUI::VFT_RAISED_BORDER | GUI::VFT_SUNKEN_BORDER );
      }
      RecalcClientRect();

      SetColor( GUI::COL_BACKGROUND, GetSysColor( GUI::COL_BTNFACE ) );
    }



    void SetItemData( const GR::up ItemData )
    {
      m_ItemData = ItemData;
    }



    GR::up ItemData() const
    {
      return m_ItemData;
    }



    virtual bool IsPushed()
    {
      return ( Style() & BF_PUSHED );
    }



    void Push()
    {
      ProcessEvent( CET_BUTTON_PUSHED );
    }



    virtual bool ProcessEvent( const GUI::ComponentEvent& Event )
    {
      switch ( Event.Type )
      {
        case CET_VISUAL_STYLE_CHANGED:
          if ( VisualStyle() & GUI::VFT_FLAT_BORDER )
          {
            if ( VisualStyle() & ( GUI::VFT_RAISED_BORDER | GUI::VFT_SUNKEN_BORDER ) )
            {
              ModifyVisualStyle( 0, GUI::VFT_RAISED_BORDER | GUI::VFT_SUNKEN_BORDER );
            }
          }
          else if ( VisualStyle() & GUI::VFT_RAISED_BORDER )
          {
            if ( VisualStyle() & ( GUI::VFT_FLAT_BORDER | GUI::VFT_SUNKEN_BORDER ) )
            {
              ModifyVisualStyle( 0, GUI::VFT_FLAT_BORDER | GUI::VFT_SUNKEN_BORDER );
            }
          }
          else if ( VisualStyle() & GUI::VFT_SUNKEN_BORDER )
          {
            if ( VisualStyle() & ( GUI::VFT_FLAT_BORDER | GUI::VFT_RAISED_BORDER ) )
            {
              ModifyVisualStyle( 0, GUI::VFT_FLAT_BORDER | GUI::VFT_RAISED_BORDER );
            }
          }
          break;
        case CET_STYLE_CHANGED:
          if ( Style() & BCS_NO_BORDER )
          {
            ModifyVisualStyle( 0, GUI::VFT_FLAT_BORDER | GUI::VFT_RAISED_BORDER | GUI::VFT_SUNKEN_BORDER );
          }
          break;
        case CET_KEY_DOWN:
          if ( ( IsEnabled() )
          &&   ( IsVisible() ) )
          {
            if ( Event.Character == Xtreme::KEY_SPACE )
            {
              if ( !( Style() & BF_PUSHED ) )
              {
                GenerateEventForParent( OET_BUTTON_DOWN, m_ItemData );
                ModifyStyle( BF_PUSHED );
                Invalidate();
              }
            }
            else if ( ( Event.Character == Xtreme::KEY_ENTER )
            ||        ( Event.Character == Xtreme::KEY_NUMPAD_ENTER ) )
            {
              ProcessEvent( CET_BUTTON_PUSHED );
              Invalidate();
            }
          }
          break;
        case CET_KEY_UP:
          if ( ( IsEnabled() )
          &&   ( IsVisible() ) )
          {
            if ( Event.Character == Xtreme::KEY_SPACE )
            {
              if ( Style() & BF_PUSHED )
              {
                ProcessEvent( CET_BUTTON_PUSHED );
                ModifyStyle( 0, BF_PUSHED | BF_CAPTURE_PUSHED );
                Invalidate();
              }
            }
          }
          break;
        case CET_MOUSE_DOWN:
        case CET_NC_MOUSE_DOWN:
          if ( ( IsEnabled() )
          &&   ( IsVisible() ) )
          {
            if ( !IsPushed() )
            {
              GenerateEventForParent( OET_BUTTON_DOWN, m_ItemData );
              ModifyStyle( BF_PUSHED | BF_CAPTURE_PUSHED );
              SetCapture();
              Invalidate();
            }
          }
          break;
        case CET_MOUSE_UP:
        case CET_NC_MOUSE_UP:
          if ( ( IsEnabled() )
          &&   ( IsVisible() ) )
          {
            if ( Style() & BF_CAPTURE_PUSHED )
            {
              ReleaseCapture();
            }
            if ( Style() & BF_PUSHED )
            {
              if ( IsMouseInside() )
              {
                ProcessEvent( CET_BUTTON_PUSHED );
              }
              else
              {
                GenerateEventForParent( OET_BUTTON_UP, m_ItemData );
              }
            }
          }
          ModifyStyle( 0, BF_PUSHED | BF_CAPTURE_PUSHED );
          Invalidate();
          break;
        case CET_MOUSE_IN:
          Invalidate();
          if ( Event.MouseButtons & 1 )
          {
            ModifyStyle( 0, BF_MBUTTON_RELEASED );
          }
          else
          {
            ModifyStyle( BF_MBUTTON_RELEASED );
          }
          if ( Style() & BF_CAPTURE_PUSHED )
          {
            if ( ( IsEnabled() )
            &&   ( IsVisible() ) )
            {
              if ( !IsPushed() )
              {
                GenerateEventForParent( OET_BUTTON_DOWN, m_ItemData );
                ModifyStyle( BF_PUSHED | BF_CAPTURE_PUSHED | BF_MBUTTON_RELEASED );
              }
            }
          }
          break;
        case CET_MOUSE_OUT:
          if ( IsPushed() )
          {
            GenerateEventForParent( OET_BUTTON_UP, m_ItemData );
            ModifyStyle( 0, BF_PUSHED );
          }
          Invalidate();
          break;
        case CET_BUTTON_PUSHED:
          if ( !m_DefaultEventScript.empty() )
          {
            RaiseDefaultEvent();
            return true;
          }
          GenerateEventForParent( OET_BUTTON_PUSHED, m_ItemData );
          break;
      }

      return BASECLASS::ProcessEvent( Event );
    }

};

#endif // ABSTRACTBUTTON_H
