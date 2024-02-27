#include <Grafik/ImageFormate/ImageFormatManager.h>
#include <Grafik/ContextDescriptor.h>
#include <Grafik/Image.h>

#include <debug/debugclient.h>

#include <Math/SectionLayouter.h>

#include <IO/FileStream.h>

#include <set>

#include "X2dFont.h"
#include "X2dRenderer.h"
#include "XTextureSection.h"



X2dFont::XLetter::~XLetter()
{
}



X2dFont::X2dFont( X2dRenderer* pRenderer, GR::IEnvironment* pEnvironment ) :
  m_iWidth( 0 ),
  m_iHeight( 0 ),
  m_pRenderer( pRenderer ),
  m_pEnvironment( pEnvironment ),
  m_TransparentColor( 0 ),
  m_CreationFlags( 0 )
{
}



X2dFont::~X2dFont()
{
  while ( m_mapLetters.size() )
  {
    delete m_mapLetters.begin()->second;
    m_mapLetters.erase( m_mapLetters.begin() );
  }
}



bool X2dFont::LoadFont( const GR::String& Filename, GR::u32 Flags, GR::u32 TransparentColor )
{
  GR::IO::FileStream   fileFont;

  if ( !fileFont.Open( Filename, IIOStream::OT_READ_ONLY ) )
  {
    return false;
  }
  if ( fileFont.ReadU8() != GR::Graphic::IGFType::FONT )
  {
    return false;
  }

  GR::Graphic::eImageFormat ucFormat = (GR::Graphic::eImageFormat)fileFont.ReadU8();

  size_t      iChars = (size_t)fileFont.ReadU32();

  GR::u32     Chunk = 0;

  std::map<GR::u32,GR::Graphic::Image>      mapLetters;



  std::multiset<GR::tPoint>         ptLetterSizes;


  while ( ( Chunk = fileFont.ReadU32() ) )
  {
    if ( Chunk == 0x0001 )
    {
      // Image-Chunk

      for ( size_t i = 0; i < iChars; ++i )
      {
        // Zeichen
        GR::u32     Char = fileFont.ReadU32();

        GR::u32     Width = fileFont.ReadU32();
        GR::u32     Height = fileFont.ReadU32();

        GR::Graphic::Image& Image = mapLetters[Char];

        Image.Set( (GR::u16)Width, (GR::u16)Height, (GR::u8)GR::Graphic::ImageData::DepthFromImageFormat( ucFormat ), 0, TransparentColor );
        //Image.Set( Width, Height, 8, 0, TransparentColor );

        fileFont.ReadBlock( Image.GetData(), Image.GetSize() );

        ptLetterSizes.insert( GR::tPoint( Width, Height ) );
      }
    }
  }

  fileFont.Close();

  // auf eine Textur packen
  math::CSectionLayouter<GR::Graphic::Image*>   SectionLayouter( GR::tRect( 0, 0, 256, 256 ) );

  std::map<GR::u32,GR::Graphic::Image>::iterator    it( mapLetters.begin() );
  while ( it != mapLetters.end() )
  {
    GR::Graphic::Image&   Img = it->second;

    if ( !SectionLayouter.Insert( &Img, GR::tPoint( Img.GetWidth(), Img.GetHeight() ) ) )
    {
      dh::Log( "Failed to insert section (Letter %d), Section full!", it->first );
      return false;
    }

    ++it;
  }

  GR::Graphic::eImageFormat    formatToUse = GR::Graphic::IF_A8;

  m_CreationFlags = Flags;

  // wenn mˆglich Alpha beibehalten
  if ( m_pRenderer->IsTextureFormatOK( GR::Graphic::IF_A8 ) )
  {
    formatToUse = GR::Graphic::IF_A8;
  }
  else if ( m_pRenderer->IsTextureFormatOK( GR::Graphic::IF_A8R8G8B8 ) )
  {
    formatToUse = GR::Graphic::IF_A8R8G8B8;
  }
  else if ( m_pRenderer->IsTextureFormatOK( GR::Graphic::IF_A4R4G4B4 ) )
  {
    formatToUse = GR::Graphic::IF_A4R4G4B4;
  }

  XTexture*   pTexture = m_pRenderer->CreateTexture( 256, 256, formatToUse );
  if ( pTexture == NULL )
  {
    return false;
  }

  if ( ucFormat == GR::Graphic::IF_PALETTED )
  {
    // es wurde keine Palette mitgespeichert, deshalb Graustufen verwenden!
    ucFormat = GR::Graphic::IF_A8;
  }

  GR::Graphic::ContextDescriptor      cdPreTexture;

  cdPreTexture.CreateData( 256, 256, ucFormat );


  std::list<std::pair<GR::tPoint,GR::Graphic::Image*> >   listResult;

  SectionLayouter.Result( listResult );

  std::list<std::pair<GR::tPoint,GR::Graphic::Image*> >::iterator   itResult( listResult.begin() );
  while ( itResult != listResult.end() )
  {
    std::pair<GR::tPoint,GR::Graphic::Image*>&  Pair = *itResult;

    GR::Graphic::ContextDescriptor      cdLetter;

    cdLetter.Attach( Pair.second->GetWidth(), Pair.second->GetHeight(), Pair.second->GetLineSize(),
                     ucFormat, Pair.second->GetData() );

    cdLetter.CopyArea( 0, 0, cdLetter.Width(), cdLetter.Height(), 
                       Pair.first.x, Pair.first.y, 
                       &cdPreTexture );

    // find letter
    bool    bFound = false;

    std::map<GR::u32,GR::Graphic::Image>::iterator    it( mapLetters.begin() );
    while ( it != mapLetters.end() )
    {
      GR::Graphic::Image&   Img = it->second;

      if ( &Img == Pair.second )
      {
        //Img.Save( CMisc::printf( "komplettfont%c.igf", it->first ) );

        /*
        dh::Log( "Letter %d (%c) at %d,%d (%dx%d)",
                 it->first, it->first,
                 Pair.first.x, Pair.first.y,
                 Img.GetWidth(), Img.GetHeight() );
                 */

        XLetter* pLetter = new XLetter();

        pLetter->m_TextureSection = XTextureSection( pTexture, Pair.first.x, Pair.first.y, Img.GetWidth(), Img.GetHeight() );
        pLetter->m_iWidth = Img.GetWidth();
        pLetter->m_iXOffset = 0;
        
        m_mapLetters[it->first] = pLetter;
        bFound = true;
        break;
      }
      ++it;
    }

    if ( !bFound )
    {
      dh::Log( "Letter not found!!!" );

      Pair.second->Save( "futsch.igf" );
    }

    ++itResult;
  }

  if ( ( cdPreTexture.ImageFormat() == GR::Graphic::IF_X8R8G8B8 )
  &&   ( formatToUse == GR::Graphic::IF_A8 ) )
  {
    // Spezial-Wurst-> Alpha drin
    GR::Graphic::ContextDescriptor    cdMask;

    cdMask.CreateData( cdPreTexture.Width(), cdPreTexture.Height(), GR::Graphic::IF_A8 );

    for ( int j = 0; j < cdPreTexture.Height(); ++j )
    {
      for ( int i = 0; i < cdPreTexture.Width(); ++i )
      {
        cdMask.PutDirectPixel( i, j,cdPreTexture.GetPixel( i, j ) >> 24 );
      }
    }
    m_pRenderer->CopyDataToTexture( pTexture, cdMask );
  }
  else
  {
    cdPreTexture.ConvertSelfTo( formatToUse, 0, true, TransparentColor );
    m_pRenderer->CopyDataToTexture( pTexture, cdPreTexture );
  }

  m_TransparentColor = TransparentColor;

  return true;
}



bool X2dFont::LoadFontSquare( const GR::String& Filename, GR::u32 Flags, GR::u32 TransparentColor )
{
  IImageFormatManager*    pManager = (IImageFormatManager*)m_pEnvironment->Service( "ImageLoader" );
  if ( pManager == NULL )
  {
    return false;
  }
  GR::Graphic::ImageData*   pImageData = pManager->LoadData( Filename );
  if ( pImageData == NULL )
  {
    dh::Log( "X2dFont::LoadFontSquare File %s not found", Filename.c_str() );
    return false;
  }

  // Textur ist geladen, zerschnippeln und verfonten
  m_strFileName        = Filename;
  m_CreationFlags    = Flags | FLF_SQUARED;
  m_TransparentColor = TransparentColor;

  m_iWidth  = pImageData->Width() / 16;
  m_iHeight = pImageData->Height() / 16;

  GR::Graphic::eImageFormat      formatToUse = pImageData->ImageFormat();

  if ( ( pImageData->ImageFormat() == GR::Graphic::IF_A1R5G5B5 )
  ||   ( pImageData->ImageFormat() == GR::Graphic::IF_A8R8G8B8 ) )
  {
    Flags |= FLF_ALPHA_FULL;
  }

  if ( m_pRenderer->IsTextureFormatOK( formatToUse ) )
  {
  }
  else
  {
    if ( Flags & FLF_ALPHA_BIT )
    {
      if ( m_pRenderer->IsTextureFormatOK( GR::Graphic::IF_A1R5G5B5 ) )
      {
        formatToUse = GR::Graphic::IF_A1R5G5B5;
      }
    }
    if ( Flags & FLF_ALPHA_FULL )
    {
      if ( m_pRenderer->IsTextureFormatOK( GR::Graphic::IF_A8 ) )
      {
        formatToUse = GR::Graphic::IF_A8;
      }
      else if ( m_pRenderer->IsTextureFormatOK( GR::Graphic::IF_A8R8G8B8 ) )
      {
        formatToUse = GR::Graphic::IF_A8R8G8B8;
      }
      else if ( m_pRenderer->IsTextureFormatOK( GR::Graphic::IF_A4R4G4B4 ) )
      {
        formatToUse = GR::Graphic::IF_A4R4G4B4;
      }
      /*
      else if ( m_pRenderer->IsTextureFormatOK(IF_A8R3G3B2 ) )
      {
        formatToUse = IF_A8R3G3B2;
      }
      */
    }
  }

  // Texture erzeugen
  XTexture*       pTexture = m_pRenderer->LoadTexture( Filename, formatToUse, m_TransparentColor );

  if ( pTexture == NULL )
  {
    delete pImageData;
    return false;
  }

  FontFromTexture( Flags, pTexture, pImageData );


  delete pImageData;

  return true;
}



int X2dFont::FontSpacing()
{
  return 2;
}



int X2dFont::TextHeight( const GR::String& Text ) const
{
  GR::String     localCopy = Text;
  if ( localCopy.empty() )
  {
    localCopy = "\xc4\xd6\xdc\xdfyg";     //"ƒ÷‹ﬂyg";
  }
  int   iHeight = 0;
  for ( size_t i = 0; i < localCopy.length(); i++ )
  {
#ifdef UNICODE
    std::map<GR::u32,XLetter*>::const_iterator   it( m_mapLetters.find( (GR::u32)(GR::u16)localCopy[i] ) );
#else
    std::map<GR::u32,XLetter*>::const_iterator   it( m_mapLetters.find( (GR::u32)(GR::u8)localCopy[i] ) );
#endif
  
    if ( it == m_mapLetters.end() )
    {
      continue;
    }
    if ( it->second->m_TextureSection.m_Height > iHeight )
    {
      iHeight = it->second->m_TextureSection.m_Height;
    }
  }
  return iHeight;
}



int X2dFont::TextLength( const GR::String& Text ) const
{
  int       iWidth = 0;
  size_t    length = Text.length();
  for ( unsigned int i = 0; i < length; i++ )
  {
#ifdef UNICODE
    std::map<GR::u32, XLetter*>::const_iterator   it( m_mapLetters.find( ( GR::u32 )( GR::u16 )strText[i] ) );
#else
    std::map<GR::u32, XLetter*>::const_iterator   it( m_mapLetters.find( ( GR::u32 )( GR::u8 )Text[i] ) );
#endif

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



void X2dFont::Recreate()
{
  if ( !m_pRenderer )
  {
    return;
  }

  if ( m_CreationFlags & FLF_SQUARED )
  {
    //LoadFontSquare( m_strFileName.c_str(), m_CreationFlags, m_TransparentColor );
  }
  else
  {
    LoadFont( m_strFileName.c_str(), m_CreationFlags, m_TransparentColor );
  }
}



void X2dFont::Release()
{
  std::map<GR::u32,XLetter*>::iterator   it( m_mapLetters.begin() );
  while ( it != m_mapLetters.end() )
  {

    XLetter*   pLetter = it->second;

    delete pLetter;

    it++;
  }
  m_mapLetters.clear();
}



X2dFont::XLetter* X2dFont::Letter( const char cLetter )
{
  std::map<GR::u32,XLetter*>::iterator   it( m_mapLetters.find( cLetter ) );
  if ( it == m_mapLetters.end() )
  {
    return NULL;
  }
  return it->second;
}



void X2dFont::Dump()
{
  /*
  std::map<char,CBasicLetter*>::iterator    it( m_mapLetters.begin() );
  while ( it != m_mapLetters.end() )
  {
    CBasicLetter*   pLetter = it->second;

    dh::Log( "Letter %c = Letter %x, Texture %x, Surface %x", 
             it->first, 
             pLetter,
             pLetter->m_pTextureSection->m_pTexture,
             pLetter->m_pTextureSection->m_pTexture->m_Surface );

    ++it;
  }
  */
}



float X2dFont::DrawLetterDetail( int iX, int iY, GR::u32 cLetter, float fScaleX, float fScaleY, GR::u32 Color )
{

  std::map<GR::u32,XLetter*>::iterator   it( m_mapLetters.find( cLetter ) );
  if ( it == m_mapLetters.end() )
  {
    return 0;
  }

  XLetter*              pLetter = it->second;

  m_pRenderer->RenderTextureSectionColorKeyed( iX - pLetter->m_iXOffset, iY,
                                               pLetter->m_TextureSection, m_TransparentColor );
  return pLetter->m_TextureSection.m_Width * fScaleX;

}



int X2dFont::DrawLetter( int iX, int iY, GR::u32 cLetter, GR::u32 Color )
{
  std::map<GR::u32,XLetter*>::iterator   it( m_mapLetters.find( cLetter ) );
  if ( it == m_mapLetters.end() )
  {
    dh::Log( "DrawLetter Unused Letter 0x%x", cLetter );
    return 0;
  }

  if ( Color == 0xffffffff )
  {
    m_pRenderer->RenderTextureSectionColorKeyed( iX, iY, it->second->m_TextureSection, m_TransparentColor );
  }
  else
  {
    m_pRenderer->RenderTextureSectionColorKeyedColorized( iX, iY, it->second->m_TextureSection, m_TransparentColor, Color );
  }
  return it->second->m_TextureSection.m_Width;
}



void X2dFont::DrawText( int iX, int iY, const GR::String& strText, GR::u32 Color )
{
  if ( m_mapLetters.empty() )
  {
    Recreate();
    if ( m_mapLetters.empty() )
    {
      dh::Log( "CBasicFont::DrawText no letters" );
      return;
    }
  }
#ifdef UNICODE
  GR::String::const_iterator   it( strText.begin() );
  while ( it != strText.end() )
  {
    iX += DrawLetter( iX, iY, (GR::u32)(GR::u16)*it, Color ) + FontSpacing();

    ++it;
  }
#else
  for ( size_t i = 0; i < strText.length(); ++i )
  {
    iX += DrawLetter( iX, iY, (GR::u32)(GR::u8)strText[i], Color ) + FontSpacing();
  }
#endif
}



void X2dFont::DrawText( int iX, int iY, const GR::String& strText, float fScaleX, float fScaleY, GR::u32 Color )
{
  float   fX = (float)iX;

  for ( size_t i = 0; i < strText.length(); ++i )
  {
#ifdef UNICODE
    fX += FontSpacing() * fScaleX + DrawLetterDetail( (int)fX, iY, (GR::u32)(GR::u16)strText[i], fScaleX, fScaleY, Color );
#else
    fX += FontSpacing() * fScaleX + DrawLetterDetail( (int)fX, iY, (GR::u32)(GR::u8)strText[i], fScaleX, fScaleY, Color );
#endif
  }
}



void X2dFont::FontFromTexture( GR::u32 Flags, XTexture* pTextureTarget, GR::Graphic::ImageData* pImageData )
{
  int     iFontLines = 8;

  bool    bOneFont = false;

  if ( Flags & FLF_SQUARED_ONE_FONT )
  {
    bOneFont = true;
    iFontLines = 16;
  }

  m_iWidth  = pImageData->Width() / 16;
  m_iHeight = pImageData->Height() / 16;

  for ( int j = 0; j < iFontLines; j++ )
  {
    for ( int i = 0; i < 16; i++ )
    {
      XTextureSection*   pTextureInfo = NULL;

      XLetter*           pNewLetter = NULL;

      try
      {
        pNewLetter = new XLetter();
      }
      catch ( std::exception& )
      {
        break;
      }

      GR::i32               wMinX = m_iWidth,
                            wMaxX = 0;


      // Buchstabenbreiten ausrechnen (BAUSTELLE - sp‰ter im Fontgenerator!)
      int   iLetterX = 0;
      do
      {
        for ( int iLetterY = 0; iLetterY < m_iHeight; ++iLetterY )
        {
          if ( ( pImageData->GetPixelRGB( i * 16 + iLetterX, j * 16 + iLetterY ) & 0xffffff ) != ( m_TransparentColor & 0xffffff ) )
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
          if ( ( pImageData->GetPixelRGB( i * 16 + iLetterX, j * 16 + iLetterY ) & 0xffffff ) != ( m_TransparentColor & 0xffffff ) )
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
      
      pNewLetter->m_TextureSection = XTextureSection( pTextureTarget, m_iWidth * i + wMinX, j * 16, pNewLetter->m_iWidth, m_iHeight );

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

  // Space auf Breite eines A
  XLetter*    pSpace = m_mapLetters[' '];

  pSpace->m_iWidth = m_mapLetters['I']->m_iWidth;
  pSpace->m_TextureSection.m_Width = m_mapLetters['I']->m_TextureSection.m_Width;
}



void X2dFont::WrapText( const GR::String& strText, GR::tRect& rectText, std::vector<GR::String>& vectText ) const
{
  vectText.clear();

  // jetzt Text schnippeln
  GR::String    strDummy = strText;

  GR::String    strNewLine;

  size_t        iPos = -1;

  do
  {
    strNewLine = "";

    ++iPos;
    if ( iPos >= strDummy.length() )
    {
      vectText.push_back( strDummy );
      break;
    }
    if ( ( strDummy[iPos] == '\\' )   // erzwungener Umbruch
    &&   ( iPos < strDummy.length() )
    &&   ( strDummy[iPos + 1] == 'n' ) )
    {
      vectText.push_back( strDummy.substr( 0, iPos ) );
      strDummy = strDummy.substr( iPos + 2 );
      iPos = -1;
    }
    else if ( strDummy[iPos] == '\n' )   // erzwungener Umbruch
    {
      vectText.push_back( strDummy.substr( 0, iPos ) );
      strDummy = strDummy.substr( iPos + 1 );
      iPos = -1;
    }
    else if ( TextLength( strDummy.substr( 0, iPos ).c_str() ) >= rectText.Width() )
    {
      if ( strDummy[iPos] != '\n' )
      {
        // hier muﬂ noch eine passende Stelle (Leerzeichen) gefunden werden
        size_t   iNewPos = iPos;
        do
        {
          iNewPos--;
          if ( iNewPos <= 0 )
          {
            iNewPos = iPos;
            break;
          }
        }
        while ( strDummy[iNewPos] != ' ' );
        iPos = iNewPos;
      }
      vectText.push_back( strDummy.substr( 0, iPos ) );
      strDummy = strDummy.substr( iPos + 1 );
      iPos = -1;
    }
  }
  while ( strDummy.length() );

  GR::i32   iMaxWidth = 0;

  for ( size_t i = 0; i < vectText.size(); ++i )
  {
    iMaxWidth = math::maxValue( iMaxWidth, TextLength( vectText[i].c_str() ) );
  }

  rectText.Width( iMaxWidth );
  rectText.Height( TextHeight() * (int)vectText.size() );
}



GR::Graphic::eImageFormat X2dFont::FontImageFormat() const
{
  if ( m_mapLetters.empty() )
  {
    return GR::Graphic::IF_UNKNOWN;
  }
  return m_mapLetters.begin()->second->m_TextureSection.m_pTexture->m_ImageFormat;
}