#include <Grafik/ImageFormate/ImageFormatManager.h>
#include <Grafik/ContextDescriptor.h>

#include <debug/debugclient.h>

#include <IO/FileStream.h>

#include <Grafik/Image.h>

#include <Misc/Misc.h>

#include <Math/SectionLayouter.h>

#include <set>

#include "XBasicFont.h"
#include "XRenderer.h"
#include "XTextureSection.h"


#undef DrawText


XBasicFont::XBasicFont( XRenderer* pRenderer, GR::IEnvironment* pEnvironment ) :
  m_Width( 0 ),
  m_Height( 0 ),
  m_pRenderer( pRenderer ),
  m_pEnvironment( pEnvironment ),
  m_CreationFlags( 0 )
{

}



XBasicFont::~XBasicFont()
{
  while ( m_Letters.size() )
  {
    XLetter*    pLetter = m_Letters.begin()->second;
    delete pLetter;

    m_Letters.erase( m_Letters.begin() );
  }
}



bool XBasicFont::LoadFontSquare( const GR::String& FileName, GR::u32 Flags, GR::u32 TransparentColor )
{
  return LoadFontSquare( FileName.c_str(), Flags, TransparentColor );
}



bool XBasicFont::LoadFontSquare( const char* FileName, GR::u32 Flags, GR::u32 TransparentColor )
{
  if ( m_pEnvironment == NULL )
  {
    return false;
  }
  ImageFormatManager*    pManager = (ImageFormatManager*)m_pEnvironment->Service( "ImageLoader" );
  if ( pManager == NULL )
  {
    return false;
  }
  GR::Graphic::ImageData*   pImageData = pManager->LoadData( FileName );
  if ( pImageData == NULL )
  {
    dh::Log( "XBasicFont::LoadFontSquare File %s not found", FileName );
    return false;
  }

  // Textur ist geladen, zerschnippeln und verfonten
  m_FileName          = FileName;
  m_CreationFlags     = Flags | FLF_SQUARED;
  m_TransparentColor  = TransparentColor;

  m_Width  = pImageData->Width() / 16;
  m_Height = pImageData->Height() / 16;

  GR::Graphic::eImageFormat      formatToUse = GR::Graphic::IF_A8R8G8B8;

  if ( ( pImageData->ImageFormat() == GR::Graphic::IF_A1R5G5B5 )
  ||   ( pImageData->ImageFormat() == GR::Graphic::IF_A8R8G8B8 ) )
  {
    Flags |= FLF_ALPHA_FULL;
  }

  if ( Flags & FLF_ALPHA_BIT )
  {
    if ( m_pRenderer->IsTextureFormatOK( GR::Graphic::IF_A1R5G5B5 ) )
    {
      formatToUse = GR::Graphic::IF_A1R5G5B5;
    }
    else if ( m_pRenderer->IsTextureFormatOK( GR::Graphic::IF_A8R8G8B8 ) )
    {
      formatToUse = GR::Graphic::IF_A8R8G8B8;
    }
  }
  if ( Flags & FLF_ALPHA_FULL )
  {
    /*
    if ( m_pRenderer->IsTextureFormatOK( IF_A8 ) )
    {
      formatToUse = IF_A8;
    }
    else*/
    if ( m_pRenderer->IsTextureFormatOK( GR::Graphic::IF_A8R8G8B8 ) )
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

  // Texture erzeugen
  XTexture*       pTexture = m_pRenderer->LoadTexture( FileName, formatToUse, m_TransparentColor );

  if ( pTexture == NULL )
  {
    delete pImageData;
    return false;
  }

  FontFromTexture( Flags, pTexture, pImageData );

  delete pImageData;

  return true;
}



int XBasicFont::FontSpacing()
{
  return 2;
}



int XBasicFont::TextLength( const GR::String& Text ) const
{
  // TODO - real UTF8 letters
  int   width = 0;
  unsigned int   length = (unsigned int)Text.length();
  for ( unsigned int i = 0; i < length; i++ )
  {
    std::map<GR::u32, XLetter*>::const_iterator   it( m_Letters.find( ( GR::u32 )( GR::u8 )Text[i] ) );

    if ( it == m_Letters.end() )
    {
      continue;
    }
    width += it->second->TextureSection.m_Width + 2;
  }
  if ( width )
  {
    width -= 2;
  }

  return width;
}



int XBasicFont::TextLength( const GR::Char* Text ) const
{
  if ( Text == NULL )
  {
    return 0;
  }
  return TextLength( GR::String( Text ) );
}



int XBasicFont::TextHeight( const GR::String& Text ) const
{
  if ( Text.empty() )
  {
    return TextHeight( "AOU!\xC4\xD6\xDC\xDFyg" );
  }
  int   height = 0;
  unsigned int   length = (unsigned int)Text.length();
  for ( unsigned int i = 0; i < length; i++ )
  {
    std::map<GR::u32, XLetter*>::const_iterator   it( m_Letters.find( ( GR::u32 )( GR::u8 )Text[i] ) );

    if ( it == m_Letters.end() )
    {
      continue;
    }
    if ( it->second->TextureSection.m_Height > height )
    {
      height = it->second->TextureSection.m_Height;
    }
  }
  return height;
}



int XBasicFont::TextHeight( const GR::Char* Text ) const
{
  const GR::Char* Copy = Text;
  if ( Copy == NULL )
  {
    Copy = "AOU!\xC4\xD6\xDC\xDFyg";
  }
  int   iHeight = 0;
  unsigned int   length = (unsigned int)strlen( Copy );
  for ( unsigned int i = 0; i < length; i++ )
  {
    std::map<GR::u32,XLetter*>::const_iterator   it( m_Letters.find( (GR::u32)(GR::u8)Copy[i] ) );
  
    if ( it == m_Letters.end() )
    {
      continue;
    }
    if ( it->second->TextureSection.m_Height > iHeight )
    {
      iHeight = it->second->TextureSection.m_Height;
    }
  }

  return iHeight;
}



void XBasicFont::Recreate()
{
  if ( !m_pRenderer )
  {
    return;
  }

  if ( m_CreationFlags & FLF_SQUARED )
  {
    LoadFontSquare( m_FileName.c_str(), m_CreationFlags, m_TransparentColor );
  }
  else
  {
    LoadFont( m_FileName.c_str(), m_CreationFlags, m_TransparentColor );
  }
}



void XBasicFont::Release()
{
  std::map<GR::u32,XLetter*>::iterator   it( m_Letters.begin() );

  while ( it != m_Letters.end() )
  {

    XLetter*   pLetter = it->second;

    delete pLetter;

    it++;
  }
  m_Letters.clear();
}



XBasicFont::XLetter* XBasicFont::Letter( const char cLetter )
{
  std::map<GR::u32,XLetter*>::iterator   it( m_Letters.find( (GR::u32)cLetter ) );
  if ( it == m_Letters.end() )
  {
    return NULL;
  }
  return it->second;
}



void XBasicFont::SetLetter( const char Letter, XLetter* pLetter )
{
  std::map<GR::u32,XLetter*>::iterator   it( m_Letters.find( Letter ) );
  if ( it != m_Letters.end() )
  {
    delete it->second;
  }
  m_Letters[Letter] = pLetter;
}



void XBasicFont::Dump()
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



float XBasicFont::DrawLetterDetail( float X, float Y, GR::u32 cLetter, float ScaleX, float ScaleY, GR::u32 Color1, GR::u32 Color2, GR::u32 Color3, GR::u32 Color4, float Z )
{
  std::map<GR::u32,XLetter*>::iterator   it( m_Letters.find( cLetter ) );
  if ( it == m_Letters.end() )
  {
    return 0;
  }

  XLetter*              pLetter = it->second;

  XTextureSection&      SectionInfo( pLetter->TextureSection );

  m_pRenderer->SetTexture( 0, SectionInfo.m_pTexture );

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  SectionInfo.GetTrueUV( UV1, UV2, UV3, UV4, SectionInfo.m_Flags );

  m_pRenderer->RenderQuad2d( (int)X, (int)Y,
                             (int)( SectionInfo.m_Width * ScaleX ), (int)( SectionInfo.m_Height * ScaleY ),
                             UV1.x, UV1.y,
                             UV2.x, UV2.y,
                             UV3.x, UV3.y,
                             UV4.x, UV4.y,
                             Color1, Color2, Color3, Color4, Z );
  return SectionInfo.m_Width * ScaleX;
}



int XBasicFont::DrawLetter( int X, int Y, GR::u32 cLetter, GR::u32 Color, float Z )
{
  std::map<GR::u32,XLetter*>::iterator   it( m_Letters.find( cLetter ) );
  if ( it == m_Letters.end() )
  {
    return 0;
  }

  XTextureSection&  SectionInfo( it->second->TextureSection );

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  SectionInfo.GetTrueUV( UV1, UV2, UV3, UV4, SectionInfo.m_Flags );

  m_pRenderer->SetTexture( 0, SectionInfo.m_pTexture );
  m_pRenderer->RenderQuad2d( X, Y,
                             SectionInfo.m_Width, SectionInfo.m_Height,
                             UV1.x, UV1.y,
                             UV2.x, UV2.y,
                             UV3.x, UV3.y,
                             UV4.x, UV4.y,
                             Color, Color, Color, Color, Z );

  return SectionInfo.m_Width;
}



void XBasicFont::DrawText( int X, int Y, const GR::String& Text, GR::u32 Color, float Z )
{
  if ( m_Letters.empty() )
  {
    Recreate();
    if ( m_Letters.empty() )
    {
      dh::Log( "XBasicFont::DrawText no letters" );
      return;
    }
  }
  for ( size_t i = 0; i < Text.length(); i++ )
  {
    X += DrawLetter( X, Y, (GR::u32)(GR::u8)Text[i], Color, Z ) + FontSpacing();
  }
}



void XBasicFont::DrawFreeText( const GR::tVector& Pos, 
                               const GR::String& Text,
                               const GR::tVector& Scale, 
                               GR::u32 Color )
{
  if ( m_Letters.empty() )
  {
    Recreate();
    if ( m_Letters.empty() )
    {
      dh::Log( "XBasicFont::DrawText no letters" );
      return;
    }
  }

  GR::tVector   posTemp( Pos );

  for ( size_t i = 0; i < Text.length(); i++ )
  {
    std::map<GR::u32,XLetter*>::iterator   it( m_Letters.find( (GR::u32)(GR::u8)Text[i] ) );
    if ( it == m_Letters.end() )
    {
      continue;
    }
    XLetter*   pLetter = it->second;

    m_pRenderer->SetTexture( 0, pLetter->TextureSection.m_pTexture );

    float   letterWidth = (float)pLetter->TextureSection.m_Width;

    float   letterHeight = (float)pLetter->TextureSection.m_Height;

    static GR::tFPoint     UV1;
    static GR::tFPoint     UV2;
    static GR::tFPoint     UV3;
    static GR::tFPoint     UV4;

    pLetter->TextureSection.GetTrueUV( UV1, UV2, UV3, UV4, pLetter->TextureSection.m_Flags );

    m_pRenderer->RenderQuad( posTemp,
                             posTemp + GR::tVector( letterWidth * Scale.x, 0, 0 ),
                             posTemp + GR::tVector( 0, letterHeight * Scale.y, 0 ),
                             posTemp + GR::tVector( letterWidth * Scale.x, letterHeight * Scale.y, 0 ),
                             UV1.x, UV1.y,
                             UV4.x, UV4.y,
                             Color );

    posTemp += GR::tVector( ( letterWidth + FontSpacing() ) * Scale.x, 0, 0 );
  }
}



void XBasicFont::DrawText( int X, int Y, const GR::String& Text, float ScaleX, float ScaleY, GR::u32 Color1, GR::u32 Color2, GR::u32 Color3, GR::u32 Color4, float Z )
{
  float   x = (float)X;
  for ( size_t i = 0; i < Text.length(); i++ )
  {
    x += FontSpacing() * ScaleX + DrawLetterDetail( x, (float)Y, (GR::u32)(GR::u8)Text[i], ScaleX, ScaleY, Color1, Color2, Color3, Color4, Z );
  }
}



void XBasicFont::FontFromTexture( GR::u32 Flags, XTexture* pTextureTarget, GR::Graphic::ImageData* pImageData )
{
  int     fontLines = 8;
   
  bool    oneFont = false;

  if ( Flags & FLF_SQUARED_ONE_FONT )
  {
    oneFont = true;
    fontLines = 16;
  }

  m_Width  = pImageData->Width() / 16;
  m_Height = pImageData->Height() / 16;

  for ( int j = 0; j < fontLines; j++ )
  {
    for ( int i = 0; i < 16; i++ )
    {
      XTextureSection*   pTextureInfo = NULL;

      XLetter*           pNewLetter = NULL;

      pNewLetter = new (std::nothrow) XLetter();

      GR::i32               minX = m_Width,
                            maxX = 0;


      // Buchstabenbreiten ausrechnen (BAUSTELLE - später im Fontgenerator!)
      int   letterX = 0;
      do
      {
        for ( int letterY = 0; letterY < m_Height; ++letterY )
        {
          if ( pImageData->GetPixelRGB( i * 16 + letterX, j * 16 + letterY ) != m_TransparentColor )
          {
            // Pixel gefunden
            minX = letterX;
            break;
          }
        }
        if ( minX < m_Width )
        {
          // in diesem Durchlauf ein Pixel gefunden
          break;
        }
        ++letterX;
        if ( letterX >= m_Width )
        {
          minX = 0;
          break;
        }
      }
      while ( true );

      letterX = minX;
      do
      {
        for ( int letterY = 0; letterY < m_Height; ++letterY )
        {
          if ( pImageData->GetPixelRGB( i * 16 + letterX, j * 16 + letterY ) != m_TransparentColor )
          {
            // Pixel gefunden
            maxX = letterX;
            break;
          }
        }
        if ( maxX < letterX )
        {
          // in diesem Durchlauf kein Pixel mehr gefunden
          break;
        }
        ++letterX;
        if ( letterX >= m_Width )
        {
          break;
        }
      }
      while ( true );

      if ( minX > maxX )
      {
        minX = 0;
        // BAUSTELLE - pfui, sowas hardcoden -> Breite eines Space
        maxX = ( m_Width - 1 ) / 2;
      }

      pNewLetter->XOffset  = minX;
      
      pNewLetter->TextureSection = XTextureSection( pTextureTarget, m_Width * i + minX, j * 16, maxX - minX + 1, m_Height );

      GR::u32   index = 0;
      if ( oneFont )
      {
        index = i + j * 16;
      }
      else
      {
        index = 32 + i + j * 16;
      }
      if ( m_Letters.find( index ) != m_Letters.end() )
      {
        delete m_Letters[index];
      }
      m_Letters[index] = pNewLetter;
    }
  }

  // Space auf Breite eines A
  XLetter*    pSpace = m_Letters[' '];

  pSpace->TextureSection.m_Width = m_Letters['I']->TextureSection.m_Width;
}



bool XBasicFont::LoadFont( const GR::String& FileName, GR::u32 Flags, GR::u32 TransparentColor )
{
  return LoadFont( FileName.c_str(), Flags, TransparentColor );
}



bool XBasicFont::LoadFont( const char* FileName, GR::u32 Flags, GR::u32 TransparentColor )
{
  GR::IO::FileStream   fileFont;

  if ( !fileFont.Open( FileName, IIOStream::OT_READ_ONLY ) )
  {
    return false;
  }
  if ( fileFont.ReadU8() != 3 ) // IGF_TYPE_FONT
  {
    return false;
  }

  m_FileName          = FileName;
  m_CreationFlags     = Flags;

  GR::Graphic::eImageFormat format = (GR::Graphic::eImageFormat)fileFont.ReadU8();

  if ( format == GR::Graphic::IF_PALETTED )
  {
    // das kann nur ein Grauverlauf sein
    format = GR::Graphic::IF_A8;
  }

  size_t      charCount = (size_t)fileFont.ReadU32();
  GR::u32     chunk = 0;

  std::map<GR::u32,GR::Graphic::Image>      letters;



  std::multiset<GR::tPoint>         ptLetterSizes;


  while ( ( chunk = fileFont.ReadU32() ) )
  {
    if ( chunk == 0x0001 )
    {
      // Image-Chunk

      for ( size_t i = 0; i < charCount; ++i )
      {
        // Zeichen
        GR::u32     Char = fileFont.ReadU32();

        GR::u32     width = fileFont.ReadU32();
        GR::u32     height = fileFont.ReadU32();

        GR::Graphic::Image& Image = letters[Char];

        Image.Set( (GR::u16)width, (GR::u16)height, (GR::u8)GR::Graphic::ImageData::DepthFromImageFormat( format ), 0, TransparentColor );

        fileFont.ReadBlock( Image.GetData(), Image.GetSize() );

        ptLetterSizes.insert( GR::tPoint( width, height ) );
      }
    }
  }

  fileFont.Close();

  std::map<GR::u32,GR::Graphic::Image*>      workLetters;

  std::map<GR::u32,GR::Graphic::Image>::iterator    itWL( letters.begin() );
  while ( itWL != letters.end() )
  {
    workLetters[itWL->first] = &itWL->second;

    ++itWL;
  }

  // auf eine Textur packen
  std::list<math::CSectionLayouter<GR::Graphic::Image*> >     listLayouts;

  listLayouts.push_back( math::CSectionLayouter<GR::Graphic::Image*>( GR::tRect( 0, 0, 256, 256 ) ) );

  {
    std::map<GR::u32,GR::Graphic::Image*>::iterator    it( workLetters.begin() );
    while ( it != workLetters.end() )
    {
      GR::Graphic::Image&   Img = *it->second;

      bool          bLayouterFound = false;

      std::list<math::CSectionLayouter<GR::Graphic::Image*> >::iterator   itLayout( listLayouts.begin() );
      while ( itLayout != listLayouts.end() )
      {
        math::CSectionLayouter<GR::Graphic::Image*>& SectionLayouter( *itLayout );

        if ( SectionLayouter.Insert( &Img, GR::tPoint( Img.GetWidth() + 2, Img.GetHeight() + 2 ) ) )
        {
          bLayouterFound = true;
          break;
        }

        if ( ( Img.GetWidth() + 2 > 256 )
        ||   ( Img.GetHeight() + 2 > 256 ) )
        {
          dh::Log( "XBasicFont::Load - Letter too big!" );
          return false;
        }
        ++itLayout;
      }

      // neuen Layouter anlegen -> neue Textur
      if ( !bLayouterFound )
      {
        listLayouts.push_back( math::CSectionLayouter<GR::Graphic::Image*>( GR::tRect( 0, 0, 256, 256 ) ) );

        if ( !listLayouts.back().Insert( &Img, GR::tPoint( Img.GetWidth() + 2, Img.GetHeight() + 2 ) ) )
        {
          dh::Log( "XBasicFont::Load - Section Layout Insert failed!" );
          return false;
        }
      }

      it = workLetters.erase( it );
    }
  }

  GR::Graphic::eImageFormat    formatToUse = format;

  /*
  if ( m_pRenderer->IsTextureFormatOK( formatToUse ) )
  {
    // das Format direkt übernehmen
  }
  else 
  */

  // wenn möglich Alpha beibehalten
  /*
  if ( m_pRenderer->IsTextureFormatOK( IF_A8 ) )
  {
    formatToUse = IF_A8;
  }
  else
  */
  if ( m_pRenderer->IsTextureFormatOK( GR::Graphic::IF_A8R8G8B8 ) )
  {
    formatToUse = GR::Graphic::IF_A8R8G8B8;
  }
  else if ( m_pRenderer->IsTextureFormatOK( GR::Graphic::IF_A4R4G4B4 ) )
  {
    formatToUse = GR::Graphic::IF_A4R4G4B4;
  }

  std::set<XTexture*>   setCreatedTextures;

  std::list<math::CSectionLayouter<GR::Graphic::Image*> >::iterator   itLayout( listLayouts.begin() );
  while ( itLayout != listLayouts.end() )
  {
    math::CSectionLayouter<GR::Graphic::Image*>& SectionLayouter( *itLayout );

    XTexture*   pTexture = m_pRenderer->CreateTexture( 256, 256, formatToUse );
    if ( pTexture == NULL )
    {
      // aufräumen
      std::set<XTexture*>::iterator   itCT( setCreatedTextures.begin() );
      while ( itCT != setCreatedTextures.end() )
      {
        m_pRenderer->DestroyTexture( *itCT );

        ++itCT;
      }
      return false;
    }
    setCreatedTextures.insert( pTexture );


    GR::Graphic::ContextDescriptor      cdPreTexture;

    cdPreTexture.CreateData( 256, 256, format );

    cdPreTexture.Box( 0, 0, 256, 256, TransparentColor );


    std::list<std::pair<GR::tPoint,GR::Graphic::Image*> >   listResult;

    SectionLayouter.Result( listResult );

    std::list<std::pair<GR::tPoint,GR::Graphic::Image*> >::iterator   itResult( listResult.begin() );
    while ( itResult != listResult.end() )
    {
      std::pair<GR::tPoint,GR::Graphic::Image*>&  Pair = *itResult;

      GR::Graphic::ContextDescriptor      cdLetter;
      
      cdLetter.Attach( Pair.second->GetWidth(), Pair.second->GetHeight(), Pair.second->GetLineSize(),
                       format, Pair.second->GetData() );

      cdLetter.CopyArea( 0, 0, cdLetter.Width(), cdLetter.Height(), 
                        Pair.first.x + 1, Pair.first.y + 1, 
                        &cdPreTexture );

      // find letter
      bool    found = false;

      std::map<GR::u32,GR::Graphic::Image>::iterator    it( letters.begin() );
      while ( it != letters.end() )
      {
        GR::Graphic::Image&   Img = it->second;

        if ( &Img == Pair.second )
        {
          //Img.Save( CMisc::printf( "komplettfont%c.igf", it->first ) );
          /*
          dh::Log( "Letter %d (%c) at %d,%d (%dx%d)",
                  it->first, it->first,
                  Pair.first.x, Pair.first.y,
                  Img.GetWidth(), Img.GetHeight() );*/

          XLetter* pLetter = new XLetter();

          pLetter->TextureSection = XTextureSection( pTexture, Pair.first.x + 1, Pair.first.y + 1, Img.GetWidth(), Img.GetHeight() );
          pLetter->XOffset = 0;

          if ( m_Letters.find( it->first ) != m_Letters.end() )
          {
            delete m_Letters[it->first];
          }
          
          m_Letters[it->first] = pLetter;
          found = true;
          break;
        }
        ++it;
      }

      if ( !found )
      {
        dh::Log( "Letter not found!!!" );

        Pair.second->Save( "futsch.igf" );
      }

      ++itResult;
    }

    cdPreTexture.ConvertSelfTo( formatToUse, 0, true, TransparentColor );
    m_pRenderer->CopyDataToTexture( pTexture, cdPreTexture );

    //GR::Graphic::Image    ImgFont( cdPreTexture );
    //ImgFont.Save( "komplettfont.igf" );

    ++itLayout;
  }

  m_TransparentColor = TransparentColor;

  return true;
}



