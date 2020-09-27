#ifndef ABSTRACTRADIOBUTTON_H
#define ABSTRACTRADIOBUTTON_H



#include <string>
#include <vector>

#include <Controls/InputEvent.h>



template <class BASECLASS> class AbstractRadioButton : public BASECLASS
{

  protected:

    int                       m_iGroupID;


  public:

    enum RadioButtonFlagType
    {
      RBS_DEFAULT       = 0x00000000l,
      RBS_PUSHED        = 0x00000001,
      RBS_GROUPED       = 0x40000000,
      RBS_CHECKED       = 0x80000000,
    };


    // usings are required for annoying standard conform ignored lookup of base class members
    // alternative would be prefixing everything with "this->" which is even worse
    using BASECLASS::m_ClassName;
    using BASECLASS::m_Style;
    using BASECLASS::m_ComponentFlags;
    using BASECLASS::m_Position;
    using BASECLASS::m_Width;
    using BASECLASS::m_Height;
    using BASECLASS::m_TextAlignment;
    using BASECLASS::m_pFont;
    using BASECLASS::Width;
    using BASECLASS::Height;
    using BASECLASS::m_ClientRect;
    using BASECLASS::RecalcClientRect;
    using BASECLASS::Style;
    using BASECLASS::IsEnabled;
    using BASECLASS::IsVisible;
    using BASECLASS::IsFocused;
    using BASECLASS::LocalToScreen;
    using BASECLASS::SetCapture;
    using BASECLASS::ReleaseCapture;
    using BASECLASS::IsMouseInside;
    using BASECLASS::GenerateEvent;
    using BASECLASS::GenerateEventForParent;
    using BASECLASS::GetComponentParent;
    using BASECLASS::ModifyStyle;
    using BASECLASS::m_DefaultEventScript;
    using BASECLASS::RaiseDefaultEvent;



    AbstractRadioButton( GR::u32 Id = 0 ) :
        BASECLASS( Id ),
        m_iGroupID( 0 )
    {
      m_ClassName      = "RadioButton";

      m_Style           = RBS_DEFAULT;
      m_ComponentFlags |= GUI::COMPFT_TAB_STOP;
    }



    AbstractRadioButton( int X, int Y, int Width, int Height, GR::u32 Id = 0 ) :
        BASECLASS( X, Y, Width, Height, Id ),
        m_iGroupID( 0 )
    {
      m_ClassName      = "RadioButton";

      m_Style           = RBS_DEFAULT;
      m_ComponentFlags |= GUI::COMPFT_TAB_STOP;
      RecalcClientRect();
    }



    AbstractRadioButton( int X, int Y, int Width, int Height, const GR::String& Caption, GR::u32 Id = 0 ) :
        BASECLASS( X, Y, Width, Height, Caption, Id ),
        m_iGroupID( 0 )
    {
      m_ClassName      = "RadioButton";

      m_Style           = RBS_DEFAULT;
      m_ComponentFlags |= GUI::COMPFT_TAB_STOP;
      RecalcClientRect();
    }



    virtual bool IsChecked() const
    {
      if ( Style() & RBS_CHECKED )
      {
        return true;
      }
      return false;
    }



    virtual bool IsPushed() const
    {
      if ( Style() & RBS_PUSHED )
      {
        return true;
      }
      return false;
    }



    virtual void SetCheck( bool bCheck = true )
    {
      if ( Style() & RBS_CHECKED )
      {
        if ( !bCheck )
        {
          ModifyStyle( 0, RBS_CHECKED );
        }
      }
      else
      {
        if ( bCheck )
        {
          ModifyStyle( RBS_CHECKED );
        }
      }
    }



    virtual bool ProcessEvent( const GUI::ComponentEvent& Event )
    {
      if ( ( !IsVisible() )
      ||   ( !IsEnabled() ) )
      {
        return true;
      }

      switch ( Event.Type )
      {
        case CET_MOUSE_OUT:
          if ( Style() & RBS_PUSHED )
          {
            GenerateEventForParent( OET_BUTTON_UP );
            ModifyStyle( 0, RBS_PUSHED );
          }
          break;
        case CET_MOUSE_UPDATE:
          if ( IsMouseInside() )
          {
            if ( Event.MouseButtons & 1 )
            {
              if ( !( Style() & RBS_PUSHED ) )
              {
                GenerateEventForParent( OET_BUTTON_DOWN );
                ModifyStyle( RBS_PUSHED );
              }
            }
            else
            {
              if ( Style() & RBS_PUSHED )
              {
                if ( Style() & RBS_CHECKED )
                {
                  /*
                  m_rbFlags &= ~RBFT_CHECKED;
                  GenerateEventForParent( OET_BUTTON_PUSHED, 0 );
                  */
                }
                else
                {
                  ModifyStyle( RBS_CHECKED );
                  // andere entchecken
                  GUI::Component*   pParent = GetComponentParent();
                  if ( pParent )
                  {
                    GUI::tListComponents::iterator    it( pParent->m_Components.begin() );
                    while ( it != pParent->m_Components.end() )
                    {
                      GUI::Component*   pComp = *it;

                      if ( ( pComp->m_ClassName == "RadioButton" )
                      &&   ( pComp != this ) )
                      {
                        ((AbstractRadioButton*)pComp)->SetCheck( false );
                      }

                      ++it;
                    }
                  }

                  if ( !m_DefaultEventScript.empty() )
                  {
                    RaiseDefaultEvent();
                  }
                  else
                  {
                    GenerateEventForParent( OET_BUTTON_PUSHED, 1 );
                  }
                }
                ModifyStyle( 0, RBS_PUSHED );
              }
            }
          }
          break;
      }
      return BASECLASS::ProcessEvent( Event );
    }

};


#endif // __ABSTRACTRADIOBUTTON_H__



