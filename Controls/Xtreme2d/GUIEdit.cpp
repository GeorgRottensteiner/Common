#include "GUIComponentDisplayer.h"
#include "GUIEdit.h"



GUI_IMPLEMENT_CLONEABLE( GUIEdit, "Edit" )



GUIEdit::GUIEdit( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 EditFlags, GR::u32 Id ) :
  AbstractEdit<GUIComponent, GUIScrollBar, GUISlider, GUIButton>( NewX, NewY, NewWidth, NewHeight, EditFlags, Id )
{
  m_TextAlignment = GUI::AF_LEFT | GUI::AF_VCENTER;

  if ( Style() & ECS_MULTILINE )
  {
    m_TextAlignment = GUI::AF_MULTILINE | GUI::AF_LEFT;
  }

  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );
}



void GUIEdit::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  GR::tRect   rc;

  GetClientRect( rc );

  if ( !( VisualStyle() & GUI::VFT_TRANSPARENT_BKGND ) )
  {
    if ( IsEnabled() )
    {
      Displayer.DrawQuad( rc.Left, rc.Top, rc.Width(), rc.Height(), GetColor( GUI::COL_WINDOW ) );
    }
    else
    {
      Displayer.DrawQuad( rc.Left, rc.Top, rc.Width(), rc.Height(), GetColor( GUI::COL_BTNFACE ) );
    }
  }

  // Cursor
  if ( GetCursorRect( rc ) )
  {
    Displayer.DrawQuad( rc.Left, rc.Top, rc.Width(), rc.Height(), GetColor( GUI::COL_CURSOR ) );
  }

  size_t    visibleLines = 1;

  if ( m_pFont )
  {
    int   textHeight = m_pFont->TextHeight();
    if ( textHeight )
    {
      visibleLines = m_ClientRect.Height() / m_pFont->TextHeight();
    }
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
    textHeight = m_pFont->TextHeight();
  }
  if ( Style() & ECS_MULTILINE )
  {
    textStartY = 0;
  }
  else
  {
    textStartY = ( m_ClientRect.Height() - textHeight ) / 2;
  }

  int       y = textStartY;
  if ( HasSelection() )
  {
    for ( size_t lineIndex = 0; lineIndex < visibleLines; ++lineIndex )
    {
      if ( lineIndex >= m_Text.size() )
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
        Displayer.DrawQuad( 0, y, m_pFont->TextLength( line.substr( m_TextOffset ).c_str() ),
                                 m_pFont->TextHeight(),
                                 GetColor( GUI::COL_HIGHLIGHT ) );
      }
      else
      {
        if ( ( curLine == ptSelStart.y )
        &&   ( curLine == ptSelEnd.y ) )
        {
          // Selektion komplett innerhalb einer Zeile
          if ( ptSelEnd.x >= (int)m_TextOffset )
          {
            int     x1 = ptSelStart.x;
            if ( x1 < (int)m_TextOffset )
            {
              x1 = (int)m_TextOffset;
            }
            int   deltaX = 0;
            if ( m_pFont )
            {
              deltaX += m_pFont->TextLength( line.substr( m_TextOffset, x1 - m_TextOffset ).c_str() );
              if ( deltaX )
              {
                deltaX += m_pFont->FontSpacing();
              }
              Displayer.DrawQuad( deltaX, y,
                                   m_pFont->TextLength( line.substr( x1, ptSelEnd.x - x1 ).c_str() ),
                                   m_pFont->TextHeight(),
                                   GetColor( GUI::COL_HIGHLIGHT ) );
            }
          }
        }
        else if ( curLine == ptSelStart.y )
        {
          // Selektion beginnt hier
          int   deltaX = 0;
          if ( ptSelStart.x > (int)m_TextOffset )
          {
            deltaX = m_pFont->TextLength( line.substr( m_TextOffset, ptSelStart.x - m_TextOffset ).c_str() );
            if ( deltaX )
            {
              deltaX += m_pFont->FontSpacing();
            }
          }
          if ( ptSelStart.x < (int)line.length() )
          {
            Displayer.DrawQuad( deltaX, y,
                                    m_pFont->TextLength( line.substr( ptSelStart.x, line.length() - ptSelStart.x ).c_str() ),
                                    m_pFont->TextHeight(),
                                    GetColor( GUI::COL_HIGHLIGHT ) );
          }
        }
        else if ( curLine == ptSelEnd.y )
        {
          // Selektion endet hier
          if ( ( ptSelEnd.x > (int)m_TextOffset )
          &&   ( m_TextOffset < line.length() ) )
          {
            int   deltaX = m_pFont->TextLength( line.substr( m_TextOffset, ptSelEnd.x - m_TextOffset ).c_str() );
            if ( deltaX )
            {
              deltaX += m_pFont->FontSpacing();
            }

            Displayer.DrawQuad( 0, y,
                                    deltaX, m_pFont->TextHeight(),
                                    GetColor( GUI::COL_HIGHLIGHT ) );
          }
        }
      }
      y += m_pFont->TextHeight();
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
    GR::String&   strLine = m_Text[m_TextOffsetLine + lineIndex];

    if ( m_TextOffset >= strLine.length() )
    {
      if ( m_pFont )
      {
        y += m_pFont->TextHeight();
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
      if ( m_TextOffset < strLine.length() )
      {
        Displayer.DrawText( m_pFont, 0, y, strLine.substr( m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
      }
    }
    else
    {
      int    curLine = (int)( m_TextOffsetLine + lineIndex );

      if ( ( curLine < ptSelStart.y )
      ||   ( curLine > ptSelEnd.y ) )
      {
        // ausserhalb der Selection
        Displayer.DrawText( m_pFont, 0, y, strLine.substr( m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
      }
      else if ( ( curLine > ptSelStart.y )
      &&        ( curLine < ptSelEnd.y ) )
      {
        // komplett innerhalb der Selection
        Displayer.DrawText( m_pFont, 0, y, strLine.substr( m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_HIGHLIGHTTEXT ), &m_ClientRect );
      }
      else
      {
        if ( ( curLine == ptSelStart.y )
        &&   ( curLine == ptSelEnd.y ) )
        {
          if ( ptSelEnd.x >= (int)m_TextOffset )
          {
            int     x1 = ptSelStart.x;
            if ( x1 < (int)m_TextOffset )
            {
              x1 = (int)m_TextOffset;
            }
            // Selektion komplett innerhalb einer Zeile
            Displayer.DrawText( m_pFont, 0, y, strLine.substr( m_TextOffset, x1 - m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );

            int   deltaX = m_pFont->TextLength( strLine.substr( m_TextOffset, x1 - m_TextOffset ) );
            if ( deltaX )
            {
              deltaX += m_pFont->FontSpacing();
            }
            Displayer.DrawText( m_pFont, deltaX, y, strLine.substr( x1, ptSelEnd.x - x1 ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_HIGHLIGHTTEXT ), &m_ClientRect );
            int deltaX2 = m_pFont->TextLength( strLine.substr( x1, ptSelEnd.x - x1 ) );
            if ( deltaX2 )
            {
              deltaX2 += m_pFont->FontSpacing();
            }
            deltaX += deltaX2;

            Displayer.DrawText( m_pFont, deltaX, y, strLine.substr( ptSelEnd.x ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
          }
        }
        else if ( curLine == ptSelStart.y )
        {
          // Selektion beginnt hier
          int     x1 = ptSelStart.x;
          if ( x1 < (int)m_TextOffset )
          {
            x1 = (int)m_TextOffset;
          }

          int   deltaX = 0;
          if ( ptSelStart.x > (int)m_TextOffset )
          {
            Displayer.DrawText( m_pFont, 0, y, strLine.substr( m_TextOffset, x1 - m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
            deltaX = m_pFont->TextLength( strLine.substr( m_TextOffset, x1 - m_TextOffset ) );
            if ( deltaX )
            {
              deltaX += m_pFont->FontSpacing();
            }
          }
          if ( x1 < (int)strLine.length() )
          {
            Displayer.DrawText( m_pFont, deltaX, y, strLine.substr( x1, strLine.length() - x1 ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_HIGHLIGHTTEXT ), &m_ClientRect );
          }
        }
        else if ( curLine == ptSelEnd.y )
        {
          // Selektion endet hier
          int deltaX = 0;
          if ( ptSelEnd.x > (int)m_TextOffset )
          {
            Displayer.DrawText( m_pFont, 0, y, strLine.substr( m_TextOffset, ptSelEnd.x - m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_HIGHLIGHTTEXT ), &m_ClientRect );
            deltaX = m_pFont->TextLength( strLine.substr( m_TextOffset, ptSelEnd.x - m_TextOffset ) );
            if ( deltaX )
            {
              deltaX += m_pFont->FontSpacing();
            }
          }

          Displayer.DrawText( m_pFont, deltaX, y, strLine.substr( ptSelEnd.x ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
        }
      }
    }

    if ( m_pFont )
    {
      y += m_pFont->TextHeight();
    }
    else
    {
      y += 12;
    }
  }
}



