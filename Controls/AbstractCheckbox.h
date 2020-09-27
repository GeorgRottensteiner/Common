#ifndef ABSTRACTCHECKBOX_H
#define ABSTRACTCHECKBOX_H



#include <string>
#include <vector>

#include <GR/GRTypes.h>

#include "InputEvent.h"



template <class BASECLASS> class AbstractCheckBox : public BASECLASS
{

  public:

    enum eCheckBoxStyle
    {
      CS_DEFAULT        = 0x00000000l,
      CS_PUSHED         = 0x00000001,
      CS_CHECKBOX_RIGHT = 0x00000002,
      CS_CHECKED        = 0x80000000,
    };


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
    using BASECLASS::m_DefaultEventScript;
    using BASECLASS::RaiseDefaultEvent;



    AbstractCheckBox( GR::u32 Id = 0 ) :
      BASECLASS( Id )
    {

      m_ClassName      = "CheckBox";

      m_Style = CS_DEFAULT;
      m_ComponentFlags |= GUI::COMPFT_TAB_STOP;
      m_TextAlignment = GUI::AF_LEFT | GUI::AF_VCENTER;
    }



    AbstractCheckBox( int X, int Y, int Width, int Height, GR::u32 Id = 0 ) :
        BASECLASS( X, Y, Width, Height, Id )
    {
      m_ClassName      = "CheckBox";

      m_Style  = CS_DEFAULT;
      m_ComponentFlags |= GUI::COMPFT_TAB_STOP;
      m_TextAlignment = GUI::AF_LEFT | GUI::AF_VCENTER;
      RecalcClientRect();
    }



    AbstractCheckBox( int X, int Y, int Width, int Height, const GR::String& Caption, GR::u32 Flags = CS_DEFAULT, GR::u32 Id = 0 ) :
        BASECLASS( X, Y, Width, Height, Caption, Id )
    {
      m_ClassName      = "CheckBox";

      m_Style  = Flags;
      m_ComponentFlags |= GUI::COMPFT_TAB_STOP;
      m_TextAlignment = GUI::AF_LEFT | GUI::AF_VCENTER;
      RecalcClientRect();
    }



    virtual bool IsChecked() const
    {
      if ( Style() & CS_CHECKED )
      {
        return true;
      }
      return false;
    }



    virtual bool IsPushed() const
    {
      if ( Style() & CS_PUSHED )
      {
        return true;
      }
      return false;
    }



    virtual void SetCheck( bool Check = true )
    {
      if ( IsChecked() )
      {
        if ( !Check )
        {
          ModifyStyle( 0, (GR::u32)CS_CHECKED );
        }
      }
      else
      {
        if ( Check )
        {
          ModifyStyle( (GR::u32)CS_CHECKED );
        }
      }
    }



    virtual bool ProcessEvent( const GUI::ComponentEvent& Event )
    {
      switch ( Event.Type )
      {
        case CET_MOUSE_OUT:
          if ( Style() & CS_PUSHED )
          {
            GenerateEventForParent( OET_BUTTON_UP );
            ModifyStyle( 0, CS_PUSHED );
          }
          break;
        case CET_MOUSE_DOWN:
          if ( ( !IsVisible() )
          ||   ( !IsEnabled() ) )
          {
            return true;
          }

          if ( !( Style() & CS_PUSHED ) )
          {
            GenerateEventForParent( OET_BUTTON_DOWN );
            ModifyStyle( CS_PUSHED );
          }
          break;
        case CET_MOUSE_UP:
          if ( IsPushed() )
          {
            if ( IsChecked() )
            {
              ModifyStyle( 0, CS_CHECKED );

              if ( !m_DefaultEventScript.empty() )
              {
                RaiseDefaultEvent();
              }
              else
              {
                GenerateEventForParent( OET_BUTTON_PUSHED, 0 );
              }
            }
            else
            {
              ModifyStyle( CS_CHECKED );
              if ( !m_DefaultEventScript.empty() )
              {
                RaiseDefaultEvent();
              }
              else
              {
                GenerateEventForParent( OET_BUTTON_PUSHED, 1 );
              }
            }
            ModifyStyle( 0, CS_PUSHED );
          }
          break;
      }
      return BASECLASS::ProcessEvent( Event );
    }

};


#endif // ABSTRACTCHECKBOX_H



