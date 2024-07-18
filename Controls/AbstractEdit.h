#ifndef ABSTRACTEDIT_H
#define ABSTRACTEDIT_H



#include <string>
#include <vector>

#include <Xtreme/XInput.h>

#include <System\Clipboard.h>

#include <Controls/InputEvent.h>
#include <Controls/ComponentDisplayerBase.h>

#include <String/StringUtil.h>

#include <Interface\IFont.h>

#include "AbstractScrollBar.h"



class CInputEvent;

template <class BASECLASS, class SBCLASS, class SLCLASS, class BTCLASS> class AbstractEdit : public BASECLASS
{

  public:

    enum EditFlagType
    {
      ECS_STRING        = 0,
      ECS_REAL_NUMBERS  = 0x00000001,
      ECS_INT_NUMBERS   = 0x00000002,
      ECS_UPPERCASE     = 0x00000004,     // wandelt alles in Uppercase
      ECS_LOWERCASE     = 0x00000008,     // wandelt alles in Lowercase
      ECS_ONLY_POSITIVE = 0x00000010,     // Zahlen nur positiv
      ECS_WANT_RETURN   = 0x00000100,     // gibt Return/Enter als speziellen Code weiter
      ECS_MULTILINE     = 0x00000200,     // mehrzeilige Eingabe
      ECS_READ_ONLY     = 0x00000400,
      ECS_WORDBREAK     = 0x00000800,     // Auto-Wordbreak
      ECS_AUTO_H_SCROLL = 0x00001000,

      ECS_DEFAULT       = ECS_STRING | ECS_AUTO_H_SCROLL,
    };


  protected:

    bool                          m_Capturing;

    GR::String                    m_AllowedChars;

    int                           m_CursorPos,
                                  m_SelectionAnchor,
                                  m_SelectionAnchorLine,
                                  m_MaxCharacters;

    size_t                        m_TextOffset,
                                  m_TextOffsetLine,
                                  m_CursorPosInText,
                                  m_CursorLine,
                                  m_VisibleItems;

    float                         m_ShowCursorDelay;

    SBCLASS*                      m_pScrollBarH;
    SBCLASS*                      m_pScrollBarV;

    std::vector<GR::String>       m_Text;

  public:


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
    using BASECLASS::Add;
    using BASECLASS::Delete;



    AbstractEdit( GR::u32 Type = ECS_DEFAULT, GR::u32 Id = 0 ) :
      BASECLASS( Id )
    {
      m_ClassName         = "Edit";

      m_ShowCursorDelay  = 0.0f;

      m_Style             = Type;
      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;
      m_TextOffset        = 0;
      m_TextOffsetLine    = 0;
      m_CursorPosInText   = 0;
      m_CursorPos         = 0;
      m_SelectionAnchor   = -1;
      m_SelectionAnchorLine = -1;
      m_CursorLine        = 0;
      m_MaxCharacters     = -1;
      m_VisibleItems      = 1;
      m_Capturing         = false;
      m_pScrollBarH       = NULL;
      m_pScrollBarV       = NULL;
      m_TextAlignment     = GUI::AF_LEFT | GUI::AF_VCENTER;

      m_Text.push_back( GR::String() );

      if ( m_Style & ECS_MULTILINE )
      {
        m_pScrollBarH = new SBCLASS( 0, 0, 0, 0, SBCLASS::SBFT_HORIZONTAL );
        m_pScrollBarH->GetComponent( SBCLASS::SB_BUTTON_LEFT_UP )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
        m_pScrollBarH->GetComponent( SBCLASS::SB_SLIDER )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
        m_pScrollBarH->GetComponent( SBCLASS::SB_BUTTON_RIGHT_DOWN )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
        m_pScrollBarH->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
        Add( m_pScrollBarH );
        m_pScrollBarH->AddListener( this );

        m_pScrollBarV = new SBCLASS( 0, 0, 0, 0, SBCLASS::SBFT_VERTICAL );
        m_pScrollBarV->GetComponent( SBCLASS::SB_BUTTON_LEFT_UP )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
        m_pScrollBarV->GetComponent( SBCLASS::SB_SLIDER )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
        m_pScrollBarV->GetComponent( SBCLASS::SB_BUTTON_RIGHT_DOWN )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
        m_pScrollBarV->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );

        Add( m_pScrollBarV );
        m_pScrollBarV->AddListener( this );
      }
      ModifyStyle( 0, 0 );
      RecalcClientRect();
      BASECLASS::SetBaseColors();
    }



    AbstractEdit( int X, int Y, int Width, int Height, GR::u32 Type = ECS_DEFAULT, GR::u32 Id = 0 ) :
        BASECLASS( X, Y, Width, Height, Id )
    {
      m_ClassName         = "Edit";

      m_ShowCursorDelay   = 0.0f;

      m_Style             = Type;

      m_ComponentFlags    |= GUI::COMPFT_TAB_STOP;
      
      m_TextOffset        = 0;
      m_TextOffsetLine    = 0;
      m_CursorPosInText   = 0;
      m_CursorPos         = 0;
      m_SelectionAnchor   = -1;
      m_SelectionAnchorLine = -1;
      m_CursorLine        = 0;
      m_MaxCharacters     = -1;
      m_VisibleItems      = 1;
      m_Capturing         = false;
      m_pScrollBarH       = NULL;
      m_pScrollBarV       = NULL;
      m_TextAlignment     = GUI::AF_LEFT | GUI::AF_VCENTER;

      m_Text.push_back( GR::String() );

      if ( m_Style & ECS_MULTILINE )
      {
        RecalcClientRect();
        m_pScrollBarV = new SBCLASS( m_ClientRect.Width() - 20, 0, 20, m_ClientRect.Height() - 20, AbstractScrollbar<BASECLASS, BTCLASS, SLCLASS>::SBFT_VERTICAL, 2000000 );
        m_pScrollBarV->GetComponent( AbstractScrollbar<BASECLASS, BTCLASS, SLCLASS>::SB_BUTTON_LEFT_UP )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
        m_pScrollBarV->GetComponent( AbstractScrollbar<BASECLASS, BTCLASS, SLCLASS>::SB_SLIDER )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
        m_pScrollBarV->GetComponent( AbstractScrollbar<BASECLASS, BTCLASS, SLCLASS>::SB_BUTTON_RIGHT_DOWN )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
        m_pScrollBarV->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );

        m_pScrollBarH = new SBCLASS( 0, m_ClientRect.Bottom - 20, m_ClientRect.Width(), 20, AbstractScrollbar<BASECLASS, BTCLASS, SLCLASS>::SBFT_HORIZONTAL, 2000001 );
        m_pScrollBarH->GetComponent( AbstractScrollbar<BASECLASS, BTCLASS, SLCLASS>::SB_BUTTON_LEFT_UP )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
        m_pScrollBarH->GetComponent( AbstractScrollbar<BASECLASS, BTCLASS, SLCLASS>::SB_SLIDER )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
        m_pScrollBarH->GetComponent( AbstractScrollbar<BASECLASS, BTCLASS, SLCLASS>::SB_BUTTON_RIGHT_DOWN )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
        m_pScrollBarH->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );

        Add( m_pScrollBarH );
        m_pScrollBarH->AddListener( this );
        Add( m_pScrollBarV );
        m_pScrollBarV->AddListener( this );
      }
      ModifyStyle( 0, 0 );
      RecalcClientRect();
      BASECLASS::SetBaseColors();
    }



    virtual GR::String GetCaption() const
    {
      GR::String     result;

      for ( size_t i = 0; i < m_Text.size(); ++i )
      {
        result += m_Text[i];
        if ( i + 1 < m_Text.size() )
        {
          result += "\r\n";
        }
      }
      return result;
    }



    void SetAllowedChars( const GR::String& AllowedChars = "" )
    {
      m_AllowedChars = AllowedChars;
    }



    virtual void SetCaption( const GR::Char* szCaption )
    {
      BASECLASS::SetCaption( szCaption );

      m_Text.clear();
      m_Text.push_back( GR::String() );

      m_TextOffset       = 0;
      m_TextOffsetLine   = 0;
      m_CursorPosInText  = 0;
      m_CursorPos        = 0;
      m_SelectionAnchor      = -1;
      m_SelectionAnchorLine  = -1;
      m_CursorLine           = 0;

      InsertText( GR::tPoint(), szCaption );

      if ( !( Style() & ECS_AUTO_H_SCROLL ) )
      {
        m_TextOffset = 0;
        m_CursorPosInText  = 0;
        m_CursorPos        = 0;
      }

      //UpdateCursorPos();
      UpdateScrollBars();
    }



    virtual void SetCaption( const GR::String& Caption )
    {
      BASECLASS::SetCaption( Caption );

      m_Text.clear();
      m_Text.push_back( GR::String() );

      m_TextOffset       = 0;
      m_TextOffsetLine   = 0;
      m_CursorPosInText  = 0;
      m_CursorPos        = 0;
      m_SelectionAnchor      = -1;
      m_SelectionAnchorLine  = -1;
      m_CursorLine           = 0;

      InsertText( GR::tPoint(), Caption );

      //UpdateCursorPos();
      UpdateScrollBars();
    }



    virtual void SetMaxCharacters( int iMaxChars )
    {
      m_MaxCharacters = iMaxChars;
    }



    size_t MaxChars() const
    {
      return m_MaxCharacters;
    }



    virtual bool ProcessEvent( const GUI::OutputEvent& Event )
    {
      switch ( Event.Type )
      {
        case OET_SCROLLBAR_SCROLLED:
          if ( Event.pComponent == m_pScrollBarV )
          {
            m_TextOffsetLine = Event.Param1;
          }
          else if ( Event.pComponent == m_pScrollBarH )
          {
            m_TextOffset = Event.Param1;
            UpdateCursorPos( true );
          }
          break;
      }
      return false;
    }



    virtual bool ProcessEvent( const GUI::ComponentEvent& Event )
    {
      if ( !IsVisible() )
      {
        return true;
      }

      switch ( Event.Type )
      {
        case CET_MOUSE_WHEEL:
          if ( m_pScrollBarV )
          {
            return m_pScrollBarV->ProcessEvent( Event );
          }
          break;
        case CET_SET_SIZE:
          BASECLASS::ProcessEvent( Event );
          if ( ( m_pScrollBarV )
          &&   ( m_pScrollBarH ) )
          {
            m_pScrollBarV->SetLocation( m_ClientRect.Width() - m_pScrollBarV->Width(), 0 );
            m_pScrollBarV->SetSize( m_pScrollBarV->Width(), m_ClientRect.Height() - m_pScrollBarH->Height() );

            m_pScrollBarH->SetLocation( 0, m_ClientRect.Height() - m_pScrollBarH->Height() );
            m_pScrollBarH->SetSize( m_ClientRect.Width() - m_pScrollBarV->Width(), m_pScrollBarH->Height() );
            UpdateScrollBars();
            BASECLASS::Invalidate();
          }
          return true;
        case CET_MOUSE_DOWN:
          if ( IsEnabled() )
          {
            m_SelectionAnchor = -1;
            m_SelectionAnchorLine = -1;
            SetCapture();
            m_Capturing = true;
            BASECLASS::Invalidate();
          }
          break;
        case CET_MOUSE_UP:
          if ( IsEnabled() )
          {
            ReleaseCapture();
            m_Capturing = false;
            if ( !HasSelection() )
            {
              m_SelectionAnchor = -1;
              m_SelectionAnchorLine = -1;
              BASECLASS::Invalidate();
            }
          }
          break;
        case CET_MOUSE_UPDATE:
          if ( IsEnabled() )
          {
            if ( Event.MouseButtons & 1 )
            {
              GR::tPoint    ptTextOffset = GUI::TextOffset( m_pFont, "", m_TextAlignment, m_ClientRect );
              int   iPos = Event.Position.x - ptTextOffset.x;
              int   iMouseLine = 0;
              if ( m_pFont )
              {
                int   TextHeight = m_pFont->TextHeight();
                if ( TextHeight )
                {
                  iMouseLine = (int)( ( Event.Position.y - ptTextOffset.y ) / m_pFont->TextHeight() + m_TextOffsetLine );
                }
              }
              if ( iMouseLine < 0 )
              {
                iMouseLine = 0;
                m_CursorLine = 0;
              }
              if ( iPos <= 0 )
              {
                m_CursorPosInText = 0;
                BASECLASS::Invalidate();
              }
              else if ( iMouseLine < (int)m_Text.size() )
              {
                m_CursorLine = iMouseLine;
                GR::tPoint    ptTextOffset = GUI::TextOffset( m_pFont, m_Text[iMouseLine], m_TextAlignment, m_ClientRect );
                int   iLength = ptTextOffset.x;
                if ( m_pFont )
                {
                  size_t i = 0;
                  for ( i = m_TextOffset; i < m_Text[iMouseLine].length(); i++ )
                  {
                    int   iTextLength = m_pFont->TextLength( m_Text[iMouseLine].substr( i, 1 ).c_str() );
                    if ( ( iPos >= iLength )
                    &&   ( iPos < iLength + iTextLength ) )
                    {
                      m_CursorPosInText = i;
                      if ( iPos - iLength > iTextLength / 2 )
                      {
                        m_CursorPosInText++;
                        BASECLASS::Invalidate();
                      }
                      break;
                    }
                    iLength += iTextLength + m_pFont->FontSpacing();
                  }
                  if ( ( i == m_Text[iMouseLine].length() )
                  &&   ( iPos > iLength ) )
                  {
                    m_CursorPosInText = m_Text[iMouseLine].length();
                    BASECLASS::Invalidate();
                  }
                }
              }
              if ( ( m_Capturing )
              &&   ( iMouseLine < (int)m_Text.size() ) )
              {
                if ( ( !m_Text[iMouseLine].empty() )
                &&   ( m_SelectionAnchor == -1 )
                &&   ( m_SelectionAnchorLine == -1 ) )
                {
                  m_SelectionAnchor      = (int)m_CursorPosInText;
                  m_SelectionAnchorLine  = iMouseLine;
                  BASECLASS::Invalidate();
                }
              }
              UpdateCursorPos();
            }
          }
          break;
        case CET_KEY_DOWN:
          if ( !IsEnabled() )
          {
            break;
          }
          if ( Style() & ECS_READ_ONLY )
          {
            break;
          }
          if ( ( Event.Character == Xtreme::KEY_DELETE )
          ||   ( Event.Character == Xtreme::KEY_NUMPAD_KOMMA ) )
          {
            if ( HasSelection() )
            {
              // Selection löschen
              ReplaceSelection( "" );
              m_SelectionAnchor = -1;
              m_SelectionAnchorLine = -1;
              UpdateScrollBars();
              BASECLASS::Invalidate();
              GenerateEventForParent( OET_EDIT_CHANGE );
              return true;
            }
            else if ( m_CursorPosInText < m_Text[m_CursorLine].length() )
            {
              GR::String   strNew = m_Text[m_CursorLine].substr( 0, m_CursorPosInText );

              if ( m_CursorPosInText < m_Text[m_CursorLine].length() - 1 )
              {
                strNew += m_Text[m_CursorLine].substr( m_CursorPosInText + 1 );
              }
              m_Text[m_CursorLine] = strNew;
              UpdateScrollBars();
              BASECLASS::Invalidate();
              GenerateEventForParent( OET_EDIT_CHANGE );
              return true;
            }
            else
            {
              // Cursor ist am Ende einer Zeile
              if ( m_CursorLine + 1 < m_Text.size() )
              {
                m_Text[m_CursorLine] += m_Text[m_CursorLine + 1];
                m_Text.erase( m_Text.begin() + m_CursorLine + 1 );
                UpdateScrollBars();
                BASECLASS::Invalidate();
                GenerateEventForParent( OET_EDIT_CHANGE );
              }
              return true;
            }
          }
          else if ( ( Event.Character == Xtreme::KEY_HOME )
          ||        ( Event.Character == Xtreme::KEY_NUMPAD_7 ) )
          {
            if ( Event.Value & Xtreme::tInputEvent::KF_CTRL_PUSHED )
            {
              // ganz nach oben
              if ( m_CursorLine > 0 )
              {
                if ( UpdateSelectionOnInput( !!( Event.Value & Xtreme::tInputEvent::KF_SHIFT_PUSHED ) ) )
                {
                  return true;
                }
                m_CursorLine = 0;
                m_TextOffsetLine = 0;
                m_CursorPos = 0;
                m_TextOffset = 0;
                m_CursorPosInText = 0;
                UpdateCursorPos();
                UpdateScrollBars();
                BASECLASS::Invalidate();
              }
            }
            else if ( m_CursorPosInText )
            {
              GR::tPoint    ptSelStart;
              GR::tPoint    ptSelEnd;

              GetCurSel( ptSelStart, ptSelEnd );

              if ( UpdateSelectionOnInput( !!( Event.Value & Xtreme::tInputEvent::KF_SHIFT_PUSHED ) ) )
              {
                m_CursorPosInText = ptSelStart.x;
                UpdateCursorPos();
                BASECLASS::Invalidate();
                return true;
              }
              m_CursorPosInText = 0;
              m_TextOffset = 0;
              UpdateCursorPos();
              UpdateScrollBars();
              BASECLASS::Invalidate();
            }
            return true;
          }
          else if ( ( Event.Character == Xtreme::KEY_END )
          ||        ( Event.Character == Xtreme::KEY_NUMPAD_1 ) )
          {
            if ( Event.Value & Xtreme::tInputEvent::KF_CTRL_PUSHED )
            {
              // ganz nach unten
              if ( m_CursorLine + 1 < m_Text.size() )
              {
                if ( UpdateSelectionOnInput( !!( Event.Value & Xtreme::tInputEvent::KF_SHIFT_PUSHED ) ) )
                {
                  return true;
                }
                m_CursorLine = m_Text.size() - 1;
                m_CursorPosInText = m_Text[m_CursorLine].length();
                UpdateCursorPos();
                UpdateScrollBars();
                EnsureCursorVisible();
                BASECLASS::Invalidate();
              }
            }
            else if ( m_CursorPosInText < (int)m_Text[m_CursorLine].length() )
            {
              if ( UpdateSelectionOnInput( !!( Event.Value & Xtreme::tInputEvent::KF_SHIFT_PUSHED ) ) )
              {
                return true;
              }
              m_CursorPosInText = (int)m_Text[m_CursorLine].length();
              UpdateCursorPos();
              UpdateScrollBars();
              BASECLASS::Invalidate();
            }
            return true;
          }
          else if ( ( Event.Character == Xtreme::KEY_UP )
          ||        ( Event.Character == Xtreme::KEY_NUMPAD_8 ) )
          {
            if ( Style() & ECS_MULTILINE )
            {
              if ( m_CursorLine > 0 )
              {
                if ( UpdateSelectionOnInput( !!( Event.Value & Xtreme::tInputEvent::KF_SHIFT_PUSHED ) ) )
                {
                  return true;
                }
                m_CursorLine--;
                if ( m_CursorPosInText >= m_Text[m_CursorLine].length() )
                {
                  m_CursorPosInText = m_Text[m_CursorLine].length();
                }
                UpdateCursorPos();
                UpdateScrollBars();
                EnsureCursorVisible();
              }
              else
              {
                m_SelectionAnchor = -1;
                m_SelectionAnchorLine = -1;
              }
              BASECLASS::Invalidate();
            }
          }
          else if ( ( Event.Character == Xtreme::KEY_DOWN )
          ||        ( Event.Character == Xtreme::KEY_NUMPAD_2 ) )
          {
            if ( Style() & ECS_MULTILINE )
            {
              if ( m_CursorLine + 1 < m_Text.size() )
              {
                if ( UpdateSelectionOnInput( !!( Event.Value & Xtreme::tInputEvent::KF_SHIFT_PUSHED ) ) )
                {
                  return true;
                }
                m_CursorLine++;
                if ( m_CursorPosInText >= m_Text[m_CursorLine].length() )
                {
                  m_CursorPosInText = m_Text[m_CursorLine].length();
                }
                UpdateCursorPos();
                UpdateScrollBars();
                EnsureCursorVisible();
              }
              else
              {
                m_SelectionAnchor = -1;
                m_SelectionAnchorLine = -1;
              }
              BASECLASS::Invalidate();
            }
          }
          else if ( ( Event.Character == Xtreme::KEY_LEFT )
          ||        ( Event.Character == Xtreme::KEY_NUMPAD_4 ) )
          {
            if ( m_CursorPosInText )
            {
              if ( UpdateSelectionOnInput( !!( Event.Value & Xtreme::tInputEvent::KF_SHIFT_PUSHED ) ) )
              {
                return true;
              }

              m_CursorPosInText--;
              UpdateCursorPos();
              UpdateScrollBars();
            }
            else
            {
              m_SelectionAnchor = -1;
              m_SelectionAnchorLine = -1;
            }
            BASECLASS::Invalidate();
            return true;
          }
          else if ( ( Event.Character == Xtreme::KEY_RIGHT )
          ||        ( Event.Character == Xtreme::KEY_NUMPAD_6 ) )
          {
            if ( m_CursorPosInText < (int)m_Text[m_CursorLine].length() )
            {
              if ( UpdateSelectionOnInput( !!( Event.Value & Xtreme::tInputEvent::KF_SHIFT_PUSHED ) ) )
              {
                return true;
              }
              m_CursorPosInText++;
              UpdateCursorPos();
              UpdateScrollBars();
            }
            else
            {
              m_SelectionAnchor = -1;
              m_SelectionAnchorLine = -1;
            }
            BASECLASS::Invalidate();
            return true;
          }
          else if ( Event.Character == Xtreme::KEY_C )
          {
            if ( Event.Value & Xtreme::tInputEvent::KF_CTRL_PUSHED )
            {
              GR::String selection = GetSelection();
              if ( !selection.empty() )
              {
                System::Clipboard::TextToClipboard( selection );
              }
              return true;
            }
          }
          else if ( Event.Character == Xtreme::KEY_X )
          {
            if ( Event.Value & Xtreme::tInputEvent::KF_CTRL_PUSHED )
            {
              GR::String selection = GetSelection();
              if ( !selection.empty() )
              {
                System::Clipboard::TextToClipboard( selection );
                ReplaceSelection( "" );
                BASECLASS::Invalidate();
                GenerateEventForParent( OET_EDIT_CHANGE );
              }
              return true;
            }
          }
          else if ( Event.Character == Xtreme::KEY_V )
          {
            if ( Event.Value & Xtreme::tInputEvent::KF_CTRL_PUSHED )
            {
              GR::String   clipText;

              if ( System::Clipboard::TextFromClipboard( clipText ) )
              {
                if ( !m_AllowedChars.empty() )
                {
                  // remove unwanted chars
                  clipText = GR::Strings::Remove( clipText, m_AllowedChars );
                }

                if ( HasSelection() )
                {
                  ReplaceSelection( clipText );
                }
                else
                {
                  InsertText( GR::tPoint( (int)m_CursorPosInText, (int)m_CursorLine ), clipText );
                }
                BASECLASS::Invalidate();
                GenerateEventForParent( OET_EDIT_CHANGE );
              }
              return true;
            }
          }
          else if ( Event.Character == Xtreme::KEY_A )
          {
            if ( Event.Value & Xtreme::tInputEvent::KF_CTRL_PUSHED )
            {
              // select all!
              SelectAll();
              return true;
            }
          }
          if ( Event.Character == Xtreme::KEY_BACKSPACE )
          {
            if ( HasSelection() )
            {
              ReplaceSelection( "" );
              m_SelectionAnchor = -1;
              m_SelectionAnchorLine = -1;
              BASECLASS::Invalidate();
              GenerateEventForParent( OET_EDIT_CHANGE );
            }
            else if ( m_CursorPosInText )
            {
              GR::String   strNew = m_Text[m_CursorLine].substr( 0, m_CursorPosInText - 1 );

              if ( m_CursorPosInText < m_Text[m_CursorLine].length() )
              {
                strNew += m_Text[m_CursorLine].substr( m_CursorPosInText );
              }
              m_Text[m_CursorLine] = strNew;
              m_CursorPosInText--;
              UpdateCursorPos();
              UpdateScrollBars();
              BASECLASS::Invalidate();
              GenerateEventForParent( OET_EDIT_CHANGE );
            }
            return true;
          }
          else if ( ( Event.Character == Xtreme::KEY_ENTER )
          ||        ( Event.Character == Xtreme::KEY_NUMPAD_ENTER ) )
          {
            bool    bChanged = false;
            if ( HasSelection() )
            {
              ReplaceSelection( "" );
              m_SelectionAnchor = -1;
              m_SelectionAnchorLine = -1;
              bChanged = true;
            }
            if ( Style() & ECS_WANT_RETURN )
            {
              GenerateEventForParent( OET_EDIT_ENTER_PRESSED );
            }
            else if ( Style() & ECS_MULTILINE )
            {
              if ( m_CursorPosInText < m_Text[m_CursorLine].length() )
              {
                // Zeile umbrechen
                m_Text.insert( m_Text.begin() + m_CursorLine + 1, m_Text[m_CursorLine].substr( m_CursorPosInText ) );
                m_Text[m_CursorLine] = m_Text[m_CursorLine].substr( 0, m_CursorPosInText );
              }
              else
              {
                // neue Zeile einsetzen
                m_Text.insert( m_Text.begin() + m_CursorLine + 1, GR::String() );
              }
              m_CursorLine++;
              m_CursorPosInText = 0;
              UpdateCursorPos();
              EnsureCursorVisible();
              bChanged = true;
            }
            if ( bChanged )
            {
              UpdateScrollBars();
              BASECLASS::Invalidate();
              GenerateEventForParent( OET_EDIT_CHANGE );
            }
            return true;
          }
          break;
        case CET_KEY_PRESSED:
          if ( Style() & ECS_READ_ONLY )
          {
            break;
          }
          if ( ( !m_AllowedChars.empty() )
          &&   ( m_AllowedChars.find( (char)Event.Character ) == GR::String::npos ) )
          {
            // diese Taste ist nicht erlaubt
            break;
          }

          if ( Event.Character >= 32 )
          {
            bool    bAllowToAdd = true;

            if ( Style() & ECS_INT_NUMBERS )
            {
              if ( ( Event.Character < '0' )
              ||   ( Event.Character > '9' ) )
              {
                bAllowToAdd = false;
              }
              if ( !( Style() & ECS_ONLY_POSITIVE ) )
              {
                if ( ( Event.Character == '-' )
                &&   ( m_CursorPosInText == 0 ) )
                {
                  bAllowToAdd = true;
                }
              }
            }
            if ( Style() & ECS_REAL_NUMBERS )
            {
              if ( ( Event.Character < '0' )
              ||   ( Event.Character > '9' ) )
              {
                bAllowToAdd = false;
              }
              if ( ( Event.Character == '.' )
              &&   ( m_Text[m_CursorLine].find( '.' ) == GR::String::npos ) )
              {
                // nur ein Komma erlaubt
                bAllowToAdd = true;
              }
              if ( !( Style() & ECS_ONLY_POSITIVE ) )
              {
                if ( ( Event.Character == '-' )
                &&   ( m_CursorPosInText == 0 ) )
                {
                  bAllowToAdd = true;
                }
              }
            }

            if ( ( m_MaxCharacters != -1 )
            &&   ( (int)m_Text[m_CursorLine].length() >= m_MaxCharacters )
            &&   ( !HasSelection() ) )
            {
              bAllowToAdd = false;
            }

            if ( bAllowToAdd )
            {
              char    cDummy = (char)Event.Character;
              if ( Style() & ECS_LOWERCASE )
              {
                cDummy = (char)tolower( cDummy );
              }
              if ( Style() & ECS_UPPERCASE )
              {
                cDummy = (char)toupper( cDummy );
              }
              if ( HasSelection() )
              {
                GR::String    strDummy;
                strDummy += cDummy;

                ReplaceSelection( strDummy );
              }
              else
              {
                GR::String   updatedText = m_Text[m_CursorLine].substr( 0, m_CursorPosInText );

                updatedText += cDummy;

                if ( m_CursorPosInText < m_Text[m_CursorLine].length() )
                {
                  updatedText += m_Text[m_CursorLine].substr( m_CursorPosInText );
                }
                m_Text[m_CursorLine] = updatedText;
                m_CursorPosInText++;
              }
              UpdateCursorPos();
              UpdateScrollBars();
              BASECLASS::Invalidate();

              GenerateEventForParent( OET_EDIT_CHANGE );
            }
            return true;
          }
          break;
      }

      return BASECLASS::ProcessEvent( Event );
    }



    virtual bool GetCursorRect( GR::tRect& rectSel )
    {

      // TODO - isfocused
      if ( ( !IsEnabled() )
      ||   ( !IsFocused() ) )
      {
        return false;
      }

      if ( m_ShowCursorDelay > 0.5f )
      {
        rectSel.Clear();
        return false;
      }

      int     iTextHeight = Height();

      if ( m_pFont )
      {
        iTextHeight = m_pFont->TextHeight();
      }

      if ( Style() & ECS_MULTILINE )
      {
        rectSel.Set( m_CursorPos, (int)( m_CursorLine - m_TextOffsetLine ) * iTextHeight, 2, iTextHeight );
      }
      else
      {
        rectSel.Set( m_CursorPos, ( m_ClientRect.Height() - iTextHeight ) / 2, 2, iTextHeight );
      }

      return true;
    }



    virtual void UpdateCursorPos( bool bNoScroll = false )
    {
      if ( m_pFont == NULL )
      {
        return;
      }

      if ( !( Style() & ECS_AUTO_H_SCROLL ) )
      {
        bNoScroll = true;
      }

      int   iX = 0;

      if ( !bNoScroll )
      {
        while ( m_CursorPosInText < m_TextOffset )
        {
          m_TextOffset -= 3;
          if ( m_TextOffset >= 1000000 )
          {
            m_TextOffset = 0;
          }
        }
      }

      // muss die Edit nach links scrollen?
      m_CursorPos = 0;
      if ( m_TextOffset < m_Text[m_CursorLine].length() )
      {
        int     iWidthBeforeScrollToRight = m_Width - 2 * m_pFont->TextLength( "W" );

        do
        {
          m_CursorPos = 0;
          iX = 0;
          for ( size_t i = m_TextOffset; i < m_Text[m_CursorLine].length(); i++ )
          {
            int   iLetterLength = m_pFont->TextLength( m_Text[m_CursorLine].substr( i, 1 ).c_str() );

            iX += iLetterLength + m_pFont->FontSpacing();

            if ( m_CursorPosInText == i + 1 )
            {
              m_CursorPos = iX;
              break;
            }
          }
          if ( !bNoScroll )
          {
            if ( m_CursorPos >= iWidthBeforeScrollToRight )
            {
              m_TextOffset += 3;
            }
          }
          else
          {
            break;
          }
        }
        while ( m_CursorPos >= iWidthBeforeScrollToRight );
      }

      m_CursorPos += GUI::TextOffset( m_pFont, m_Text[m_CursorLine], m_TextAlignment, m_ClientRect ).x;
    }



    virtual GR::String GetSelection()
    {
      if ( !HasSelection() )
      {
        return GR::String();
      }

      GR::tPoint    ptSelStart;
      GR::tPoint    ptSelEnd;

      if ( !GetCurSel( ptSelStart, ptSelEnd ) )
      {
        return GR::String();
      }

      GR::String     strResult;

      int     iY = ptSelStart.y;

      while ( iY <= ptSelEnd.y )
      {
        if ( ( iY == ptSelEnd.y )
        &&   ( iY == ptSelStart.y ) )
        {
          strResult = m_Text[iY].substr( ptSelStart.x, ptSelEnd.x - ptSelStart.x );
          break;
        }
        if ( iY == ptSelStart.y )
        {
          strResult = m_Text[iY].substr( ptSelStart.x ) + "\n";
        }
        else if ( iY == ptSelEnd.y )
        {
          strResult += m_Text[iY].substr( 0, ptSelEnd.x );
          break;
        }
        else
        {
          strResult += m_Text[iY] + "\n";
        }
        ++iY;
      }
      return strResult;
    }

    
    
    virtual void SelectAll()
    {
      bool  modifiedSelection = false;
      if ( m_SelectionAnchorLine != 0 )
      {
        m_SelectionAnchorLine = 0;
        modifiedSelection     = true;
      }
      if ( m_SelectionAnchor != 0 )
      {
        m_SelectionAnchor = 0;
        modifiedSelection = true;
      }
      if ( m_Text.empty() )
      {
        if ( m_CursorLine != 0 )
        {
          m_CursorLine = 0;
          modifiedSelection = true;
        }
        if ( m_CursorPosInText != 0 )
        {
          m_CursorPosInText = 0;
          modifiedSelection = true;
        }
      }
      else
      {
        if ( m_CursorLine != (int)m_Text.size() - 1 )
        {
          m_CursorLine = (int)( m_Text.size() - 1 );
          modifiedSelection = true;
        }
        if ( m_CursorPosInText != (int)m_Text.back().length() )
        {
          m_CursorPosInText = (int)m_Text.back().length();
          if ( m_pFont != NULL )
          {
            m_CursorPos = m_pFont->TextLength( m_Text.back() );
          }
          else
          {
            m_CursorPos = 0;
          }
          modifiedSelection = true;
        }
      }
      if ( modifiedSelection )
      {
        BASECLASS::Invalidate();
      }
    }



    virtual bool HasSelection() const
    {
      if ( ( m_SelectionAnchor == -1 )
      ||   ( m_SelectionAnchorLine == -1 ) )
      {
        return false;
      }
      if ( ( (size_t)m_SelectionAnchor == m_CursorPosInText )
      &&   ( (size_t)m_SelectionAnchorLine == m_CursorLine ) )
      {
        return false;
      }
      return true;
    }



    virtual void SetSelection( const GR::tPoint& ptStart, const GR::tPoint& ptEnd )
    {
      if ( ( ptStart.x == -1 )
      &&   ( ptStart.y == -1 )
      &&   ( ptEnd.x == -1 )
      &&   ( ptEnd.y == -1 ) )
      {
        // alles selektieren
        m_SelectionAnchor = 0;
        m_SelectionAnchorLine = 0;
        m_CursorPosInText = m_Text[m_Text.size() - 1].length();
        m_CursorLine = m_Text.size() - 1;
        UpdateCursorPos();
        return;
      }
      if ( ( ptStart.y < 0 )
      ||   ( ptStart.y >= (int)m_Text.size() ) )
      {
        return;
      }
      if ( ( ptEnd.y < 0 )
      ||   ( ptEnd.y >= (int)m_Text.size() ) )
      {
        return;
      }
      if ( ( ptStart.x < 0 )
      ||   ( ptStart.x >= (int)m_Text[ptStart.y].length() ) )
      {
        return;
      }
      if ( ( ptEnd.x < 0 )
      ||   ( ptEnd.x >= (int)m_Text[ptEnd.y].length() ) )
      {
        return;
      }
      m_SelectionAnchor      = ptStart.x;
      m_SelectionAnchorLine  = ptStart.y;

      m_CursorPosInText      = ptEnd.x;
      m_CursorLine           = ptEnd.y;

      UpdateCursorPos();
    }



    void ReplaceSelection( const GR::String& strReplacement )
    {
      if ( !HasSelection() )
      {
        return;
      }

      // zuerst Selection entfernen
      GR::tPoint    ptSelStart;
      GR::tPoint    ptSelEnd;

      if ( !GetCurSel( ptSelStart, ptSelEnd ) )
      {
        return;
      }

      if ( ptSelStart.y == ptSelEnd.y )
      {
        // alles in einer Zeile
        m_Text[m_CursorLine] = m_Text[m_CursorLine].substr( 0, ptSelStart.x ) 
                                  + m_Text[m_CursorLine].substr( ptSelEnd.x );
      }
      else
      {
        int   iCurLine = ptSelStart.y;

        m_Text[iCurLine] = m_Text[iCurLine].substr( 0, ptSelStart.x );
        ++iCurLine;

        while ( iCurLine <= ptSelEnd.y )
        {
          if ( iCurLine < ptSelEnd.y )
          {
            m_Text.erase( m_Text.begin() + iCurLine );
          }
          else
          {
            m_Text[iCurLine] = m_Text[iCurLine].substr( ptSelEnd.x );
            break;
          }

          ptSelEnd.y--;
        }
        //m_Text[ptSelStart.y];
      }

      m_SelectionAnchor      = -1;
      m_SelectionAnchorLine  = -1;

      m_CursorLine       = ptSelStart.y;
      m_CursorPosInText  = ptSelStart.x;

      InsertText( ptSelStart, strReplacement );

      if ( m_CursorPosInText > (int)m_Text[m_CursorLine].length() )
      {
        m_CursorPosInText = m_Text[m_CursorLine].length();
      }

      UpdateCursorPos();
      UpdateScrollBars();
    }

    virtual void Update( float fElapsedTime )
    {
      m_ShowCursorDelay += fElapsedTime;
      if ( m_ShowCursorDelay >= 1.0f )
      {
        m_ShowCursorDelay -= 1.0f;
      }
    }

    bool UpdateSelectionOnInput( bool bShiftPressed )
    {
      if ( bShiftPressed )
      {
        if ( ( !HasSelection() )
        &&   ( !m_Text[m_CursorLine].empty() ) )
        {
          if ( m_SelectionAnchor != (int)m_CursorPosInText )
          {
            m_SelectionAnchor = (int)m_CursorPosInText;
            BASECLASS::Invalidate();
          }
          if ( m_SelectionAnchorLine != (int)m_CursorLine )
          {
            m_SelectionAnchorLine = (int)m_CursorLine;
            BASECLASS::Invalidate();
          }
        }
      }
      else if ( HasSelection() )
      {
        m_SelectionAnchor = -1;
        m_SelectionAnchorLine = -1;
        BASECLASS::Invalidate();
        return true;
      }
      return false;
    }

    void UpdateScrollBars()
    {
      if ( Style() & ECS_WORDBREAK )
      {
        m_pScrollBarH->SetVisible( false );
      }

      if ( m_pScrollBarH )
      {
        m_pScrollBarH->SetScrollRange( 0, 2000 );
      }
      if ( m_pScrollBarV )
      {
        int   iHeight = 20;
        if ( m_pFont )
        {
          iHeight = m_pFont->TextHeight( "\xC4\xD6\xDC\xDFyg" );  // ÄÖÜß
        }

        m_VisibleItems = ( m_ClientRect.Height() - m_pScrollBarH->Height() ) / iHeight;

        if ( (int)m_Text.size() < m_VisibleItems )
        {
          m_pScrollBarV->SetScrollRange( 0, 0 );
        }
        else
        {
          m_pScrollBarV->SetScrollRange( 0, (int)( m_Text.size() - m_VisibleItems ) );
        }
      }
    }

    void EnsureCursorVisible()
    {
      if ( m_CursorLine < m_TextOffsetLine )
      {
        m_TextOffsetLine = m_CursorLine;
      }
      if ( m_CursorLine >= m_TextOffsetLine + m_VisibleItems )
      {
        m_TextOffsetLine = m_CursorLine - m_VisibleItems + 1;
      }
    }

    bool GetCurSel( GR::tPoint& ptStart, GR::tPoint& ptEnd )
    {
      if ( !HasSelection() )
      {
        return false;
      }
      ptStart.Set( m_SelectionAnchor, m_SelectionAnchorLine );
      ptEnd.Set( (int)m_CursorPosInText, (int)m_CursorLine );

      if ( ( ptStart.y > ptEnd.y )
      ||   ( ( ptStart.y == ptEnd.y )
      &&     ( ptStart.x > ptEnd.x ) ) )
      {
        GR::tPoint    ptTemp( ptStart );
        ptStart = ptEnd;
        ptEnd = ptTemp;
      }
      return true;
    }



    void InsertText( const GR::tPoint& ptPos, const GR::String& strText, bool bParseEscape = false )
    {
      GR::tPoint        ptStart( ptPos );

      GR::String    strEndOfOldLine;

      strEndOfOldLine = m_Text[ptStart.y].substr( ptStart.x );

      m_Text[ptStart.y] = m_Text[ptStart.y].substr( 0, ptStart.x );

      size_t    iPos = 0;

      bool      bLineBreak = false;

      bool      bSpecialSign = false;

      int       iWidthToCap = m_ClientRect.Width();
      if ( m_pScrollBarV )
      {
        iWidthToCap -= m_pScrollBarV->Width();
      }

      while ( iPos < strText.length() )
      {
        GR::Char   cDummy = strText[iPos];

        if ( m_Text[ptStart.y].length() >= (size_t)m_MaxCharacters )
        {
          break;
        }
        if ( ( Style() & ECS_WORDBREAK )
        &&   ( m_pFont ) )
        {
          if ( m_pFont->TextLength( m_Text[ptStart.y].c_str() ) >= iWidthToCap - m_pFont->TextLength( "W" ) )
          {
            // zum letzten Space suchen?
            GR::String     strLeft;

            size_t    iTempPos = m_Text[ptStart.y].length();
            while ( iTempPos > 0 )
            {
              --iTempPos;
              if ( m_Text[ptStart.y][iTempPos] == ' ' )
              {
                // hier ein Wordbreak!
                strLeft = m_Text[ptStart.y].substr( iTempPos + 1 );
                m_Text[ptStart.y] = m_Text[ptStart.y].substr( 0, iTempPos );
                break;
              }
            }
            m_Text.insert( m_Text.begin() + ptStart.y + 1, strLeft );
            bLineBreak = true;
          }
        }

        if ( (unsigned int)cDummy >= 32 )
        {
          if ( bLineBreak )
          {
            ptStart.y++;
            bLineBreak = false;
          }
          if ( cDummy == '\\' )
          {
            if ( ( !bSpecialSign )
            &&   ( bParseEscape ) )
            {
              bSpecialSign = true;
              ++iPos;
              continue;
            }
          }

          if ( bSpecialSign )
          {
            bSpecialSign = false;
            if ( cDummy == 'n' )
            {
              // \n
              m_Text.insert( m_Text.begin() + ptStart.y + 1, GR::String() );
              bLineBreak = true;
              ++iPos;
              continue;
            }
          }
          m_Text[ptStart.y] += cDummy;
        }
        else if ( cDummy == 10 )
        {
          // ein Zeilenumbruch
          m_Text.insert( m_Text.begin() + ptStart.y + 1, GR::String() );
          //m_Text.insert( m_Text.begin() + ptStart.y, GR::String() );
          //bLineBreak = true;
          ptStart.y++;
        }

        ++iPos;
      }

      m_CursorPosInText = m_Text[ptStart.y].length();
      m_Text[ptStart.y] += strEndOfOldLine;

      m_CursorLine = ptStart.y;
      UpdateCursorPos();
      UpdateScrollBars();
      if ( Style() & ECS_AUTO_H_SCROLL )
      {
        EnsureCursorVisible();
      }
    }



    virtual GR::u32 ModifyStyle( GR::u32 dwAdd, GR::u32 dwRemove = 0 )
    {
      BASECLASS::ModifyStyle( dwAdd, dwRemove );

      if ( m_pScrollBarH )
      {
        if ( Style() & ECS_WORDBREAK )
        {
          m_pScrollBarH->SetSize( m_pScrollBarH->Width(), 0 );
        }
        else
        {
          m_pScrollBarH->SetSize( m_pScrollBarH->Width(), 20 );
        }
      }
      if ( Style() & ECS_MULTILINE )
      {
        if ( m_pScrollBarH == NULL )
        {
          m_pScrollBarH = new SBCLASS( 0, m_ClientRect.Bottom - 20, m_ClientRect.Width(), 20, SBCLASS::SBFT_HORIZONTAL );
          m_pScrollBarH->GetComponent( SBCLASS::SB_BUTTON_LEFT_UP )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
          m_pScrollBarH->GetComponent( SBCLASS::SB_SLIDER )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
          m_pScrollBarH->GetComponent( SBCLASS::SB_BUTTON_RIGHT_DOWN )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
          m_pScrollBarH->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );

          Add( m_pScrollBarH );
          m_pScrollBarH->AddListener( this );
        }
        if ( m_pScrollBarV == NULL )
        {
          m_pScrollBarV = new SBCLASS( m_ClientRect.Width() - 20, 0, 20, m_ClientRect.Height() - 20, SBCLASS::SBFT_VERTICAL );
          m_pScrollBarV->GetComponent( SBCLASS::SB_BUTTON_LEFT_UP )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
          m_pScrollBarV->GetComponent( SBCLASS::SB_SLIDER )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
          m_pScrollBarV->GetComponent( SBCLASS::SB_BUTTON_RIGHT_DOWN )->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
          m_pScrollBarV->ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );

          Add( m_pScrollBarV );
          m_pScrollBarV->AddListener( this );
        }
      }
      else
      {
        if ( m_pScrollBarH )
        {
          Delete( m_pScrollBarH );
          m_pScrollBarH = NULL;
        }
        if ( m_pScrollBarV )
        {
          Delete( m_pScrollBarV );
          m_pScrollBarV = NULL;
        }
      }
      return Style();
    }



    SBCLASS* GetHScroll()
    {
      return m_pScrollBarH;
    }



    SBCLASS* GetVScroll()
    {
      return m_pScrollBarV;
    }



    virtual void ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment )
    {
      BASECLASS::ParseXML( pElement, Environment );

      if ( pElement == NULL )
      {
        return;
      }
      if ( pElement->HasAttribute( "AllowedCharacters" ) )
      {
        m_AllowedChars = pElement->Attribute( "AllowedCharacters" );
      }
      if ( pElement->HasAttribute( "MaxCharacters" ) )
      {
        m_MaxCharacters = GR::Convert::ToI32( pElement->Attribute( "MaxCharacters" ) );
      }
    }

};


#endif // __ABSTRACTEDIT_H__