#include <Grafik/Animation.h>
#include <Grafik/Image.h>

#include <Misc/Misc.h>

#include <Interface/IIOStream.h>
#include <IO/FileStream.h>

#include <debug/debugclient.h>


namespace GR
{


  Animation::Animation( GR::u32 NewType )
  {
    Reset();
    Set( NewType );
  }



  Animation::Animation( const GR::Char* lpszFileName )
  {
    Reset();
    Load( lpszFileName );
  }



  Animation::Animation( IIOStream& Stream )
  {

    Reset();

    Load( Stream );

  }



  Animation::~Animation()
  {

    Reset();

  }



  void Animation::Set( GR::u32 NewType )
  {

    Reset();
    m_Type          = NewType;
    Position      = 0;

  }



  void Animation::Reset()
  {

    std::vector<tAnimFrame>::iterator    itImages( m_Images.begin() );

    // Images freigeben
    while ( itImages != m_Images.end() )
    {
      delete itImages->m_pImage;
      itImages = m_Images.erase( itImages );
    }

    m_Type          = AT_INVALID;
    Position      = 0;

  }



  bool Animation::AddFrame( GR::Graphic::Image* pImage, GR::f32 fLength )
  {
    if ( pImage == NULL )
    {
      return false;
    }
    m_Images.push_back( tAnimFrame( pImage, fLength ) );
    return true;
  }



  bool Animation::InsertFrame( GR::u32 Index, GR::Graphic::Image* pImage, GR::f32 fLength )
  {
    if ( pImage == NULL )
    {
      return false;
    }

    std::vector<tAnimFrame>::iterator    itImages = m_Images.begin();

    while ( ( itImages != m_Images.end() )
    &&      ( Index ) )
    {
      Index--;
      itImages++;
    }
    if ( itImages != m_Images.end() )
    {
      m_Images.insert( itImages, tAnimFrame( pImage, fLength ) );
    }
    return true;
  }



  bool Animation::InsertFrameBehind( GR::u32 Index, GR::Graphic::Image* pImage, GR::f32 fLength )
  {
    if ( pImage == NULL )
    {
      return false;
    }

    std::vector<tAnimFrame>::iterator    itImages = m_Images.begin();

    // Images freigeben
    itImages++;
    while ( ( itImages != m_Images.end() )
    &&      ( Index ) )
    {
      Index--;
      itImages++;
    }
    if ( Index == 0 )
    {
      m_Images.insert( itImages, tAnimFrame( pImage, fLength ) );
    }
    return true;
  }



  bool Animation::RemoveFrame( GR::Graphic::Image *pImage )
  {
    if ( pImage == NULL )
    {
      // das Image kann nicht drin sein
      return true;
    }

    tVectImages::iterator   itImages( m_Images.begin() );

    // Images freigeben
    while ( itImages != m_Images.end() )
    {
      if ( itImages->m_pImage == pImage )
      {
        m_Images.erase( itImages );
        return true;
      }
      itImages++;
    }

    // das Bild war gar nicht drin
    return true;
  }



  bool Animation::DeleteFrame( GR::Graphic::Image* pImage )
  {
    if ( pImage == NULL )
    {
      // das Image kann nicht drin sein
      return true;
    }

    tVectImages::iterator    itImages( m_Images.begin() );


    // Images freigeben
    while ( itImages != m_Images.end() )
    {
      if ( itImages->m_pImage == pImage )
      {
        delete pImage;
        m_Images.erase( itImages );
        return true;
      }
      itImages++;
    }

    // das Bild war gar nicht drin
    return true;
  }



  bool Animation::Load( const GR::String& FileName )
  {
    unsigned char   ucFileType,
                    ucBpp;

    unsigned long   ulCounter,
                    ulBytesRead = 0,
                    i;

    GR::Graphic::Image*       pImage     = NULL;

    GR::IO::FileStream    ioIn;

    if ( ioIn.Open( FileName ) )
    {
      ucFileType  = ioIn.ReadU8();
      ucBpp       = ioIn.ReadU8();
      ulCounter   = ioIn.ReadU32();

      if ( ucFileType == GR::Graphic::IGFType::ANIMATION_EXTENDED )
      {
        m_Type = ioIn.ReadU32();
        Set( m_Type );
      }
      else
      {
        Set( AT_LOOP | AT_FORWARD );
      }

      // kleiner Hack, sollten wir wirklich 16 haben, oder 15 drin?
      if ( ucBpp == 16 )
      {
        ucBpp = 15;
      }

      float   fFrameLength = 1.0f;

      for ( i = 0; i < ulCounter; i++ )
      {
        if ( ucFileType == GR::Graphic::IGFType::ANIMATION_EXTENDED )
        {
          // Frame-Länge
          fFrameLength = ioIn.ReadF32();
          // Reserve-GR::u32
          ioIn.ReadU32();
        }

        pImage = new GR::Graphic::Image();
        pImage->LoadAt( ioIn, ucBpp );
        AddFrame( pImage, fFrameLength );
      }
      ioIn.Close();
      return true;
    }
    else
    {
      dh::Log( "Fehler: Animation.Load Could'nt load %s.", FileName.c_str() );
    }
    return false;
  }



  bool Animation::Load( IIOStream& Stream )
  {
    unsigned char   ucFileType,
                    ucBpp;

    unsigned long   ulCounter,
                    i;

    GR::Graphic::Image       *pImage     = NULL;

    ucFileType  = Stream.ReadU8();
    ucBpp       = Stream.ReadU8();
    ulCounter   = Stream.ReadU32();

    if ( ucFileType == GR::Graphic::IGFType::ANIMATION_EXTENDED )
    {
      m_Type = Stream.ReadU32();
      Set( m_Type );
    }
    else
    {
      Set( AT_LOOP | AT_FORWARD );
    }

    // kleiner Hack, sollten wir wirklich 16 haben, oder 15 drin?
    if ( ucBpp == 16 )
    {
      ucBpp = 15;
    }

    for ( i = 0; i < ulCounter; i++ )
    {
      pImage = new GR::Graphic::Image();

      GR::u16     wWidth,
                  wHeight;

      float     fFrameLength = 1.0f;

      GR::u32   Transparent;

      if ( ucFileType == GR::Graphic::IGFType::ANIMATION_EXTENDED )
      {
        // Frame-Länge
        fFrameLength = Stream.ReadF32();
        // Reserve-GR::u32
        Stream.ReadU32();
      }

      wWidth        = Stream.ReadU16();
      wHeight       = Stream.ReadU16();
      Transparent = Stream.ReadU32();

      pImage->Set( wWidth, wHeight, ucBpp, 0, Transparent );

      Stream.ReadBlock( pImage->GetData(), pImage->GetSize() );
      AddFrame( pImage, fFrameLength );
    }

    Stream.Close();

    return true;
  }



  bool Animation::Save( const GR::String& FileName )
  {
    unsigned char   ucFileType      = GR::Graphic::IGFType::ANIMATION_EXTENDED,
                    ucBpp           = 0;

    unsigned long   ulBytesWritten  = 0,
                    ulImagesSaved   = 0;

    GR::IO::FileStream    ioOut;

    GR::Graphic::Image       *pImage         = GetFirstImage();

    tVectImages::iterator    itImages( m_Images.begin() );

    if ( pImage != NULL )
    {
      ucBpp = pImage->GetDepth();
    }

    if ( ioOut.Open( FileName, IIOStream::OT_WRITE_ONLY ) )
    {
      ioOut.WriteU8( ucFileType );
      ioOut.WriteU8( ucBpp );

      GR::u32   Size = (GR::u32)m_Images.size();
      ioOut.WriteU32( Size );
      ioOut.WriteU32( m_Type );

      // Reserve-GR::u32
      Size = 0;

      // Images speichern
      while ( itImages != m_Images.end() )
      {
        // Frame-Länge
        ioOut.WriteF32( itImages->m_fLength );
        // Reserve-GR::u32
        ioOut.WriteU32( 0 );
        // Image
        itImages->m_pImage->SaveAt( ioOut );
        itImages++;
        ulImagesSaved++;
      }
      ioOut.Close();
      if ( ulImagesSaved == m_Images.size() )
      {
        return true;
      }
      dh::Log( "Fehler: Animation.Save %lu Images saved out of %lu.", ulImagesSaved, m_Images.size() );
      dh::Log( "Note: Animation.Save Please delete the File %s. It may contain errors now.", FileName.c_str() );
    }
    else
    {
      dh::Log( "Fehler: Animation.Save Could'nt save %s.", FileName.c_str() );
    }
    return false;
  }



  bool Animation::LoadAni( const GR::String& FileName, unsigned char ucBpp )
  {
    bool    bResult     = false;

    GR::IO::FileStream    ioIn;

    if ( ioIn.Open( FileName ) )
    {
      bResult = LoadAniAt( ioIn, ucBpp );
      ioIn.Close();
      return bResult;
    }
    else
    {
      dh::Log( "Fehler: Animation.LoadAni Could'nt load %s.", FileName.c_str() );
    }

    return false;
  }



  bool Animation::LoadAniAt( IIOStream& ioIn, unsigned char ucBpp )
  {
    unsigned long   ulBytesRead = 0;

    GR::u32         FrameCount;

    GR::Graphic::Image       *pImage     = NULL;

    if ( ioIn.IsGood() )
    {
      FrameCount = ioIn.ReadU16();
      Set( AT_LOOP | AT_FORWARD );

      // kleiner Hack, sollten wir wirklich 16 haben, oder 15 drin?
      if ( ucBpp == 16 )
      {
        ucBpp = 15;
      }

      for ( GR::u32 i = 0; i < FrameCount; i++ )
      {
        pImage = new GR::Graphic::Image();

        float     fFrameLength = 1.0f;

        pImage->LoadBTNAt( ioIn, ucBpp );

        AddFrame( pImage, fFrameLength );
      }
      return true;
    }
    return false;
  }



  GR::u32 Animation::GetType()
  {
    return m_Type;
  }



  GR::u16 Animation::GetWidth()
  {
    GR::Graphic::Image       *pNextImage = GetFirstImage();

    if ( pNextImage != NULL )
    {
      return pNextImage->GetWidth();
    }
    return 0;
  }



  GR::u16 Animation::GetHeight()
  {
    GR::Graphic::Image       *pNextImage = GetFirstImage();

    if ( pNextImage != NULL )
    {
      return pNextImage->GetHeight();
    }
    return 0;
  }



  GR::u32 Animation::GetFrames()
  {
    return (GR::u32)m_Images.size();
  }



  GR::u32 Animation::GetPosition()
  {
    return Position;
  }



  GR::Graphic::Image* Animation::GetFirstImage()
  {
    if ( !m_Images.empty() )
    {
      return m_Images.front().m_pImage;
    }
    return NULL;
  }



  void Animation::SetFrame( GR::u32 Nr, GR::Graphic::Image* pImage )
  {
    // Achtung - der alte wird gelöscht!
    tVectImages::iterator    itImages( m_Images.begin() );
    std::advance( itImages, Nr );

    if ( itImages != m_Images.end() )
    {
      delete itImages->m_pImage;

      itImages->m_pImage = pImage;
    }
  }



  GR::Graphic::Image* Animation::GetImage( GR::u32 Number )
  {
    if ( Number >= m_Images.size() )
    {
      return NULL;
    }
    tVectImages::iterator    itImages( m_Images.begin() );

    std::advance( itImages, Number );
    return itImages->m_pImage;
  }



  void Animation::SetType( GR::u32 ulNewType )
  {
    m_Type = ulNewType;
  }



  void Animation::SetPosition( GR::u32 ulNewPosition )
  {
    if ( ulNewPosition < m_Images.size() )
    {
      Position = ulNewPosition;
    }
    else
    {
      dh::Log( "Fehler: Animation.SetPosition Position %lu out of Range.", ulNewPosition );
    }
  }



  void Animation::Next()
  {

    if ( m_Type & AT_FORWARD )
    {
      Position++;
      if ( ( Position >= m_Images.size() )
      &&   ( m_Type & AT_LOOP ) )
      {
        Position = 0;
      }
    }
    else if ( m_Type & AT_PING )
    {
      Position++;
      if ( Position >= m_Images.size() )
      {
        Position = (GR::u32)m_Images.size() - 1;
        m_Type     &= ~AT_PING;
        m_Type     |= AT_PONG;
      }
    }
    else if ( m_Type & AT_PONG )
    {
      if ( Position > 0 )
      {
        Position--;
      }
      else
      {
        m_Type     &= ~AT_PONG;
        m_Type     |= AT_PING;
      }
    }
    else if ( m_Type & AT_REVERSE )
    {
      if ( Position > 0 )
      {
        Position--;
      }
      else if ( m_Type & AT_LOOP )
      {
        if ( !m_Images.empty() )
        {
          Position = (GR::u32)m_Images.size() - 1;
        }
      }
    }

  }



  bool Animation::PutAnimation( GR::Graphic::GFXPage *pActualPage, signed long slXPos, signed long slYPos, GR::u32 ulFlags, GR::Graphic::Image *pMaskImage )
  {
    GR::Graphic::Image       *pNextImage = GetImage( Position );


    if ( pNextImage != NULL )
    {
      pNextImage->PutImage( pActualPage, slXPos, slYPos, ulFlags, pMaskImage );
      return true;
    }
    else
    {
      dh::Log( "Fehler: Animation.PutAnimation No Image found." );
    }
    return false;
  }



  void Animation::Compress( GR::u32 TransparentColor )
  {
    tVectImages::iterator   it( m_Images.begin() );
    while ( it != m_Images.end() )
    {
      tAnimFrame&   Anim = *it;

      Anim.m_pImage->SetTransparentColorRGB( TransparentColor );
      Anim.m_pImage->Compress();

      ++it;
    }
  }

};    // namespace GR