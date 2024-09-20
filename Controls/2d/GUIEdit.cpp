#include <Grafik/Font.h>

#include "GUIComponentDisplayer.h"
#include "GUIEdit.h"



GUIEdit::GUIEdit( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId, GR::u32 efType ) :
  AbstractEdit<GUIComponent, GUIScrollbar, GUISlider, GUIButton>( iNewX, iNewY, iNewWidth, iNewHeight, efType, dwId )

{
  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );
}



void GUIEdit::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{
  GR::tRect   rectSel;

  if ( m_pFont )
  {
    size_t    visibleLines = m_ClientRect.Height() / m_pFont->TextHeight();

    // Selektionshintergrund
    int       y = 0;
    if ( HasSelection() )
    {
      for ( size_t iLine = 0; iLine < visibleLines; ++iLine )
      {
        if ( iLine >= m_Text.size() )
        {
          break;
        }
        GR::String&   lineText = m_Text[m_TextOffsetLine + iLine];

        GR::tPoint    ptSelStart;
        GR::tPoint    ptSelEnd;

        GetCurSel( ptSelStart, ptSelEnd );

        int    curLine = (int)( m_TextOffsetLine + iLine );

        if ( ( curLine < ptSelStart.y )
        ||   ( curLine > ptSelEnd.y ) )
        {
          // ausserhalb der Selection
        }
        else if ( ( curLine > ptSelStart.y )
        &&        ( curLine < ptSelEnd.y ) )
        {
          // komplett innerhalb der Selection
          pPage->Box( 0, y,
                      m_pFont->TextLength( lineText.substr( m_TextOffset ).c_str() ),
                      y + m_pFont->TextHeight(),
                      pPage->GetRGB256( GetSysColor( GUI::COL_HIGHLIGHT ) ) );
        }
        else
        {
          if ( ( curLine == ptSelStart.y )
          &&   ( curLine == ptSelEnd.y ) )
          {
            // Selektion komplett innerhalb einer Zeile
            int   deltaX = m_pFont->TextLength( lineText.substr( m_TextOffset, ptSelStart.x - m_TextOffset ).c_str() );
            if ( deltaX )
            {
              deltaX += m_pFont->FontSpacing();
            }
            pPage->Box( deltaX, y,
                        deltaX + m_pFont->TextLength( lineText.substr( ptSelStart.x, ptSelEnd.x - ptSelStart.x ).c_str() ),
                        y + m_pFont->TextHeight(),
                        pPage->GetRGB256( GetSysColor( GUI::COL_HIGHLIGHT ) ) );
          }
          else if ( curLine == ptSelStart.y )
          {
            // Selektion beginnt hier
            int   iDeltaX = 0;
            if ( ptSelStart.x > (int)m_TextOffset )
            {
              iDeltaX = m_pFont->TextLength( lineText.substr( m_TextOffset, ptSelStart.x - m_TextOffset ).c_str() );
              if ( iDeltaX )
              {
                iDeltaX += m_pFont->FontSpacing();
              }
            }
            pPage->Box( iDeltaX, y,
                        iDeltaX + m_pFont->TextLength( lineText.substr( ptSelStart.x, lineText.length() - ptSelStart.x ).c_str() ),
                        y + m_pFont->TextHeight(),
                        pPage->GetRGB256( GetSysColor( GUI::COL_HIGHLIGHT ) ) );
          }
          else if ( curLine == ptSelEnd.y )
          {
            // Selektion endet hier
            if ( ptSelEnd.x > (int)m_TextOffset )
            {
              int   deltaX = m_pFont->TextLength( lineText.substr( m_TextOffset, ptSelEnd.x - m_TextOffset ).c_str() );
              if ( deltaX )
              {
                deltaX += m_pFont->FontSpacing();
              }

              pPage->Box( 0, y,
                          deltaX, y + m_pFont->TextHeight(),
                          pPage->GetRGB256( GetSysColor( GUI::COL_HIGHLIGHT ) ) );
            }
          }
        }
        y += m_pFont->TextHeight();
      }
    }

    y = 0;
    for ( size_t iLine = 0; iLine < visibleLines; ++iLine )
    {
      if ( m_TextOffsetLine + iLine >= m_Text.size() )
      {
        break;
      }
      GR::String&   strLine = m_Text[m_TextOffsetLine + iLine];

      if ( m_TextOffset >= strLine.length() )
      {
        y += m_pFont->TextHeight();
        continue;
      }

      int   x = 0;

      for ( size_t iPos = m_TextOffset; iPos < strLine.length(); ++iPos )
      {
        GR::Graphic::Image*  pLetter = ( (GR::Font*)m_pFont )->GetLetter( strLine[iPos] );
        if ( pLetter == NULL )
        {
          continue;
        }

        pLetter->PutImage( pPage, x, y, IMAGE_METHOD_TRANSPARENT );
        x += pLetter->GetWidth() + 1;
        if ( x >= m_ClientRect.Width() )
        {
          break;
        }
      }
      y += m_pFont->TextHeight();
    }

    int   textHeight = m_pFont->TextHeight();

    /*
    int   iX = 0,
          iY = 0;

    GR::tPoint    ptOffset = TextOffset( m_strCaption.c_str(), m_textAlignment );

    iX += ptOffset.x;
    iY += ptOffset.y;

    for ( size_t i = m_iTextOffset; i < m_strCaption.length(); i++ )
    {
      GR::Graphic::Image*  pLetter = ( (GR::CFont*)m_pFont )->GetLetter( m_strCaption[i] );
      if ( pLetter == NULL )
      {
        continue;
      }

      pLetter->PutImage( pPage, iX, iY, IMAGE_METHOD_TRANSPARENT );
      iX += pLetter->GetWidth() + 1;
    }
    */
    // Cursor darstellen
    if ( ( IsFocused() )
    &&   ( IsEnabled() ) )
    {
      static    int   iLastTicks = GetTickCount();
      if ( GetTickCount() - iLastTicks >= 300 )
      {
        pPage->Box( m_CursorPos, (int)( m_CursorLine - m_TextOffsetLine ) * textHeight,
                    m_CursorPos + 1, (int)( m_CursorLine - m_TextOffsetLine ) * textHeight + textHeight - 1,
                    pPage->GetRGB256( GetSysColor( GUI::COL_WINDOWTEXT ) ) );

      }
      if ( GetTickCount() - iLastTicks >= 600 )
      {
        iLastTicks = GetTickCount();
      }
    }
  }
}


