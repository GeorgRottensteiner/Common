#ifndef ABSTRACTLABEL_H
#define ABSTRACTLABEL_H



#include <string>
#include <vector>

#include <Controls/Component.h>

#include <Controls/ComponentDisplayerBase.h>



template <class BASECLASS> class AbstractLabel : public BASECLASS
{

  protected:

    GR::tRect     m_TextPos;

    GR::f32       m_ScaleFaktor;

    bool          m_ButtonReleased;

    GR::u64       m_LastClickTicks;


  public:

    enum eLabelStyles
    {
      LS_SCALE_TEXT = 0x00000001,
    };

    

    // usings are required for annoying standard conform ignored lookup of base class members
    // alternative would be prefixing everything with "this->" which is even worse
    using BASECLASS::m_ClassName;
    using BASECLASS::m_Style;
    using BASECLASS::m_ComponentFlags;
    using BASECLASS::m_TextAlignment;
    using BASECLASS::ModifyFlags;
    using BASECLASS::ModifyStyle;
    using BASECLASS::ModifyVisualStyle;
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



    AbstractLabel( GR::u32 Type = GUI::AF_DEFAULT, GR::u32 Id = 0 ) :
        BASECLASS( Id )
    {
      m_ScaleFaktor       = 1.0f;
      m_TextAlignment     = Type;
      m_ClassName         = "Label";
      m_ButtonReleased    = false;
      m_LastClickTicks    = GUI::GetTicks();
      ModifyVisualStyle( 0, GUI::VFT_SUNKEN_BORDER | GUI::VFT_RAISED_BORDER );
      ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE, GUI::COMPFT_TAB_STOP );
    }



    AbstractLabel( int X, int Y, int Width, int Height, GR::u32 Type = GUI::AF_DEFAULT, GR::u32 Id = 0 ) :
        BASECLASS( X, Y, Width, Height, Id )
    {
      m_ScaleFaktor       = 1.0f;
      m_TextAlignment     = Type;
      m_ClassName         = "Label";
      m_ButtonReleased    = false;
      m_LastClickTicks    = GUI::GetTicks();

      ModifyVisualStyle( 0, GUI::VFT_SUNKEN_BORDER | GUI::VFT_RAISED_BORDER );
      ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE, GUI::COMPFT_TAB_STOP );

      RecalcClientRect();
    }



    AbstractLabel( int X, int Y, int Width, int Height, const GR::String& Caption, GR::u32 Type = GUI::AF_DEFAULT, GR::u32 Id = 0 ) :
        BASECLASS( X, Y, Width, Height, Caption, Id )
    {
      m_ScaleFaktor       = 1.0f;
      m_TextAlignment     = Type;
      m_ClassName         = "Label";
      m_ButtonReleased    = false;
      m_LastClickTicks    = GUI::GetTicks();

      ModifyVisualStyle( 0, GUI::VFT_SUNKEN_BORDER | GUI::VFT_RAISED_BORDER );
      ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE, GUI::COMPFT_TAB_STOP );

      RecalcClientRect();
    }



    void SetTextOffsets( const GR::i32 OffsetLeft, const GR::i32 OffsetTop, const GR::i32 OffsetRight = 0, const GR::i32 OffsetBottom = 0 )
    {
      m_TextPos.set( OffsetLeft, OffsetTop, OffsetRight, OffsetBottom );
    }



    void SetScaleFactor( const GR::f32 ScaleFaktor )
    {
      m_ScaleFaktor = ScaleFaktor;
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
        case CET_MOUSE_UPDATE:
          {
            if ( !( Event.MouseButtons & 1 ) )
            {
              m_ButtonReleased = true;
            }
            
            if ( ( Event.MouseButtons & 1 )
            &&   ( m_ButtonReleased ) )
            {
              m_ButtonReleased = false;

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP 
              bool    sendDblClk = false;
              if ( GUI::GetTicks() - m_LastClickTicks <= GUI::GetDoubleClickTimeMS() )
              {
                sendDblClk = true;
              }
              m_LastClickTicks = GUI::GetTicks();
              if ( sendDblClk )
              {
                GenerateEventForParent( OET_LABEL_DBLCLK );
              }
#endif
              return true;
            }
          }
          break;
      }
      return BASECLASS::ProcessEvent( Event );
    }

};


#endif // __ABSTRACTLABEL_H__



