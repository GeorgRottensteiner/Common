#include "GUIComponentDisplayer.h"
#include "GUIEdit.h"

GUI_IMPLEMENT_CLONEABLE( GUIEdit, "Edit" )



GUIEdit::GUIEdit( int X, int Y, int Width, int Height, GR::u32 Type, GR::u32 Id ) :
  AbstractEdit<GUIComponent, GUIScrollBar, GUISlider, GUIButton>( X, Y, Width, Height, Type, Id )
{
  m_TextAlignment = GUI::AF_LEFT | GUI::AF_VCENTER;

  if ( Style() & ECS_MULTILINE )
  {
    m_TextAlignment = GUI::AF_MULTILINE | GUI::AF_LEFT;
  }

  if ( IsEnabled() )
  {
    SetColor( GUI::COL_BTNFACE, GetColor( GUI::COL_WINDOW ) );
  }
  else
  {
    SetColor( GUI::COL_BTNFACE, GetSysColor( GUI::COL_BTNFACE ) );
  }

  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );
}



void GUIEdit::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  GR::tRect   rc;

  // Cursor
  if ( GetCursorRect( rc ) )
  {
    Displayer.DrawQuad( rc.Left, rc.Top, rc.Width(), rc.Height(), GetColor( GUI::COL_CURSOR ) );
  }

  size_t    visibleLines = 1;
  
  if ( m_pFont )
  {
    visibleLines = m_ClientRect.Height() / Displayer.TextHeight( m_pFont );
    if ( visibleLines == 0 )
    {
      visibleLines = 1;
    }
  }

  // Selektionshintergrund
  int   textStartY = 0;

  int     textHeight = Height();

  if ( m_pFont )
  {
    textHeight = Displayer.TextHeight( m_pFont );
  }
  if ( Style() & ECS_MULTILINE )
  {
    //textStartY = (int)( m_CursorLine - m_TextOffsetLine ) * textHeight;
    textStartY = 0;
  }
  else
  {
    textStartY = ( m_ClientRect.Height() - textHeight ) / 2;
  }

  int       y = textStartY;
  if ( ( HasSelection() )
  &&   ( m_pFont != NULL ) )
  {
    for ( size_t lineIndex = 0; lineIndex < visibleLines; ++lineIndex )
    {
      if ( m_TextOffsetLine + lineIndex >= m_Text.size() )
      {
        break;
      }
      GR::String&   line = m_Text[m_TextOffsetLine + lineIndex];

      GR::tPoint    ptSelStart( m_SelectionAnchor, m_SelectionAnchorLine );
      GR::tPoint    ptSelEnd( (int)m_CursorPosInText, (int)m_CursorLine );

      if ( ( ptSelStart.y > ptSelEnd.y )
      ||   ( ( ptSelStart.y == ptSelEnd.y )
      &&     ( ptSelStart.x > ptSelEnd.x ) ) )
      {
        GR::tPoint    ptTemp( ptSelStart );
        ptSelStart = ptSelEnd;
        ptSelEnd = ptTemp;
      }

      int     x = m_ClientRect.Left;

      int     length = Displayer.TextLength( m_pFont, line );

      if ( ( m_TextAlignment & GUI::AF_CENTER ) == GUI::AF_CENTER )
      {
        x += ( m_ClientRect.Width() - length ) / 2;
      }
      else if ( m_TextAlignment & GUI::AF_RIGHT )
      {
        x = m_ClientRect.Right - length;
      }

      int    curLine = (int)( m_TextOffsetLine + lineIndex );

      if ( ( curLine < ptSelStart.y )
      ||   ( curLine > ptSelEnd.y ) )
      {
        // ausserhalb der Selection
      }
      else if ( ( curLine > ptSelStart.y )
      &&        ( curLine < ptSelEnd.y ) )
      {
        // komplett innerhalb der Selection
        if ( m_TextOffset < line.length() )
        {
          Displayer.DrawQuad( x, y, 
                              Displayer.TextLength( m_pFont,  line.substr( m_TextOffset ) ), 
                              Displayer.TextHeight( m_pFont ),
                              GetSysColor( GUI::COL_HIGHLIGHT ) );
        }
      }
      else
      {
        if ( ( curLine == ptSelStart.y )
        &&   ( curLine == ptSelEnd.y ) )
        {
          // Selektion komplett innerhalb einer Zeile
          if ( ptSelEnd.x >= (int)m_TextOffset )
          {
            int     X1 = ptSelStart.x;
            if ( X1 < (int)m_TextOffset )
            {
              X1 = (int)m_TextOffset;
            }
            int   deltaX = Displayer.TextLength( m_pFont, line.substr( m_TextOffset, X1 - m_TextOffset ) );
            if ( deltaX )
            {
              deltaX += m_pFont->FontSpacing();
            }
            Displayer.DrawQuad( x + deltaX, y, 
                                Displayer.TextLength( m_pFont, line.substr( X1, ptSelEnd.x - X1 ) ),
                                Displayer.TextHeight( m_pFont ),
                                GetSysColor( GUI::COL_HIGHLIGHT ) );
          }
        }
        else if ( curLine == ptSelStart.y )
        {
          // Selektion beginnt hier
          int   deltaX = 0;
          if ( ptSelStart.x > (int)m_TextOffset )
          {
            deltaX = Displayer.TextLength( m_pFont, line.substr( m_TextOffset, ptSelStart.x - m_TextOffset ) );
            if ( deltaX )
            {
              deltaX += m_pFont->FontSpacing();
            }
          }
          if ( ptSelStart.x < (int)line.length() )
          {
            Displayer.DrawQuad( x + deltaX, y, 
                                Displayer.TextLength( m_pFont, line.substr( ptSelStart.x, line.length() - ptSelStart.x ) ),
                                Displayer.TextHeight( m_pFont ),
                                GetSysColor( GUI::COL_HIGHLIGHT ) );
          }
        }
        else if ( curLine == ptSelEnd.y )
        {
          // Selektion endet hier
          if ( ( ptSelEnd.x > (int)m_TextOffset )
          &&   ( m_TextOffset < line.length() ) )
          {
            int   deltaX = Displayer.TextLength( m_pFont,  line.substr( m_TextOffset, ptSelEnd.x - m_TextOffset ) );
            if ( deltaX )
            {
              deltaX += m_pFont->FontSpacing();
            }

            Displayer.DrawQuad( x, y, 
                                deltaX, Displayer.TextHeight( m_pFont ),
                                GetSysColor( GUI::COL_HIGHLIGHT ) );
          }
        }
      }
      y += Displayer.TextHeight( m_pFont );
    }
  }

  y = 0;
  if ( Style() & ECS_MULTILINE )
  {
    textStartY = (int)( m_CursorLine - m_TextOffsetLine ) * textHeight;
  }

  for ( size_t lineIndex = 0; lineIndex < visibleLines; ++lineIndex )
  {
    if ( m_TextOffsetLine + lineIndex >= m_Text.size() )
    {
      break;
    }
    GR::String&   line = m_Text[m_TextOffsetLine + lineIndex];

    if ( m_TextOffset >= line.length() )
    {
      if ( m_pFont )
      {
        y += Displayer.TextHeight( m_pFont );
      }
      else
      {
        y += 12;
      }
      continue;
    }

    GR::tPoint    ptSelStart;
    GR::tPoint    ptSelEnd;

    if ( !GetCurSel( ptSelStart, ptSelEnd ) )
    {
      if ( m_TextOffset < line.length() )
      {
        Displayer.DrawText( m_pFont, 0, y, line.substr( m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
      }
    }
    else
    {
      int    curLine = (int)( m_TextOffsetLine + lineIndex );

      if ( ( curLine < ptSelStart.y )
      ||   ( curLine > ptSelEnd.y ) )
      {
        // ausserhalb der Selection
        Displayer.DrawText( m_pFont, 0, y, line.substr( m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
      }
      else if ( ( curLine > ptSelStart.y )
      &&        ( curLine < ptSelEnd.y ) )
      {
        // komplett innerhalb der Selection
        Displayer.DrawText( m_pFont, 0, y, line.substr( m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_HIGHLIGHTTEXT ), &m_ClientRect );
      }
      else
      {
        if ( ( curLine == ptSelStart.y )
        &&   ( curLine == ptSelEnd.y ) )
        {
          if ( ptSelEnd.x >= (int)m_TextOffset )
          {
            int     X1 = ptSelStart.x;
            if ( X1 < (int)m_TextOffset )
            {
              X1 = (int)m_TextOffset;
            }
            // Selektion komplett innerhalb einer Zeile
            Displayer.DrawText( m_pFont, 0, y, line.substr( m_TextOffset, X1 - m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );

            int   deltaX = Displayer.TextLength( m_pFont, line.substr( m_TextOffset, X1 - m_TextOffset ) );
            if ( deltaX )
            {
              deltaX += m_pFont->FontSpacing();
            }
            Displayer.DrawText( m_pFont, deltaX, y, line.substr( X1, ptSelEnd.x - X1 ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_HIGHLIGHTTEXT ), &m_ClientRect );
            int deltaX2 = Displayer.TextLength( m_pFont, line.substr( X1, ptSelEnd.x - X1 ) );
            if ( deltaX2 )
            {
              deltaX2 += m_pFont->FontSpacing();
            }
            deltaX += deltaX2;

            Displayer.DrawText( m_pFont, deltaX, y, line.substr( ptSelEnd.x ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
          }
        }
        else if ( curLine == ptSelStart.y )
        {
          // Selektion beginnt hier
          int     X1 = ptSelStart.x;
          if ( X1 < (int)m_TextOffset )
          {
            X1 = (int)m_TextOffset;
          }

          int   deltaX = 0;
          if ( ptSelStart.x > (int)m_TextOffset )
          {
            Displayer.DrawText( m_pFont, 0, y, line.substr( m_TextOffset, X1 - m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
            deltaX = Displayer.TextLength( m_pFont, line.substr( m_TextOffset, X1 - m_TextOffset ) );
            if ( deltaX )
            {
              deltaX += m_pFont->FontSpacing();
            }
          }
          if ( X1 < (int)line.length() )
          {
            Displayer.DrawText( m_pFont, deltaX, y, line.substr( X1, line.length() - X1 ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_HIGHLIGHTTEXT ), &m_ClientRect );
          }
        }
        else if ( curLine == ptSelEnd.y )
        {
          // Selektion endet hier
          int iDeltaX = 0;
          if ( ptSelEnd.x > (int)m_TextOffset )
          {
            Displayer.DrawText( m_pFont, 0, y, line.substr( m_TextOffset, ptSelEnd.x - m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_HIGHLIGHTTEXT ), &m_ClientRect );
            iDeltaX = Displayer.TextLength( m_pFont,  line.substr( m_TextOffset, ptSelEnd.x - m_TextOffset ) );
            if ( iDeltaX )
            {
              iDeltaX += m_pFont->FontSpacing();
            }
          }

          Displayer.DrawText( m_pFont, iDeltaX, y, line.substr( ptSelEnd.x ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
        }
      }
    }

    if ( m_pFont )
    {
      y += Displayer.TextHeight( m_pFont );
    }
    else
    {
      y += 12;
    }
  }
}



bool GUIEdit::ProcessEvent( const GUI::ComponentEvent& Event )
{
  switch ( Event.Type )
  {
    case CET_ENABLE:
      if ( Event.Value )
      {
        SetColor( GUI::COL_BTNFACE, GetColor( GUI::COL_WINDOW ) );
      }
      else
      {
        SetColor( GUI::COL_BTNFACE, GetSysColor( GUI::COL_BTNFACE ) );
      }
      break;
  }

  return AbstractEdit<GUIComponent, GUIScrollBar, GUISlider, GUIButton>::ProcessEvent( Event );
}