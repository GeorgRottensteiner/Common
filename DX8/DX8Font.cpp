#include "DX8Font.h"
#include "DX8Viewer.h"
#include "TextureSection.h"

#include <Misc/Misc.h>

#include <Grafik/ImageFormate/ImageFormatManager.h>

#include <debug\debugclient.h>



CDX8Letter::~CDX8Letter()
{

  if ( m_pTextureSection != NULL )
  {
    delete m_pTextureSection;
    m_pTextureSection = NULL;
  }

}



CDX8Font::CDX8Font( CD3DViewer* pViewer ) :
  m_iWidth( 0 ),
  m_iHeight( 0 ),
  m_pViewer( pViewer )
{

  m_pViewer->m_vectFonts.push_back( this );

}



CDX8Font::~CDX8Font()
{

  while ( m_mapLetters.size() )
  {
    delete m_mapLetters.begin()->second;
    m_mapLetters.erase( m_mapLetters.begin() );
  }

}



bool CDX8Font::LoadFont( const char* szFileName, DWORD dwFlags, DWORD dwTransparentColor )
{

  GR::Graphic::ImageData* pImageData = ImageFormatManager::Instance().LoadData( szFileName );

  if ( pImageData == NULL )
  {
    return false;
  }

  if ( pImageData->ImageFormat() != GR::Graphic::IF_PALETTED )
  {
    return false;
  }

  GR::Graphic::ImageData*   pNewData = new GR::Graphic::ImageData();
  pNewData->CreateData( pImageData->Width(), pImageData->Height(), GR::Graphic::IF_A8, pImageData->LineOffsetInBytes() );
  memcpy( pNewData->Data(), pImageData->Data(), pImageData->DataSize() );
  delete pImageData;
  pImageData = pNewData;

  // Textur ist geladen, zerschnippeln und verfonten
  WORD                      wZeichenAnzahl;


  m_strFileName          = szFileName;
  m_dwCreationFlags      = dwFlags;
  m_dwTransparentColor   = dwTransparentColor;
  if ( ( pImageData->Height() % 224 ) == 0 ) 
  {
    wZeichenAnzahl = 224;
  }
  else
  {
    wZeichenAnzahl = 96;
  }
  m_iWidth  = pImageData->Width();
  m_iHeight = pImageData->Height() / wZeichenAnzahl;

  for ( int iZeichen = 0; iZeichen < wZeichenAnzahl; iZeichen++ )
  {
    LPDIRECT3DTEXTURE8    resultingSurface  = NULL;

    D3DFORMAT             formatToUse = m_pViewer->m_pCurrentModeInfo->m_d3dfPixel;

    tTextureSection*      pTextureInfo = NULL;

    CDX8Letter*           pNewLetter = NULL;
    try
    {
      pNewLetter = new CDX8Letter();
    }
    catch ( std::exception& )
    {
      delete pTextureInfo;
      break;
    }


    WORD                  wMinX = (WORD)pImageData->Width(),
                          wMaxX = 0;


    for ( int j = 0; j < m_iHeight; j++ )
    {
      BYTE*     pData = ( (BYTE*)pImageData->Data() ) + m_iWidth * ( iZeichen * m_iHeight + j );

      for ( int x = 0; x < m_iWidth; x++ )
      {
        BYTE ucColor = *pData;//++;
        pData++;
        if ( ucColor )
        {
          // ein Pixel (der Font ist eigentlich nur eine Alphamaske
          if ( wMinX > x )
          {
            wMinX = x;
          }
          if ( wMaxX < x )
          {
            wMaxX = x;
          }
        }
      }
    }

    wMinX = 0;    // unerklärlich? // MARKER

    pNewLetter->m_iWidth    = wMaxX - wMinX + 1;
    pNewLetter->m_iXOffset  = wMinX;

    pNewLetter->m_pTextureSection = pTextureInfo;

    resultingSurface = NULL;

    dwFlags |= DX8::TF_ALPHA_MASK;

    if ( m_pViewer->IsTextureFormatOk( D3DFMT_A8 ) )
    {
      formatToUse = D3DFMT_A8;
    }
    else if ( m_pViewer->IsTextureFormatOk( D3DFMT_A8R8G8B8 ) )
    {
      formatToUse = D3DFMT_A8R8G8B8;
    }
    else if ( m_pViewer->IsTextureFormatOk( D3DFMT_A4R4G4B4 ) )
    {
      formatToUse = D3DFMT_A4R4G4B4;
    }
    else if ( m_pViewer->IsTextureFormatOk( D3DFMT_A8R3G3B2 ) )
    {
      formatToUse = D3DFMT_A8R3G3B2;
    }

    // Texture erzeugen
    // die passende Größe hat nicht geklappt
    CDX8Texture*    pFontTexture = m_pViewer->CreateTexture( pImageData->Width(), m_iHeight, formatToUse );
    pFontTexture->m_bIsLoaded = true;

    char      szEintrag[2 * MAX_PATH];
    wsprintf( szEintrag, "CreateTexture%s%x%x%x%x", szFileName, pFontTexture->m_dwCreationFlags, 0, 1, iZeichen );
    CTextureManager::Instance().Insert( szEintrag, pFontTexture );

    // eventuell (sehr wahrscheinlich sogar) angepaßte Größen der Surfaces holen
    pTextureInfo = new tTextureSection( pFontTexture, wMinX, 0, pNewLetter->m_iWidth, m_iHeight );
    pNewLetter->m_pTextureSection = pTextureInfo;

    /*
    LogText( "textureinfo für zeichen %d (%c) %dx%d (%dx%d)", iZeichen, iZeichen, 
          pTextureInfo->m_sizeRealSurfaceExtents.x,
          pTextureInfo->m_sizeRealSurfaceExtents.y,
          pTextureInfo->m_sizeVirtualSurfaceExtents.x,
          pTextureInfo->m_sizeVirtualSurfaceExtents.y );*/

    dwTransparentColor &= 0x00ffffff;

    if ( FAILED( m_pViewer->CopyImageToSingleSurface( pImageData, 
                                           pFontTexture->m_Surface, 
                                           pImageData->Width(),
                                           m_iHeight,
                                           dwTransparentColor,
                                           dwFlags,
                                           0,
                                           iZeichen * m_iHeight,
                                           pImageData->Width() - 1,
                                           iZeichen * m_iHeight + m_iHeight - 1 ) ) )
    {
      return false;
    }

    m_mapLetters[(char)( 32 + iZeichen )] = pNewLetter;
  }

  delete pImageData;

  // Breite des Space nach dem kleinen "i" anpassen
  std::map<char,CDX8Letter*>::iterator    itSpace( m_mapLetters.find( (char)32 ) ),
                                          itSmallI( m_mapLetters.find( (char)'i' ) );
  
  if ( ( itSpace != m_mapLetters.end() )
  &&   ( itSmallI != m_mapLetters.end() ) )
  {
    itSpace->second->m_iWidth = itSmallI->second->m_iWidth * 3 / 2;
  }

  return true;

}



bool CDX8Font::LoadFontSquare( const char *szFileName, DWORD dwFlags, DWORD dwTransparentColor )
{

  GR::Graphic::ImageData*   pImageData = ImageFormatManager::Instance().LoadData( szFileName );
  if ( pImageData == NULL )
  {
    return false;
  }

  dwFlags |= DX8::TF_SQUARED;

  // Textur ist geladen, zerschnippeln und verfonten
  m_strFileName        = szFileName;
  m_dwCreationFlags    = dwFlags;
  m_dwTransparentColor = dwTransparentColor;

  m_iWidth  = pImageData->Width() / 16;
  m_iHeight = pImageData->Height() / 16;

  D3DFORMAT             formatToUse = m_pViewer->m_pCurrentModeInfo->m_d3dfPixel;

  if ( ( pImageData->ImageFormat() == GR::Graphic::IF_A1R5G5B5 )
  ||   ( pImageData->ImageFormat() == GR::Graphic::IF_A8R8G8B8 ) )
  {
    dwFlags |= DX8::TF_ALPHA_MASK;
  }

  if ( dwFlags & DX8::TF_HOLES )
  {
    if ( m_pViewer->IsTextureFormatOk( D3DFMT_A1R5G5B5 ) )
    {
      formatToUse = D3DFMT_A1R5G5B5;
    }
  }
  if ( dwFlags & DX8::TF_ALPHA_MASK )
  {
    if ( m_pViewer->IsTextureFormatOk( D3DFMT_A8 ) )
    {
      formatToUse = D3DFMT_A8;
    }
    else if ( m_pViewer->IsTextureFormatOk( D3DFMT_A8R8G8B8 ) )
    {
      formatToUse = D3DFMT_A8R8G8B8;
    }
    else if ( m_pViewer->IsTextureFormatOk( D3DFMT_A4R4G4B4 ) )
    {
      formatToUse = D3DFMT_A4R4G4B4;
    }
    else if ( m_pViewer->IsTextureFormatOk( D3DFMT_A8R3G3B2 ) )
    {
      formatToUse = D3DFMT_A8R3G3B2;
    }
  }

  // Texture erzeugen
  CDX8Texture*    pTexture = m_pViewer->CreateTexture( pImageData->Width(), pImageData->Height(), formatToUse );
  if ( pTexture == NULL )
  {
    return false;
  }

  char      szEintrag[2 * MAX_PATH];
  wsprintf( szEintrag, "CreateTexture%s%x%x%x", szFileName, pTexture->m_dwCreationFlags, 0, 1 );
  CTextureManager::Instance().Insert( szEintrag, pTexture );

  if ( FAILED( m_pViewer->CopyImageToSingleSurface( pImageData, 
                                         pTexture->m_Surface, 
                                         pImageData->Width(),
                                         pImageData->Height(),
                                         dwTransparentColor,
                                         dwFlags,
                                         0, 0,
                                         pImageData->Width() - 1,
                                         pImageData->Height() - 1 ) ) )
  {
    delete pImageData;
    return false;
  }

  // jein
  pTexture->m_bIsLoaded = true;



  // eventuell (sehr wahrscheinlich sogar) angepaßte Größen der Surfaces holen
  int     iFontLines = 8;

  bool    bOneFont = false;

  if ( dwFlags & DX8::TF_SQUARED_ONE_FONT )
  {
    bOneFont = true;
    iFontLines = 16;
  }

  

  for ( int j = 0; j < iFontLines; j++ )
  {
    for ( int i = 0; i < 16; i++ )
    {
      tTextureSection*      pTextureInfo = NULL;

      CDX8Letter*           pNewLetter = NULL;

      try
      {
        pNewLetter = new CDX8Letter();
      }
      catch ( std::exception& )
      {
        break;
      }

      WORD                  wMinX = (WORD)m_iWidth,
                            wMaxX = 0;


      // Buchstabenbreiten ausrechnen (BAUSTELLE - später im Fontgenerator!)
      int   iLetterX = 0;
      do
      {
        for ( int iLetterY = 0; iLetterY < m_iHeight; ++iLetterY )
        {
          if ( pImageData->GetPixelRGB( i * 16 + iLetterX, j * 16 + iLetterY ) != dwTransparentColor )
          {
            // Pixel gefunden
            wMinX = iLetterX;
            break;
          }
        }
        if ( wMinX < m_iWidth )
        {
          // in diesem Durchlauf ein Pixel gefunden
          break;
        }
        iLetterX++;
        if ( iLetterX >= m_iWidth )
        {
          break;
        }
      }
      while ( true );

      iLetterX = m_iWidth - 1;
      do
      {
        for ( int iLetterY = 0; iLetterY < m_iHeight; ++iLetterY )
        {
          if ( pImageData->GetPixelRGB( i * 16 + iLetterX, j * 16 + iLetterY ) != dwTransparentColor )
          {
            // Pixel gefunden
            wMaxX = iLetterX;
            break;
          }
        }
        if ( wMaxX > 0 )
        {
          // in diesem Durchlauf kein Pixel mehr gefunden
          break;
        }
        iLetterX--;
        if ( iLetterX < 0 )
        {
          break;
        }
      }
      while ( true );

      if ( wMinX > wMaxX )
      {
        wMinX = 0;
        // BAUSTELLE - pfui, sowas hardcoden -> Breite eines Space
        wMaxX = ( m_iWidth - 1 ) / 2;
      }

      pNewLetter->m_iXOffset  = wMinX;
      pNewLetter->m_iWidth    = wMaxX - wMinX + 1;

      pTextureInfo = new tTextureSection( pTexture, m_iWidth * i + wMinX, j * 16, pNewLetter->m_iWidth, m_iHeight );

      pNewLetter->m_pTextureSection = pTextureInfo;

      //dwTransparentColor &= 0x00ffffff;

      if ( bOneFont )
      {
        m_mapLetters[(char)( i + j * 16 )] = pNewLetter;
      }
      else
      {
        m_mapLetters[(char)( 32 + i + j * 16 )] = pNewLetter;
      }
    }
  }

  delete pImageData;

  return true;

}



/*-DrawLetterDetail-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

float CD3DViewer::DrawLetterDetail( CDX8Font *pFont, int iX, int iY, char cLetter, float fScaleX, float fScaleY, DWORD dwColor1, DWORD dwColor2, DWORD dwColor3, DWORD dwColor4 )
{

  if ( pFont == NULL )
  {
    return 0;
  }
  std::map<char,CDX8Letter*>::iterator   it( pFont->m_mapLetters.find( cLetter ) );
  
  if ( it == pFont->m_mapLetters.end() )
  {
    return 0;
  }

  tTextureSection*      pSectionInfo = it->second->m_pTextureSection;
  if ( pSectionInfo == NULL )
  {
    return 0;
  }


  struct CUSTOMVERTEX
  {
      D3DXVECTOR3 position; // The position
      float       fRHW;
      D3DCOLOR    color;    // The color
      FLOAT       tu, tv;   // The texture coordinates
  };

  CUSTOMVERTEX          vertData[4];

  float   fWidth = it->second->m_iWidth * fScaleX,
          fHeight = pFont->m_iHeight * fScaleY;

  SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  SetTexture( 0, pSectionInfo->m_pTexture );

  float     fX = float(iX),// - it->second->m_iXOffset),
            fY = float(iY),
            fDeltaX = -0.5f,
            fDeltaY = -0.5f,
            fZ = 0.0f;

  vertData[0].position.x = fX + fDeltaX;
  vertData[0].position.y = fY + fDeltaY;
  vertData[0].position.z = fZ;
  vertData[0].fRHW = 1.0f;
  vertData[0].color = dwColor1;
  vertData[0].tu = pSectionInfo->m_TU[0];
  vertData[0].tv = pSectionInfo->m_TV[0];

  vertData[1].position.x = fX + fWidth + fDeltaX;
  vertData[1].position.y = fY + fDeltaY;
  vertData[1].position.z = fZ;
  vertData[1].fRHW = 1.0f;
  vertData[1].color = dwColor2;
  vertData[1].tu = pSectionInfo->m_TU[1];
  vertData[1].tv = pSectionInfo->m_TV[0];

  vertData[2].position.x = fX + fDeltaX;
  vertData[2].position.y = fY + fHeight + fDeltaY;
  vertData[2].position.z = fZ;
  vertData[2].fRHW = 1.0f;
  vertData[2].color = dwColor3;
  vertData[2].tu = pSectionInfo->m_TU[0];
  vertData[2].tv = pSectionInfo->m_TV[1];

  vertData[3].position.x = fX + fWidth + fDeltaX;
  vertData[3].position.y = fY + fHeight + fDeltaY;
  vertData[3].position.z = fZ;
  vertData[3].fRHW = 1.0f;
  vertData[3].color = dwColor4;
  vertData[3].tu = pSectionInfo->m_TU[1];
  vertData[3].tv = pSectionInfo->m_TV[1];
    
  m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( vertData[0] ) );

  return floorf(fWidth);

}



/*-DrawLetter-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CD3DViewer::DrawLetter( CDX8Font* pFont, int iX, int iY, char cLetter, DWORD dwColor )
{

  if ( pFont == NULL )
  {
    return 0;
  }
  std::map<char,CDX8Letter*>::iterator   it( pFont->m_mapLetters.find( cLetter ) );
  
  if ( it == pFont->m_mapLetters.end() )
  {
    return 0;
  }

  tTextureSection*      pSectionInfo = it->second->m_pTextureSection;


  struct CUSTOMVERTEX
  {
      D3DXVECTOR3 position; // The position
      float       fRHW;
      D3DCOLOR    color;    // The color
      FLOAT       tu, tv;   // The texture coordinates
  };

  CUSTOMVERTEX          vertData[4];

  // MARKER
  float   fWidth  = (float)it->second->m_iWidth,
          fHeight = (float)pFont->m_iHeight;

  if ( FAILED( SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 ) ) )
  {
    dh::Log( "CD3DViewer::DrawLetter SetVertexshader failed\n" );
  }
  SetTexture( 0, pSectionInfo->m_pTexture );

  float     fX = (float)(iX),
            fY = (float)(iY),
            fDeltaX = -0.5f,
            fDeltaY = -0.5f,
            fZ = 0.0f,
            fTU1 = pSectionInfo->m_TU[0],
            fTV1 = pSectionInfo->m_TV[0],
            fTU2 = pSectionInfo->m_TU[1],
            fTV2 = pSectionInfo->m_TV[1];

  vertData[0].position.x = fX + fDeltaX;
  vertData[0].position.y = fY + fDeltaY;
  vertData[0].position.z = fZ;
  vertData[0].color = dwColor;
  vertData[0].fRHW = 1.0f;
  vertData[0].tu = fTU1;
  vertData[0].tv = fTV1;

  vertData[1].position.x = fX + fWidth + fDeltaX;
  vertData[1].position.y = fY + fDeltaY;
  vertData[1].position.z = fZ;
  vertData[1].fRHW = 1.0f;
  vertData[1].color = dwColor;
  vertData[1].tu = fTU2;
  vertData[1].tv = fTV1;

  vertData[2].position.x = fX + fDeltaX;
  vertData[2].position.y = fY + fHeight + fDeltaY;
  vertData[2].position.z = fZ;
  vertData[2].fRHW = 1.0f;
  vertData[2].color = dwColor;
  vertData[2].tu = fTU1;
  vertData[2].tv = fTV2;

  vertData[3].position.x = fX + fWidth + fDeltaX;
  vertData[3].position.y = fY + fHeight + fDeltaY;
  vertData[3].position.z = fZ;
  vertData[3].fRHW = 1.0f;
  vertData[3].color = dwColor;
  vertData[3].tu = fTU2;
  vertData[3].tv = fTV2;
    
  if ( FAILED( m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertData, sizeof( vertData[0] ) ) ) )
  {
    dh::Log( "CD3DViewer::DrawLetter failed\n" );
  }

  return it->second->m_iWidth;

}



/*-DrawText-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DrawText( CDX8Font* pFont, int iX, int iY, const char *szText, DWORD dwColor )
{

  if ( pFont == NULL )
  {
    dh::Log( "CD3DViewer::DrawText no font\n" );
    return;
  }
  if ( pFont->m_mapLetters.size() == 0 )
  {
    pFont->Recreate();
    if ( pFont->m_mapLetters.size() == 0 )
    {
      dh::Log( "CD3DViewer::DrawText no letters\n" );
      return;
    }
  }
  for ( unsigned int i = 0; i < strlen( szText ); i++ )
  {
    iX += DrawLetter( pFont, iX, iY, szText[i], dwColor ) + 2;
  }

}



/*-DrawFreeText---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DrawFreeText( CDX8Font* pFont, D3DXVECTOR3& vectPos, const char* szText, D3DXVECTOR3& vectPlaneAndScale, DWORD dwColor )
{

  if ( pFont == NULL )
  {
    dh::Log( "CD3DViewer::DrawFreeText no font\n" );
    return;
  }
  if ( pFont->m_mapLetters.size() == 0 )
  {
    pFont->Recreate();
    if ( pFont->m_mapLetters.size() == 0 )
    {
      dh::Log( "CD3DViewer::DrawFreeText no letters\n" );
      return;
    }
  }
  for ( unsigned int i = 0; i < strlen( szText ); i++ )
  {
    std::map<char,CDX8Letter*>::iterator   it( pFont->m_mapLetters.find( szText[i] ) );
    if ( it == pFont->m_mapLetters.end() )
    {
      continue;
    }
    CDX8Letter*   pLetter = it->second;

    SetTexture( 0, pLetter->m_pTextureSection->m_pTexture );

    float   fLetterWidth = vectPlaneAndScale.x * (float)pLetter->m_iWidth / pLetter->m_pTextureSection->m_pTexture->m_vSize.x * 16.0f;

    DrawFreeQuadDetail( vectPos, 
                        D3DXVECTOR3( vectPos.x + fLetterWidth, vectPos.y, vectPos.z ),
                        D3DXVECTOR3( vectPos.x, vectPos.y + vectPlaneAndScale.y, vectPos.z ),
                        D3DXVECTOR3( vectPos.x + fLetterWidth, vectPos.y + vectPlaneAndScale.y, vectPos.z ),
                        pLetter->m_pTextureSection->m_TU[0],
                        pLetter->m_pTextureSection->m_TV[0],
                        pLetter->m_pTextureSection->m_TU[1],
                        pLetter->m_pTextureSection->m_TV[1],
                        dwColor );

    vectPos.x += fLetterWidth;
  }

}



/*-DrawText-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DrawText( CDX8Font* pFont, int iX, int iY, const char *szText, float fScaleX, float fScaleY, DWORD dwColor1, DWORD dwColor2, DWORD dwColor3, DWORD dwColor4 )
{

  if ( pFont == NULL )
  {
    return;
  }
  float   fX = float( iX );
  for ( unsigned int i = 0; i < strlen( szText ); i++ )
  {
    fX += 2.0f * fScaleX + DrawLetterDetail( pFont, int( fX ), int( iY ), szText[i], fScaleX, fScaleY, dwColor1, dwColor2, dwColor3, dwColor4 );
  }

}



/*-FontSpacing----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CDX8Font::FontSpacing()
{

  return 2;

}



/*-TextLength-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CDX8Font::TextLength( const TCHAR* szText )
{

  int   iWidth = 0;
  for ( unsigned int i = 0; i < strlen( szText ); i++ )
  {
    std::map<char,CDX8Letter*>::iterator   it( m_mapLetters.find( szText[i] ) );
  
    if ( it == m_mapLetters.end() )
    {
      continue;
    }
    iWidth += it->second->m_iWidth + 2;
  }
  if ( iWidth )
  {
    iWidth -= 2;
  }

  return iWidth;

}



int CDX8Font::TextHeight( const TCHAR* szText )
{

  if ( szText == NULL )
  {
    static char*   szDummy = "ÄÖÜßyg";

    szText = szDummy;
  }
  int   iHeight = 0;
  for ( unsigned int i = 0; i < strlen( szText ); i++ )
  {
    std::map<char,CDX8Letter*>::iterator   it( m_mapLetters.find( szText[i] ) );
  
    if ( it == m_mapLetters.end() )
    {
      continue;
    }
    tTextureSection*    pSectionInfo = it->second->m_pTextureSection;

    if ( pSectionInfo->m_Height > iHeight )
    {
      iHeight = pSectionInfo->m_Height;
    }
  }

  return iHeight;

}



void CD3DViewer::DestroyFont( CDX8Font* pFont )
{

  std::vector<CDX8Font*>::iterator    it( m_vectFonts.begin() );

  while ( it != m_vectFonts.end() )
  {
    if ( *it == pFont )
    {
      delete pFont;
      m_vectFonts.erase( it );
      return;
    }
    it++;
  }

}



void CD3DViewer::DestroyAllFonts()
{

  for ( unsigned int i = 0; i < m_vectFonts.size(); i++ )
  {
    delete m_vectFonts[i];
  }
  m_vectFonts.clear();

}



BOOL CD3DViewer::RecreateFonts()
{

  ReleaseFonts();

  for ( unsigned int i = 0; i < m_vectFonts.size(); i++ )
  {
    CDX8Font      *pFont = m_vectFonts[i];

    pFont->Recreate();
  }
  return TRUE;

}



/*-Recreate-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Font::Recreate()
{

  if ( !m_pViewer )
  {
    return;
  }

  if ( m_dwCreationFlags & DX8::TF_SQUARED )
  {
    LoadFontSquare( m_strFileName.c_str(), m_dwCreationFlags, m_dwTransparentColor );
  }
  else
  {
    LoadFont( m_strFileName.c_str(), m_dwCreationFlags, m_dwTransparentColor );
  }

}



/*-Release--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Font::Release()
{

  std::map<char,CDX8Letter*>::iterator   it( m_mapLetters.begin() );

  while ( it != m_mapLetters.end() )
  {

    CDX8Letter*   pLetter = it->second;

    delete pLetter;

    it++;
  }
  m_mapLetters.clear();

}



/*-ReleaseFonts---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::ReleaseFonts()
{

  for ( unsigned int i = 0; i < m_vectFonts.size(); i++ )
  {
    CDX8Font*     pFont = m_vectFonts[i];

    pFont->Release();
  }

}



CDX8Letter* CDX8Font::Letter( const char cLetter )
{

  std::map<char,CDX8Letter*>::iterator   it( m_mapLetters.find( cLetter ) );
  if ( it == m_mapLetters.end() )
  {
    return NULL;
  }
  return it->second;

}



void CDX8Font::Dump()
{

  std::map<char,CDX8Letter*>::iterator    it( m_mapLetters.begin() );
  while ( it != m_mapLetters.end() )
  {
    CDX8Letter*   pLetter = it->second;

    dh::Log( "Letter %c = Letter %x, Texture %x, Surface %x", 
             it->first, 
             pLetter,
             pLetter->m_pTextureSection->m_pTexture,
             pLetter->m_pTextureSection->m_pTexture->m_Surface );

    ++it;
  }

}