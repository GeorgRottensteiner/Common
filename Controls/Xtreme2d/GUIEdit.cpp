#include "GUIComponentDisplayer.h"
#include "GUIEdit.h"



GUI_IMPLEMENT_CLONEABLE( GUIEdit, "Edit" )



GUIEdit::GUIEdit( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 efType, GR::u32 dwId ) :
  AbstractEdit<GUIComponent, GUIScrollBar, GUISlider, GUIButton>( iNewX, iNewY, iNewWidth, iNewHeight, efType, dwId )
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
      Displayer.DrawQuad( rc.Left, rc.Top, rc.width(), rc.height(), GetColor( GUI::COL_WINDOW ) );
    }
    else
    {
      Displayer.DrawQuad( rc.Left, rc.Top, rc.width(), rc.height(), GetColor( GUI::COL_BTNFACE ) );
    }
  }

  // Cursor
  if ( GetCursorRect( rc ) )
  {
    Displayer.DrawQuad( rc.Left, rc.Top, rc.width(), rc.height(), GetColor( GUI::COL_CURSOR ) );
  }

  size_t    iVisibleLines = 1;

  if ( m_pFont )
  {
    int   TextHeight = m_pFont->TextHeight();
    if ( TextHeight )
    {
      iVisibleLines = m_ClientRect.height() / m_pFont->TextHeight();
    }
    if ( iVisibleLines == 0 )
    {
      iVisibleLines = 1;
    }
  }

  // Selektionshintergrund
  int   iTextStartY = 0;

  int     iTextHeight = Height();

  if ( m_pFont )
  {
    iTextHeight = m_pFont->TextHeight();
  }
  if ( Style() & ECS_MULTILINE )
  {
    //iTextStartY = (int)( m_iCursorLine - m_iTextOffsetLine ) * iTextHeight;
    iTextStartY = 0;
  }
  else
  {
    iTextStartY = ( m_ClientRect.height() - iTextHeight ) / 2;
  }

  int       iY = iTextStartY;
  if ( HasSelection() )
  {
    for ( size_t iLine = 0; iLine < iVisibleLines; ++iLine )
    {
      if ( iLine >= m_Text.size() )
      {
        break;
      }
      GR::String&   strLine = m_Text[m_TextOffsetLine + iLine];

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

      int    iCurLine = (int)( m_TextOffsetLine + iLine );

      if ( ( iCurLine < ptSelStart.y )
      ||   ( iCurLine > ptSelEnd.y ) )
      {
        // ausserhalb der Selection
      }
      else if ( ( iCurLine > ptSelStart.y )
      &&        ( iCurLine < ptSelEnd.y ) )
      {
        // komplett innerhalb der Selection
        Displayer.DrawQuad( 0, iY, m_pFont->TextLength( strLine.substr( m_TextOffset ).c_str() ),
                                 m_pFont->TextHeight(),
                                 GetColor( GUI::COL_HIGHLIGHT ) );
      }
      else
      {
        if ( ( iCurLine == ptSelStart.y )
        &&   ( iCurLine == ptSelEnd.y ) )
        {
          // Selektion komplett innerhalb einer Zeile
          if ( ptSelEnd.x >= (int)m_TextOffset )
          {
            int     iX1 = ptSelStart.x;
            if ( iX1 < (int)m_TextOffset )
            {
              iX1 = (int)m_TextOffset;
            }
            int   iDeltaX = 0;
            if ( m_pFont )
            {
              iDeltaX += m_pFont->TextLength( strLine.substr( m_TextOffset, iX1 - m_TextOffset ).c_str() );
              if ( iDeltaX )
              {
                iDeltaX += m_pFont->FontSpacing();
              }
              Displayer.DrawQuad( iDeltaX, iY,
                                   m_pFont->TextLength( strLine.substr( iX1, ptSelEnd.x - iX1 ).c_str() ),
                                   m_pFont->TextHeight(),
                                   GetColor( GUI::COL_HIGHLIGHT ) );
            }
          }
        }
        else if ( iCurLine == ptSelStart.y )
        {
          // Selektion beginnt hier
          int   iDeltaX = 0;
          if ( ptSelStart.x > (int)m_TextOffset )
          {
            iDeltaX = m_pFont->TextLength( strLine.substr( m_TextOffset, ptSelStart.x - m_TextOffset ).c_str() );
            if ( iDeltaX )
            {
              iDeltaX += m_pFont->FontSpacing();
            }
          }
          if ( ptSelStart.x < (int)strLine.length() )
          {
            Displayer.DrawQuad( iDeltaX, iY,
                                    m_pFont->TextLength( strLine.substr( ptSelStart.x, strLine.length() - ptSelStart.x ).c_str() ),
                                    m_pFont->TextHeight(),
                                    GetColor( GUI::COL_HIGHLIGHT ) );
          }
        }
        else if ( iCurLine == ptSelEnd.y )
        {
          // Selektion endet hier
          if ( ( ptSelEnd.x > (int)m_TextOffset )
          &&   ( m_TextOffset < strLine.length() ) )
          {
            int   iDeltaX = m_pFont->TextLength( strLine.substr( m_TextOffset, ptSelEnd.x - m_TextOffset ).c_str() );
            if ( iDeltaX )
            {
              iDeltaX += m_pFont->FontSpacing();
            }

            Displayer.DrawQuad( 0, iY,
                                    iDeltaX, m_pFont->TextHeight(),
                                    GetColor( GUI::COL_HIGHLIGHT ) );
          }
        }
      }
      iY += m_pFont->TextHeight();
    }
  }

  iY = 0;
  if ( Style() & ECS_MULTILINE )
  {
    iTextStartY = (int)( m_CursorLine - m_TextOffsetLine ) * iTextHeight;
  }

  for ( size_t iLine = 0; iLine < iVisibleLines; ++iLine )
  {
    if ( m_TextOffsetLine + iLine >= m_Text.size() )
    {
      break;
    }
    GR::String&   strLine = m_Text[m_TextOffsetLine + iLine];

    if ( m_TextOffset >= strLine.length() )
    {
      if ( m_pFont )
      {
        iY += m_pFont->TextHeight();
      }
      else
      {
        iY += 12;
      }
      continue;
    }

    GR::tPoint    ptSelStart;
    GR::tPoint    ptSelEnd;

    if ( !GetCurSel( ptSelStart, ptSelEnd ) )
    {
      if ( m_TextOffset < strLine.length() )
      {
        Displayer.DrawText( m_pFont, 0, iY, strLine.substr( m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
      }
    }
    else
    {
      int    iCurLine = (int)( m_TextOffsetLine + iLine );

      if ( ( iCurLine < ptSelStart.y )
      ||   ( iCurLine > ptSelEnd.y ) )
      {
        // ausserhalb der Selection
        Displayer.DrawText( m_pFont, 0, iY, strLine.substr( m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
      }
      else if ( ( iCurLine > ptSelStart.y )
      &&        ( iCurLine < ptSelEnd.y ) )
      {
        // komplett innerhalb der Selection
        Displayer.DrawText( m_pFont, 0, iY, strLine.substr( m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_HIGHLIGHTTEXT ), &m_ClientRect );
      }
      else
      {
        if ( ( iCurLine == ptSelStart.y )
        &&   ( iCurLine == ptSelEnd.y ) )
        {
          if ( ptSelEnd.x >= (int)m_TextOffset )
          {
            int     iX1 = ptSelStart.x;
            if ( iX1 < (int)m_TextOffset )
            {
              iX1 = (int)m_TextOffset;
            }
            // Selektion komplett innerhalb einer Zeile
            Displayer.DrawText( m_pFont, 0, iY, strLine.substr( m_TextOffset, iX1 - m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );

            int   iDeltaX = m_pFont->TextLength( strLine.substr( m_TextOffset, iX1 - m_TextOffset ) );
            if ( iDeltaX )
            {
              iDeltaX += m_pFont->FontSpacing();
            }
            Displayer.DrawText( m_pFont, iDeltaX, iY, strLine.substr( iX1, ptSelEnd.x - iX1 ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_HIGHLIGHTTEXT ), &m_ClientRect );
            int iDeltaX2 = m_pFont->TextLength( strLine.substr( iX1, ptSelEnd.x - iX1 ) );
            if ( iDeltaX2 )
            {
              iDeltaX2 += m_pFont->FontSpacing();
            }
            iDeltaX += iDeltaX2;

            Displayer.DrawText( m_pFont, iDeltaX, iY, strLine.substr( ptSelEnd.x ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
          }
        }
        else if ( iCurLine == ptSelStart.y )
        {
          // Selektion beginnt hier
          int     iX1 = ptSelStart.x;
          if ( iX1 < (int)m_TextOffset )
          {
            iX1 = (int)m_TextOffset;
          }

          int   iDeltaX = 0;
          if ( ptSelStart.x > (int)m_TextOffset )
          {
            Displayer.DrawText( m_pFont, 0, iY, strLine.substr( m_TextOffset, iX1 - m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
            iDeltaX = m_pFont->TextLength( strLine.substr( m_TextOffset, iX1 - m_TextOffset ) );
            if ( iDeltaX )
            {
              iDeltaX += m_pFont->FontSpacing();
            }
          }
          if ( iX1 < (int)strLine.length() )
          {
            Displayer.DrawText( m_pFont, iDeltaX, iY, strLine.substr( iX1, strLine.length() - iX1 ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_HIGHLIGHTTEXT ), &m_ClientRect );
          }
        }
        else if ( iCurLine == ptSelEnd.y )
        {
          // Selektion endet hier
          int iDeltaX = 0;
          if ( ptSelEnd.x > (int)m_TextOffset )
          {
            Displayer.DrawText( m_pFont, 0, iY, strLine.substr( m_TextOffset, ptSelEnd.x - m_TextOffset ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_HIGHLIGHTTEXT ), &m_ClientRect );
            iDeltaX = m_pFont->TextLength( strLine.substr( m_TextOffset, ptSelEnd.x - m_TextOffset ) );
            if ( iDeltaX )
            {
              iDeltaX += m_pFont->FontSpacing();
            }
          }

          Displayer.DrawText( m_pFont, iDeltaX, iY, strLine.substr( ptSelEnd.x ), m_TextAlignment & ~GUI::AF_MULTILINE, GetColor( GUI::COL_WINDOWTEXT ), &m_ClientRect );
        }
      }
    }

    if ( m_pFont )
    {
      iY += m_pFont->TextHeight();
    }
    else
    {
      iY += 12;
    }
  }
}



