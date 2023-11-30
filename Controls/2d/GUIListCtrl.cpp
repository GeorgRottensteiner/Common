#include <Grafik\ContextDescriptor.h>
#include <Grafik/Image.h>
#include <Grafik/Font.h>

#include "GUIComponentDisplayer.h"
#include "GUIListCtrl.h"



GUIListCtrl::GUIListCtrl( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwType, GR::u32 dwId ) :
  AbstractListCtrl<GUIComponent,GUIScrollbar>( iNewX, iNewY, iNewWidth, iNewHeight, dwType, dwId ),
  m_pImageHdrLeft( NULL ),
  m_pImageHdrCenter( NULL ),
  m_pImageHdrRight( NULL )
{
  m_ItemHeight     = 16;
  m_Offset         = 0;
  m_HeaderHeight   = 20;

  ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER );

  m_pScrollBar->ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  m_pScrollBar->SetLocation( m_ClientRect.size().x - m_pScrollBar->Width(), 0 );
  m_pScrollBar->SetSize( m_pScrollBar->Width(), m_ClientRect.size().y );
}



void GUIListCtrl::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{
  GR::Graphic::ContextDescriptor    cdPage( pPage );
  
  if ( m_pFont )
  {
    int   iYOffset = -(int)m_Offset * m_ItemHeight;

    // Header anzeigen
    if ( Style() & LCS_SHOW_HEADER )
    {
      iYOffset += m_HeaderHeight;

      int   iXOffset = 0;

      for ( size_t iColumn = 0; iColumn < m_vectColumns.size(); ++iColumn )
      {
        tListCtrlColumn&  Column = m_vectColumns[iColumn];

        GR::tRect   rectHeader;
        
        GetHeaderRect( iColumn, rectHeader );

        GR::tPoint   ptPos = rectHeader.position();

        if ( ( m_pImageHdrLeft )
        &&   ( m_pImageHdrCenter )
        &&   ( m_pImageHdrRight ) )
        {
          int   iTimes = ( rectHeader.width() - m_pImageHdrLeft->GetWidth() - m_pImageHdrRight->GetWidth() ) / m_pImageHdrCenter->GetWidth() + 1;

          for ( int i = 0; i < iTimes; ++i )
          {
            m_pImageHdrCenter->PutImage( pPage, 
                                         rectHeader.Left + m_pImageHdrLeft->GetWidth() + i * m_pImageHdrCenter->GetWidth(),
                                         rectHeader.Top );
          }
          m_pImageHdrLeft->PutImage( pPage, rectHeader.Left, rectHeader.Top );
          m_pImageHdrRight->PutImage( pPage, rectHeader.Right - m_pImageHdrRight->GetWidth(), rectHeader.Top );
        }
        else
        {
          DrawEdge( pPage, GUI::VFT_RAISED_BORDER, rectHeader );
        }

        rectHeader.inflate( -4, 0 );
        rectHeader.Top += 2;

        DrawText( pPage, Column.Description.c_str(), rectHeader, Column.TextAlignment );

        iXOffset += Column.CurrentWidth;
      }
      // rechts ist noch Platz, leeren Header zeichnen
      if ( iXOffset < m_ClientRect.size().x - m_pScrollBar->Width() )
      {
        GR::tRect   rectHeader;

        rectHeader.position( iXOffset, 0 );
        rectHeader.size( m_ClientRect.size().x - iXOffset - m_pScrollBar->Width(), m_HeaderHeight );
        DrawEdge( pPage, GUI::VFT_RAISED_BORDER, rectHeader );
      }
    }

    GR::tRect     rectList;

    GetListRect( rectList );

    for ( size_t iItem = m_Offset; iItem < m_Items.NumRows(); ++iItem )
    {
      const auto& Row = m_Items.Row( iItem );

      GR::tRect         rcItem;

      if ( !GetItemRect( iItem, -1, rcItem ) )
      {
        break;
      }

      GR::tRect   rcOldClip;

      rcOldClip.Left   = pPage->GetLeftBorder();
      rcOldClip.Top    = pPage->GetTopBorder();
      rcOldClip.Right  = pPage->GetRightBorder();
      rcOldClip.Bottom = pPage->GetBottomBorder();

      GR::tRect   rcClient;

      GetClientRect( rcClient );
      LocalToScreen( rcClient );

      int   iXOffset = 0;
      for ( size_t iColumn = 0; iColumn < m_vectColumns.size(); ++iColumn )
      {
        const tListCtrlItem&    Item = Row.SubItems[iColumn];

        if ( GetItemRect( iItem, iColumn, rcItem ) )
        {
          GR::u32   dwColor = 0xffffffff;
          if ( iItem == m_SelectedItem )
          {
            dwColor = 0xffffffff;
          }
          else if ( iItem == m_MouseOverItem )
          {
            dwColor = 0xffffa000;
          }


          GR::tRect   rcTemp( rcItem );
          LocalToScreen( rcTemp );
          rcTemp = rcTemp.intersection( rcClient );
          pPage->SetRange( rcTemp.Left, rcTemp.Top, 
                           rcTemp.Right, rcTemp.Bottom );
          DrawText( pPage, Item.Text.c_str(), rcItem, m_vectColumns[iColumn].TextAlignment );
        }

        iXOffset += m_vectColumns[iColumn].CurrentWidth;

      }
      pPage->SetRange( rcOldClip.Left, rcOldClip.Top, 
                       rcOldClip.Right, rcOldClip.Bottom );
    }

    GR::tRect   rcSelectionBox;

    if ( GetItemRect( m_MouseOverItem, -1, rcSelectionBox ) )
    {
      cdPage.AlphaBox( rcSelectionBox.Left,
                       rcSelectionBox.Top,
                       rcSelectionBox.width(),
                       rcSelectionBox.height(),
                       0x3030a0, 128 );
    }
    if ( GetItemRect( m_SelectedItem, -1, rcSelectionBox ) )
    {
      cdPage.AlphaBox( rcSelectionBox.Left,
                       rcSelectionBox.Top,
                       rcSelectionBox.width(),
                       rcSelectionBox.height(),
                       0xc0c0a0, 128 );
    }

    if ( ( GetItemRect( GetSelectedItem(), -1, rcSelectionBox ) )
    &&   ( IsFocused() ) )
    {
      DrawFocusRect( pPage, rcSelectionBox );
    }
  }
}



void GUIListCtrl::SetHeaderImages( GR::Graphic::Image* pImageHdrLeft, GR::Graphic::Image* pImageHdrCenter, GR::Graphic::Image* pImageHdrRight )
{
  m_pImageHdrLeft     = pImageHdrLeft;
  m_pImageHdrCenter   = pImageHdrCenter;
  m_pImageHdrRight    = pImageHdrRight;

  if ( ( m_pImageHdrLeft )
  &&   ( m_pImageHdrLeft->GetHeight() > m_HeaderHeight ) )
  {
    m_HeaderHeight = m_pImageHdrLeft->GetHeight();
  }
  if ( ( m_pImageHdrCenter )
  &&   ( m_pImageHdrCenter->GetHeight() > m_HeaderHeight ) )
  {
    m_HeaderHeight = m_pImageHdrCenter->GetHeight();
  }
  if ( ( m_pImageHdrRight )
  &&   ( m_pImageHdrRight->GetHeight() > m_HeaderHeight ) )
  {
    m_HeaderHeight = m_pImageHdrRight->GetHeight();
  }

}