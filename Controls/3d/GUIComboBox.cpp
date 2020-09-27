/*--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 28.08.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <Debug\debugclient.h>

#include "GUIComponentDisplayer.h"
#include "GUIComboBox.h"



/*-Variablen------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGUIComboBox::CGUIComboBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 cfType, DWORD dwId ) :
  CAbstractComboBox<CGUIComponent,CGUIEdit,CGUIButton,CGUIListBox>( iNewX, iNewY, iNewWidth, iNewHeight, cfType, dwId ),
  m_pTexture( NULL )
{

  RecalcClientRect();

  m_pEdit->SetLocation( 0, 0 );
  m_pEdit->SetSize( m_ClientRect.size().x - 20, 20 );
  m_pEdit->Enable( FALSE );

  m_pButtonDropDown->SetLocation( m_ClientRect.size().x - 20, 0 );
  m_pButtonDropDown->SetSize( 20, 20 );

}



/*-SetFont--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIComboBox::SetFont( CDX8Font* pFont )
{

  CGUIComponent::SetFont( pFont );

  m_pListBox->SetFont( pFont );
  m_pEdit->SetFont( pFont );
  m_pButtonDropDown->SetFont( pFont );

}



/*-SetTexture-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIComboBox::SetTexture( CDX8Texture* pTexture )
{

  m_pListBox->SetTextureSection( GUI::BT_BACKGROUND, pTexture );
  //m_pButtonDropDown->SetTexture( pTexture );
  m_pTexture = pTexture;

}



void CGUIComboBox::SetTextureSection( const GUI::eBorderType eType, const tTextureSection& TexSection )
{

  CAbstractComboBox<CGUIComponent,CGUIEdit,CGUIButton,CGUIListBox>::SetTextureSection( eType, TexSection );

  m_pListBox->SetTextureSection( eType, TexSection );

}



/*-DisplayOnPage--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGUIComboBox::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{

  /*
  //PrepareStatesForAlpha( pViewer, 0x60000000 );
  pViewer->SetTexture( 0, NULL );
  //pViewer->DrawQuad( D3DXVECTOR3( fOffsetX, fOffsetY, 0.0f ),
    //                 D3DXVECTOR3( m_iWidth, m_iHeight, 0.0f ),
      //               0x60000000 );

  DWORD   dwColor = 0xffffffff;

  if ( IsMouseInside() )
  {
    dwColor = 0xffff00 | ( m_dwAlpha << 24 );
  }
  else
  {
    dwColor = ( m_dwColor & 0x00ffffff ) | ( m_dwAlpha << 24 );
  }

  PrepareStatesForAlpha( pViewer, dwColor );

  if ( m_pFont )
  {
    int   iYOffset = -m_iFirstVisibleItem * m_iItemHeight;

    int   iItem = GetSelectedItem();
    if ( iItem != -1 )
    {
      pViewer->DrawBox( D3DXVECTOR3( fOffsetX, fOffsetY + iYOffset + iItem * m_iItemHeight, 0.0f ),
                        D3DXVECTOR3( m_ClientRect.size().x - m_pScrollBar->Width(), m_iItemHeight, 0.0f ),
                        0xffffa000 );
    }

    iItem = 0;
    tAbstractListBoxItemList::iterator    it( m_listItems.begin() );
    while ( it != m_listItems.end() )
    {
      tListBoxItem&   Item = *it;

      if ( iYOffset >= m_iHeight )
      {
        break;
      }
      if ( iYOffset >= 0.0f )
      {
        DWORD   dwColor = 0xffffffff;
        if ( iItem == m_iSelectedItem )
        {
          dwColor = 0xffffffff;
        }
        else if ( iItem == m_iMouseOverItem )
        {
          dwColor = 0xffffa000;
        }
        pViewer->DrawText( m_pFont, 
                        fOffsetX,
                        fOffsetY + iYOffset, 
                        Item.m_strText.c_str(),
                        dwColor );
      }

      iYOffset += m_iItemHeight;

      ++iItem;
      ++it;
    }
  }
  */

}



ICloneAble* CGUIComboBox::Clone()
{

  return new CGUIComboBox( *this );

}
