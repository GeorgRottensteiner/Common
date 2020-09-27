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
    size_t    iVisibleLines = m_ClientRect.height() / m_pFont->TextHeight();

    // Selektionshintergrund
    int       iY = 0;
    if ( HasSelection() )
    {
      for ( size_t iLine = 0; iLine < iVisibleLines; ++iLine )
      {
        if ( iLine >= m_Text.size() )
        {
          break;
        }
        GR::String&   strLine = m_Text[m_TextOffsetLine + iLine];

        GR::tPoint    ptSelStart;
        GR::tPoint    ptSelEnd;

        GetCurSel( ptSelStart, ptSelEnd );

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
          pPage->Box( 0, iY,
                      m_pFont->TextLength( strLine.substr( m_TextOffset ).c_str() ),
                      iY + m_pFont->TextHeight(),
                      pPage->GetRGB256( GetSysColor( GUI::COL_HIGHLIGHT ) ) );
        }
        else
        {
          if ( ( iCurLine == ptSelStart.y )
          &&   ( iCurLine == ptSelEnd.y ) )
          {
            // Selektion komplett innerhalb einer Zeile
            int   iDeltaX = m_pFont->TextLength( strLine.substr( m_TextOffset, ptSelStart.x - m_TextOffset ).c_str() );
            if ( iDeltaX )
            {
              iDeltaX += m_pFont->FontSpacing();
            }
            pPage->Box( iDeltaX, iY,
                        iDeltaX + m_pFont->TextLength( strLine.substr( ptSelStart.x, ptSelEnd.x - ptSelStart.x ).c_str() ),
                        iY + m_pFont->TextHeight(),
                        pPage->GetRGB256( GetSysColor( GUI::COL_HIGHLIGHT ) ) );
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
            pPage->Box( iDeltaX, iY,
                        iDeltaX + m_pFont->TextLength( strLine.substr( ptSelStart.x, strLine.length() - ptSelStart.x ).c_str() ),
                        iY + m_pFont->TextHeight(),
                        pPage->GetRGB256( GetSysColor( GUI::COL_HIGHLIGHT ) ) );
          }
          else if ( iCurLine == ptSelEnd.y )
          {
            // Selektion endet hier
            if ( ptSelEnd.x > (int)m_TextOffset )
            {
              int   iDeltaX = m_pFont->TextLength( strLine.substr( m_TextOffset, ptSelEnd.x - m_TextOffset ).c_str() );
              if ( iDeltaX )
              {
                iDeltaX += m_pFont->FontSpacing();
              }

              pPage->Box( 0, iY,
                          iDeltaX, iY + m_pFont->TextHeight(),
                          pPage->GetRGB256( GetSysColor( GUI::COL_HIGHLIGHT ) ) );
            }
          }
        }
        iY += m_pFont->TextHeight();
      }
    }

    iY = 0;
    for ( size_t iLine = 0; iLine < iVisibleLines; ++iLine )
    {
      if ( m_TextOffsetLine + iLine >= m_Text.size() )
      {
        break;
      }
      GR::String&   strLine = m_Text[m_TextOffsetLine + iLine];

      if ( m_TextOffset >= strLine.length() )
      {
        iY += m_pFont->TextHeight();
        continue;
      }

      int   iX = 0;

      for ( size_t iPos = m_TextOffset; iPos < strLine.length(); ++iPos )
      {
        GR::Graphic::Image*  pLetter = ( (GR::CFont*)m_pFont )->GetLetter( strLine[iPos] );
        if ( pLetter == NULL )
        {
          continue;
        }

        pLetter->PutImage( pPage, iX, iY, IMAGE_METHOD_TRANSPARENT );
        iX += pLetter->GetWidth() + 1;
        if ( iX >= m_ClientRect.width() )
        {
          break;
        }
      }
      iY += m_pFont->TextHeight();
    }

    int   iTextHeight = m_pFont->TextHeight();

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
        pPage->Box( m_CursorPos, (int)( m_CursorLine - m_TextOffsetLine ) * iTextHeight,
                    m_CursorPos + 1, (int)( m_CursorLine - m_TextOffsetLine ) * iTextHeight + iTextHeight - 1,
                    pPage->GetRGB256( GetSysColor( GUI::COL_WINDOWTEXT ) ) );

      }
      if ( GetTickCount() - iLastTicks >= 600 )
      {
        iLastTicks = GetTickCount();
      }
    }
  }
}


