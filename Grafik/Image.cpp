#include <Grafik/Image.h>
#include <Grafik/ImageData.h>

#include <Grafik/GfxPage.h>
#include <Grafik/ContextDescriptor.h>
#include <Misc/Misc.h>

#include <Interface/IIOStream.h>
#include <IO/FileStream.h>
#include <Memory/MemoryStream.h>

#include <debug/debugclient.h>

#include <assert.h>



namespace GR
{

  namespace Graphic
  {

    Image::Image( const GR::Char* FileName )
    {
      assert( FileName );
      pData             = NULL;
      pCompressList     = NULL;
      CompressListSize  = 0;
      Reset();
      Load( FileName );

    }



    Image::Image( IIOStream& Stream )
    {
      pData             = NULL;
      pCompressList     = NULL;
      CompressListSize  = 0;
      Reset();
      Load( Stream );
    }



    Image::Image( const unsigned long NewWidth, 
                  const unsigned long NewHeight, 
                  const unsigned char Bpp, 
                  const unsigned long NewFlags, 
                  const unsigned long NewTransparent )
    {
      pData             = NULL;
      pCompressList     = NULL;
      CompressListSize  = 0;
      Set( (GR::u16)NewWidth, ( GR::u16 )NewHeight, Bpp, NewFlags, NewTransparent );
    }



    Image::Image( const Image* pCopyImage ) : 
      pData( NULL ),
      pCompressList( NULL ),
      CompressListSize( 0 )
    {
      assert( pCopyImage );

      const Image& image = *pCopyImage;
      Set(  image.GetWidth(), 
            image.GetHeight(), 
            image.GetDepth(), 
            image.GetFlags(), 
            image.GetTransparentColor() );
  
      if ( !const_cast<Image*>(&image)->GetData() )
      {
        dh::Log( "Fehler: Image::Image( const Image& image ): image.GetData()==NULL!" );
      }
      else
      {
        memcpy( (unsigned char *)GetData(),
                (unsigned char *)const_cast<Image*>(&image)->GetData(), 
                GetSize() );
      } 

      if ( ( const_cast<Image*>(&image)->pCompressList )
      &&   ( image.GetCompressListSize() > 0 ) )
      {
        CompressListSize  = image.GetCompressListSize();
        pCompressList     = new ( std::nothrow )GR::u8[CompressListSize];
        if ( pCompressList )
        {
          memcpy( (unsigned char *)pCompressList,
                  ( unsigned char * )const_cast<Image*>( &image )->pCompressList,
                  CompressListSize );
        }
      } 
    }



    Image::Image( const Image& ImageRHS )
    : pData( NULL ),
      pCompressList( NULL ),
      CompressListSize( 0 )
    {
      Set( ImageRHS.GetWidth(), 
           ImageRHS.GetHeight(), 
           ImageRHS.GetDepth(), 
           ImageRHS.GetFlags(), 
           ImageRHS.GetTransparentColor() );
  
      if ( !const_cast<Image*>(&ImageRHS )->GetData() )
      {
        dh::Log( "Fehler: Image::Image( const Image& image ): image.GetData()==NULL!" );
      }
      else
      {
        memcpy(  (unsigned char *)GetData(),
                        (unsigned char *)const_cast<Image*>(&ImageRHS )->GetData(),
                        GetSize() );
      } 

      if ( const_cast<Image*>( &ImageRHS )->pCompressList && ImageRHS.GetCompressListSize() > 0 )
      {
        CompressListSize = ImageRHS.GetCompressListSize();
        pCompressList = new ( std::nothrow )GR::u8[CompressListSize];
        if ( pCompressList != NULL )
        {
          memcpy( (unsigned char *)pCompressList,
                  ( unsigned char * )const_cast<Image*>( &ImageRHS )->pCompressList,
                  CompressListSize );
        }
      } 
    }



    Image& Image::operator=( const Image& ImageRHS )
    {
      if ( this == &ImageRHS ) return *this;

      Set( ImageRHS.GetWidth(), 
           ImageRHS.GetHeight(), 
           ImageRHS.GetDepth(), 
           ImageRHS.GetFlags(), 
           ImageRHS.GetTransparentColor() );
  
      if ( !const_cast<Image*>( &ImageRHS )->GetData() )
      {
        dh::Log( "Fehler: Image::Image( const Image& image ): image.GetData()==NULL!" );
      }
      else
      {
        memcpy(  (unsigned char *)GetData(),
                        (unsigned char *)const_cast<Image*>(&ImageRHS )->GetData(),
                        GetSize() );
      } 

      if ( const_cast<Image*>(&ImageRHS )->pCompressList && ImageRHS.GetCompressListSize() > 0 )
      {
        CompressListSize = ImageRHS.GetCompressListSize();
        pCompressList = new ( std::nothrow )GR::u8[CompressListSize];
        if ( pCompressList != NULL )
        {
          memcpy(  (unsigned char *)pCompressList,
                   (unsigned char *)const_cast<Image*>(&ImageRHS )->pCompressList,
                   CompressListSize );
        }
      } 

      return *this;
    }



    Image::~Image()
    {
      Reset();
    }



    Image::Image( const ImageData& ImageData ) :
      pData( NULL ),
      pCompressList( NULL ),
      CompressListSize( 0 )
    {

      Reset();

      if ( ( ImageData.ImageFormat() != GR::Graphic::IF_X1R5G5B5 )
      &&   ( ImageData.ImageFormat() != GR::Graphic::IF_R5G6B5 )
      &&   ( ImageData.ImageFormat() != GR::Graphic::IF_A8 )
      &&   ( ImageData.ImageFormat() != GR::Graphic::IF_A1R5G5B5 )
      &&   ( ImageData.ImageFormat() != GR::Graphic::IF_PALETTED )
      &&   ( ImageData.ImageFormat() != GR::Graphic::IF_INDEX1 )
      &&   ( ImageData.ImageFormat() != GR::Graphic::IF_INDEX2 )
      &&   ( ImageData.ImageFormat() != GR::Graphic::IF_INDEX4 )
      &&   ( ImageData.ImageFormat() != GR::Graphic::IF_R8G8B8 )
      &&   ( ImageData.ImageFormat() != GR::Graphic::IF_A8R8G8B8 )
      &&   ( ImageData.ImageFormat() != GR::Graphic::IF_X8R8G8B8 ) )
      {
        // da gibt es keine Konvertierung für
        dh::Log( "Cannot create image from ImageData Format %d\n", ImageData.ImageFormat() );
        return;
      }

      unsigned char   Bpp = (unsigned char)ImageData::DepthFromImageFormat( ImageData.ImageFormat() );

      if ( ImageData.ImageFormat() == GR::Graphic::IF_X1R5G5B5 )
      {
        Bpp = 15;
      }

      Set( ImageData.Width(), ImageData.Height(), Bpp, 0, 0 );

      memcpy( GetData(), 
              ImageData.Data(), 
              ImageData.DataSize() );
    }



    bool Image::Set( const GR::u16 NewWidth,
                     const GR::u16 NewHeight,
                     const unsigned char Bpp, 
                     const unsigned long NewFlags, 
                     const unsigned long NewTransparent )
    {
      Reset();

      BitsPerPixel      = Bpp;
      Width              = NewWidth;
      Height             = NewHeight;
      Flags             = NewFlags;
      TransparentColor  = NewTransparent;

      int   iSize = GetSize();

      pData = new unsigned char[iSize];

      memset( pData, 0, iSize );

      return true;
    }



    void Image::Reset()
    {

      if ( pData != NULL )
      {
        delete[] pData;
        pData = NULL;
      }
      if ( pCompressList != NULL )
      {
        delete[] pCompressList;
      }
      pData             = NULL;
      pCompressList       = NULL;
      CompressListSize  = 0;
      BitsPerPixel      = 0;
      Width              = 0;
      Height             = 0;
      Flags             = 0;

    }



    bool Image::Load( const GR::String& FileName )
    {
      unsigned char   ucFileType,
                      Bpp;

      unsigned long   ulCounter,
                      ulBytesRead = 0;

      GR::IO::FileStream    ioIn;

      if ( ioIn.Open( FileName ) )
      {
        ucFileType  = ioIn.ReadU8();
        Bpp       = ioIn.ReadU8();
        ulCounter   = ioIn.ReadU32();
        LoadAt( ioIn, Bpp );
        if ( ucFileType == GR::Graphic::IGFType::IMAGE_WITH_COMPRESSION_LIST )
        {
          CompressListSize = ioIn.ReadU32();
          pCompressList = new ( std::nothrow )GR::u8[CompressListSize];
          ioIn.ReadBlock( pCompressList, CompressListSize );
        }
        ioIn.Close();
        return true;
      }
      else
      {
        dh::Log( "Error: Image.Load Could'nt load %s", FileName.c_str() );
      }
      return false;
    }



    bool Image::Load( IIOStream& Stream )
    {
      if ( !Stream.IsGood() )
      {
        return false;
      }

      unsigned char   ucFileType  = Stream.ReadU8();
      unsigned char   Bpp       = Stream.ReadU8();

      unsigned long   ulCounter   = Stream.ReadU32();

      Width               = Stream.ReadU16();
      Height              = Stream.ReadU16();
      TransparentColor    = Stream.ReadU32();

      Set( Width, Height, Bpp, 0, TransparentColor );

      Stream.ReadBlock( GetData(), GetSize() );

      if ( ucFileType == IGFType::IMAGE_WITH_COMPRESSION_LIST )
      {
        CompressListSize = Stream.ReadU32();

        pCompressList = new ( std::nothrow )GR::u8[CompressListSize];

        Stream.ReadBlock( pCompressList, CompressListSize );
      }
      Stream.Close();

      return true;
    }



    bool Image::LoadAt( IIOStream& ioIn, const unsigned char Bpp )
    {
      GR::u16            NewWidth,
                      NewHeight;

      unsigned long   ulBytesRead       = 0,
                      NewTransparent  = 0;

      if ( ioIn.IsGood() )
      {
        NewWidth         = ioIn.ReadU16();
        NewHeight        = ioIn.ReadU16();
        NewTransparent  = ioIn.ReadU32();
        Set( NewWidth, NewHeight, Bpp, 0, NewTransparent );
        ioIn.ReadBlock( GetData(), GetSize() );
        return true;
      }
      else
      {
        dh::Log( "Fehler: Image.LoadAt Could'nt load Image." );
      }
      return false;
    }



    bool Image::LoadBTN( const GR::String& FileName, const unsigned char Bpp )
    {
      GR::IO::FileStream    ioIn;

      if ( ioIn.Open( FileName ) )
      {
        LoadBTNAt( ioIn, Bpp );
        ioIn.Close();
        return true;
      }
      else
      {
        dh::Log( "Fehler: Image.LoadBTN Could'nt load %s", FileName.c_str() );
      }
      return false;
    }



    bool Image::LoadBTNAt( IIOStream& ioIn, const unsigned char Bpp )
    {
      GR::u16            NewWidth,
                      NewHeight;

      unsigned long   ulBytesRead = 0;

      if ( ioIn.IsGood() )
      {
        NewWidth = ioIn.ReadU16();
        NewHeight = ioIn.ReadU16();
    
        unsigned char BppTemp =Bpp;
    
        if ( BppTemp == 16 )
        {
          BppTemp = 15;
        }
    
        Set( NewWidth, NewHeight, BppTemp, 0, 0 );

        ioIn.ReadBlock( GetData(), GetSize() );
        return true;
      }
      else
      {
        dh::Log( "Fehler: Image.LoadBTNAt No Filehandle." );
      }
      return false;
    }



    bool Image::LoadTGA( const GR::String& FileName )
    {
      int             i;

      unsigned char   ucHeader[18];

      GR::IO::FileStream    ioIn;

      if ( ioIn.Open( FileName ) )
      {
        ioIn.ReadBlock( ucHeader, 18 );
        Set( ucHeader[12] + 256 * ucHeader[13], ucHeader[14] + 256 * ucHeader[15], ucHeader[16], 0, 0 );

        // ID-Feld überlesen
        ioIn.SetPosition( ucHeader[0] );
        // BAUSTELLE!!
        if ( ucHeader[1] != 0 )
        {
          // Color-Map überlesen
          if ( ucHeader[7] == 15 )
          {
            ucHeader[7] = 16;
          }
          ioIn.SetPosition( ( ucHeader[5] + 256 * ucHeader[6] ) * ucHeader[7] / 8 );
        }
        for ( i = ucHeader[14] + 256 * ucHeader[15] - 1; i >= 0; i-- )
        {
          ioIn.ReadBlock( (GR::u8*)( GetData() ) + i * GetFaktor( GetDepth() ) * ( ucHeader[12] + 256 * ucHeader[13] ),
                          ( ucHeader[12] + 256 * ucHeader[13] ) * GetFaktor( GetDepth() ) );
        }
        ioIn.Close();
        return true;
      }
      else
      {
        dh::Log( "Fehler: Image.LoadTGA Could'nt load %s", FileName.c_str() );
      }
      return false;
    }



    bool Image::Save( const GR::String& FileName, const bool bSaveWithCompressList, GR::Graphic::Palette* pPal ) const
    {
      unsigned char   ucFileType      = IGFType::IMAGE,
                      Bpp             = GetDepth();

      unsigned long   ulCounter       = 1,
                      ulBytesWritten  = 0;

      GR::IO::FileStream    ioOut;

      if ( ioOut.Open( FileName, IIOStream::OT_WRITE_ONLY ) )
      {
        if ( bSaveWithCompressList )
        {
          ucFileType = IGFType::IMAGE_WITH_COMPRESSION_LIST;
          if ( pPal != NULL )
          {
            ucFileType = IGFType::IMAGE_WITH_COMPRESSION_LIST_AND_PALETTE;
          }
        }
        else if ( pPal != NULL )
        {
          ucFileType = IGFType::IMAGE_WITH_PALETTE;
        }
        ioOut.WriteU8( ucFileType );
        ioOut.WriteU8( Bpp );
        ioOut.WriteU32( ulCounter );
        SaveAt( ioOut );
        if ( bSaveWithCompressList )
        {
          ioOut.WriteU32( CompressListSize );
          ioOut.WriteBlock( pCompressList, CompressListSize );
        }
        if ( pPal != NULL )
        {
          ioOut.WriteU8( 0 );
          for ( int i = 0; i < 256; i++ )
          {
            ioOut.WriteU8( pPal->Red( i ) );
            ioOut.WriteU8( pPal->Green( i ) );
            ioOut.WriteU8( pPal->Blue( i ) );
          }
        }
        ioOut.Close();
        return true;
      }
      else
      {
        dh::Log( "Fehler: Image.Save Couldn't save %s", FileName.c_str() );
      }
      return false;
    }



    bool Image::SaveAt( IIOStream& ioOut ) const
    {
      unsigned long   ulBytesWritten  = 0;

      if ( ioOut.IsGood() )
      {
        ioOut.WriteU16( Width );
        ioOut.WriteU16( Height );
        ioOut.WriteU32( TransparentColor );
        ioOut.WriteBlock( pData, GetSize() );
        return true;
      }
      else
      {
        dh::Log( "Fehler: Image.SaveAt Could'nt save Image." );
      }

      return false;

    }



    bool Image::GetImage( GFXPage* pPage,
                           const int iX, 
                           const int iY )
    {
      pPage->BeginAccess();

      ContextDescriptor   cdSource( pPage );
      ContextDescriptor   cdTarget( this );

      cdSource.CopyArea( iX, iY, GetWidth(), GetHeight(), 0, 0, &cdTarget );

      pPage->EndAccess();
      return true;
    }



    bool Image::GetNewImage( GFXPage *pPage,
                                 const signed long slXPos, 
                                 const signed long slYPos, 
                                 const GR::u16 NewWidth,
                                 const GR::u16 NewHeight )
    {
      if ( pPage != NULL )
      {
        Set( NewWidth, NewHeight, pPage->GetDepth(), 0, 0 );
        GetImage( pPage, slXPos, slYPos );
        return true;
      }
      dh::Log( "Fehler: Image.GetNewImage No ActualPage." );

      return false;
    }



    Image* Image::GetTileImage( const GR::u16 wPosX,
                                const GR::u16 wPosY,
                                const GR::u16 NewWidth,
                                const GR::u16 NewHeight )
    {
      if ( pData == NULL )
      {
        return NULL;
      }

      if ( ( wPosX >= Width )
      ||   ( wPosY >= Height ) )
      {
        return NULL;
      }

      Image*    pNewImage = new Image( NewWidth, NewHeight, BitsPerPixel );

      GR::Graphic::ContextDescriptor    cdSource( this );
      GR::Graphic::ContextDescriptor    cdTarget( pNewImage );

      cdSource.CopyArea( wPosX, wPosY, NewWidth, NewHeight, 0, 0, &cdTarget );
      return pNewImage;
    }



    bool Image::PutImage( GFXPage* pPage,
                           const signed long sltxpos, 
                           const signed long sltypos, 
                           const unsigned long FlagsArg, 
                           Image* pMaskImage ) const
    {
      if ( pData == NULL )
      {
        return false;
      }

      int     slxpos = sltxpos + pPage->m_iXOffset,
              slypos = sltypos + pPage->m_iYOffset;

      if ( pPage == NULL )
      {
        dh::Log( "Image.PutImage: pPage = NULL" );
        return false;
      }

      bool        bAllOk = false;

      unsigned long Flags = FlagsArg;
      signed long slXPos = slxpos;
      signed long slYPos = slypos;

      if ( pPage->GetDepth() == GetDepth() )
      {
        bAllOk = true;
      }
      else if ( ( Flags == IMAGE_METHOD_ALPHA_VALUE )
      &&        ( GetDepth() == 8 ) )
      {
        bAllOk = true;
      }
      else
      {
        GR::Graphic::Palette  Palette = GR::Graphic::Palette::AlphaPalette();
        if ( pPage->GetDepth() == 8 )
        {
          pPage->GetPalette( &Palette );
        }
        //dh::Log( "Converted image from depth %d to %d", GetDepth(), pPage->GetDepth() );
        if ( const_cast<Image*>(this)->Convert( pPage->GetDepth(), &Palette ) )
        {
          //dh::Log( "Converted image to depth" );
          bAllOk = true;
        }
      }
      if ( !bAllOk )
      {
        dh::Log( "Image.PutImage: Depth %u is not equal to actual screen %d.", GetDepth(), pPage->GetDepth() );
        return false;
      }

      if ( Flags == IMAGE_METHOD_OPTIMAL )
      {
        if ( pCompressList != NULL )
        {
          Flags = IMAGE_METHOD_COMPRESSED;
        }
        else
        {
          Flags = IMAGE_METHOD_PLAIN;
        }
      }

      GR::u8            *pSource                = NULL,
                      *pDestination           = NULL,
                      *pMask                  = NULL,
                      *pSourceBase            = NULL,
                      *pDestinationBase       = NULL,
                      *pMaskBase              = NULL;

      GR::u16         wAlpha50Mask            = IMAGE_ALPHA50_MASK_555;

      signed long     slXEnd,
                      slYEnd,
                      slXOffset               = 0,
                      slYOffset               = 0,
                      slXDelta                = GetWidth(),
                      slYDelta                = GetHeight(),
                      slXStep                 = 0,
                      slYStep                 = 0,
                      i,
                      j;

      unsigned long   ulFaktor                = 0,
                      ulMaskTransparentColor  = 0;

      slXEnd = slXPos + slXDelta;
      slYEnd = slYPos + slYDelta;
      if ( ( slXEnd > (signed long)pPage->GetLeftBorder() )
      &&   ( slYEnd > (signed long)pPage->GetTopBorder() )
      &&   ( slXPos <= (signed long)pPage->GetRightBorder() )
      &&   ( slYPos <= (signed long)pPage->GetBottomBorder() ) )
      {
        // es ist etwas des Bildes zu sehen
        pPage->BeginAccess();
        if ( slXPos < (signed long)pPage->GetLeftBorder() )
        {
          // Das Bild ragt links über den Bildrand hinaus
          slXDelta -= ( pPage->GetLeftBorder() - slXPos );
          slXOffset = pPage->GetLeftBorder() - slXPos;
          slXPos    = pPage->GetLeftBorder();
        }
        if ( slYPos < (signed long)pPage->GetTopBorder() )
        {
          // Das Bild ragt oben über den Bildrand hinaus
          slYDelta -= ( pPage->GetTopBorder() - slYPos );
          slYOffset = pPage->GetTopBorder() - slYPos;
          slYPos    = pPage->GetTopBorder();
        }
        if ( slXEnd > (signed long)pPage->GetRightBorder() + 1 )
        {
          // Das Bild ragt rechts über den Bildrand hinaus
          slXDelta -= ( slXEnd - pPage->GetRightBorder() - 1 );
          //slXEnd    = pPage->GetRightBorder();
        }
        if ( slYEnd > (signed long)pPage->GetBottomBorder() + 1 )
        {
          // Das Bild ragt unten über den Bildrand hinaus
          slYDelta -= ( slYEnd - pPage->GetBottomBorder() - 1 );
          //slYEnd    = pPage->GetBottomBorder();
        }
        switch ( pPage->GetDepth() )
        {
          case 1:
    //-  1 Bit Routinen ------------------------------------------------------------
            pDestinationBase  = ( GR::u8 *)pPage->GetData() + slXPos / 8 + slYPos * (signed long)pPage->GetLineOffset();
            pSourceBase       = ( GR::u8 *)const_cast<Image*>(this)->GetData() + slXOffset / 8 + slYOffset * GetLineSize();

            slXStep           = 1;
            slYStep           = 1;
            if ( Flags & IMAGE_METHOD_COMPRESSED )
            {
    //--- Compressed ---------------------------------------------------------------
              //PutCompressed( pPage, pDestinationBase, pSourceBase, slXDelta, slYDelta, slXOffset, slYOffset, 1 );
              dh::Log( "1-bit-Compressed not supported yet\n" );
            }
            else if ( Flags & IMAGE_METHOD_TRANSPARENT )
            {
              dh::Log( "1-bit transparent not supported yet\n" );
    //--- Transparent --------------------------------------------------------------
              /*
              if ( Flags & IMAGE_METHOD_MIRROR_H )
              {
                pSourceBase  += ( GetWidth() - 1 - ( slXOffset * 2 ) );
                slXStep         = -1;
              }
              if ( Flags & IMAGE_METHOD_MIRROR_V )
              {
                pSourceBase    += ( GetHeight() - 1 - ( slYOffset * 2 ) ) * GetWidth();
                slYStep         = -1;
              }
              pDestination      = pDestinationBase;
              pSource           = pSourceBase;
              for ( i = 0; i < slYDelta; i++ )
              {
                for ( j = 0; j < slXDelta; j++ )
                {
                  if ( *pSource != (GR::u8)TransparentColor )
                  {
                    *pDestination++ = *pSource;
                  }
                  else
                  {
                    pDestination++;
                  }
                  pSource += slXStep;
                }
                pDestinationBase += pPage->GetLineOffset();
                pSourceBase      += GetWidth() * slYStep;
                pDestination      = pDestinationBase;
                pSource           = pSourceBase;
              }
              */
            }
            else
            {
    //--- Blockmode (fast) ---------------------------------------------------------
              // TODO 8 ist hier nicht richtig - angeknapste Bits!
              for ( int i = 0; i < slYDelta; i++ )
              {
                //memcpy( pDestinationBase, pSourceBase, slXDelta / 8 );
                memcpy( pDestinationBase, pSourceBase, GetLineSize() );
                pDestinationBase += pPage->GetLineOffset();
                pSourceBase      += GetLineSize();
              }
            }
            pPage->EndAccess();
            return true;
          case 4:
    //-  4 Bit Routinen ------------------------------------------------------------
            pDestinationBase  = ( GR::u8 *)pPage->GetData() + slXPos / 2 + slYPos * (signed long)pPage->GetLineOffset();
            pSourceBase       = ( GR::u8 *)const_cast<Image*>(this)->GetData() + slXOffset / 2 + slYOffset * GetWidth();
            slXStep           = 1;
            slYStep           = 1;
            if ( Flags & IMAGE_METHOD_COMPRESSED )
            {
    //--- Compressed ---------------------------------------------------------------
              //PutCompressed( pPage, pDestinationBase, pSourceBase, slXDelta, slYDelta, slXOffset, slYOffset, 1 );
              dh::Log( "4-bit-Compressed not supported yet\n" );
            }
            else if ( Flags & IMAGE_METHOD_TRANSPARENT )
            {
              dh::Log( "4-bit transparent not supported yet\n" );
    //--- Transparent --------------------------------------------------------------
              /*
              if ( Flags & IMAGE_METHOD_MIRROR_H )
              {
                pSourceBase  += ( GetWidth() - 1 - ( slXOffset * 2 ) );
                slXStep         = -1;
              }
              if ( Flags & IMAGE_METHOD_MIRROR_V )
              {
                pSourceBase    += ( GetHeight() - 1 - ( slYOffset * 2 ) ) * GetWidth();
                slYStep         = -1;
              }
              pDestination      = pDestinationBase;
              pSource           = pSourceBase;
              for ( i = 0; i < slYDelta; i++ )
              {
                for ( j = 0; j < slXDelta; j++ )
                {
                  if ( *pSource != (GR::u8)TransparentColor )
                  {
                    *pDestination++ = *pSource;
                  }
                  else
                  {
                    pDestination++;
                  }
                  pSource += slXStep;
                }
                pDestinationBase += pPage->GetLineOffset();
                pSourceBase      += GetWidth() * slYStep;
                pDestination      = pDestinationBase;
                pSource           = pSourceBase;
              }
              */
            }
            else
            {
    //--- Blockmode (fast) ---------------------------------------------------------
              // TODO 2 ist hier nicht richtig
              for ( int i = 0; i < slYDelta; i++ )
              {
                memcpy( pDestinationBase, pSourceBase, slXDelta / 2 );
                pDestinationBase += pPage->GetLineOffset();
                pSourceBase      += GetLineSize();
              }
            }
            pPage->EndAccess();
            return true;
          case 8:
    //-  8 Bit Routinen ------------------------------------------------------------
            pDestinationBase  = ( GR::u8 *)pPage->GetData() + slXPos + slYPos * (signed long)pPage->GetLineOffset();
            pSourceBase       = ( GR::u8 *)const_cast<Image*>(this)->GetData() + slXOffset + slYOffset * GetWidth();
            slXStep           = 1;
            slYStep           = 1;
            if ( Flags & IMAGE_METHOD_COMPRESSED )
            {
    //--- Compressed ---------------------------------------------------------------
              PutCompressed( pPage, pDestinationBase, pSourceBase, slXDelta, slYDelta, slXOffset, slYOffset, 1 );
            }
            else if ( Flags & IMAGE_METHOD_TRANSPARENT )
            {
    //--- Transparent --------------------------------------------------------------
              if ( Flags & IMAGE_METHOD_MIRROR_H )
              {
                pSourceBase  += ( GetWidth() - 1 - ( slXOffset * 2 ) );
                slXStep         = -1;
              }
              if ( Flags & IMAGE_METHOD_MIRROR_V )
              {
                pSourceBase    += ( GetHeight() - 1 - ( slYOffset * 2 ) ) * GetWidth();
                slYStep         = -1;
              }
              pDestination      = pDestinationBase;
              pSource           = pSourceBase;
              for ( i = 0; i < slYDelta; i++ )
              {
                for ( j = 0; j < slXDelta; j++ )
                {
                  if ( *pSource != ( GR::u8 )TransparentColor )
                  {
                    *pDestination++ = *pSource;
                  }
                  else
                  {
                    pDestination++;
                  }
                  pSource += slXStep;
                }
                pDestinationBase += pPage->GetLineOffset();
                pSourceBase      += GetWidth() * slYStep;
                pDestination      = pDestinationBase;
                pSource           = pSourceBase;
              }
            }
            else
            {
    //--- Blockmode (fast) ---------------------------------------------------------
              PutBlock( pPage, pDestinationBase, pSourceBase, slXDelta, slYDelta, 1 );
            }
            pPage->EndAccess();
            return true;
          case 15:
          case 16:
    //- 16 Bit Routinen ------------------------------------------------------------
            pDestinationBase  = ( GR::u8 *)pPage->GetData() + slXPos * 2 + slYPos * (signed long)pPage->GetLineOffset();
            if ( Flags & IMAGE_METHOD_ALPHA_VALUE )
            {
              pSourceBase     = ( GR::u8 *)const_cast<Image*>(this)->GetData() + slXOffset + slYOffset * GetWidth();
            }
            else
            {
              pSourceBase     = ( GR::u8 *)const_cast<Image*>(this)->GetData() + slXOffset * 2 + slYOffset * GetWidth() * 2;
            }
            if ( pMaskImage != NULL )
            {
              if ( Flags & IMAGE_METHOD_ALPHA_MASK )
              {
                pMaskBase     = ( GR::u8 *)pMaskImage->GetData() + slXOffset + slYOffset * GetWidth();
              }
              else
              {
                pMaskBase     = ( GR::u8 *)pMaskImage->GetData() + slXOffset * 2 + slYOffset * GetWidth() * 2;
              }
            }
            slXStep           = 2;
            slYStep           = 2;
            if ( Flags & IMAGE_METHOD_COMPRESSED )
            {
    //--- Compressed ---------------------------------------------------------------
              // Sourcebild muá 16 Bit sein
              PutCompressed( pPage, pDestinationBase, pSourceBase, slXDelta, slYDelta, slXOffset, slYOffset, 2 );
            }
            else if ( Flags & IMAGE_METHOD_ALPHA_VALUE )
            {
    //--- Alpha Value --------------------------------------------------------------
              // Sourcebild muá 8 Bit sein
              // von Maskbild wird nur die transparente Farbe benutzt
              PutCompressedAlpha( pPage, pDestinationBase, pSourceBase, slXDelta, slYDelta, slXOffset, slYOffset, 2, pMaskImage );
            }
            else if ( Flags & IMAGE_METHOD_ALPHA_MASK )
            {
    //--- Alpha Mask ---------------------------------------------------------------
              // Sourcebild muá 16 Bit sein
              // Maskbild muá 8 Bit sein
              PutCompressedAlphaMask( pPage, pDestinationBase, pSourceBase, pMaskBase, slXDelta, slYDelta, slXOffset, slYOffset, 2, pMaskImage );
            }
            else if ( Flags & IMAGE_METHOD_TRANSPARENT )
            {
              if ( Flags & IMAGE_METHOD_MIRROR_H )
              {
                pSourceBase    += ( GetWidth() - 1 - ( slXOffset * 2 ) ) * 2;
                pMaskBase      += ( GetWidth() - 1 - ( slXOffset * 2 ) ) * 2;
                slXStep         = -2;
              }
              if ( Flags & IMAGE_METHOD_MIRROR_V )
              {
                pSourceBase    += ( GetHeight() - 1 - ( slYOffset * 2 ) ) * GetWidth() * 2;
                pMaskBase      += ( GetHeight() - 1 - ( slYOffset * 2 ) ) * GetWidth() * 2;
                slYStep         = -2;
              }
              pDestination      = pDestinationBase;
              pSource           = pSourceBase;
              pMask             = pMaskBase;
              if ( GetDepth() == 16 )
              {
                wAlpha50Mask    = IMAGE_ALPHA50_MASK_565;
              }
              if ( Flags & IMAGE_METHOD_SHADOW_MASK )
              {
                //--- Shadow Mask --------------------------------------------------------------
                // Sourcebild muá 16 Bit sein
                // Maskbild muá 16 Bit sein
                if ( pMaskImage != NULL )
                {
                  if ( pMaskImage->GetDepth() != pPage->GetDepth() )
                  {
                    pMaskImage->Convert( pPage->GetDepth() );
                  }
                  if ( pMaskImage->GetDepth() == pPage->GetDepth() )
                  {
                    ulMaskTransparentColor = pMaskImage->GetTransparentColor();
                    for ( i = 0; i < slYDelta; i++ )
                    {
                      for ( j = 0; j < slXDelta; j++ )
                      {
                        if ( *( GR::u16 *)pSource != ( GR::u16 )TransparentColor )
                        {
                          if ( *( GR::u16 *)pMask == ( GR::u16 )ulMaskTransparentColor )
                          {
                            *( GR::u16 *)pDestination = *( GR::u16 *)pSource;
                          }
                          else
                          {
                            *( GR::u16 *)pDestination = ( ( *( GR::u16 *)pSource ) & wAlpha50Mask ) >> 1;
                          }
                        }
                        pDestination   += 2;
                        pSource        += slXStep;
                        pMask          += slXStep;
                      }
                      pDestinationBase += pPage->GetLineOffset();
                      pSourceBase      += GetWidth() * slYStep;
                      pMaskBase        += GetWidth() * slYStep;
                      pDestination      = pDestinationBase;
                      pSource           = pSourceBase;
                      pMask             = pMaskBase;
                    }
                  }
                  else
                  {
                    dh::Log( "Fehler: Image.PutImage MaskImage depth does not match" );
                  }
                }
                else
                {
                  dh::Log( "Fehler: Image.PutImage No MaskImage found for Mask Operation." );
                }
              }
              else if ( Flags & IMAGE_METHOD_ALPHA50 )
              {
    //--- Alpha 50 -----------------------------------------------------------------
                // Sourcebild muá 16 Bit sein
                for ( i = 0; i < slYDelta; i++ )
                {
                  for ( j = 0; j < slXDelta; j++ )
                  {
                    if ( *( GR::u16 *)pSource != ( GR::u16 )TransparentColor )
                    {
                      *( GR::u16 *)pDestination = ( ( *( GR::u16 *)pSource & wAlpha50Mask ) + ( *( GR::u16 *)pDestination & wAlpha50Mask ) ) >> 1;
                    }
                    pDestination   += 2;
                    pSource        += slXStep;
                  }
                  pDestinationBase += pPage->GetLineOffset();
                  pSourceBase      += GetWidth() * slYStep;
                  pDestination      = pDestinationBase;
                  pSource           = pSourceBase;
                }
              }
              else
              {
    //--- Transparent --------------------------------------------------------------
                // Sourcebild muá 16 Bit sein
                for ( i = 0; i < slYDelta; i++ )
                {
                  for ( j = 0; j < slXDelta; j++ )
                  {
                    if ( *( GR::u16 *)pSource != ( GR::u16 )TransparentColor )
                    {
                      *( GR::u16 *)pDestination = *( GR::u16 *)pSource;
                    }
                    pDestination   += 2;
                    pSource        += slXStep;
                  }
                  pDestinationBase += pPage->GetLineOffset();
                  pSourceBase      += GetWidth() * slYStep;
                  pDestination      = pDestinationBase;
                  pSource           = pSourceBase;
                }
              }
            }
            else
            {
    //--- Blockmode (fast) ---------------------------------------------------------
              PutBlock( pPage, pDestinationBase, pSourceBase, slXDelta, slYDelta, 2 );
            }
            pPage->EndAccess();
            return true;
          case 24:
    //- 24 Bit Routinen ------------------------------------------------------------
            pDestinationBase  = ( GR::u8 *)pPage->GetData() + slXPos * 3 + slYPos * (signed long)pPage->GetLineOffset();
            pSourceBase       = ( GR::u8 *)const_cast<Image*>(this)->GetData() + slXOffset * 3 + slYOffset * GetWidth() * 3;
            slXStep           = 1;
            slYStep           = 3;
            if ( Flags & IMAGE_METHOD_COMPRESSED )
            {
    //--- Compressed ---------------------------------------------------------------
              PutCompressed( pPage, pDestinationBase, pSourceBase, slXDelta, slYDelta, slXOffset, slYOffset, 3 );
            }
            else if ( Flags & IMAGE_METHOD_TRANSPARENT )
            {
              if ( Flags & IMAGE_METHOD_MIRROR_H )
              {
                pSourceBase    += ( GetWidth() - 1 - ( slXOffset * 2 ) ) * 3;
                slXStep         = -1;
              }
              if ( Flags & IMAGE_METHOD_MIRROR_V )
              {
                pSourceBase    += ( GetHeight() - 1 - ( slYOffset * 2 ) ) * GetWidth() * 3;
                slYStep         = -3;
              }
              pDestination      = pDestinationBase;
              pSource           = pSourceBase;
              if ( Flags & IMAGE_METHOD_SHADOW_MASK )
              {
                dh::Log( "Status: Image.PutImage Baustelle." );
              }
              else
              {
    //--- Transparent --------------------------------------------------------------
                if ( GetDepth() == 24 )
                {
                  for ( i = 0; i < slYDelta; i++ )
                  {
                    for ( j = 0; j < slXDelta; j++ )
                    {
                      // Kann ein 32 Bit Pointer auch auf ungerade Adressen zeigen?
                      if ( ( ( *(unsigned long *)pSource ) & 0x00ffffff ) != ( TransparentColor & 0x00ffffff ) )
                      {
                        *pDestination++ = *pSource;
                        pSource        += slXStep;
                        *pDestination++ = *pSource;
                        pSource        += slXStep;
                        *pDestination++ = *pSource;
                        pSource        += slXStep;
                      }
                      else
                      {
                        pDestination   += 3;
                        pSource        += ( slXStep + slXStep + slXStep );
                      }
                    }
                    pDestinationBase += pPage->GetLineOffset();
                    pSourceBase      += GetWidth() * slYStep;
                    pDestination      = pDestinationBase;
                    pSource           = pSourceBase;
                  }
                }
                else if ( GetDepth() == 32 )
                {
                  unsigned long ulPixel,
                                ulAlpha;
                  if ( slXStep > 0 )
                  {
                    slXStep = 4;
                  }
                  else
                  {
                    slXStep = -4;
                  }
                  if ( slYStep > 0 )
                  {
                    slYStep = 4;
                  }
                  else
                  {
                    slYStep = -4;
                  }
                  for ( i = 0; i < slYDelta; i++ )
                  {
                    for ( j = 0; j < slXDelta; j++ )
                    {
                      if ( *(unsigned long *)pSource )
                      {
                        if ( ( *(unsigned long *)pSource & 0xff000000 ) == 0xff000000 )
                        {
                          *( GR::u16 *)pDestination = *( GR::u16 *)pSource;
                          *( pDestination + 2 ) = *( pSource + 2 );
                        }
                        else
                        {
                          ulAlpha = *(unsigned long *)pSource >> 24;
                          ulPixel = ( ( ( *(unsigned long *)pDestination & 0x00ff00ff ) * ( 255 - ulAlpha ) ) & 0xff00ff00 );
                          ulPixel += ( ( ( *(unsigned long *)pDestination & 0x0000ff00 ) * ( 255 - ulAlpha ) ) & 0x00ff0000 );
                          ulPixel += ( ( *(unsigned long *)pSource & 0x00ff00ff ) * ulAlpha & 0xff00ff00 );
                          ulPixel += ( ( *(unsigned long *)pSource & 0x0000ff00 ) * ulAlpha & 0x00ff0000 );
                          ulPixel >>= 8;
                          *( GR::u16 *)pDestination = ( GR::u16 )ulPixel;
                          ulPixel >>= 16;
                          *( pDestination + 2 ) = (unsigned char)ulPixel;
                        }
                      }
                      pDestination   += 3;
                      pSource        += slXStep;
                    }
                    pDestinationBase += pPage->GetLineOffset();
                    pSourceBase      += GetWidth() * slYStep;
                    pDestination      = pDestinationBase;
                    pSource           = pSourceBase;
                  }
                }
                else
                {
                  dh::Log( "Fehler: Image.PutImage Image is not True Color." );
                }
              }
            }
            else if ( Flags & IMAGE_METHOD_ALPHA_VALUE )
            {
    //--- Alpha Value --------------------------------------------------------------
              // Sourcebild muá 8 Bit sein
              // von Maskbild wird nur die transparente Farbe benutzt
              PutCompressedAlpha( pPage, pDestinationBase, pSourceBase, slXDelta, slYDelta, slXOffset, slYOffset, 3, pMaskImage );
            }
            else
            {
    //--- Blockmode (fast) ---------------------------------------------------------
              PutBlock( pPage, pDestinationBase, pSourceBase, slXDelta, slYDelta, 3 );
            }
            pPage->EndAccess();
            return true;
          case 32:  // ???
    //- 32 Bit Routinen ------------------------------------------------------------
            pDestinationBase  = ( GR::u8 *)pPage->GetData() + slXPos * 4 + slYPos * (signed long)pPage->GetLineOffset();
            pSourceBase       = ( GR::u8 *)const_cast<Image*>(this)->GetData() + slXOffset * 4 + slYOffset * GetWidth() * 4;
            slXStep           = 4;
            slYStep           = 4;
            if ( Flags & IMAGE_METHOD_COMPRESSED )
            {
    //--- Compressed ---------------------------------------------------------------
              PutCompressed( pPage, pDestinationBase, pSourceBase, slXDelta, slYDelta, slXOffset, slYOffset, 4 );
            }
            else if ( Flags & IMAGE_METHOD_TRANSPARENT )
            {
              if ( Flags & IMAGE_METHOD_MIRROR_H )
              {
                pSourceBase    += ( GetWidth() - 1 - ( slXOffset * 2 ) ) * 4;
                slXStep         = -4;
              }
              if ( Flags & IMAGE_METHOD_MIRROR_V )
              {
                pSourceBase    += ( GetHeight() - 1 - ( slYOffset * 2 ) ) * GetWidth() * 4;
                slYStep         = -4;
              }
              pDestination      = pDestinationBase;
              pSource           = pSourceBase;
              if ( Flags & IMAGE_METHOD_SHADOW_MASK )
              {
                dh::Log( "Status: Image.PutImage Baustelle." );
              }
              else
              {
    //--- Transparent --------------------------------------------------------------
                for ( i = 0; i < slYDelta; i++ )
                {
                  for ( j = 0; j < slXDelta; j++ )
                  {
                    if ( *(GR::u32 *)pSource != TransparentColor )
                    {
                      *(GR::u32 *)pDestination = *(GR::u32 *)pSource;
                    }
                    pDestination   += 4;
                    pSource        += slXStep;
                  }
                  pDestinationBase += pPage->GetLineOffset();
                  pSourceBase      += GetWidth() * slYStep;
                  pDestination      = pDestinationBase;
                  pSource           = pSourceBase;
                }
              }
            }
            else if ( Flags & IMAGE_METHOD_ALPHA_VALUE )
            {
    //--- Alpha Value --------------------------------------------------------------
              // Sourcebild muá 8 Bit sein
              // von Maskbild wird nur die transparente Farbe benutzt
              PutCompressedAlpha( pPage, pDestinationBase, pSourceBase, slXDelta, slYDelta, slXOffset, slYOffset, 4, pMaskImage );
            }
            else
            {
    //--- Blockmode (fast) ---------------------------------------------------------
              PutBlock( pPage, pDestinationBase, pSourceBase, slXDelta, slYDelta, 4 );
            }
            pPage->EndAccess();
            return true;
          default:
            dh::Log( "Fehler: Image.PutImage Unsupported Format %lu.", GetDepth() );
            break;
        }
        pPage->EndAccess();
      }
      return true;
    }



    void Image::PutCompressed(  GFXPage *pPage, 
                               GR::u8 *pDestinationBase,
                               GR::u8 *pSourceBase,
                                    const signed long slXDelta, 
                                    const signed long slYDelta, 
                                    const signed long slXOffset, 
                                    const signed long slYOffset, 
                                    const unsigned long ulStep ) const
    {
      GR::u8            *pDestination,
                      *pSource,
                      *pCompress    = pCompressList;

      GR::u16            wSectors;

      signed long     slOffset,
                      slDelta,
                      slSize,
                      i;

      if ( pCompress != NULL )
      {
        for ( i = 0; i < slYOffset; i++ )
        {
          wSectors        = *( GR::u16 *)pCompress;
          pCompress      += 2 + wSectors;
        }
        for ( i = 0; i < slYDelta; i++ )
        {
          pDestination    = pDestinationBase;
          pSource         = pSourceBase;
          slOffset        = slXOffset;
          slDelta         = slXDelta;
          wSectors        = *( GR::u16 *)pCompress;
          pCompress      += 2;
          while ( wSectors )
          {
            slSize = (signed long)*pCompress & 0x7f;
            if ( slSize == 0 )
            {
              slSize = GetWidth();
            }
            if ( slOffset )
            {
              if ( slOffset >= slSize )
              {
                // noch ausserhalb
                slOffset -= slSize;
                slSize    = 0;
              }
              else
              {
                // schon innerhalb
                slSize  -= slOffset;
                slOffset = 0;
              }
            }
            if ( slSize )
            {
              if ( slDelta )
              {
                if ( slDelta >= slSize )
                {
                  // noch komplett innerhalb
                  slDelta -= slSize;
                }
                else
                {
                  slSize  = slDelta;
                  slDelta = 0;
                }
                if ( !( *pCompress & 0x80 ) )
                {
                  memcpy( pDestination, pSource, slSize * ulStep );
                }
              }
              pDestination  += slSize * ulStep;
              pSource       += slSize * ulStep;
            }
            pCompress++;
            wSectors--;
          }
          pDestinationBase += pPage->GetLineOffset();
          pSourceBase      += GetWidth() * ulStep;
        }
      }
      else
      {
        dh::Log( "Fehler: Image.PutCompressed No CompressList found." );
      }

    }



    void Image::PutCompressedAlphaMask( GFXPage *pPage, 
                                        GR::u8 *pDestinationBase,
                                        GR::u8 *pSourceBase,
                                        GR::u8 *pMaskBase,
                                            const signed long slXDelta, 
                                            const signed long slYDelta, 
                                            const signed long slXOffset, 
                                            const signed long slYOffset, 
                                            const unsigned long ulStep, 
                                            Image *pMaskImage ) const
    {

      GR::u8            *pDestination,
                      *pSource,
                      *pMask,
                      *pCompress;

      GR::u16            wSectors;

      signed long     slOffset,
                      slDelta,
                      slSize,
                      i;

      if ( pMaskImage != NULL )
      {
        pCompress = pMaskImage->GetCompressList();
        if ( pCompress != NULL )
        {
          for ( i = 0; i < slYOffset; i++ )
          {
            wSectors        = *( GR::u16 *)pCompress;
            pCompress      += 2 + wSectors;
          }
          for ( i = 0; i < slYDelta; i++ )
          {
            pDestination    = pDestinationBase;
            pSource         = pSourceBase;
            pMask           = pMaskBase;
            slOffset        = slXOffset;
            slDelta         = slXDelta;
            wSectors        = *( GR::u16 *)pCompress;
            pCompress      += 2;
            while ( wSectors )
            {
              slSize = (signed long)*pCompress & 0x7f;
              if ( slSize == 0 )
              {
                slSize = GetWidth();
              }
              if ( slOffset )
              {
                if ( slOffset >= slSize )
                {
                  // noch ausserhalb
                  slOffset -= slSize;
                  slSize    = 0;
                }
                else
                {
                  // schon innerhalb
                  slSize  -= slOffset;
                  slOffset = 0;
                }
              }
              if ( slSize )
              {
                if ( slDelta )
                {
                  if ( slDelta >= slSize )
                  {
                    // noch komplett innerhalb
                    slDelta -= slSize;
                  }
                  else
                  {
                    slSize  = slDelta;
                    slDelta = 0;
                  }
                  if ( !( *pCompress & 0x80 ) )
                  {
                    PutAlphaMaskSector( ( GR::u16 *)pDestination, ( GR::u16 *)pSource, pMask, slSize * ulStep );
                  }
                }
                pDestination  += slSize * ulStep;
                pSource       += slSize * ulStep;
                pMask         += slSize;
              }
              pCompress++;
              wSectors--;
            }
            pDestinationBase += pPage->GetLineOffset();
            pSourceBase      += GetWidth() * ulStep;
            pMaskBase        += GetWidth();
          }
        }
        else
        {
          dh::Log( "Fehler: Image.PutCompressedAlphaMask No CompressList found." );
        }
      }
      else
      {
        dh::Log( "Fehler: Image.PutCompressedAlphaMask No MaskImage found." );
      }

    }



    void Image::PutCompressedAlpha(  GFXPage *pPage, 
                                    GR::u8 *pDestinationBase,
                                    GR::u8 *pSourceBase,
                                         const signed long slXDelta, 
                                         const signed long slYDelta, 
                                         const signed long slXOffset, 
                                         const signed long slYOffset, 
                                         const unsigned long ulStep, 
                                         Image *pValueImage ) const
    {

      unsigned char   Bpp;

      GR::u8            *pDestination,
                      *pSource,
                      *pCompress    = pCompressList;

      GR::u16            wSectors;

      signed long     slOffset,
                      slDelta,
                      slSize,
                      i;

      unsigned long   ulAlpha       = 0;

      if ( pPage != NULL )
      {
        if ( pValueImage != NULL )
        {
          ulAlpha = pValueImage->GetTransparentColor();
        }
        Bpp = pPage->GetDepth();
        if ( pCompress != NULL )
        {
          for ( i = 0; i < slYOffset; i++ )
          {
            wSectors        = *( GR::u16 *)pCompress;
            pCompress      += 2 + wSectors;
          }
          for ( i = 0; i < slYDelta; i++ )
          {
            pDestination    = pDestinationBase;
            pSource         = pSourceBase;
            slOffset        = slXOffset;
            slDelta         = slXDelta;
            wSectors        = *( GR::u16 *)pCompress;
            pCompress      += 2;
            while ( wSectors )
            {
              slSize = (signed long)*pCompress & 0x7f;
              if ( slSize == 0 )
              {
                slSize = GetWidth();
              }
              if ( slOffset )
              {
                if ( slOffset >= slSize )
                {
                  // noch ausserhalb
                  slOffset -= slSize;
                  slSize    = 0;
                }
                else
                {
                  // schon innerhalb
                  slSize  -= slOffset;
                  slOffset = 0;
                }
              }
              if ( slSize )
              {
                if ( slDelta )
                {
                  if ( slDelta >= slSize )
                  {
                    // noch komplett innerhalb
                    slDelta -= slSize;
                  }
                  else
                  {
                    slSize  = slDelta;
                    slDelta = 0;
                  }
                  if ( !( *pCompress & 0x80 ) )
                  {
                    PutAlphaSector( ( GR::u16 *)pDestination, pSource, slSize * ulStep, ulAlpha, Bpp );
                  }
                }
                pDestination  += slSize * ulStep;
                pSource       += slSize;
              }
              pCompress++;
              wSectors--;
            }
            pDestinationBase += pPage->GetLineOffset();
            pSourceBase      += GetWidth();
          }
        }
        else
        {
          dh::Log( "Fehler: Image.PutCompressedAlpha No CompressList found." );
        }
      }
      else
      {
        dh::Log( "Fehler: Image.PutCompressedAlpha No ActualPage found." );
      }

    }



    void Image::PutAlphaMaskSector( GR::u16 *pDestination,
                                    GR::u16 *pSource,
                                    GR::u8 *pMask,
                                        const unsigned long ullength ) const
    {

      unsigned long   ulSource,
                      ulDestination,
                      ulResult;

      unsigned long ulLength = ullength / 2;
      if ( GetDepth() == 16 )
      {
        while ( ulLength )
        {
          ulSource        = ( (unsigned long)( *pSource      & IMAGE_RED_MASK_565 ) << 9 ) + ( (unsigned long)( *pSource      & IMAGE_GREEN_MASK_565 ) << 4 ) + (unsigned long)( *pSource    & IMAGE_BLUE_MASK_565 );
          ++pSource;
          ulDestination   = ( (unsigned long)( *pDestination & IMAGE_RED_MASK_565 ) << 9 ) + ( (unsigned long)( *pDestination & IMAGE_GREEN_MASK_565 ) << 4 ) + (unsigned long)( *pDestination & IMAGE_BLUE_MASK_565 );
          ulResult        = ( ulSource * ( *pMask >> 3 ) ) + ulDestination * ( 32 - ( *pMask >> 3 ) );
          ++pMask;
          *pDestination++ = ( GR::u16 )( ( ( ulResult & 0x3e000000 ) >> 14 ) + ( ( ulResult & 0xf8000 ) >> 9 ) + ( ( ulResult & 0x003e0 ) >> 5 ) );
          ulLength--;
        }
      }
      else if ( GetDepth() == 15 )
      {
        while ( ulLength )
        {
          ulSource        = ( (unsigned long)( *pSource      & IMAGE_RED_MASK_555 ) << 10 ) + ( (unsigned long)( *pSource      & IMAGE_GREEN_MASK_555 ) << 5 ) + (unsigned long)( *pSource    & IMAGE_BLUE_MASK_555 );
          ++pSource;
          ulDestination   = ( (unsigned long)( *pDestination & IMAGE_RED_MASK_555 ) << 10 ) + ( (unsigned long)( *pDestination & IMAGE_GREEN_MASK_555 ) << 5 ) + (unsigned long)( *pDestination & IMAGE_BLUE_MASK_555 );
          ulResult        = ( ulSource * ( *pMask >> 3 ) ) + ulDestination * ( 32 - ( *pMask >> 3 ) );
          ++pMask;
          *pDestination++ = ( GR::u16 )( ( ( ulResult & 0x3e000000 ) >> 15 ) + ( ( ulResult & 0xf8000 ) >> 10 ) + ( ( ulResult & 0x003e0 ) >> 5 ) );
          ulLength--;
        }
      }
      else
      {
        dh::Log( "Fehler: Image.PutAlphaMaskSector Unsupported Imageformat for Operation." );
      }

    }



    void Image::PutAlphaSector( GR::u16 *pDestination,
                                GR::u8 *pSource,
                                    const unsigned long ullength, 
                                    const unsigned long ulAlpha, 
                                    const unsigned char Bpp ) const
    {

      unsigned char   ucSource;

      unsigned long   ulResult,
                      ulDummy,
                      ulDest;

      unsigned long ulLength = ullength;  

      if ( Bpp == 16 )      
      {
        ulLength /= 2;
        ulResult = ( ( ulAlpha & IMAGE_RED_MASK_565 ) << 10 ) + ( ( ulAlpha & IMAGE_GREEN_MASK_565 ) << 5 ) + ( ulAlpha & IMAGE_BLUE_MASK_565 );
        while ( ulLength )
        {
          if ( ( ucSource = *pSource >> 3 ) )
          {
            ulDest = *pDestination;
            ulDummy = ( ulResult * ucSource ) + 
                  ( ( (unsigned long )( ulDest & IMAGE_RED_MASK_565 ) << 10 ) + ( (unsigned long)( ulDest & IMAGE_GREEN_MASK_565 ) << 5 ) + (unsigned long)( ulDest & IMAGE_BLUE_MASK_565 ) ) * ( 32 - ucSource );
            *pDestination = ( GR::u16 )( ( ( ulDummy & 0x7c000000 ) >> 15 ) + ( ( ulDummy & 0x1f8000 ) >> 10 ) + ( ( ulDummy & 0x03e0 ) >> 5 ) );
          }
          pSource++;
          pDestination++;
          ulLength--;
        }
      }
      else if ( Bpp == 15 )
      {
        ulLength /= 2;
        ulResult = ( ( ulAlpha & IMAGE_RED_MASK_555 ) << 10 ) + ( ( ulAlpha & IMAGE_GREEN_MASK_555 ) << 5 ) + ( ulAlpha & IMAGE_BLUE_MASK_555 );
        while ( ulLength )
        {
          if ( ( ucSource = *pSource >> 3 ) )
          {
            ulDummy = ( ulResult * ucSource ) + ( ( (unsigned long )( *pDestination & IMAGE_RED_MASK_555 ) << 10 ) + ( (unsigned long)( *pDestination & IMAGE_GREEN_MASK_555 ) << 5 ) + (unsigned long)( *pDestination & IMAGE_BLUE_MASK_555 ) ) * ( 32 - ucSource );
            *pDestination = ( GR::u16 )( ( ( ulDummy & 0x3e000000 ) >> 15 ) + ( ( ulDummy & 0x00f8000 ) >> 10 ) + ( ( ulDummy & 0x000003e0 ) >> 5 ) );
          }
          pSource++;
          pDestination++;
          ulLength--;
        }
      }
      else if ( Bpp == 24 )
      {
        GR::u32     dwR1,
                  dwG1,
                  dwB1,
                  dwR2,
                  dwG2,
                  dwB2,
                  dwRG,
                  dwGG,
                  dwBG;

        GR::u8      *pDest = ( GR::u8*)pDestination;


        dwR1 =   ulAlpha & 0x0ff;
        dwG1 = ( ulAlpha & 0x0ff00 ) >> 8;
        dwB1 = ( ulAlpha & 0x0ff0000 ) >> 16;
        ulLength /= 3;
        while ( ulLength )
        {
          if ( ( ucSource = *pSource >> 3 ) )
          {
            dwR2 = *( GR::u8*)pDest;
            dwG2 = *( ( GR::u8*)pDest + 1 );
            dwB2 = *( ( GR::u8*)pDest + 2 );
            dwRG = ( ( dwR1 * ucSource ) + dwR2 * ( 32 - ucSource ) ) >> 5;
            dwGG = ( ( dwG1 * ucSource ) + dwG2 * ( 32 - ucSource ) ) >> 5;
            dwBG = ( ( dwB1 * ucSource ) + dwB2 * ( 32 - ucSource ) ) >> 5;
            *pDest = ( GR::u8 )dwRG;
            *( pDest + 1 ) = ( GR::u8 )dwGG;
            *( pDest + 2 ) = ( GR::u8 )dwBG;
            //ulDummy = ( ulResult * ucSource ) + ( ( (unsigned long )( *pDestination & IMAGE_RED_MASK_555 ) << 10 ) + ( (unsigned long)( *pDestination & IMAGE_GREEN_MASK_555 ) << 5 ) + (unsigned long)( *pDestination & IMAGE_BLUE_MASK_555 ) ) * ( 32 - ucSource );
            //*pDestination = (GR::u16)( ( ( ulDummy & 0x3e000000 ) >> 15 ) + ( ( ulDummy & 0x00f8000 ) >> 10 ) + ( ( ulDummy & 0x000003e0 ) >> 5 ) );
          }
          pSource++;
          pDest += 3;
          ulLength--;
        }
      }
      else if ( Bpp == 32 )
      {
        GR::u32     dwR1,
                  dwG1,
                  dwB1,
                  dwR2,
                  dwG2,
                  dwB2,
                  dwRG,
                  dwGG,
                  dwBG;

        GR::u8      *pDest = ( GR::u8*)pDestination;


        dwR1 =   ulAlpha & 0x0ff;
        dwG1 = ( ulAlpha & 0x0ff00 ) >> 8;
        dwB1 = ( ulAlpha & 0x0ff0000 ) >> 16;
        ulLength /= 4;
        while ( ulLength )
        {
          if ( ( ucSource = *pSource >> 3 ) )
          {
            dwR2 = *( GR::u8*)pDest;
            dwG2 = *( ( GR::u8*)pDest + 1 );
            dwB2 = *( ( GR::u8*)pDest + 2 );
            dwRG = ( ( dwR1 * ucSource ) + dwR2 * ( 32 - ucSource ) ) >> 5;
            dwGG = ( ( dwG1 * ucSource ) + dwG2 * ( 32 - ucSource ) ) >> 5;
            dwBG = ( ( dwB1 * ucSource ) + dwB2 * ( 32 - ucSource ) ) >> 5;
            *pDest = ( GR::u8 )dwRG;
            *( pDest + 1 ) = ( GR::u8 )dwGG;
            *( pDest + 2 ) = ( GR::u8 )dwBG;
            //ulDummy = ( ulResult * ucSource ) + ( ( (unsigned long )( *pDestination & IMAGE_RED_MASK_555 ) << 10 ) + ( (unsigned long)( *pDestination & IMAGE_GREEN_MASK_555 ) << 5 ) + (unsigned long)( *pDestination & IMAGE_BLUE_MASK_555 ) ) * ( 32 - ucSource );
            //*pDestination = (GR::u16)( ( ( ulDummy & 0x3e000000 ) >> 15 ) + ( ( ulDummy & 0x00f8000 ) >> 10 ) + ( ( ulDummy & 0x000003e0 ) >> 5 ) );
          }
          pSource++;
          pDest += 4;
          ulLength--;
        }
      }
      else
      {
        dh::Log( "Fehler: Image.PutAlphaSector Unsupported Imageformat for Operation." );
      }

    }



    void Image::PutBlock( GFXPage *pPage, 
                          GR::u8 *pDestination,
                          GR::u8 *pSource,
                              const signed long slXDelta, 
                              const signed long slYDelta, 
                              const unsigned long ulStep ) const
    {

      signed long     i;

      for ( i = 0; i < slYDelta; i++ )
      {
        memcpy( pDestination, pSource, slXDelta * ulStep );
        pDestination += pPage->GetLineOffset();
        pSource      += GetWidth() * ulStep;
      }

    }



    unsigned long Image::GetPixel( const unsigned long ulXPos, 
                                       const unsigned long ulYPos ) const
    {

      if ( ( ulXPos < 0 )
      ||   ( ulXPos >= GetWidth() )
      ||   ( ulYPos < 0 )
      ||   ( ulYPos >= GetHeight() ) )
      {
        return 0;
      }

      switch ( GetDepth() )
      {
        case 8:
          return (GR::u32)*( ( GR::u8 *)const_cast<Image*>(this)->GetData() + ulXPos + ulYPos * GetWidth() );
        case 15:
        case 16:
          return *( GR::u16 *)( ( GR::u8 *)const_cast<Image*>(this)->GetData() + ulXPos * 2 + ulYPos * GetWidth() * 2 );
        case 24:
          return *(unsigned long *)( ( GR::u8 *)const_cast<Image*>(this)->GetData() + ulXPos * 3 + ulYPos * GetWidth() * 3 ) & 0x00ffffff;
        case 32:
          return *(unsigned long *)( ( GR::u8 *)const_cast<Image*>(this)->GetData() + ulXPos * 4 + ulYPos * GetWidth() * 4 );
        default:
          dh::Log( "Fehler: Image.GetPixel Unsupported Format %lu.", GetDepth() );
          return 0;
      }

    }



    void Image::SetPixel( const unsigned long ulXPos, 
                           const unsigned long ulYPos,
                           unsigned long dwColor )
    {

      if ( ( ulXPos < 0 )
      ||   ( ulXPos >= GetWidth() )
      ||   ( ulYPos < 0 )
      ||   ( ulYPos >= GetHeight() ) )
      {
        return;
      }

      switch ( GetDepth() )
      {
        case 8:
          *( ( GR::u8 *)const_cast<Image*>(this)->GetData() + ulXPos + ulYPos * GetWidth() ) = ( GR::u8 )dwColor;
          break;
        case 15:
        case 16:
          *( GR::u16 *)( ( GR::u8 *)const_cast<Image*>(this)->GetData() + ulXPos * 2 + ulYPos * GetWidth() * 2 ) = ( GR::u16 )dwColor;
          break;
        case 24:
          *( ( GR::u8 *)const_cast<Image*>(this)->GetData() + 3 * ulXPos + 3 * ulYPos * GetWidth() )      = ( GR::u8 )( dwColor & 0x0ff );
          *( ( GR::u8 *)const_cast<Image*>(this)->GetData() + 3 * ulXPos + 3 * ulYPos * GetWidth() + 1 )  = ( GR::u8 )( ( dwColor & 0x00ff00 ) >> 8 );
          *( ( GR::u8 *)const_cast<Image*>(this)->GetData() + 3 * ulXPos + 3 * ulYPos * GetWidth() + 2 )  = ( GR::u8 )( ( dwColor & 0xff0000 ) >> 16 );
          break;
        case 32:
          *(unsigned long *)( ( GR::u8 *)const_cast<Image*>(this)->GetData() + ulXPos * 4 + ulYPos * GetWidth() * 4 ) = dwColor;
          break;
        default:
          dh::Log( "Fehler: Image.GetPixel Unsupported Format %lu.", GetDepth() );
          break;
      }

    }



    bool Image::Convert( unsigned char Bpp, GR::Graphic::Palette *pPalette )
    {

      // Die Kompresslist geht bei Set verloren, aufbewahren
      GR::u8*   pTempCompressList = pCompressList;

      GR::u32   dwTempCompressListSize = CompressListSize;

      pCompressList = NULL;
      CompressListSize = 0;

      if ( Bpp != GetDepth() )
      {
        switch ( GetDepth() )
        {
          case 8:
            // Baustelle
            if ( pPalette == NULL )
            {
              dh::Log( "Fehler: Image.Convert No Palette (%d to %d).", GetDepth(), Bpp );
              pCompressList = pTempCompressList;
              return false;
            }
            switch ( Bpp )
            {
              case 15:
                {
                  int           i,
                                j;

                  Image     *pDummyImage;


                  pDummyImage = new Image( this );

              
                  Set( Width, Height, 15, 0, 
                          ( ( pPalette->Red( ( GR::u8 )GetTransparentColor() ) / 8 ) << 10 )
                        + ( ( pPalette->Green( ( GR::u8 )GetTransparentColor() ) / 8 ) << 5 )
                        +   ( pPalette->Blue( ( GR::u8 )GetTransparentColor() ) / 8 ) );
                  for ( j = 0; j < (int)Height; j++ )
                  {
                    for ( i = 0; i < (int)Width; i++ )
                    {
                      ( ( GR::u16*)pData )[i + j * Width] =
                          ( ( pPalette->Red(   ( ( GR::u8*)pDummyImage->GetData() )[i + j * Width] ) >> 3 ) << 10 )
                        + ( ( pPalette->Green( ( ( GR::u8*)pDummyImage->GetData() )[i + j * Width] ) >> 3 ) << 5 )
                        + ( ( pPalette->Blue(  ( ( GR::u8*)pDummyImage->GetData() )[i + j * Width] ) >> 3 ) );
                    }
                  }
                  delete pDummyImage;
                }
                pCompressList = pTempCompressList;
                CompressListSize = dwTempCompressListSize;
                return true;
              case 16:
                {
                  int           i,
                                j;

                  Image     *pDummyImage;


                  pDummyImage = new Image( this );

              
                  Set( Width, Height, 16, 0, 
                          ( ( pPalette->Red( ( GR::u8 )GetTransparentColor() ) / 8 ) << 11 )
                        + ( ( pPalette->Green( ( GR::u8 )GetTransparentColor() ) / 4 ) << 5 )
                        +   ( pPalette->Blue( ( GR::u8 )GetTransparentColor() ) / 8 ) );
                  for ( j = 0; j < (int)Height; j++ )
                  {
                    for ( i = 0; i < (int)Width; i++ )
                    {
                      ( ( GR::u16*)pData )[i + j * Width] =
                          ( ( pPalette->Red(   ( ( GR::u8*)pDummyImage->GetData() )[i + j * Width] ) >> 3 ) << 11 )
                        + ( ( pPalette->Green( ( ( GR::u8*)pDummyImage->GetData() )[i + j * Width] ) >> 2 ) << 5 )
                        + ( ( pPalette->Blue(  ( ( GR::u8*)pDummyImage->GetData() )[i + j * Width] ) >> 3 ) );
                    }
                  }
                  delete pDummyImage;
                }
                pCompressList = pTempCompressList;
                CompressListSize = dwTempCompressListSize;
                return true;
              case 24:
                {
                  int           i,
                                j;

                  Image     *pDummyImage;


                  pDummyImage = new Image( this );

                  Set( Width, Height, 24, 0, 
                          ( ( pPalette->Red( ( GR::u8 )GetTransparentColor() ) ) << 16 )
                        + ( ( pPalette->Green( ( GR::u8 )GetTransparentColor() ) ) << 8 )
                        +   ( pPalette->Blue( ( GR::u8 )GetTransparentColor() ) ) );
                  for ( j = 0; j < (int)Height; j++ )
                  {
                    for ( i = 0; i < (int)Width; i++ )
                    {
                      ( ( GR::u8*)pData )[3 * i + 3 * j * Width + 2] = ( pPalette->Red(   ( ( GR::u8*)pDummyImage->GetData() )[i + j * Width] ) );
                      ( ( GR::u8*)pData )[3 * i + 3 * j * Width + 1] = ( pPalette->Green(   ( ( GR::u8*)pDummyImage->GetData() )[i + j * Width] ) );
                      ( ( GR::u8*)pData )[3 * i + 3 * j * Width + 0] = ( pPalette->Blue(   ( ( GR::u8*)pDummyImage->GetData() )[i + j * Width] ) );
                    }
                  }
                  delete pDummyImage;
                }
                pCompressList = pTempCompressList;
                CompressListSize = dwTempCompressListSize;
                return true;
              case 32:
                {
                  int           i,
                                j;

                  Image     *pDummyImage;


                  pDummyImage = new Image( this );

                  Set( Width, Height, 32, 0, 
                          ( ( pPalette->Red( ( GR::u8 )GetTransparentColor() ) ) << 16 )
                        + ( ( pPalette->Green( ( GR::u8 )GetTransparentColor() ) ) << 8 )
                        +   ( pPalette->Blue( ( GR::u8 )GetTransparentColor() ) ) );
                  for ( j = 0; j < (int)Height; j++ )
                  {
                    for ( i = 0; i < (int)Width; i++ )
                    {
                      ( ( GR::u8*)pData )[4 * i + 4 * j * Width + 2] = ( pPalette->Red(   ( ( GR::u8*)pDummyImage->GetData() )[i + j * Width] ) );
                      ( ( GR::u8*)pData )[4 * i + 4 * j * Width + 1] = ( pPalette->Green(   ( ( GR::u8*)pDummyImage->GetData() )[i + j * Width] ) );
                      ( ( GR::u8*)pData )[4 * i + 4 * j * Width + 0] = ( pPalette->Blue(   ( ( GR::u8*)pDummyImage->GetData() )[i + j * Width] ) );
                    }
                  }
                  delete pDummyImage;
                }
                pCompressList = pTempCompressList;
                CompressListSize = dwTempCompressListSize;
                return true;
              default:
                dh::Log( "Fehler: Image.Convert Baustelle (%d to %d).", GetDepth(), Bpp );
                break;
            }
            break;
          case 15:
            switch ( Bpp )
            {
              case 8:
                // Baustelle
                dh::Log( "Fehler: Image.Convert Baustelle (%d to %d).", GetDepth(), Bpp );
                /*for ( i = 0; i < GetWidth(); i++ )
                {
                  for ( j = 0; j < GetHeight(); j++ )
                  {
                    ulColor1 = GetPixel( i, j );
                    ulColor2 = ( ( ( ulColor1 & IMAGE_RED_MASK_555 ) >> IMAGE_RED_SHIFT_555  ) + ( ( ulColor1 & IMAGE_GREEN_MASK_555 ) >> IMAGE_GREEN_SHIFT_555  ) + ( ( ulColor1 & IMAGE_BLUE_MASK_555 ) >> IMAGE_BLUE_SHIFT_555  ) ) / 3;
                  }
                } */
                break;
              case 15:
                // Baustelle
                dh::Log( "Fehler: Image.Convert Baustelle (%d to %d).", GetDepth(), Bpp );
                break;
              case 16:
                // von 15 auf 16 Bit
                GR::u16      *pPos;
                GR::u32     dwCount;
                pPos = ( GR::u16*)pData;
                dwCount = Width * Height;
                while ( dwCount-- )
                {
                  *pPos = ( ( *pPos & 0x7c00 ) << 1 ) 
                            + ( ( ( ( ( *pPos & 0x3e0 ) >> 5 ) * 63 + 15 ) / 31 ) << 5 )
                            + ( *pPos & 0x1f );
                  ++pPos;
                }

                TransparentColor = ( ( TransparentColor & 0x7c00 ) << 1 ) 
                                    + ( ( ( ( ( TransparentColor & 0x3e0 ) >> 5 ) * 63 + 15 ) / 31 ) << 5 )
                                    + ( TransparentColor & 0x1f );

                BitsPerPixel = Bpp;
                pCompressList = pTempCompressList;
                CompressListSize = dwTempCompressListSize;
                return true;
              case 24:
                {
                  // 15 bpp nach 24 bpp
                  int             i,
                                  j;

                  Image       *pNewImage;

                  unsigned char   ucPart[3];


                  // BAUSTELLE - Transparent-Color von 15 auf 24 bit!
                  pNewImage = new Image( 
                        Width, 
                        Height, 
                        24, 
                        0, 
                        GetTransparentColor() );

                  for ( j = 0; j < (int)Height; j++ )
                  {
                    for ( i = 0; i < (int)Width; i++ )
                    {
                      ucPart[0] = ( ( ( ( GR::u16*)pData )[i + j * Width] & 0x001f ) * 255 / 31 );
                      ucPart[1] = ( ( ( ( GR::u16*)pData )[i + j * Width] & 0x03e0 ) >> 5 ) * 255 / 31;
                      ucPart[2] = ( ( ( ( GR::u16*)pData )[i + j * Width] & 0x7c00 ) >> 10 ) * 255 / 31;

                      ( ( GR::u8*)pNewImage->GetData() )[3 * i + j * 3 * Width]     = ucPart[0];
                      ( ( GR::u8*)pNewImage->GetData() )[3 * i + j * 3 * Width + 1] = ucPart[1];
                      ( ( GR::u8*)pNewImage->GetData() )[3 * i + j * 3 * Width + 2] = ucPart[2];
                    }
                  }
                  if ( pData != NULL )
                  {
                    delete[] pData;
                    pData = NULL;
                  }
                  // BAUSTELLE - ALLES abchecken (auch ob NULL zurückkommt)
                  BitsPerPixel      = 24;

                  pData = new unsigned char[GetSize()];
                  memcpy( ( GR::u8*)pData, ( GR::u8*)pNewImage->GetData(), GetSize() );
                  delete pNewImage;

                  pCompressList = pTempCompressList;
                  CompressListSize = dwTempCompressListSize;
                  return true;
                }
                break;
              case 32:
                {
                  // 15 bpp nach 32 bpp
                  int             i,
                                  j;

                  Image       *pNewImage;

                  unsigned char   ucPart[3];


                  // BAUSTELLE - Transparent-Color von 15 auf 32 bit!
                  pNewImage = new Image( 
                        Width, 
                        Height, 
                        32, 
                        0, 
                        GetTransparentColor() );

                  for ( j = 0; j < (int)Height; j++ )
                  {
                    for ( i = 0; i < (int)Width; i++ )
                    {
                      ucPart[0] = ( ( ( ( GR::u16*)pData )[i + j * Width] & 0x001f ) * 255 / 31 );
                      ucPart[1] = ( ( ( ( GR::u16*)pData )[i + j * Width] & 0x03e0 ) >> 5 ) * 255 / 31;
                      ucPart[2] = ( ( ( ( GR::u16*)pData )[i + j * Width] & 0x7c00 ) >> 10 ) * 255 / 31;

                      ( ( GR::u8*)pNewImage->GetData() )[4 * i + j * 4 * Width]     = ucPart[0];
                      ( ( GR::u8*)pNewImage->GetData() )[4 * i + j * 4 * Width + 1] = ucPart[1];
                      ( ( GR::u8*)pNewImage->GetData() )[4 * i + j * 4 * Width + 2] = ucPart[2];
                    }
                  }
                  if ( pData != NULL )
                  {
                    delete[] pData;
                    pData = NULL;
                  }
                  // BAUSTELLE - ALLES abchecken (auch ob NULL zurückkommt)
                  BitsPerPixel      = 32;

                  pData = new unsigned char[GetSize()];
                  memcpy( ( GR::u8*)pData, ( GR::u8*)pNewImage->GetData(), Width * Height * GetFaktor( BitsPerPixel ) );
                  delete pNewImage;

                  pCompressList = pTempCompressList;
                  CompressListSize = dwTempCompressListSize;
                  return true;
                }
                // Baustelle
                dh::Log( "Fehler: Image.Convert Baustelle (%d to %d).", GetDepth(), Bpp );
                break;
              default:
                dh::Log( "Fehler: Image.Convert Unsupported Format %lu.", GetDepth() );
                break;
            }
            break;
          case 16:
            // Baustelle
            switch ( Bpp )
            {
              case 15:
                // von 15 auf 16 Bit
                GR::u16      *pPos;
                GR::u32     dwCount;
                pPos = ( GR::u16*)pData;
                dwCount = Width * Height;
                while ( dwCount-- )
                {
                  *pPos = ( ( *pPos & 0xf800 ) >> 1 ) + ( ( *pPos & 0x7c0) >> 1 ) + ( *pPos & 0x1f );
                  ++pPos;
                }
                BitsPerPixel = Bpp;

                TransparentColor = ( ( TransparentColor & 0xf800 ) >> 1 ) + ( ( TransparentColor & 0x7c0) >> 1 ) + ( TransparentColor & 0x1f );

                pCompressList = pTempCompressList;
                CompressListSize = dwTempCompressListSize;
                return true;
              case 24:
                {
                  // 15 bpp nach 24 bpp
                  int             i,
                                  j;

                  Image       *pNewImage;

                  unsigned char   ucPart[3];


                  // BAUSTELLE - Transparent-Color von 16 auf 24 bit!
                  pNewImage = new Image( 
                        Width, 
                        Height, 
                        24, 
                        0, 
                        GetTransparentColor() );

                  for ( j = 0; j < (int)Height; j++ )
                  {
                    for ( i = 0; i < (int)Width; i++ )
                    {
                      ucPart[0] = ( ( ( ( GR::u16*)pData )[i + j * Width] & 0x001f ) << 3 );
                      ucPart[1] = ( ( ( ( GR::u16*)pData )[i + j * Width] & 0x07e0 ) >> 3 );
                      ucPart[2] = ( ( ( ( GR::u16*)pData )[i + j * Width] & 0xf800 ) >> 8 );

                      ( ( GR::u8*)pNewImage->GetData() )[3 * i + j * 3 * Width]     = ucPart[0];
                      ( ( GR::u8*)pNewImage->GetData() )[3 * i + j * 3 * Width + 1] = ucPart[1];
                      ( ( GR::u8*)pNewImage->GetData() )[3 * i + j * 3 * Width + 2] = ucPart[2];
                    }
                  }
                  if ( pData != NULL )
                  {
                    delete[] pData;
                    pData = NULL;
                  }
                  // BAUSTELLE - ALLES abchecken (auch ob NULL zurückkommt)
                  BitsPerPixel      = 24;

                  pData = new unsigned char[GetSize()];
                  memcpy( ( GR::u8*)pData, ( GR::u8*)pNewImage->GetData(), Width * Height * GetFaktor( BitsPerPixel ) );
                  delete pNewImage;

                  pCompressList = pTempCompressList;
                  CompressListSize = dwTempCompressListSize;
                  return true;
                }
                break;
              default:
                dh::Log( "Fehler: Image.Convert Baustelle (%d to %d).", GetDepth(), Bpp );
                break;
            }
            dh::Log( "Fehler: Image.Convert Baustelle (%d to %d).", GetDepth(), Bpp );
            break;
          case 24:
            switch ( Bpp )
            {
    //-------------------------- 24 nach 15 ----------------------------------------------
              case 15:
                {
                  // 24 bpp nach 15 bpp
                  int             i,
                                  j;

                  Image       *pNewImage;

                  unsigned char   ucPart[3];


                  // BAUSTELLE - Transparent-Color von 15 auf 24 bit!
                  pNewImage = new Image( 
                        Width, 
                        Height, 
                        15, 
                        0, 
                        GetTransparentColor() );

                  for ( j = 0; j < (int)Height; j++ )
                  {
                    for ( i = 0; i < (int)Width; i++ )
                    {
                      ucPart[0] = ( ( GR::u8*)pData )[3 * i + j * 3 * Width];
                      ucPart[1] = ( ( GR::u8*)pData )[3 * i + j * 3 * Width + 1];
                      ucPart[2] = ( ( GR::u8*)pData )[3 * i + j * 3 * Width + 2];

                      ( ( GR::u16*)pNewImage->GetData() )[i + j * Width] =
                                  ( ( ucPart[2] >> 3 ) << 10 )
                                + ( ( ucPart[1] >> 3 ) << 5 )
                                + ( ( ucPart[0] >> 3 ) );
                    }
                  }
                  if ( pData != NULL )
                  {
                    delete[] pData;
                    pData = NULL;
                  }
                  // BAUSTELLE - ALLES abchecken (auch ob NULL zurückkommt)
                  BitsPerPixel      = 15;

                  pData = new unsigned char[GetSize()];
                  memcpy( ( GR::u8*)pData, ( GR::u8*)pNewImage->GetData(), Width * Height * GetFaktor( BitsPerPixel ) );
                  delete pNewImage;

                  pCompressList = pTempCompressList;
                  CompressListSize = dwTempCompressListSize;
                  return true;
                }
                break;
    //-------------------------- 24 nach 16 ----------------------------------------------
              case 16:
                {
                  // 24 bpp nach 16 bpp
                  int             i,
                                  j;

                  Image       *pNewImage;

                  unsigned char   ucPart[3];


                  // BAUSTELLE - Transparent-Color von 15 auf 24 bit!
                  pNewImage = new Image( 
                        Width, 
                        Height, 
                        16, 
                        0, 
                        GetTransparentColor() );

                  for ( j = 0; j < (int)Height; j++ )
                  {
                    for ( i = 0; i < (int)Width; i++ )
                    {
                      ucPart[0] = ( ( GR::u8*)pData )[3 * i + j * 3 * Width];
                      ucPart[1] = ( ( GR::u8*)pData )[3 * i + j * 3 * Width + 1];
                      ucPart[2] = ( ( GR::u8*)pData )[3 * i + j * 3 * Width + 2];

                      ( ( GR::u16*)pNewImage->GetData() )[i + j * Width] =
                                  ( ( ucPart[2] >> 3 ) << 11 )
                                + ( ( ucPart[1] >> 2 ) << 5 )
                                + ( ( ucPart[0] >> 3 ) );
                    }
                  }
                  if ( pData != NULL )
                  {
                    delete[] pData;
                    pData = NULL;
                  }
                  // BAUSTELLE - ALLES abchecken (auch ob NULL zurückkommt)
                  BitsPerPixel      = 16;

                  pData = new unsigned char[GetSize()];
                  memcpy( ( GR::u8*)pData, ( GR::u8*)pNewImage->GetData(), Width * Height * GetFaktor( BitsPerPixel ) );
                  delete pNewImage;

                  pCompressList = pTempCompressList;
                  CompressListSize = dwTempCompressListSize;
                  return true;
                }
                break;
    //-------------------------- 24 nach 32 ----------------------------------------------
              case 32:
                {
                  int             i,
                                  j;

                  Image       *pNewImage;

                  unsigned char   ucPart[3];
                  pNewImage = new Image( Width, Height, 32, 0, 0 );
                  for ( j = 0; j < (int)Height; j++ )
                  {
                    for ( i = 0; i < (int)Width; i++ )
                    {
                      ucPart[0] = ( ( GR::u8*)pData )[3 * i + j * 3 * Width];
                      ucPart[1] = ( ( GR::u8*)pData )[3 * i + j * 3 * Width + 1];
                      ucPart[2] = ( ( GR::u8*)pData )[3 * i + j * 3 * Width + 2];

                      ( ( GR::u8*)pNewImage->GetData() )[4 * i + j * 4 * Width]     = ucPart[0];
                      ( ( GR::u8*)pNewImage->GetData() )[4 * i + j * 4 * Width + 1] = ucPart[1];
                      ( ( GR::u8*)pNewImage->GetData() )[4 * i + j * 4 * Width + 2] = ucPart[2];
                    }
                  }
                  if ( pData != NULL )
                  {
                    delete[] pData;
                    pData = NULL;
                  }
                  // BAUSTELLE - ALLES abchecken (auch ob NULL zurückkommt)
                  BitsPerPixel      = 32;

                  pData = new unsigned char[GetSize()];
                  memcpy( ( GR::u8*)pData, ( GR::u8*)pNewImage->GetData(), Width * Height * GetFaktor( BitsPerPixel ) );
                  delete pNewImage;

                  pCompressList = pTempCompressList;
                  CompressListSize = dwTempCompressListSize;
                  return true;
                }
                break;
            }
            dh::Log( "Fehler: Image.Convert Baustelle (%d to %d).", GetDepth(), Bpp );
            break;
          case 32:
            switch ( Bpp )
            {
    //-------------------------- 32 nach 15 ----------------------------------------------
              case 15:
                {
                  // 32 bpp nach 15 bpp
                  int             i,
                                  j;

                  Image       *pNewImage;

                  unsigned char   ucPart[3];


                  // BAUSTELLE - Transparent-Color von 15 auf 24 bit!
                  pNewImage = new Image( 
                        Width, 
                        Height, 
                        16, 
                        0, 
                        GetTransparentColor() );

                  for ( j = 0; j < (int)Height; j++ )
                  {
                    for ( i = 0; i < (int)Width; i++ )
                    {
                      ucPart[0] = ( ( GR::u8*)pData )[4 * i + j * 4 * Width];
                      ucPart[1] = ( ( GR::u8*)pData )[4 * i + j * 4 * Width + 1];
                      ucPart[2] = ( ( GR::u8*)pData )[4 * i + j * 4 * Width + 2];

                      ( ( GR::u16*)pNewImage->GetData() )[i + j * Width] =
                                  ( ( ucPart[2] >> 3 ) << 10 )
                                + ( ( ucPart[1] >> 3 ) << 5 )
                                + ( ( ucPart[0] >> 3 ) );
                    }
                  }
                  if ( pData != NULL )
                  {
                    delete[] pData;
                    pData = NULL;
                  }
                  // BAUSTELLE - ALLES abchecken (auch ob NULL zurückkommt)
                  BitsPerPixel      = 15;

                  pData = new unsigned char[GetSize()];
                  memcpy( ( GR::u8*)pData, ( GR::u8*)pNewImage->GetData(), Width * Height * GetFaktor( BitsPerPixel ) );
                  delete pNewImage;

                  pCompressList = pTempCompressList;
                  CompressListSize = dwTempCompressListSize;
                  return true;
                }
                break;
    //-------------------------- 32 nach 16 ----------------------------------------------
              case 16:
                {
                  // 32 bpp nach 16 bpp
                  int             i,
                                  j;

                  Image       *pNewImage;

                  unsigned char   ucPart[3];


                  // BAUSTELLE - Transparent-Color von 15 auf 24 bit!
                  pNewImage = new Image( 
                        Width, 
                        Height, 
                        16, 
                        0, 
                        GetTransparentColor() );

                  for ( j = 0; j < (int)Height; j++ )
                  {
                    for ( i = 0; i < (int)Width; i++ )
                    {
                      ucPart[0] = ( ( GR::u8*)pData )[4 * i + j * 4 * Width];
                      ucPart[1] = ( ( GR::u8*)pData )[4 * i + j * 4 * Width + 1];
                      ucPart[2] = ( ( GR::u8*)pData )[4 * i + j * 4 * Width + 2];

                      ( ( GR::u16*)pNewImage->GetData() )[i + j * Width] =
                                  ( ( ucPart[2] >> 3 ) << 11 )
                                + ( ( ucPart[1] >> 2 ) << 5 )
                                + ( ( ucPart[0] >> 3 ) );
                    }
                  }
                  if ( pData != NULL )
                  {
                    delete[] pData;
                    pData = NULL;
                  }
                  // BAUSTELLE - ALLES abchecken (auch ob NULL zurückkommt)
                  BitsPerPixel      = 16;

                  pData = new unsigned char[GetSize()];
                  memcpy( ( GR::u8*)pData, ( GR::u8*)pNewImage->GetData(), Width * Height * GetFaktor( BitsPerPixel ) );
                  delete pNewImage;

                  pCompressList = pTempCompressList;
                  CompressListSize = dwTempCompressListSize;
                  return true;
                }
                break;
    //-------------------------- 32 nach 24 ----------------------------------------------
              case 24:
                {
                  int             i,
                                  j;

                  Image       *pNewImage;

                  unsigned char   ucPart[3];
                  pNewImage = new Image( Width, Height, 24, 0, 0 );
                  for ( j = 0; j < (int)Height; j++ )
                  {
                    for ( i = 0; i < (int)Width; i++ )
                    {
                      ucPart[0] = ( ( GR::u8*)pData )[4 * i + j * 4 * Width];
                      ucPart[1] = ( ( GR::u8*)pData )[4 * i + j * 4 * Width + 1];
                      ucPart[2] = ( ( GR::u8*)pData )[4 * i + j * 4 * Width + 2];

                      ( ( GR::u8*)pNewImage->GetData() )[3 * i + j * 3 * Width]     = ucPart[0];
                      ( ( GR::u8*)pNewImage->GetData() )[3 * i + j * 3 * Width + 1] = ucPart[1];
                      ( ( GR::u8*)pNewImage->GetData() )[3 * i + j * 3 * Width + 2] = ucPart[2];
                    }
                  }
                  if ( pData != NULL )
                  {
                    delete[] pData;
                    pData = NULL;
                  }
                  // BAUSTELLE - ALLES abchecken (auch ob NULL zurückkommt)
                  BitsPerPixel      = 24;

                  pData = new unsigned char[GetSize()];
                  memcpy( ( GR::u8*)pData, ( GR::u8*)pNewImage->GetData(), Width * Height * GetFaktor( BitsPerPixel ) );
                  delete pNewImage;

                  pCompressList = pTempCompressList;
                  CompressListSize = dwTempCompressListSize;
                  return true;
                }
                break;
            }
            dh::Log( "Fehler: Image.Convert Baustelle (%d to %d).", GetDepth(), Bpp );
            break;
            dh::Log( "Fehler: Image.Convert Baustelle (%d to %d).", GetDepth(), Bpp );
            break;
          default:
            dh::Log( "Fehler: Image.Convert Unsupported Format %lu.", GetDepth() );
            break;
        }
      }

      pCompressList = pTempCompressList;
      CompressListSize = dwTempCompressListSize;
      return false;

    }



    unsigned long Image::Compress( bool bWriteList )
    {

      bool            bTransparent      = false,
                      bLastTransparent  = false;

      unsigned char   ucValue           = 0,
                      *pList            = NULL,
                      *pLineCounter     = NULL;

      unsigned long   ulCounter         = 0,
                      i,
                      j;


      if ( bWriteList )
      {
        if ( pCompressList != NULL )
        {
          delete[] pCompressList;
        }
        CompressListSize = 0;
        pCompressList = new ( std::nothrow )GR::u8[Compress( false )];
        if ( pCompressList == NULL )
        {
          dh::Log( "Fehler: Image.Compress Could'nt allocate memory for List." );
          return 0;
        }
      }
      pList = pCompressList;
      for ( i = 0; i < GetHeight(); i++ )
      {
        //dh::Log( "Zeile %lu:", i );
        pLineCounter = pList;
        pList += 2;
        ulCounter = 0;
        for ( j = 0; j < GetWidth(); j++ )
        {
          if ( GetPixel( j, i ) == GetTransparentColor() )
          {
            // durchsichtiger Pixel
            bTransparent = true;
          }
          else
          {
            // massiver Pixel
            bTransparent = false;
          }
          if ( ( bTransparent != bLastTransparent )
          &&   ( j > 0 ) )
          {
            // Noch nicht gesetzte Žnderungen merken
            WriteSector( &pList, ulCounter, bLastTransparent, bWriteList );
            ulCounter = 0;
          }
          ulCounter++;
          bLastTransparent = bTransparent;
        }
        WriteSector( &pList, ulCounter, bLastTransparent, bWriteList );
        if ( bWriteList )
        {
          *( GR::u16 *)pLineCounter = ( GR::u16 )( pList - pLineCounter - 2 );
        }
      }
      //dh::Log( "Liste hat %lu Bytes.", pList - pCompressList );
      if ( bWriteList )
      {
        CompressListSize = (unsigned long)( pList - pCompressList );
      }

      return (unsigned long)( pList - pCompressList );

    }



    void Image::WriteSector( unsigned char **pList, 
                                 const unsigned long ulsize, 
                                 const bool bTransparent,
                                 const bool bWrite ) const
    {
      unsigned long ulSize = ulsize;
      unsigned char   ucValue;

      if ( bTransparent )
      {
        ucValue = FLAG_COMPRESS_TRANSPARENT;
        //dh::Log( "%lu transparente Pixel.", ulSize );
      }
      else
      {
        ucValue = 0;
        //dh::Log( "%lu solide Pixel.", ulSize );
      }
      if ( ulSize == GetWidth() )
      {
        //dh::Log( "Ne ganze Zeile." );
        if ( bWrite )
        {
          **pList = ucValue;
        }
        ( *pList )++;
      }
      else
      {
        while ( ulSize > MAX_COMPRESS_SECTOR )
        {
          //dh::Log( "127 Pixel." );
          if ( bWrite )
          {
            **pList = ucValue + MAX_COMPRESS_SECTOR;
          }
          ( *pList )++;
          ulSize -= MAX_COMPRESS_SECTOR;
        }
        //dh::Log( "%lu Pixel.", ulSize );
        if ( bWrite )
        {
          **pList = ucValue + (unsigned char)ulSize;
        }
        ( *pList )++;
      }

    }



    GR::u8 *Image::GetCompressList()
    {

      return pCompressList;

    }


    unsigned long Image::GetFaktor( const unsigned char Bpp ) const
    {

      switch ( Bpp )
      {
        case 8:
          return 1;
        case 15:
        case 16:
          return 2;
        case 24:
          return 3;
        case 32:
          return 4;
        default:
          dh::Log( "Fehler: Image.GetFaktor Unsupported Format %u.", Bpp );
          return 0;
      }

    }



    bool Image::PutImageReplaceColor( GFXPage *pPage,
                                          const signed long slxpos, 
                                          const signed long slypos, 
                                          GR::u32 Color1,
                                          GR::u32 Color2 ) const
    {

      if ( pPage == NULL )
      {
        dh::Log( "Image.PutImageReplaceColor: pPage = NULL" );
        return false;
      }

      bool        bAllOk = false;

      if ( pPage->GetDepth() != GetDepth() )  return false;


      signed long slXPos = slxpos;
      signed long slYPos = slypos;
      signed long     slXEnd,
                      slYEnd,
                      slXDelta                = GetWidth(),
                      slYDelta                = GetHeight();
  
      slXEnd = slXPos + slXDelta;
      slYEnd = slYPos + slYDelta;
      if ( ( slXEnd <= (signed long)pPage->GetLeftBorder() )
      ||   ( slYEnd <= (signed long)pPage->GetTopBorder() )
      ||   ( slXPos > (signed long)pPage->GetRightBorder() )
      ||   ( slYPos > (signed long)pPage->GetBottomBorder() ) ) return false;


      GR::u8            *pSource                = NULL,
                      *pDestination           = NULL,
                      *pSourceBase            = NULL,
                      *pDestinationBase       = NULL;

      signed long     slXOffset               = 0,
                      slYOffset               = 0,
                      i,
                      j;

      // unsigned long   ulFaktor                = 0,
                      // ulMaskTransparentColor  = 0;

      // es ist etwas des Bildes zu sehen
      // PUSH_GFX_BEGIN;
      if ( slXPos < (signed long)pPage->GetLeftBorder() )
      {
        // Das Bild ragt links über den Bildrand hinaus
        slXDelta -= ( pPage->GetLeftBorder() - slXPos );
        slXOffset = pPage->GetLeftBorder() - slXPos;
        slXPos    = pPage->GetLeftBorder();
      }
      if ( slYPos < (signed long)pPage->GetTopBorder() )
      {
        // Das Bild ragt oben über den Bildrand hinaus
        slYDelta -= ( pPage->GetTopBorder() - slYPos );
        slYOffset = pPage->GetTopBorder() - slYPos;
        slYPos    = pPage->GetTopBorder();
      }
      if ( slXEnd > (signed long)pPage->GetRightBorder() + 1 )
      {
        // Das Bild ragt rechts über den Bildrand hinaus
        slXDelta -= ( slXEnd - pPage->GetRightBorder() - 1 );
      }
      if ( slYEnd > (signed long)pPage->GetBottomBorder() + 1 )
      {
        // Das Bild ragt unten über den Bildrand hinaus
        slYDelta -= ( slYEnd - pPage->GetBottomBorder() - 1 );
      }


      switch ( GetDepth() )
      {
        case 8:
          {
            pSourceBase     = ( GR::u8 *)const_cast<Image*>(this)->GetData() + slXOffset + slYOffset * GetWidth();
            pSource           = pSourceBase;
            pDestinationBase  = ( GR::u8 *)pPage->GetData() + slXPos + slYPos * (signed long)pPage->GetLineOffset();
            pDestination      = pDestinationBase;
            for ( i = 0; i < slYDelta; i++ )
            {
              for ( j = 0; j < slXDelta; j++ )
              {
                if ( *pSource != TransparentColor )
                {
                  if ( *pSource == Color1 )
                  {
                    *pDestination++ = ( GR::u8 )Color2;
                  }
                  else
                  {
                    *pDestination++ = *pSource;
                  }
                }
                else
                {
                  pDestination++;
                }
                pSource++;
              }

              pDestinationBase += pPage->GetLineOffset();
              pSourceBase      += GetWidth() * 1;
              pDestination      = pDestinationBase;
              pSource           = pSourceBase;
            }
          }
          break;
      }

      return true;
    }



    bool Image::PutImageScale( GFXPage *pPage,
                                   signed long slXPos, 
                                   signed long slYPos, 
                                   const float *pScaleBuffer,
                                   float fScaleOffset,
                                   unsigned long ulScaleSize,
                                   unsigned long Flags )
    {

      if ( pPage == NULL )
      {
        dh::Log( "Image.PutImageScale pPage = NULL." );
        return false;
      }
      if ( pPage->GetDepth() != GetDepth() )
      {
        dh::Log( "Image.PutImageScale Wrong Depth." );
        return false;
      }
      switch ( GetDepth() )
      {
        case 8:
          dh::Log( "Status: Image.PutImageScale 8 Bit gibt's noch nicht." );
          break;
        case 15:
        case 16:
          {
            for ( int i = 0; i < GetHeight(); i++ )
            {
              if ( ( slYPos + i >= 0 )
              &&   ( slYPos + i < pPage->GetHeight() ) )
              {
                float fScalePosition = ( (float)ulScaleSize - 1 ) * (float)i / ( GetHeight() - 1 ) + fScaleOffset;
                if ( fScalePosition >= ulScaleSize )
                {
                  fScalePosition -= ulScaleSize;
                }
                unsigned long ulScalePosition = (unsigned long)fScalePosition;
                float fScaleRest = fScalePosition - ulScalePosition;
                float fDelta = *( pScaleBuffer + ulScalePosition + 1 ) - *( pScaleBuffer + ulScalePosition );
                float fLiNewWidth = ( *( pScaleBuffer + ulScalePosition ) + fDelta * fScaleRest ) * GetWidth();
                unsigned long ulLiNewWidth = (unsigned long)fLiNewWidth;
                signed long slXOffset = ( GetWidth() - ulLiNewWidth ) / 2;
                GR::u16  *pSource            = ( GR::u16 *)GetData() + i * GetWidth(),
                      *pDestination       = ( GR::u16 *)pPage->GetData() + ( slXPos + slXOffset + ( slYPos + i ) * pPage->GetWidth() ),
                      *pDestinationBegin  = ( GR::u16 *)pPage->GetData() + ( slYPos + i ) * pPage->GetWidth(),
                      *pDestinationEnd    = ( GR::u16 *)pPage->GetData() + ( slYPos + i + 1 ) * pPage->GetWidth();
                //dh::Log( "Status: Image.PutImageScale Page:0x%08x Img:0x%08x Dest:0x%08x Source:0x%08x.", (GR::u16 *)pPage->GetData(), (GR::u16 *)GetData(), pDestination, pSource );
                float fFaktorX      = (float)GetWidth() / (float)ulLiNewWidth;
                float fCurrentX     = 0.0f;
                //dh::Log( "Status: Image.PutImageScale X:%ld Y:%ld SO:%s SS:%ld SP:%s D:%s LW:%s XO:%ld FX:%s.",
                //         slXPos, slYPos, ftos( fScaleOffset ).c_str(), ulScaleSize, ftos( fScalePosition ).c_str(),
                //         ( fDelta ).c_str(), ( fLiNewWidth ).c_str(), slXOffset, ( fFaktorX ).c_str() );
                for ( unsigned long j = 0; j <= ulLiNewWidth; j++ )
                {
                  signed long slCurrentX = (signed long)fCurrentX;
                  //dh::Log( "Status: .PutImageScale Dest:0x%08x Source:0x%08x CurrentX:%ld.", pDestination, pSource, slCurrentX );
                  if ( ( pDestination >= pDestinationBegin )
                  &&   ( pDestination < pDestinationEnd ) )
                  {
                    *pDestination = *( pSource + slCurrentX );
                  }
                  pDestination++;
                  fCurrentX += fFaktorX;
                }
              }
            }
          }
          break;
        case 24:
          dh::Log( "Status: Image.PutImageScale 24 Bit gibt's noch nicht." );
          break;
        case 32:
          dh::Log( "Status: Image.PutImageScale 32 Bit gibt's noch nicht." );
          break;
      }
      return true;
    }



    unsigned long Image::GetSize() const
    { 
      return GetLineSize() * GetHeight(); 
    }



    unsigned long Image::GetLineSize() const
    { 

      if ( GetDepth() == 15 )
      {
        return 16 * GetWidth() / 8;
      }
      if ( GetDepth() >= 8 )
      {
        return GetDepth() * GetWidth() / 8;
      }

      int     iLineSize = GetWidth() * GetDepth();

      if ( iLineSize % 8 )
      {
        return ( iLineSize / 8 ) + 1;
      }
      return iLineSize / 8;

    }



    ImageData* Image::CreateImageData()
    {

      GR::Graphic::eImageFormat    ImageFormat = GR::Graphic::IF_UNKNOWN;

      switch ( BitsPerPixel )
      {
        case 1:
          ImageFormat = GR::Graphic::IF_INDEX1;
          break;
        case 2:
          ImageFormat = GR::Graphic::IF_INDEX2;
          break;
        case 4:
          ImageFormat = GR::Graphic::IF_INDEX4;
          break;
        case 8:
          ImageFormat = GR::Graphic::IF_PALETTED;
          break;
        case 15:
          ImageFormat = GR::Graphic::IF_X1R5G5B5;
          break;
        case 16:
          ImageFormat = GR::Graphic::IF_R5G6B5;
          break;
        case 24:
          ImageFormat = GR::Graphic::IF_R8G8B8;
          break;
        case 32:
          ImageFormat = GR::Graphic::IF_X8R8G8B8;
          break;
      }
      if ( ImageFormat == GR::Graphic::IF_UNKNOWN )
      {
        return NULL;
      }

      ImageData*   pImageData = new ImageData();

      pImageData->CreateData( GetWidth(), GetHeight(), ImageFormat );

      memcpy( pImageData->Data(),
              GetData(), 
              pImageData->DataSize() );

      return pImageData;

    }



    void Image::SetTransparentColorRGB( const unsigned long NewTransparentColor )
    {

      switch ( BitsPerPixel )
      {
        case 1:
        case 2:
        case 4:
        case 8:
          TransparentColor = NewTransparentColor;
          break;
        case 15:
          TransparentColor = ( ( NewTransparentColor & 0xf80000 ) >> 9 )
                             + ( ( NewTransparentColor & 0x00f800 ) >> 6 )
                             + ( ( NewTransparentColor & 0x0000f8 ) >> 3 );
          break;
        case 16:
          TransparentColor = ( ( NewTransparentColor & 0xf80000 ) >> 8 )
                             + ( ( NewTransparentColor & 0x00fc00 ) >> 5 )
                             + ( ( NewTransparentColor & 0x0000f8 ) >> 3 );
          break;
        case 24:
        case 32:
          TransparentColor = NewTransparentColor;
          break;
      }

    }



    unsigned long Image::GetTransparentColor32( GR::Graphic::Palette* pPal ) const
    {

      switch ( BitsPerPixel )
      {
        case 1:
        case 2:
        case 4:
        case 8:
          if ( pPal )
          {
            return ( pPal->Red( TransparentColor ) << 16 )
                 | ( pPal->Green( TransparentColor ) << 8 )
                 | ( pPal->Blue( TransparentColor ) );
          }
          return TransparentColor;
        case 15:
          return ( ( ( ( TransparentColor & 0x7c00 ) >> 10 ) * 255 / 31 ) << 16 )
               | ( ( ( ( TransparentColor & 0x03e0 ) >> 5 ) * 255 / 31 ) << 8 )
               |   ( ( TransparentColor & 0x001f ) * 255 / 31 );
        case 16:
          return ( ( ( ( TransparentColor & 0xf800 ) >> 11 ) * 255 / 31 ) << 16 )
               | ( ( ( ( TransparentColor & 0x07e0 ) >> 5 ) * 255 / 63 ) << 8 )
               |   ( ( TransparentColor & 0x001f ) * 255 / 31 );
        case 24:
        case 32:
          return TransparentColor;
      }

      return TransparentColor;

    }

  } // namespace Graphic

} // NAMESPACE GR

